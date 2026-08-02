#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_skilltree.h"
#include <VGUI_App.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace vgui;

// =====================================================================
// Text measurement
//
// vgui::Font::getTextSize is the real thing -- the previous code assumed
// fixed character widths (11px for titles, 7px for body) because the tech
// debt entry recorded proper measurement as only "if available in VGUI
// APIs". It is available, and every heuristic here is gone as a result.
// =====================================================================
static int TextWidth(vgui::Font* font, const char* text)
{
    if (!font || !text || !text[0])
        return 0;

    int w = 0, h = 0;
    font->getTextSize(text, w, h);
    return w;
}

static int TextWidth(vgui::Font* font, const std::string& text)
{
    return TextWidth(font, text.c_str());
}

// Greedy word wrap to a PIXEL width, measuring each candidate line.
//
// Guards the two pathological inputs the debt entry called out: explicit
// newlines, and a single word wider than the box (broken by character rather
// than allowed to overflow).
static void WrapToWidth(vgui::Font* font, const char* text, int maxW,
                        std::vector<std::string>& lines)
{
    lines.clear();
    if (!text || !text[0] || !font)
        return;

    if (maxW < 8)
        maxW = 8;

    std::string line;
    std::string word;

    auto flushLine = [&]()
    {
        lines.push_back(line);
        line.clear();
    };

    // Appends 'word' to the current line, wrapping first if it will not fit,
    // and hard-breaking the word itself if it cannot fit on a line of its own.
    auto pushWord = [&]()
    {
        if (word.empty())
            return;

        std::string candidate = line.empty() ? word : line + " " + word;
        if (TextWidth(font, candidate) <= maxW)
        {
            line.swap(candidate);
            word.clear();
            return;
        }

        if (!line.empty())
            flushLine();

        // The word alone on a line: break it by character if even that overflows.
        while (TextWidth(font, word) > maxW && word.size() > 1)
        {
            std::string head;
            for (size_t i = 0; i < word.size(); ++i)
            {
                std::string next = head + word[i];
                if (!head.empty() && TextWidth(font, next) > maxW)
                    break;
                head = next;
            }
            lines.push_back(head);
            word.erase(0, head.size());
        }

        line.swap(word);
        word.clear();
    };

    for (const char* c = text; ; ++c)
    {
        if (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\0')
        {
            pushWord();

            if (*c == '\n')
                flushLine();
            if (*c == '\0')
                break;
            continue;
        }
        word += *c;
    }

    if (!line.empty())
        lines.push_back(line);
}

// =====================================================================
// CSkillTreeView
//
// Names, descriptions, sprites, positions, costs, prerequisites and
// tiers all come from k_SkillDefs, which the client and server share.
// The server sends state and nothing else.
// =====================================================================
CSkillTreeView::CSkillTreeView()
    : m_iSkillPoints(0)
{
    RebuildNodeList();
}

// =====================================================================
// RebuildNodeList - which Skills have a node in the tree
//
// An id with no name is a reserved id rather than a Skill: ids are
// frozen, so a Skill removed from the tree keeps its number and simply
// stops appearing.
// =====================================================================
void CSkillTreeView::RebuildNodeList()
{
    m_nodes.clear();
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        const SkillDef& def = k_SkillDefs[i];
        if (def.id == ESkillId::None || !def.name || !def.name[0])
            continue;
        m_nodes.push_back(i);
    }

    m_nodeRects.clear();
    m_nodeSprites.clear();
    m_lastW = 0;
}

void CSkillTreeView::UpdateState(const unsigned char* unlockedMask, int skillPoints, int resetTokens)
{
    m_iSkillPoints = skillPoints;
    m_iResetTokens = resetTokens;

    if (unlockedMask)
        memcpy(m_unlockedMask, unlockedMask, k_SkillMaskBytes);
    else
        memset(m_unlockedMask, 0, k_SkillMaskBytes);

    m_iHoverNode = -1;
    m_iHoverX = -1;
    m_iHoverY = -1;

    // The state we were confirming against has just changed underneath us.
    m_flResetConfirmUntil = 0.0f;
}

