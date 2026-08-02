#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_skilltree.h"
#include <VGUI_App.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace vgui;

static void WrapTooltipText(const char* text, int maxCharsPerLine, std::vector<std::string>& lines)
{
    lines.clear();
    if (!text || !text[0])
        return;

    if (maxCharsPerLine < 1)
        maxCharsPerLine = 1;

    const char* cursor = text;
    while (*cursor)
    {
        if (*cursor == '\n')
        {
            lines.emplace_back();
            ++cursor;
            continue;
        }

        while (*cursor == ' ' || *cursor == '\t')
            ++cursor;
        if (!*cursor)
            break;
        if (*cursor == '\n')
            continue;

        const char* lineStart = cursor;
        const char* bestBreak = nullptr;
        int lineLen = 0;

        while (*cursor && *cursor != '\n' && lineLen < maxCharsPerLine)
        {
            if (*cursor == ' ' || *cursor == '\t')
                bestBreak = cursor;
            ++cursor;
            ++lineLen;
        }

        if (*cursor && *cursor != '\n' && lineLen >= maxCharsPerLine && bestBreak)
        {
            cursor = bestBreak;
            lineLen = (int)(bestBreak - lineStart);
        }

        while (lineLen > 0 && (lineStart[lineLen - 1] == ' ' || lineStart[lineLen - 1] == '\t'))
            --lineLen;

        lines.emplace_back(lineStart, lineLen);

        while (*cursor == ' ' || *cursor == '\t')
            ++cursor;
        if (*cursor == '\n')
            ++cursor;
    }
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

    int minLeft = 0;
    int maxRight = 0;
    bool first = true;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        int tierIdx = (int)def.tier < 3 ? (int)def.tier : 0;
        int nw = k_TierNodeW[tierIdx];
        int left = def.gridCol * k_ColStep + (k_ColStep - nw) / 2;
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

    int usedWidth = maxRight - minLeft;
    int centeredX0 = x0 + std::max(0, (areaW - usedWidth) / 2) - minLeft;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        int tierIdx = (int)def.tier < 3 ? (int)def.tier : 0;
        int nw = k_TierNodeW[tierIdx], nh = k_TierNodeH[tierIdx];
        int cx = centeredX0 + def.gridCol * k_ColStep + (k_ColStep - nw) / 2;
        int cy = y0 + def.gridRow * k_RowStep + (k_RowStep - nh) / 2;
        m_nodeRects[i] = { cx, cy, nw, nh };
    }
    m_lastX0 = x0; m_lastY0 = y0; m_lastW = areaW; m_lastH = areaH;
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
                if (sprW > 0 && sprH > 0)
                {
                    int iconAreaH = std::max(8, r.h - 8);
                    int drawX = r.x + (r.w - sprW) / 2;
                    int drawY = r.y + 3 + (iconAreaH - sprH) / 2;
                    // Tint: white=unlocked, gold=available, grey=locked
                    int tr = bUnlocked ? 255 : (bAvailable ? 255 : 100);
                    int tg = bUnlocked ? 255 : (bAvailable ? 200 :  80);
                    int tb = bUnlocked ? 255 : (bAvailable ?  60 :  80);
                    SPR_Set(ns.hSprite, tr, tg, tb);
                    SPR_DrawAdditive(0, drawX, drawY, &ns.rc);
                }
            }
        }

    }

    // ---- Hover tooltip bubble ----
    if (m_iHoverNode >= 0 && m_iHoverNode < (int)m_nodes.size() && m_iHoverNode < (int)m_nodeRects.size())
    {
        const SkillDef& def = k_SkillDefs[m_nodes[m_iHoverNode]];
        if (def.description && def.description[0] != '\0')
        {
            const char* title = def.name ? def.name : "Skill";
            const char* desc  = def.description;
            int titleLen = (int)strlen(title);
            constexpr int kPadX = 6;
            constexpr int kFrameInset = 2;
            constexpr int kTitleCharW = 11;
            constexpr int kDescCharW = 7;

            int maxTooltipW = std::max(190, areaW - 6);
            maxTooltipW = std::min(maxTooltipW, 460);

            int titleW = titleLen * kTitleCharW;
            std::vector<std::string> descLines;
            int tooltipW = std::max(190, std::min(titleW + kPadX * 2 + kFrameInset * 2 + 2, maxTooltipW));

            // Fit width/wrap iteratively so title and wrapped description both fit final width.
            for (int pass = 0; pass < 3; ++pass)
            {
                int innerW = std::max(40, tooltipW - (kPadX * 2 + kFrameInset * 2));
                int maxDescChars = std::max(6, innerW / kDescCharW);
                WrapTooltipText(desc, maxDescChars, descLines);
                if (descLines.empty())
                    descLines.emplace_back();

                int descW = 0;
                for (const auto& line : descLines)
                    descW = std::max(descW, (int)line.size() * kDescCharW);

                int neededW = std::max(titleW, descW) + kPadX * 2 + kFrameInset * 2 + 2;
                int newW = std::max(190, std::min(neededW, maxTooltipW));
                if (newW == tooltipW)
                    break;
                tooltipW = newW;
            }

            if (descLines.empty())
                descLines.emplace_back();

            int titleY = 6;
            int titleH = 10;
            int titleDescGap = 20;
            int descY = titleY + titleH + titleDescGap;
            int descH = (int)descLines.size() * 12;
            int bottomPad = 16;
            int tooltipH = descY + descH + bottomPad;

            const IRect& r = m_nodeRects[m_iHoverNode];
            int tooltipX = r.x + r.w + 10;
            int tooltipY = r.y + 2;
            if (tooltipX + tooltipW > x0 + areaW)
                tooltipX = r.x - tooltipW - 10;
            if (tooltipY + tooltipH > y0 + areaH)
                tooltipY = y0 + areaH - tooltipH - 4;
            if (tooltipY < y0)
                tooltipY = y0 + 4;
            if (tooltipX < x0)
                tooltipX = x0 + 4;

            FillRGBA(tooltipX, tooltipY, tooltipW, tooltipH, 0, 0, 0, 210);
            ctx->drawSetColor(255, 170, 0, 120);
            ctx->drawOutlinedRect(tooltipX, tooltipY, tooltipX + tooltipW, tooltipY + tooltipH);

            if (titleFont)
            {
                ctx->drawSetTextFont(titleFont);
                ctx->drawSetTextColor(255, 210, 80, 0);
                ctx->drawSetTextPos(tooltipX + kPadX, tooltipY + titleY);
                ctx->drawPrintText(title, titleLen);
            }
            if (smallFont)
            {
                ctx->drawSetTextFont(smallFont);
                ctx->drawSetTextColor(220, 220, 220, 0);
                for (int lineIdx = 0; lineIdx < (int)descLines.size(); ++lineIdx)
                {
                    const std::string& line = descLines[lineIdx];
                    ctx->drawSetTextPos(tooltipX + kPadX, tooltipY + descY + lineIdx * 12);
                    ctx->drawPrintText(line.c_str(), (int)line.size());
                }
            }
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
