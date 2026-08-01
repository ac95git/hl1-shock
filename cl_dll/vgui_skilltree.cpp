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
// Client-side skill metadata used for labels and tooltips.
// The usermessage only sends ids and state bits, so the text lives here.
// =====================================================================
struct SkillUiInfo
{
    const char* displayName;
    const char* description;
    const char* spriteName;
};

static const SkillUiInfo k_SkillUiInfo[] =
{
    { "None",           "",                                  nullptr },
    { "Crowbar Reach",  "+25% melee range.",                "d_crowbar" },
    { "Crowbar Force",  "+50% melee damage.",               "d_crowbar" },
    { "Fast Reload",    "-20% reload time.",                "d_9mmhandgun" },
    { "Weapon Mastery", "+10% weapon damage.",              "d_9mmar" },
    { "High Jump",      "+30% jump height.",                nullptr },
    { "Sprint",         "+15% movement speed.",             nullptr },
    { "Fall Resist",    "-50% fall damage.",                nullptr },
    { "Fortitude",      "+25 max health.",                  "cross" },
    { "Armor Expert",   "Armor absorbs 10% more damage.",   "suit_full" },
    { "Regen",          "Slowly regenerate health.",        "cross" },
    { "Crowbar Speed",  "+30% crowbar attack speed.",       "d_crowbar" },
    { "Pulse Window",   "+0.15s Pulse Window.",             "suit_full" },
    { "Battery Capacity", "+50 max battery.",               "suit_full" },
    { "Battery Regen",  "Regenerate armor over time.",      "suit_full" },
    { "Pulse Recharge", "-33% Pulse Recharge.",             "suit_full" },
    { "Pulse Discharge", "Negated hits vent energy at your crosshair.", "suit_full" },
    { "Pulse Rebound",  "A deflect skips the Recharge. Once, until you sit through a normal one.", "suit_full" },
    { "Follow-Up",      "After a deflect, your next crowbar hit lands far harder.", "d_crowbar" },
};

static const SkillUiInfo* GetSkillUiInfo(int id)
{
    if (id < 0 || id >= (int)(sizeof(k_SkillUiInfo) / sizeof(k_SkillUiInfo[0])))
        return nullptr;
    return &k_SkillUiInfo[id];
}

// =====================================================================
// CSkillTreeView
// =====================================================================
CSkillTreeView::CSkillTreeView()
    : m_iSkillPoints(0)
{}

void CSkillTreeView::UpdateNodes(const SkillNode* nodes, int count, int skillPoints)
{
    m_iSkillPoints = skillPoints;
    m_nodes.assign(nodes, nodes + count);
    for (auto& node : m_nodes)
    {
        const SkillUiInfo* info = GetSkillUiInfo(node.id);
        if (!info) continue;
        if (!node.displayName) node.displayName = info->displayName;
        if (!node.description) node.description = info->description;
    }
    // Invalidate cached rects and sprites (node IDs may have changed)
    m_lastW = 0;
    m_nodeSprites.clear();
    m_iHoverNode = -1;
    m_iHoverX = -1;
    m_iHoverY = -1;
}