// =====================================================================
// IsAvailable - prereqs met, not held, and affordable
// =====================================================================
bool CSkillTreeView::IsAvailable(int skillId) const
{
    const SkillDef* def = GetSkillDef(skillId);
    if (!def || IsUnlocked(skillId))
        return false;

    if (!SkillPrereqMet(skillId, [this](ESkillId prereq) { return IsUnlocked(static_cast<int>(prereq)); }))
        return false;

    return m_iSkillPoints >= def->cost;
}

// =====================================================================// EnsureSprites – lazily load HUD sprites for each node
// =====================================================================
void CSkillTreeView::EnsureSprites()
{
    m_nodeSprites.resize(m_nodes.size());

    bool bLoadedAny = false;
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        if (m_nodeSprites[i].hSprite != 0) continue;
        const char* name = k_SkillDefs[m_nodes[i]].spriteName;
        if (!name) continue;
        int idx = gHUD.GetSpriteIndex(name);
        if (idx < 0) continue;
        m_nodeSprites[i].hSprite = gHUD.GetSprite(idx);
        m_nodeSprites[i].rc      = gHUD.GetSpriteRect(idx);
        bLoadedAny = true;
    }

    // Node size is derived from the sprites, and these load lazily -- so a
    // sprite arriving after the layout was computed has to invalidate it.
    // Without this the first paint locks in the fallback sizes and the layout
    // never recomputes, because the panel geometry never changed.
    if (bLoadedAny)
        m_lastW = 0;
}

// =====================================================================
// Node dimensions at the current scale
// =====================================================================
int CSkillTreeView::NodeW(ENodeTier tier) const
{
    int t = (int)tier < 3 ? (int)tier : 0;
    return std::max(20, (int)(m_baseNodeW[t] * m_scale));
}

int CSkillTreeView::NodeH(ENodeTier tier) const
{
    int t = (int)tier < 3 ? (int)tier : 0;
    return std::max(16, (int)(m_baseNodeH[t] * m_scale));
}

// =====================================================================
// RebuildNodeMetrics - size a node so its icon fits inside it
// =====================================================================
void CSkillTreeView::RebuildNodeMetrics()
{
    int maxSprW = 0, maxSprH = 0;
    for (const NodeSprite& ns : m_nodeSprites)
    {
        if (ns.hSprite == 0)
            continue;
        maxSprW = std::max(maxSprW, ns.rc.right  - ns.rc.left);
        maxSprH = std::max(maxSprH, ns.rc.bottom - ns.rc.top);
    }

    // Nothing loaded yet -- keep the floors and try again next paint.
    if (maxSprW <= 0 || maxSprH <= 0)
        return;

    const int iconW = maxSprW + 8;
    const int iconH = maxSprH + 8 + k_CostRoom;

    for (int t = 0; t < 3; ++t)
    {
        // The tier increment survives even when icons set the size, so the
        // Minor/Medium/Major weighting still reads.
        m_baseNodeW[t] = std::max(k_TierNodeW[t], iconW + t * 8);
        m_baseNodeH[t] = std::max(k_TierNodeH[t], iconH + t * 6);
    }
}

