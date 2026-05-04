#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_skilltree.h"
#include <VGUI_App.h>
#include <algorithm>
#include <cstring>

using namespace vgui;

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
    // Invalidate cached rects
    m_lastW = 0;
}

// =====================================================================
// RebuildRects – compute screen-space rect for each node
// =====================================================================
void CSkillTreeView::RebuildRects(int x0, int y0, int areaW, int areaH)
{
    m_nodeRects.resize(m_nodes.size());
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillNode& n = m_nodes[i];
        int cx = x0 + n.gridCol * k_ColStep + (k_ColStep - k_NodeW) / 2;
        int cy = y0 + n.gridRow * k_RowStep + (k_RowStep - k_NodeH) / 2;
        m_nodeRects[i] = { cx, cy, k_NodeW, k_NodeH };
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
    // Background
    ctx->drawSetColor(10, 10, 10, 60);
    ctx->drawFilledRect(x0, y0, x0 + areaW, y0 + areaH);
    ctx->drawSetColor(100, 60, 200, 80);
    ctx->drawOutlinedRect(x0, y0, x0 + areaW, y0 + areaH);

    // Rebuild rects if geometry changed
    if (m_lastW != areaW || m_lastH != areaH || m_lastX0 != x0 || m_lastY0 != y0)
        RebuildRects(x0, y0, areaW, areaH);

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

        // Center points
        int ax = ra.x + ra.w / 2, ay = ra.y + ra.h / 2;
        int bx = rb.x + rb.w / 2, by = rb.y + rb.h / 2;

        bool pathUnlocked = m_nodes[prereqIdx].bUnlocked;
        if (pathUnlocked)
            ctx->drawSetColor(255, 170, 0, 40);
        else
            ctx->drawSetColor(80, 80, 80, 120);

        // Draw a thick line by drawing a filled rect between the two midpoints.
        // For simplicity use a 2-pixel wide axis-aligned segment or diagonal series.
        // We draw horizontal then vertical segments (L-route).
        int mx = bx; // go horizontal first, then vertical
        ctx->drawFilledRect(std::min(ax, mx) - 1, ay - 1, std::max(ax, mx) + 1, ay + 1);
        ctx->drawFilledRect(mx - 1, std::min(ay, by) - 1, mx + 1, std::max(ay, by) + 1);
    }

    // ---- Skill point counter ----
    if (smallFont)
    {
        char spStr[32]; snprintf(spStr, sizeof(spStr), "Skill Points: %d", m_iSkillPoints);
        int len = (int)strlen(spStr);
        ctx->drawSetTextFont(smallFont);
        ctx->drawSetTextColor(255, 200, 60, 0);
        ctx->drawSetTextPos(x0 + 8, y0 + 6);
        ctx->drawPrintText(spStr, len);
    }

    // ---- Node boxes ----
    int nodeTopOffset = (smallFont ? 22 : 4); // leave room for skill-point text at top

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillNode& n = m_nodes[i];
        IRect& r = m_nodeRects[i];
        // Shift down to leave room for header text
        r.y = m_lastY0 + nodeTopOffset + n.gridRow * k_RowStep + (k_RowStep - k_NodeH) / 2;
        r.x = m_lastX0 + n.gridCol * k_ColStep + (k_ColStep - k_NodeW) / 2;

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

        // Top accent stripe
        ctx->drawSetColor(fr, fg, fb_col, n.bUnlocked ? 0 : 100);
        ctx->drawFilledRect(r.x, r.y, r.x + r.w, r.y + 2);

        // Node label
        if (n.displayName && smallFont)
        {
            int len = (int)strlen(n.displayName);
            ctx->drawSetTextFont(smallFont);
            ctx->drawSetTextColor(fr, fg, fb_col, 0);
            int charH = 10;
            ctx->drawSetTextPos(r.x + 4, r.y + (r.h - charH) / 2);
            ctx->drawPrintText(n.displayName, len);
        }

        // Cost badge (bottom-right corner)
        if (smallFont && !n.bUnlocked)
        {
            char costStr[8]; snprintf(costStr, sizeof(costStr), "%dsp", n.cost);
            int clen = (int)strlen(costStr);
            int charW = 6, charH = 10;
            int textW = clen * charW;
            int tx = r.x + r.w - textW - 3;
            int ty = r.y + r.h - charH - 2;
            FillRGBA(tx - 1, ty - 1, textW + 2, charH + 2, 0, 0, 0, 160);
            ctx->drawSetTextFont(smallFont);
            ctx->drawSetTextColor(n.bAvailable ? 255 : 120, n.bAvailable ? 220 : 120, 50, 0);
            ctx->drawSetTextPos(tx, ty);
            ctx->drawPrintText(costStr, clen);
        }

        // Lock icon placeholder (simple "L" for locked state)
        if (!n.bUnlocked && !n.bAvailable && smallFont)
        {
            static const char* lockStr = "[X]";
            int llen = 3;
            ctx->drawSetTextFont(smallFont);
            ctx->drawSetTextColor(80, 80, 80, 0);
            ctx->drawSetTextPos(r.x + 4, r.y + 3);
            ctx->drawPrintText(lockStr, llen);
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
}

// =====================================================================
// HandleMousePress
// =====================================================================
bool CSkillTreeView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY)
{
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
        return true;
    }
    return false;
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