// =====================================================================// EnsureSprites – lazily load HUD sprites for each node
// =====================================================================
void CSkillTreeView::EnsureSprites()
{
    m_nodeSprites.resize(m_nodes.size());
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        if (m_nodeSprites[i].hSprite != 0) continue;
        const SkillUiInfo* info = GetSkillUiInfo(m_nodes[i].id);
        if (!info || !info->spriteName) continue;
        const char* name = info->spriteName;
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
        const SkillNode& n = m_nodes[i];
        int tierIdx = (int)n.tier < 3 ? (int)n.tier : 0;
        int nw = k_TierNodeW[tierIdx];
        int left = n.gridCol * k_ColStep + (k_ColStep - nw) / 2;
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
        const SkillNode& n = m_nodes[i];
        int tierIdx = (int)n.tier < 3 ? (int)n.tier : 0;
        int nw = k_TierNodeW[tierIdx], nh = k_TierNodeH[tierIdx];
        int cx = centeredX0 + n.gridCol * k_ColStep + (k_ColStep - nw) / 2;
        int cy = y0 + n.gridRow * k_RowStep + (k_RowStep - nh) / 2;
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

    // Load sprites (no-op after first successful load)
    EnsureSprites();

    // Rebuild rects if geometry changed
    if (m_lastW != areaW || m_lastH != areaH || m_lastX0 != x0 || m_lastY0 != y0)
        RebuildRects(x0, y0 + nodeTopOffset, areaW, areaH - nodeTopOffset);

    // ---- Connector lines between nodes and their prerequisites ----
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillNode& n = m_nodes[i];
        if (n.prereqId < 0) continue;

        // Find prereq index
        int prereqIdx = -1;
        for (int j = 0; j < (int)m_nodes.size(); ++j)
            if (m_nodes[j].id == n.prereqId) { prereqIdx = j; break; }
        if (prereqIdx < 0) continue;

        const IRect& ra = m_nodeRects[prereqIdx];
        const IRect& rb = m_nodeRects[i];

        int acx = ra.x + ra.w / 2;
        int acy = ra.y + ra.h / 2;
        int bcx = rb.x + rb.w / 2;
        int bcy = rb.y + rb.h / 2;
        int dx = bcx - acx;
        int dy = bcy - acy;

        // Compute sibling order so children sharing one prereq fan out cleanly.
        std::vector<int> siblings;
        siblings.reserve(m_nodes.size());
        for (int j = 0; j < (int)m_nodes.size(); ++j)
        {
            if (m_nodes[j].prereqId == n.prereqId)
                siblings.push_back(j);
        }

        // C++98-safe sort by center X, then center Y.
        for (int a = 0; a < (int)siblings.size(); ++a)
        {
            for (int b = a + 1; b < (int)siblings.size(); ++b)
            {
                const IRect& ra2 = m_nodeRects[siblings[a]];
                const IRect& rb2 = m_nodeRects[siblings[b]];
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
            if (siblings[j] == i)
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

        bool pathUnlocked = m_nodes[prereqIdx].bUnlocked;
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
        const SkillNode& n = m_nodes[i];
        IRect& r = m_nodeRects[i];
        // Tier-based sizing
        int tierIdx = (int)n.tier < 3 ? (int)n.tier : 0;
        int stripeH = k_TierStripeH[tierIdx];
        int borderW = k_TierBorderW[tierIdx];

        int br, bg, bb_col;
        int fr, fg, fb_col;

        if (n.bUnlocked)
        {
            // Fully unlocked: orange fill
            br = 200; bg = 120; bb_col = 0;
            fr = 255; fg = 170; fb_col = 0;
        }
        else if (n.bAvailable)
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
        ctx->drawSetColor(fr, fg, fb_col, n.bAvailable ? 20 : 80);
        ctx->drawOutlinedRect(r.x, r.y, r.x + r.w, r.y + r.h);
        if (borderW >= 2)
        {
            // Double outline for Major nodes: second rect inset by 1 px
            ctx->drawSetColor(fr, fg, fb_col, n.bAvailable ? 60 : 140);
            ctx->drawOutlinedRect(r.x + 1, r.y + 1, r.x + r.w - 1, r.y + r.h - 1);
        }

        // Top accent stripe (height driven by tier)
        ctx->drawSetColor(fr, fg, fb_col, n.bUnlocked ? 0 : 100);
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
                    int tr = n.bUnlocked ? 255 : (n.bAvailable ? 255 : 100);
                    int tg = n.bUnlocked ? 255 : (n.bAvailable ? 200 :  80);
                    int tb = n.bUnlocked ? 255 : (n.bAvailable ?  60 :  80);
                    SPR_Set(ns.hSprite, tr, tg, tb);
                    SPR_DrawAdditive(0, drawX, drawY, &ns.rc);
                }
            }
        }

    }

    // ---- Hover tooltip bubble ----
    if (m_iHoverNode >= 0 && m_iHoverNode < (int)m_nodes.size() && m_iHoverNode < (int)m_nodeRects.size())
    {
        const SkillNode& n = m_nodes[m_iHoverNode];
        if (n.description && n.description[0] != '\0')
        {
            const char* title = n.displayName ? n.displayName : "Skill";
            const char* desc  = n.description;
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

    // ---- Empty state message ----
    if (m_nodes.empty() && smallFont)
    {
        static const char* msg = "No skill data received from server.";
        int len = (int)strlen(msg);
        ctx->drawSetTextFont(smallFont);
        ctx->drawSetTextColor(120, 120, 120, 0);
        ctx->drawSetTextPos(x0 + areaW / 4, y0 + areaH / 2 - 5);
        ctx->drawPrintText(msg, len);
    }

    // Draw top-left title text after sprites to avoid stale text cursor state
    // affecting sprite rendering in this pass.
    if (smallFont)
    {
        char spStr[32]; snprintf(spStr, sizeof(spStr), "Skill Points: %d", m_iSkillPoints);
        int len = (int)strlen(spStr);
        ctx->drawSetTextFont(smallFont);
        ctx->drawSetTextColor(255, 200, 60, 0);
        ctx->drawSetTextPos(x0 + 8, y0 + 6);
        ctx->drawPrintText(spStr, len);
    }

    ctx->drawSetTextPos(0, 0);
}

// =====================================================================
// HandleMousePress
// =====================================================================
bool CSkillTreeView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY)
{
    if (m_nodeRects.size() != m_nodes.size())
        return false;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const IRect& r = m_nodeRects[i];
        if (localX < r.x || localX >= r.x + r.w || localY < r.y || localY >= r.y + r.h)
            continue;

        SkillNode& n = m_nodes[i];
        if (!n.bAvailable || n.bUnlocked) return true; // hit but not actionable

        // Send unlock command to server
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "skill_unlock %d\n", n.id);
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
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const IRect& r = m_nodeRects[i];
        if (localX >= r.x && localX < r.x + r.w && localY >= r.y && localY < r.y + r.h)
            return m_nodes[i].description;
    }
    return nullptr;
}