// =====================================================================// RebuildRects � compute screen-space rect for each node
// =====================================================================
void CSkillTreeView::RebuildRects(int x0, int y0, int areaW, int areaH)
{
    m_nodeRects.resize(m_nodes.size());
    if (m_nodes.empty())
    {
        m_lastX0 = x0; m_lastY0 = y0; m_lastW = areaW; m_lastH = areaH;
        return;
    }

    // ---- Fit the whole tree to the area ----
    RebuildNodeMetrics();

    int cols = 1, rows = 1;
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        cols = std::max(cols, def.gridCol + 1);
        rows = std::max(rows, def.gridRow + 1);
    }

    // The step follows the largest node, which follows the largest icon.
    const int colStepBase = m_baseNodeW[2] + k_ColGap;
    const int rowStepBase = m_baseNodeH[2] + k_RowGap;

    const float sx = (float)areaW / (float)(cols * colStepBase);
    const float sy = (float)areaH / (float)(rows * rowStepBase);

    // Never magnify past the designed size -- a two-node tree on a wide screen
    // should not become billboards.
    float scale = std::min(1.0f, std::min(sx, sy));
    scale = std::max(scale, k_MinScale);

    m_scale   = scale;
    m_colStep = std::max(8, (int)(colStepBase * scale));
    m_rowStep = std::max(8, (int)(rowStepBase * scale));

    int minLeft = 0;
    int maxRight = 0;
    bool first = true;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        int nw = NodeW(def.tier);
        int left = def.gridCol * m_colStep + (m_colStep - nw) / 2;
        int right = left + nw;

        if (first)
        {
            minLeft = left;
            maxRight = right;
            first = false;
        }
        else
        {
            minLeft = std::min(minLeft, left);
            maxRight = std::max(maxRight, right);
        }
    }

    int usedWidth  = maxRight - minLeft;
    int usedHeight = rows * m_rowStep;

    int centeredX0 = x0 + std::max(0, (areaW - usedWidth) / 2) - minLeft;
    int centeredY0 = y0 + std::max(0, (areaH - usedHeight) / 2);

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        int nw = NodeW(def.tier), nh = NodeH(def.tier);
        int cx = centeredX0 + def.gridCol * m_colStep + (m_colStep - nw) / 2;
        int cy = centeredY0 + def.gridRow * m_rowStep + (m_rowStep - nh) / 2;
        m_nodeRects[i] = { cx, cy, nw, nh };
    }
    m_lastX0 = x0; m_lastY0 = y0; m_lastW = areaW; m_lastH = areaH;
}

// =====================================================================
// BuildTooltip
//
// The bubble is the only place a Skill's identity lives -- the nodes carry
// an icon and a cost and nothing else -- so it also has to explain why a
// locked node is locked. With two prerequisites and connectors arriving
// from two directions, "Requires:" is what makes the tree navigable.
// =====================================================================
void CSkillTreeView::BuildTooltip(int skillId, vgui::Font* titleFont, vgui::Font* smallFont,
                                  int maxW, TooltipLayout& out) const
{
    out = TooltipLayout();

    const SkillDef* def = GetSkillDef(skillId);
    if (!def || !def->name)
        return;

    constexpr int kPad = 8;
    constexpr int kTitleGap = 6;
    constexpr int kMinW = 150;

    out.title  = def->name;
    out.titleH = titleFont ? titleFont->getTall() : 12;
    out.lineH  = smallFont ? smallFont->getTall() : 12;

    const int innerMax = std::max(40, maxW - kPad * 2);

    // ---- Description ----
    std::vector<std::string> body;
    WrapToWidth(smallFont, def->description, innerMax, body);
    for (const auto& line : body)
        out.lines.push_back({ line, 205, 205, 205 });

    // ---- Requires ----
    const ESkillId prereqs[2] = { def->prereq, def->prereq2 };
    bool anyPrereq = (prereqs[0] != ESkillId::None) || (prereqs[1] != ESkillId::None);

    if (anyPrereq && !IsUnlocked(skillId))
    {
        out.lines.push_back({ "", 0, 0, 0 });
        out.lines.push_back({ "Requires:", 150, 150, 150 });

        for (int slot = 0; slot < 2; ++slot)
        {
            if (prereqs[slot] == ESkillId::None)
                continue;

            const SkillDef* p = GetSkillDef(prereqs[slot]);
            if (!p || !p->name)
                continue;

            const bool met = IsUnlocked(static_cast<int>(prereqs[slot]));
            std::string text = std::string(met ? "  " : "  ") + p->name;

            // Met is stated but muted; what is still missing is what the
            // player needs to read.
            if (met)
                out.lines.push_back({ text, 110, 140, 110 });
            else
                out.lines.push_back({ text, 255, 150, 80 });
        }
    }

    // ---- Geometry: shrink to the widest line actually produced ----
    int widest = TextWidth(titleFont, out.title);
    for (const auto& line : out.lines)
        widest = std::max(widest, TextWidth(smallFont, line.text));

    out.w = std::min(std::max(widest + kPad * 2, kMinW), maxW);

    out.bodyTop = kPad + out.titleH + kTitleGap;
    out.h = out.bodyTop + (int)out.lines.size() * out.lineH + kPad;
}

