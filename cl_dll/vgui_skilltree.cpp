#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_skilltree.h"
#include "spr_fit.h"
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

    // Sprites have no say in the layout -- the icon is fitted into whatever
    // node the layout gives it -- so one arriving late changes nothing but
    // the next paint.
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        if (m_nodeSprites[i].hSprite != 0) continue;
        const char* name = k_SkillDefs[m_nodes[i]].spriteName;
        if (!name) continue;
        int idx = gHUD.GetSpriteIndex(name);
        if (idx < 0) continue;
        m_nodeSprites[i].hSprite = gHUD.GetSprite(idx);
        m_nodeSprites[i].rc      = gHUD.GetSpriteRect(idx);
    }
}

// =====================================================================
// Node dimensions at the current scale
// =====================================================================
int CSkillTreeView::NodeW(ENodeTier tier) const
{
    int t = (int)tier < (int)ENodeTier::_Count ? (int)tier : 0;
    return std::max(12, (int)(k_TierNodeW[t] * m_scale));
}

int CSkillTreeView::NodeH(ENodeTier tier) const
{
    int t = (int)tier < (int)ENodeTier::_Count ? (int)tier : 0;
    return std::max(12, (int)(k_TierNodeH[t] * m_scale));
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
    int cols = 1, rows = 1;
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        cols = std::max(cols, def.gridCol + 1);
        rows = std::max(rows, def.gridRow + 1);
    }

    // The preview can only widen the grid: a layout the table already needs
    // is never hidden by asking for a smaller one.
    cols = std::max(cols, m_previewCols);
    rows = std::max(rows, m_previewRows);

    // One square step on both axes, at the designed size.
    const int colStepBase = m_step;
    const int rowStepBase = m_step;

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

    // With a preview grid the empty columns count too, or the ghost cells
    // would hang off one side of a tree centred on its nodes alone.
    if (m_previewCols > 0 || m_previewRows > 0)
    {
        minLeft   = 0;
        usedWidth = cols * m_colStep;
    }

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

    m_gridCols = cols; m_gridRows = rows;
    m_gridX0 = centeredX0; m_gridY0 = centeredY0;
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
// GridCellsAdjacent
//
// The one rule the skilltree_debug_edges overlay tests every prerequisite
// edge against: two cells are neighbours, diagonals included, if they
// differ by at most one column AND at most one row. Kept as the single
// place that defines "adjacent" so the overlay and the console listing it
// prints can never disagree with each other (docs/SKILL_TREE.md, "The
// matrix").
// =====================================================================
static bool GridCellsAdjacent(int col1, int row1, int col2, int row2)
{
    return std::abs(col1 - col2) <= 1 && std::abs(row1 - row2) <= 1;
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

    // The preview grid is part of the layout, so a change to it is a
    // geometry change like any other.
    const int previewCols = std::max(0, std::min(24, (int)CVAR_GET_FLOAT("skilltree_preview_cols")));
    const int previewRows = std::max(0, std::min(24, (int)CVAR_GET_FLOAT("skilltree_preview_rows")));
    int step = (int)CVAR_GET_FLOAT("skilltree_step");
    step = step > 0 ? std::max(40, std::min(200, step)) : k_Step;
    if (previewCols != m_previewCols || previewRows != m_previewRows || step != m_step)
    {
        m_previewCols = previewCols;
        m_previewRows = previewRows;
        m_step = step;
        m_lastW = 0;
    }

    // Rebuild rects if geometry changed
    if (m_lastW != areaW || m_lastH != areaH || m_lastX0 != x0 || m_lastY0 != y0)
        RebuildRects(x0, y0 + nodeTopOffset, areaW, areaH - nodeTopOffset);

    // ---- Preview: ghost cells where the grid has no node ----
    //
    // A Stat-sized outline in every empty cell, so the footprint of a
    // layout the Routes will need can be judged at a real resolution
    // before the nodes exist. Off unless a preview cvar is set.
    if (m_previewCols > 0 || m_previewRows > 0)
    {
        const int gw = NodeW(ENodeTier::Stat), gh = NodeH(ENodeTier::Stat);
        for (int col = 0; col < m_gridCols; ++col)
        {
            for (int row = 0; row < m_gridRows; ++row)
            {
                bool occupied = false;
                for (int i = 0; i < (int)m_nodes.size() && !occupied; ++i)
                {
                    const SkillDef& def = k_SkillDefs[m_nodes[i]];
                    occupied = (def.gridCol == col && def.gridRow == row);
                }
                if (occupied)
                    continue;

                const int gx = m_gridX0 + col * m_colStep + (m_colStep - gw) / 2;
                const int gy = m_gridY0 + row * m_rowStep + (m_rowStep - gh) / 2;
                ctx->drawSetColor(100, 60, 200, 170);
                ctx->drawOutlinedRect(gx, gy, gx + gw, gy + gh);
            }
        }
    }

    // ---- Connector lines between nodes and their prerequisites ----
    //
    // One edge per prerequisite, so a Skill gated on two branches draws
    // two lines.  Every line means the same thing: you need this.
    // dCol/dRow and nonAdjacent are filled in here rather than recomputed at
    // draw time, so the overlay and the console listing read the same
    // numbers for the same edge.
    struct ConnEdge { int from; int to; int dCol; int dRow; bool nonAdjacent; };
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

            const SkillDef& prereqDef = k_SkillDefs[m_nodes[prereqIdx]];
            const int dCol = def.gridCol - prereqDef.gridCol;
            const int dRow = def.gridRow - prereqDef.gridRow;
            const bool adjacent = GridCellsAdjacent(prereqDef.gridCol, prereqDef.gridRow, def.gridCol, def.gridRow);

            edges.push_back({ prereqIdx, i, dCol, dRow, !adjacent });
        }
    }

    // ---- skilltree_debug_edges: console listing ----
    //
    // Printed once when the overlay turns on, and again only if the count of
    // flagged edges changes -- never every frame, or the console is useless.
    const bool bDebugEdges = CVAR_GET_FLOAT("skilltree_debug_edges") != 0.0f;
    if (bDebugEdges)
    {
        int nonAdjCount = 0;
        for (const ConnEdge& edge : edges)
            if (edge.nonAdjacent)
                ++nonAdjCount;

        if (!m_bDebugEdgesOn || nonAdjCount != m_iDebugEdgesLastCount)
        {
            gEngfuncs.Con_Printf("skilltree: %d non-adjacent edge%s\n",
                nonAdjCount, nonAdjCount == 1 ? "" : "s");
            for (const ConnEdge& edge : edges)
            {
                if (!edge.nonAdjacent)
                    continue;
                const char* fromName = k_SkillDefs[m_nodes[edge.from]].name;
                const char* toName   = k_SkillDefs[m_nodes[edge.to]].name;
                gEngfuncs.Con_Printf("skilltree: edge %s -> %s spans (%d,%d)\n",
                    fromName ? fromName : "?", toName ? toName : "?", edge.dCol, edge.dRow);
            }
            m_iDebugEdgesLastCount = nonAdjCount;
        }
        m_bDebugEdgesOn = true;
    }
    else
    {
        m_bDebugEdgesOn = false;
    }

    // Text for any flagged edge's midpoint marker, collected during the
    // sprite pass below and flushed with the rest of the text -- see
    // docs/TECH_DEBT.md, "VGUI Draw Order: All Sprites, Then All Text".
    struct DebugEdgeLabel { int x, y; char text[16]; };
    std::vector<DebugEdgeLabel> debugEdgeLabels;
    if (bDebugEdges)
        debugEdgeLabels.reserve(edges.size());

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

        // ---- skilltree_debug_edges overlay ----
        //
        // A non-adjacent edge is drawn again on top of its normal path, red
        // and three times as thick, so it reads as wrong even next to a
        // dozen ordinary connectors. The redraw uses the same dogleg the
        // normal pass just used (trunk/approach recomputed here rather than
        // shared, to keep this block self-contained and easy to delete).
        if (bDebugEdges && edges[e].nonAdjacent)
        {
            const int hw = 3;
            ctx->drawSetColor(255, 40, 40, 40);

            if (verticalDominant)
            {
                int trunkX = ax + siblingOffset;
                int approachY = by - (by > ay ? 12 : -12);

                ctx->drawFilledRect(std::min(ax, trunkX) - hw, ay - hw, std::max(ax, trunkX) + hw, ay + hw);
                ctx->drawFilledRect(trunkX - hw, std::min(ay, approachY) - hw, trunkX + hw, std::max(ay, approachY) + hw);
                ctx->drawFilledRect(std::min(trunkX, bx) - hw, approachY - hw, std::max(trunkX, bx) + hw, approachY + hw);
                ctx->drawFilledRect(bx - hw, std::min(approachY, by) - hw, bx + hw, std::max(approachY, by) + hw);
            }
            else
            {
                int trunkY = ay + siblingOffset;
                int approachX = bx - (bx > ax ? 12 : -12);

                ctx->drawFilledRect(ax - hw, std::min(ay, trunkY) - hw, ax + hw, std::max(ay, trunkY) + hw);
                ctx->drawFilledRect(std::min(ax, approachX) - hw, trunkY - hw, std::max(ax, approachX) + hw, trunkY + hw);
                ctx->drawFilledRect(approachX - hw, std::min(trunkY, by) - hw, approachX + hw, std::max(trunkY, by) + hw);
                ctx->drawFilledRect(std::min(approachX, bx) - hw, by - hw, std::max(approachX, bx) + hw, by + hw);
            }

            // Marker at the true geometric midpoint (node centre to node
            // centre, not the dogleg's own bend), plus its "(dc,dr)" label
            // queued for the text pass.
            const int midX = (acx + bcx) / 2;
            const int midY = (acy + bcy) / 2;
            FillRGBA(midX - 4, midY - 4, 8, 8, 255, 40, 40, 230);

            DebugEdgeLabel label;
            label.x = midX + 6;
            label.y = midY - 6;
            snprintf(label.text, sizeof(label.text), "(%d,%d)", edges[e].dCol, edges[e].dRow);
            debugEdgeLabels.push_back(label);
        }
    }

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const int skillId = m_nodes[i];
        const SkillDef& def = k_SkillDefs[skillId];
        const bool bUnlocked  = IsUnlocked(skillId);
        const bool bAvailable = IsAvailable(skillId);

        IRect& r = m_nodeRects[i];
        // Tier-based sizing
        int tierIdx = (int)def.tier < (int)ENodeTier::_Count ? (int)def.tier : 0;
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

        // Sprite icon, fitted into the node
        //
        // The node is sized by the layout, not by the art, so the icon is
        // shrunk to the room it has: a HUD sprite that is 88px at 1280 and
        // 132px at 2560 lands the same at both. The 640 bucket's 44px is
        // smaller than the room and stays 44px, since the engine will not
        // magnify. Nothing else is drawn on a node: every node costs one,
        // so there is no price to print (docs/SKILL_TREE.md).
        if (i < (int)m_nodeSprites.size())
        {
            const NodeSprite& ns = m_nodeSprites[i];
            if (ns.hSprite != 0)
            {
                const int pad  = std::max(1, (int)(k_IconPad * m_scale));
                const int boxW = r.w - 2 * pad;
                const int boxH = r.h - 2 * pad;
                if (boxW >= 4 && boxH >= 4)
                {
                    // Tint: white=unlocked, gold=available, grey=locked
                    int tr = bUnlocked ? 255 : (bAvailable ? 255 : 100);
                    int tg = bUnlocked ? 255 : (bAvailable ? 200 :  80);
                    int tb = bUnlocked ? 255 : (bAvailable ?  60 :  80);
                    SPR_Set(ns.hSprite, tr, tg, tb);
                    // Shrunk to fit, never magnified: the engine clips a
                    // sprite drawn larger than its frame (see spr_fit.h), so
                    // an icon smaller than its node sits centred at 1:1.
                    SPR_DrawFitted(ns.hSprite, ns.rc, r.x + pad, r.y + pad, boxW, boxH,
                                   SPR_BLEND_ONE, SPR_BLEND_ONE);
                }
            }
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
    }

    // ---- skilltree_debug_edges labels, deferred from the connector pass ----
    if (smallFont && !debugEdgeLabels.empty())
    {
        ctx->drawSetTextFont(smallFont);
        ctx->drawSetTextColor(255, 80, 80, 0);
        for (const DebugEdgeLabel& label : debugEdgeLabels)
        {
            ctx->drawSetTextPos(label.x, label.y);
            ctx->drawPrintText(label.text, (int)strlen(label.text));
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