// =====================================================================
// Paint
// =====================================================================
void CSkillTreeView::Paint(CInventoryPanel* ctx,
    int x0, int y0, int areaW, int areaH,
    vgui::Font* smallFont, vgui::Font* titleFont)
{
    int nodeTopOffset = (smallFont ? 22 : 4); // leave room for skill-point text at top

    // Background
    ctx->drawSetColor(10, 10, 10, 60);
    ctx->drawFilledRect(x0, y0, x0 + areaW, y0 + areaH);
    ctx->drawSetColor(100, 60, 200, 80);
    ctx->drawOutlinedRect(x0, y0, x0 + areaW, y0 + areaH);

    // ---- Reset button ----
    //
    // Top-right, opposite the Skill Point readout and clear of the node
    // field, which is centered and starts below nodeTopOffset.  The box is
    // drawn here; its label goes out with the rest of the text at the end of
    // the paint, because text drawn before a sprite gets eaten (see
    // docs/TECH_DEBT.md, "VGUI Draw Order").
    if (ResetArmed() && gHUD.m_flTime > m_flResetConfirmUntil)
        m_flResetConfirmUntil = 0.0f;

    const bool bCanReset = (m_iResetTokens > 0);
    m_resetBtnRect = { x0 + areaW - k_ResetBtnW - 8, y0 + 3, k_ResetBtnW, k_ResetBtnH };

    if (ResetArmed())
    {
        FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 90, 20, 20, 220);
        ctx->drawSetColor(255, 90, 70, 0);
    }
    else if (bCanReset)
    {
        FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 30, 25, 25, 160);
        ctx->drawSetColor(200, 120, 80, 80);
    }
    else
    {
        FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 20, 20, 20, 120);
        ctx->drawSetColor(90, 90, 90, 140);
    }
    ctx->drawOutlinedRect(m_resetBtnRect.x, m_resetBtnRect.y,
        m_resetBtnRect.x + m_resetBtnRect.w, m_resetBtnRect.y + m_resetBtnRect.h);

    // Load sprites (no-op after first successful load)
    EnsureSprites();

    // Rebuild rects if geometry changed
    if (m_lastW != areaW || m_lastH != areaH || m_lastX0 != x0 || m_lastY0 != y0)
        RebuildRects(x0, y0 + nodeTopOffset, areaW, areaH - nodeTopOffset);

    // ---- Connector lines between nodes and their prerequisites ----
    //
    // One edge per prerequisite, so a Skill gated on two branches draws
    // two lines.  Every line means the same thing: you need this.
    struct ConnEdge { int from; int to; };
    std::vector<ConnEdge> edges;
    edges.reserve(m_nodes.size() * 2);

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        const ESkillId prereqs[2] = { def.prereq, def.prereq2 };

        for (int slot = 0; slot < 2; ++slot)
        {
            if (prereqs[slot] == ESkillId::None)
                continue;

            int prereqIdx = -1;
            for (int j = 0; j < (int)m_nodes.size(); ++j)
                if (m_nodes[j] == static_cast<int>(prereqs[slot])) { prereqIdx = j; break; }
            if (prereqIdx < 0) continue;

            edges.push_back({ prereqIdx, i });
        }
    }

    for (int e = 0; e < (int)edges.size(); ++e)
    {
        const int prereqIdx = edges[e].from;
        const int childIdx  = edges[e].to;

        const IRect& ra = m_nodeRects[prereqIdx];
        const IRect& rb = m_nodeRects[childIdx];

        int acx = ra.x + ra.w / 2;
        int acy = ra.y + ra.h / 2;
        int bcx = rb.x + rb.w / 2;
        int bcy = rb.y + rb.h / 2;
        int dx = bcx - acx;
        int dy = bcy - acy;

        // Compute sibling order so edges leaving one prerequisite fan out cleanly.
        std::vector<int> siblings;
        siblings.reserve(edges.size());
        for (int j = 0; j < (int)edges.size(); ++j)
        {
            if (edges[j].from == prereqIdx)
                siblings.push_back(j);
        }

        // C++98-safe sort by child center X, then center Y.
        for (int a = 0; a < (int)siblings.size(); ++a)
        {
            for (int b = a + 1; b < (int)siblings.size(); ++b)
            {
                const IRect& ra2 = m_nodeRects[edges[siblings[a]].to];
                const IRect& rb2 = m_nodeRects[edges[siblings[b]].to];
                int ax2 = ra2.x + ra2.w / 2;
                int bx2 = rb2.x + rb2.w / 2;
                int ay2 = ra2.y + ra2.h / 2;
                int by2 = rb2.y + rb2.h / 2;

                bool swapNeeded = (bx2 < ax2) || (bx2 == ax2 && by2 < ay2);
                if (swapNeeded)
                {
                    int tmp = siblings[a];
                    siblings[a] = siblings[b];
                    siblings[b] = tmp;
                }
            }
        }

        int siblingIndex = 0;
        for (int j = 0; j < (int)siblings.size(); ++j)
        {
            if (siblings[j] == e)
            {
                siblingIndex = j;
                break;
            }
        }

        int siblingOffset = (int)((siblingIndex * 2 - ((int)siblings.size() - 1)) * 4);

        // Choose anchor edges by dominant direction so stacked nodes connect top-to-bottom.
        int ax = acx, ay = acy;
        int bx = bcx, by = bcy;
        bool verticalDominant = std::abs(dy) > std::abs(dx);
        if (verticalDominant)
        {
            bool bDown = dy >= 0;
            ay = bDown ? (ra.y + ra.h) : ra.y;
            by = bDown ? rb.y : (rb.y + rb.h);
        }
        else
        {
            bool bRight = dx >= 0;
            ax = bRight ? (ra.x + ra.w) : ra.x;
            bx = bRight ? rb.x : (rb.x + rb.w);
        }

        bool pathUnlocked = IsUnlocked(m_nodes[prereqIdx]);
        if (pathUnlocked)
            ctx->drawSetColor(255, 170, 0, 40);
        else
            ctx->drawSetColor(80, 80, 80, 120);

        // Fan-out near the prerequisite so sibling connectors do not stack on top of each other.
        // Add a final vertical segment before the child node for clean 90-degree approach.
        if (verticalDominant)
        {
            int trunkX = ax + siblingOffset;
            int approachY = by - (by > ay ? 12 : -12);  // offset 12px above/below child

            ctx->drawFilledRect(std::min(ax, trunkX) - 1, ay - 1, std::max(ax, trunkX) + 1, ay + 1);
            ctx->drawFilledRect(trunkX - 1, std::min(ay, approachY) - 1, trunkX + 1, std::max(ay, approachY) + 1);
            ctx->drawFilledRect(std::min(trunkX, bx) - 1, approachY - 1, std::max(trunkX, bx) + 1, approachY + 1);
            ctx->drawFilledRect(bx - 1, std::min(approachY, by) - 1, bx + 1, std::max(approachY, by) + 1);
        }
        else
        {
            int trunkY = ay + siblingOffset;
            int approachX = bx - (bx > ax ? 12 : -12);  // offset 12px left/right of child

            ctx->drawFilledRect(ax - 1, std::min(ay, trunkY) - 1, ax + 1, std::max(ay, trunkY) + 1);
            ctx->drawFilledRect(std::min(ax, approachX) - 1, trunkY - 1, std::max(ax, approachX) + 1, trunkY + 1);
            ctx->drawFilledRect(approachX - 1, std::min(trunkY, by) - 1, approachX + 1, std::max(trunkY, by) + 1);
            ctx->drawFilledRect(std::min(approachX, bx) - 1, by - 1, std::max(approachX, bx) + 1, by + 1);
        }
    }

    // Costs are text, and text drawn before a sprite gets eaten, so they are
    // collected here and flushed with the rest of the text at the end of the
    // paint. See docs/TECH_DEBT.md, "VGUI Draw Order".
    struct DeferredCost { int x, y, r, g, b; char text[8]; };
    std::vector<DeferredCost> costLabels;
    costLabels.reserve(m_nodes.size());

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const int skillId = m_nodes[i];
        const SkillDef& def = k_SkillDefs[skillId];
        const bool bUnlocked  = IsUnlocked(skillId);
        const bool bAvailable = IsAvailable(skillId);

        IRect& r = m_nodeRects[i];
        // Tier-based sizing
        int tierIdx = (int)def.tier < 3 ? (int)def.tier : 0;
        int stripeH = k_TierStripeH[tierIdx];
        int borderW = k_TierBorderW[tierIdx];

        int br, bg, bb_col;
        int fr, fg, fb_col;

        if (bUnlocked)
        {
            // Fully unlocked: orange fill
            br = 200; bg = 120; bb_col = 0;
            fr = 255; fg = 170; fb_col = 0;
        }
        else if (bAvailable)
        {
            // Can be unlocked: bright gold border, dark fill
            br = 60;  bg = 50;  bb_col = 0;
            fr = 255; fg = 200; fb_col = 60;
        }
        else
        {
            // Locked: dark grey
            br = 25;  bg = 25;  bb_col = 25;
            fr = 80;  fg = 80;  fb_col = 80;
        }

        // Fill + border
        FillRGBA(r.x, r.y, r.w, r.h, br, bg, bb_col, 180);
        ctx->drawSetColor(fr, fg, fb_col, bAvailable ? 20 : 80);
        ctx->drawOutlinedRect(r.x, r.y, r.x + r.w, r.y + r.h);
        if (borderW >= 2)
        {
            // Double outline for Major nodes: second rect inset by 1 px
            ctx->drawSetColor(fr, fg, fb_col, bAvailable ? 60 : 140);
            ctx->drawOutlinedRect(r.x + 1, r.y + 1, r.x + r.w - 1, r.y + r.h - 1);
        }

        // Top accent stripe (height driven by tier)
        ctx->drawSetColor(fr, fg, fb_col, bUnlocked ? 0 : 100);
        ctx->drawFilledRect(r.x, r.y, r.x + r.w, r.y + stripeH);

        // Sprite icon (native size, centered)
        if (i < (int)m_nodeSprites.size())
        {
            const NodeSprite& ns = m_nodeSprites[i];
            if (ns.hSprite != 0)
            {
                int sprW = ns.rc.right  - ns.rc.left;
                int sprH = ns.rc.bottom - ns.rc.top;
                // Nodes are sized to hold their icon, so this normally passes.
                // It only bites when the tree has been scaled down to fit a
                // small panel, and skipping beats spilling a sprite across the
                // node border, which reads as corruption.
                const int costRoom = (int)(k_CostRoom * m_scale);
                if (sprW > 0 && sprH > 0 && sprW <= r.w - 2 && sprH <= r.h - 2 - costRoom)
                {
                    int iconAreaH = std::max(8, r.h - 4 - costRoom);
                    int drawX = r.x + (r.w - sprW) / 2;
                    int drawY = r.y + 2 + (iconAreaH - sprH) / 2;
                    // Tint: white=unlocked, gold=available, grey=locked
                    int tr = bUnlocked ? 255 : (bAvailable ? 255 : 100);
                    int tg = bUnlocked ? 255 : (bAvailable ? 200 :  80);
                    int tb = bUnlocked ? 255 : (bAvailable ?  60 :  80);
                    SPR_Set(ns.hSprite, tr, tg, tb);
                    SPR_DrawAdditive(0, drawX, drawY, &ns.rc);
                }
            }
        }

        // ---- Cost, bottom-right of the node ----
        //
        // The price belongs next to the thing, not one hover away. Unlocked
        // nodes show nothing: what it cost stopped being a decision.
        if (!bUnlocked && def.cost > 0 && smallFont)
        {
            DeferredCost dc = {};
            snprintf(dc.text, sizeof(dc.text), "%d", def.cost);

            int cw = TextWidth(smallFont, dc.text);
            int ch = smallFont->getTall();

            dc.x = r.x + r.w - cw - 4;
            dc.y = r.y + r.h - ch - 2;

            if (bAvailable)
            {
                // Affordable now.
                dc.r = 255; dc.g = 220; dc.b = 120;
            }
            else if (SkillPrereqMet(skillId, [this](ESkillId p) { return IsUnlocked(static_cast<int>(p)); }))
            {
                // Reachable, just not affordable yet -- the one case worth
                // distinguishing, because it is the thing to save up for.
                dc.r = 200; dc.g = 120; dc.b = 90;
            }
            else
            {
                dc.r = 120; dc.g = 120; dc.b = 120;
            }

            costLabels.push_back(dc);
        }
    }

    // ---- Hover tooltip bubble ----
    //
    // Geometry and content come back from one layout pass, so the box can no
    // longer disagree with what is drawn into it.
    TooltipLayout tip;
    int tipX = 0, tipY = 0;
    bool bHaveTip = false;

    if (m_iHoverNode >= 0 && m_iHoverNode < (int)m_nodes.size() && m_iHoverNode < (int)m_nodeRects.size())
    {
        const int maxTipW = std::min(std::max(190, areaW - 16), 420);
        BuildTooltip(m_nodes[m_iHoverNode], titleFont, smallFont, maxTipW, tip);

        if (tip.w > 0 && tip.h > 0)
        {
            const IRect& r = m_nodeRects[m_iHoverNode];
            tipX = r.x + r.w + 10;
            tipY = r.y + 2;

            // Flip to the other side rather than overhang, then clamp -- the
            // bubble stays inside the tree area at any node position.
            if (tipX + tip.w > x0 + areaW)
                tipX = r.x - tip.w - 10;
            if (tipX < x0)
                tipX = x0 + 4;
            if (tipY + tip.h > y0 + areaH)
                tipY = y0 + areaH - tip.h - 4;
            if (tipY < y0)
                tipY = y0 + 4;

            FillRGBA(tipX, tipY, tip.w, tip.h, 0, 0, 0, 220);
            ctx->drawSetColor(255, 170, 0, 120);
            ctx->drawOutlinedRect(tipX, tipY, tipX + tip.w, tipY + tip.h);
            bHaveTip = true;
        }
    }

    // Draw top-left title text after sprites to avoid stale text cursor state
    // affecting sprite rendering in this pass.
    if (smallFont)
    {
        char spStr[64];
        snprintf(spStr, sizeof(spStr), "Skill Points: %d    Reset Tokens: %d",
            m_iSkillPoints, m_iResetTokens);
        int len = (int)strlen(spStr);
        ctx->drawSetTextFont(smallFont);
        ctx->drawSetTextColor(255, 200, 60, 0);
        ctx->drawSetTextPos(x0 + 8, y0 + 6);
        ctx->drawPrintText(spStr, len);

        // The Reset button's label, deferred to here with the rest of the text.
        const char* resetLabel = ResetArmed() ? "CONFIRM - 1 TOKEN" : "RESET TREE";
        int resetLen = (int)strlen(resetLabel);

        int labelW = 0, labelH = 0;
        smallFont->getTextSize(resetLabel, labelW, labelH);

        if (ResetArmed())
            ctx->drawSetTextColor(255, 180, 160, 0);
        else if (m_iResetTokens > 0)
            ctx->drawSetTextColor(220, 160, 110, 0);
        else
            ctx->drawSetTextColor(110, 110, 110, 0);

        ctx->drawSetTextPos(m_resetBtnRect.x + std::max(2, (m_resetBtnRect.w - labelW) / 2),
                            m_resetBtnRect.y + std::max(0, (m_resetBtnRect.h - labelH) / 2));
        ctx->drawPrintText(resetLabel, resetLen);

        // ---- Node costs ----
        for (const DeferredCost& dc : costLabels)
        {
            ctx->drawSetTextColor(dc.r, dc.g, dc.b, 0);
            ctx->drawSetTextPos(dc.x, dc.y);
            ctx->drawPrintText(dc.text, (int)strlen(dc.text));
        }
    }

    // ---- Tooltip text, over everything ----
    if (bHaveTip)
    {
        if (titleFont && !tip.title.empty())
        {
            ctx->drawSetTextFont(titleFont);
            ctx->drawSetTextColor(255, 210, 80, 0);
            ctx->drawSetTextPos(tipX + 8, tipY + 8);
            ctx->drawPrintText(tip.title.c_str(), (int)tip.title.size());
        }

        if (smallFont)
        {
            ctx->drawSetTextFont(smallFont);
            for (int i = 0; i < (int)tip.lines.size(); ++i)
            {
                const TipLine& line = tip.lines[i];
                if (line.text.empty())
                    continue;

                ctx->drawSetTextColor(line.r, line.g, line.b, 0);
                ctx->drawSetTextPos(tipX + 8, tipY + tip.bodyTop + i * tip.lineH);
                ctx->drawPrintText(line.text.c_str(), (int)line.text.size());
            }
        }
    }

    ctx->drawSetTextPos(0, 0);
}

// =====================================================================
// HandleMousePress
// =====================================================================
bool CSkillTreeView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY)
{
    // The Reset button first -- it sits outside the node field, but testing it
    // first also means a click that lands on it can never also hit a node.
    if (m_resetBtnRect.w > 0 &&
        localX >= m_resetBtnRect.x && localX < m_resetBtnRect.x + m_resetBtnRect.w &&
        localY >= m_resetBtnRect.y && localY < m_resetBtnRect.y + m_resetBtnRect.h)
    {
        if (m_iResetTokens <= 0)
            return true; // hit but not actionable

        if (ResetArmed())
        {
            gEngfuncs.pfnClientCmd("skill_reset\n");
            m_flResetConfirmUntil = 0.0f;
            // A falling tone for the commit, against the rising one that armed
            // it: the tree switching off, not an action being refused.
            PlaySound("common/wpn_hudoff.wav", 1.0f);
        }
        else
        {
            m_flResetConfirmUntil = gHUD.m_flTime + k_ResetConfirmTime;
            PlaySound("common/wpn_hudon.wav", 1.0f);
        }
        return true;
    }

    // Anything else the player does disarms the confirmation.
    m_flResetConfirmUntil = 0.0f;

    if (m_nodeRects.size() != m_nodes.size())
        return false;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const IRect& r = m_nodeRects[i];
        if (localX < r.x || localX >= r.x + r.w || localY < r.y || localY >= r.y + r.h)
            continue;

        const int skillId = m_nodes[i];
        if (!IsAvailable(skillId)) return true; // hit but not actionable

        // Send unlock command to server
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "skill_unlock %d\n", skillId);
        gEngfuncs.pfnClientCmd(cmd);
        PlaySound("common/wpn_select.wav", 1.0f);
        return true;
    }
    return false;
}

// =====================================================================
// HandleMouseMove
// =====================================================================
void CSkillTreeView::HandleMouseMove(int localX, int localY)
{
    m_iHoverX = localX;
    m_iHoverY = localY;
    m_iHoverNode = -1;

    if (m_nodeRects.size() != m_nodes.size())
        return;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const IRect& r = m_nodeRects[i];
        if (localX >= r.x && localX < r.x + r.w && localY >= r.y && localY < r.y + r.h)
        {
            m_iHoverNode = i;
            break;
        }
    }
}

// =====================================================================
// GetTooltip
// =====================================================================
const char* CSkillTreeView::GetTooltip(int localX, int localY) const
{
    if (m_nodeRects.size() != m_nodes.size())
        return nullptr;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const IRect& r = m_nodeRects[i];
        if (localX >= r.x && localX < r.x + r.w && localY >= r.y && localY < r.y + r.h)
            return k_SkillDefs[m_nodes[i]].description;
    }
    return nullptr;
}
