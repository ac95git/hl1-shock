#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_skilltree.h"
#include "spr_fit.h"
#include <VGUI_App.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>

using namespace vgui;

// =====================================================================
// Pan persistence across a close/reopen on the same map
//
// The Viewport -- and this view along with it -- is rebuilt on every map
// load (TeamFortressViewport::CreateInventory), so a CSkillTreeView member
// alone does not survive that. A file static keyed by the level name does:
// it outlives the object, and a new map's name simply misses the cache, so
// the view re-centres on the Suit (SKILL_PANEL.md "View").
// =====================================================================
namespace
{
    struct SavedPan
    {
        char mapName[64] = {};
        int  panX = 0;
        int  panY = 0;
        bool valid = false;
    };
    SavedPan s_savedPan;

    void RememberPan(int panX, int panY)
    {
        s_savedPan.panX = panX;
        s_savedPan.panY = panY;
        s_savedPan.valid = true;

        const char* lvl = gEngfuncs.pfnGetLevelName();
        if (lvl)
        {
            strncpy(s_savedPan.mapName, lvl, sizeof(s_savedPan.mapName) - 1);
            s_savedPan.mapName[sizeof(s_savedPan.mapName) - 1] = '\0';
        }
    }

    // True only if the last remembered pan was on the map we are on now.
    bool RecallPan(int& panX, int& panY)
    {
        if (!s_savedPan.valid)
            return false;

        const char* lvl = gEngfuncs.pfnGetLevelName();
        if (!lvl || strcmp(lvl, s_savedPan.mapName) != 0)
            return false;

        panX = s_savedPan.panX;
        panY = s_savedPan.panY;
        return true;
    }
}

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
// Names, descriptions, sprites, positions and tiers all come from
// k_SkillDefs, which the client and server share.  The server sends
// state and nothing else.
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

void CSkillTreeView::UpdateState(const unsigned char* unlockedMask, int skillPoints, int resetTokens, unsigned char openGates)
{
    m_iSkillPoints = skillPoints;
    m_iResetTokens = resetTokens;
    m_openGates    = openGates;

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
// IsAvailable - a held neighbour, not held, and affordable
//
// The board has no prerequisites: any owned orthogonal neighbour opens a
// node (ADR-0012), through the same SkillReachable the server validates
// with.  The Suit is held and never available.
// =====================================================================
bool CSkillTreeView::IsAvailable(int skillId) const
{
    const SkillDef* def = GetSkillDef(skillId);
    if (!def || IsUnlocked(skillId))
        return false;

    // A hidden node -- its gate closed -- is a blank pad and never
    // available, whatever it is otherwise reachable through (ADR-0012,
    // "Hidden means impassable").
    if (IsHidden(skillId))
        return false;

    if (!SkillReachable(skillId, [this](int other) { return IsUnlocked(other); }))
        return false;

    return m_iSkillPoints >= def->cost;
}

// =====================================================================
// IsHidden -- gate closed, so drawn as a blank pad and never buyable.
// EGate::None is never hidden.  Mirrors CPlayerSkills::IsNodeHidden
// (dlls/player_skills.h), reading m_openGates -- the bitmask the server
// sent -- instead of the server's own state.
// =====================================================================
bool CSkillTreeView::IsHidden(int skillId) const
{
    const SkillDef* def = GetSkillDef(skillId);
    if (!def || def->gate == EGate::None)
        return false;
    return (m_openGates & (1 << (int)def->gate)) == 0;
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

// =====================================================================
// ClampPan -- the board's edges may go no further than one step past the
// field's edges (SKILL_PANEL.md "View"), so the player can always find their
// way back by dragging. A board axis smaller than the field (a tiny preview,
// a very wide screen) has no meaningful clamp range, so it is centred
// instead of pinned to a limit that would put it the wrong way round.
// =====================================================================
static int ClampPanAxis(int pan, int areaSize, int boardSize, int step)
{
    const int minPan = areaSize - boardSize - step;
    const int maxPan = step;
    if (minPan > maxPan)
        return (areaSize - boardSize) / 2;
    return std::min(maxPan, std::max(minPan, pan));
}

void CSkillTreeView::ClampPan(int areaW, int areaH)
{
    const int cols = std::max(k_BoardCols, m_previewCols);
    const int rows = std::max(k_BoardRows, m_previewRows);
    m_panX = ClampPanAxis(m_panX, areaW, cols * m_step, m_step);
    m_panY = ClampPanAxis(m_panY, areaH, rows * m_step, m_step);
}

// =====================================================================
// IntersectRect
// =====================================================================
bool CSkillTreeView::IntersectRect(const IRect& r, const IRect& field, IRect& out)
{
    int x1 = std::max(r.x, field.x);
    int y1 = std::max(r.y, field.y);
    int x2 = std::min(r.x + r.w, field.x + field.w);
    int y2 = std::min(r.y + r.h, field.y + field.h);
    if (x2 <= x1 || y2 <= y1)
        return false;
    out = { x1, y1, x2 - x1, y2 - y1 };
    return true;
}

// =====================================================================
// RebuildRects -- compute screen-space rect for each node at the current pan
//
// The board is fixed at k_BoardCols x k_BoardRows and drawn at 1:1
// (SKILL_PANEL.md "View", docs/SKILL_MAP.md "Sizes"): m_scale is always 1,
// so a node is the same pixel size on every screen and the field pans
// across the board instead of shrinking it to fit.
// =====================================================================
void CSkillTreeView::RebuildRects(int x0, int y0, int areaW, int areaH)
{
    m_nodeRects.resize(m_nodes.size());

    m_scale   = 1.0f;
    m_colStep = m_step;
    m_rowStep = m_step;

    // The preview can only widen the board: a layout the table already
    // needs is never hidden by asking for a smaller one.
    const int cols = std::max(k_BoardCols, m_previewCols);
    const int rows = std::max(k_BoardRows, m_previewRows);

    if (!m_bPanInitialized)
    {
        if (!RecallPan(m_panX, m_panY))
        {
            // Opens centred on the Suit's cell.
            const SkillDef& suit = k_SkillDefs[(int)ESkillId::Suit];
            const int suitCx = suit.gridCol * m_step + m_step / 2;
            const int suitCy = suit.gridRow * m_step + m_step / 2;
            m_panX = areaW / 2 - suitCx;
            m_panY = areaH / 2 - suitCy;
        }
        m_bPanInitialized = true;
    }

    ClampPan(areaW, areaH);
    RememberPan(m_panX, m_panY);

    const int boardX0 = x0 + m_panX;
    const int boardY0 = y0 + m_panY;

    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const SkillDef& def = k_SkillDefs[m_nodes[i]];
        int nw = NodeW(def.tier), nh = NodeH(def.tier);
        int cx = boardX0 + def.gridCol * m_colStep + (m_colStep - nw) / 2;
        int cy = boardY0 + def.gridRow * m_rowStep + (m_rowStep - nh) / 2;
        m_nodeRects[i] = { cx, cy, nw, nh };
    }

    m_gridCols = cols; m_gridRows = rows;
    m_gridX0 = boardX0; m_gridY0 = boardY0;
    m_lastX0 = x0; m_lastY0 = y0; m_lastW = areaW; m_lastH = areaH;
}

// =====================================================================
// BuildTooltip
//
// Name and effect only (docs/SKILL_PANEL.md): the bubble is the one place
// a Skill's identity lives, and nothing else is said in it -- no Route
// line, no totals, no requirements, since the board has none.
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

    // A hidden node gives away nothing: no name, no effect, just the blank
    // pad's own tooltip (SKILL_PANEL.md "Hidden pads").
    const bool bHidden = IsHidden(skillId);

    out.title  = bHidden ? "No signal" : def->name;
    out.titleH = titleFont ? titleFont->getTall() : 12;
    out.lineH  = smallFont ? smallFont->getTall() : 12;

    const int innerMax = std::max(40, maxW - kPad * 2);

    // ---- Description: none for a hidden node ----
    if (!bHidden)
    {
        std::vector<std::string> body;
        WrapToWidth(smallFont, def->description, innerMax, body);
        for (const auto& line : body)
            out.lines.push_back({ line, 205, 205, 205 });
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
    // Background
    ctx->drawSetColor(10, 10, 10, 60);
    ctx->drawFilledRect(x0, y0, x0 + areaW, y0 + areaH);
    ctx->drawSetColor(100, 60, 200, 80);
    ctx->drawOutlinedRect(x0, y0, x0 + areaW, y0 + areaH);

    // The suit palette (SKILL_PANEL.md: the chrome follows the suit colour;
    // the board itself is the one place that steps back from it).
    int sr, sg, sb, dr, dg, db, lr, lg, lb, orr, ogg, obb;
    UnpackRGB(sr, sg, sb, RGB_SUIT);
    UnpackRGB(dr, dg, db, RGB_SUIT_DIM);
    UnpackRGB(lr, lg, lb, RGB_SUIT_LIT);
    UnpackRGB(orr, ogg, obb, RGB_SUIT_OFF);

    // Text drawn before a later sprite pass gets eaten (docs/TECH_DEBT.md,
    // "VGUI Draw Order"), so every label in this paint -- the gauge strip's,
    // the Reset switch's, the tooltip's -- is collected here and flushed once
    // at the very end, after all fills and sprites.
    struct DeferredLabel { int x, y; std::string text; int r, g, b; vgui::Font* font; };
    std::vector<DeferredLabel> deferredLabels;

    // ---- Reset switch and gauge strip geometry ----
    //
    // Both sit fixed over the field -- unaffected by the pan -- and are
    // hit-tested before the board (HandleMouseRelease), so their rects are
    // computed here regardless of layout; only the drawing is deferred, to
    // after the board, so they sit over any node panned underneath
    // (SKILL_PANEL.md "Gauge strip", "Reset switch").
    if (ResetArmed() && gHUD.m_flTime > m_flResetConfirmUntil)
        m_flResetConfirmUntil = 0.0f;

    const bool bCanReset = (m_iResetTokens > 0);
    m_resetBtnRect = { x0 + areaW - k_ResetBtnW - 8, y0 + 3, k_ResetBtnW, k_ResetBtnH };

    const int gaugeH = 22;
    const IRect gaugeRect = { x0 + 6, y0 + 3,
        std::max(0, m_resetBtnRect.x - 10 - (x0 + 6)), gaugeH };

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

    // Rebuild rects if geometry changed -- also where the pan is clamped,
    // and, the first time, centred on the Suit or recalled for this map.
    if (m_lastW != areaW || m_lastH != areaH || m_lastX0 != x0 || m_lastY0 != y0)
        RebuildRects(x0, y0, areaW, areaH);

    const IRect field{ x0, y0, areaW, areaH };

    // ---- Preview: ghost cells where the grid has no node ----
    //
    // A Stat-sized outline in every empty cell, so the footprint of a
    // layout the Routes will need can be judged at a real resolution
    // before the nodes exist. Off unless a preview cvar is set. Clipped to
    // the field like every other panned rect.
    if (m_previewCols > 0 || m_previewRows > 0)
    {
        const int gw = NodeW(ENodeTier::Stat), gh = NodeH(ENodeTier::Stat);
        for (int col = 0; col < m_gridCols; ++col)
        {
            for (int row = 0; row < m_gridRows; ++row)
            {
                if (SkillIdAtCell(col, row) != 0)
                    continue;

                const int gx = m_gridX0 + col * m_colStep + (m_colStep - gw) / 2;
                const int gy = m_gridY0 + row * m_rowStep + (m_rowStep - gh) / 2;
                IRect ghost{ gx, gy, gw, gh }, clipped;
                if (!IntersectRect(ghost, field, clipped))
                    continue;
                ctx->drawSetColor(100, 60, 200, 170);
                ctx->drawOutlinedRect(clipped.x, clipped.y, clipped.x + clipped.w, clipped.y + clipped.h);
            }
        }
    }

    // ---- Nodes ----
    //
    // No connectors are drawn between nodes any more: the board has no
    // edges, only neighbours (ADR-0012).  The traces between adjacent nodes
    // arrive with the circuit drawing (docs/SKILL_PANEL.md).
    //
    // A node panned entirely outside the field is skipped outright; one
    // straddling the edge has every filled/outlined rect clamped to the
    // field by intersection, so it never draws over the left column or
    // header (SKILL_PANEL.md "View").
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        const int skillId = m_nodes[i];
        const SkillDef& def = k_SkillDefs[skillId];

        IRect& r = m_nodeRects[i];

        // A hidden node draws as one uniform blank pad, whatever its tier
        // would otherwise be: a Stat-sized square centred in the cell, so a
        // gated region gives away nothing about what is behind it
        // (SKILL_PANEL.md "Hidden pads"). r is already centred in its cell
        // regardless of tier (RebuildRects), so the pad shares r's centre.
        if (IsHidden(skillId))
        {
            const int hw = NodeW(ENodeTier::Stat), hh = NodeH(ENodeTier::Stat);
            const IRect pad{ r.x + r.w / 2 - hw / 2, r.y + r.h / 2 - hh / 2, hw, hh };
            IRect hc;
            if (IntersectRect(pad, field, hc))
            {
                FillRGBA(hc.x, hc.y, hc.w, hc.h, 35, 35, 35, 160);
                ctx->drawSetColor(70, 70, 70, 90);
                ctx->drawOutlinedRect(hc.x, hc.y, hc.x + hc.w, hc.y + hc.h);
            }
            continue;
        }

        const bool bUnlocked  = IsUnlocked(skillId);
        const bool bAvailable = IsAvailable(skillId);

        IRect c;
        if (!IntersectRect(r, field, c))
            continue;

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

        // Fill + border, clamped to the field by intersection
        FillRGBA(c.x, c.y, c.w, c.h, br, bg, bb_col, 180);
        ctx->drawSetColor(fr, fg, fb_col, bAvailable ? 20 : 80);
        ctx->drawOutlinedRect(c.x, c.y, c.x + c.w, c.y + c.h);
        if (borderW >= 2 && c.w > 2 && c.h > 2)
        {
            // Double outline for Major nodes: second rect inset by 1 px
            ctx->drawSetColor(fr, fg, fb_col, bAvailable ? 60 : 140);
            ctx->drawOutlinedRect(c.x + 1, c.y + 1, c.x + c.w - 1, c.y + c.h - 1);
        }

        // Top accent stripe (height driven by tier), clamped the same way
        ctx->drawSetColor(fr, fg, fb_col, bUnlocked ? 0 : 100);
        ctx->drawFilledRect(c.x, c.y, c.x + c.w, c.y + std::min(stripeH, c.h));

        // Sprite icon, fitted into the node
        //
        // The node is sized by the layout, not by the art, so the icon is
        // shrunk to the room it has: a HUD sprite that is 88px at 1280 and
        // 132px at 2560 lands the same at both. The 640 bucket's 44px is
        // smaller than the room and stays 44px, since the engine will not
        // magnify. Nothing else is drawn on a node: every node costs one,
        // so there is no price to print (docs/SKILL_TREE.md).
        //
        // Clipped with the engine scissor rather than another rect
        // intersection: SPR_DrawFitted centres the sprite from the node's
        // own (unclipped) box, and the scissor cuts what lands outside the
        // field without disturbing that centring.
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
                    SPR_EnableScissor(c.x, c.y, c.w, c.h);
                    SPR_DrawFitted(ns.hSprite, ns.rc, r.x + pad, r.y + pad, boxW, boxH,
                                   SPR_BLEND_ONE, SPR_BLEND_ONE);
                    SPR_DisableScissor();
                }
            }
        }

    }

    // ---- Gauge strip: SKILL POINTS, the segment counter, RESET TOKENS and
    // its pips. Drawn after the board so it sits over any node panned
    // underneath it (SKILL_PANEL.md "Gauge strip").
    if (gaugeRect.w > 4)
    {
        int gx = gaugeRect.x;
        const int lineH = smallFont ? smallFont->getTall() : 10;
        const int textY = gaugeRect.y + std::max(0, (gaugeRect.h - lineH) / 2);

        if (smallFont)
        {
            deferredLabels.push_back({ gx, textY, "SKILL POINTS", lr, lg, lb, smallFont });
            gx += TextWidth(smallFont, "SKILL POINTS") + 8;
        }

        // Boxed two-digit segment counter, the HUD's own digit sprites,
        // tinted with the suit colour rather than the old gold.
        const Rect& digitRc = gHUD.GetSpriteRect(gHUD.m_HUD_number_0);
        const int digitW = digitRc.right - digitRc.left;
        const int digitH = digitRc.bottom - digitRc.top;
        const int boxPad = 3;
        const int boxW = digitW * 2 + boxPad * 2;
        const int boxH = digitH + boxPad * 2;
        const int boxY = gaugeRect.y + std::max(0, (gaugeRect.h - boxH) / 2);

        FillRGBA(gx, boxY, boxW, boxH, dr, dg, db, 200);
        ctx->drawSetColor(sr, sg, sb, 60);
        ctx->drawOutlinedRect(gx, boxY, gx + boxW, boxY + boxH);

        const int displayPts = std::min(99, std::max(0, m_iSkillPoints));
        const int tens = (displayPts / 10) % 10;
        const int ones = displayPts % 10;
        int dx = gx + boxPad, dy = boxY + boxPad;
        SPR_Set(gHUD.GetSprite(gHUD.m_HUD_number_0 + tens), sr, sg, sb);
        SPR_DrawAdditive(0, dx, dy, &gHUD.GetSpriteRect(gHUD.m_HUD_number_0 + tens));
        dx += digitW;
        SPR_Set(gHUD.GetSprite(gHUD.m_HUD_number_0 + ones), sr, sg, sb);
        SPR_DrawAdditive(0, dx, dy, &gHUD.GetSpriteRect(gHUD.m_HUD_number_0 + ones));

        gx += boxW + 14;

        if (smallFont)
        {
            deferredLabels.push_back({ gx, textY, "RESET TOKENS", lr, lg, lb, smallFont });
            gx += TextWidth(smallFont, "RESET TOKENS") + 8;
        }

        // Pips: one lit per Reset Token, dim ones for empty, at least five
        // slots so the strip reads the same whether or not any are lit.
        const int pipSize = 8, pipGap = 4;
        const int numPips = std::max(5, m_iResetTokens);
        const int pipY = gaugeRect.y + std::max(0, (gaugeRect.h - pipSize) / 2);
        for (int p = 0; p < numPips && gx + pipSize <= gaugeRect.x + gaugeRect.w; ++p)
        {
            if (p < m_iResetTokens)
            {
                FillRGBA(gx, pipY, pipSize, pipSize, lr, lg, lb, 220);
            }
            else
            {
                FillRGBA(gx, pipY, pipSize, pipSize, orr, ogg, obb, 50);
                ctx->drawSetColor(orr, ogg, obb, 120);
                ctx->drawOutlinedRect(gx, pipY, gx + pipSize, pipY + pipSize);
            }
            gx += pipSize + pipGap;
        }
    }

    // ---- Reset switch: hazard-striped when a Token is available, blinking
    // once armed, dimmed and unstriped with none (SKILL_PANEL.md "Reset
    // switch"). Drawn after the board for the same reason as the gauge strip.
    {
        const bool bArmed = ResetArmed();

        if (bArmed)
            FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 40, 10, 10, 220);
        else if (bCanReset)
            FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 25, 20, 20, 170);
        else
            FillRGBA(m_resetBtnRect.x, m_resetBtnRect.y, m_resetBtnRect.w, m_resetBtnRect.h, 20, 20, 20, 120);

        if (bCanReset)
        {
            int alpha = 220;
            if (bArmed)
            {
                // Blinks: lit for the first fifth of each 0.4s cycle.
                const bool bBlinkOn = fmodf(gHUD.m_flTime, 0.4f) < 0.2f;
                alpha = bBlinkOn ? 230 : 90;
            }
            DrawHazardFrame(ctx, m_resetBtnRect, 6, alpha);
        }
        else
        {
            // Dimmed and unstriped with no Token.
            ctx->drawSetColor(orr, ogg, obb, 100);
            ctx->drawOutlinedRect(m_resetBtnRect.x, m_resetBtnRect.y,
                m_resetBtnRect.x + m_resetBtnRect.w, m_resetBtnRect.y + m_resetBtnRect.h);
        }

        if (smallFont)
        {
            const char* label = bArmed ? "CONFIRM  -1 TOKEN" : "RESET TREE";
            int labelW = 0, labelH = 0;
            smallFont->getTextSize(label, labelW, labelH);

            int tr2, tg2, tb2;
            if (bArmed)          { tr2 = 255; tg2 = 190; tb2 = 160; }
            else if (bCanReset)  { tr2 = lr;  tg2 = lg;  tb2 = lb;  }
            else                 { tr2 = orr; tg2 = ogg; tb2 = obb; }

            deferredLabels.push_back({
                m_resetBtnRect.x + std::max(2, (m_resetBtnRect.w - labelW) / 2),
                m_resetBtnRect.y + std::max(0, (m_resetBtnRect.h - labelH) / 2),
                label, tr2, tg2, tb2, smallFont });
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

    // ---- All text, last (docs/TECH_DEBT.md, "VGUI Draw Order") ----
    if (!deferredLabels.empty())
    {
        vgui::Font* curFont = nullptr;
        for (const DeferredLabel& lbl : deferredLabels)
        {
            if (lbl.font && lbl.font != curFont)
            {
                curFont = lbl.font;
                ctx->drawSetTextFont(curFont);
            }
            ctx->drawSetTextColor(lbl.r, lbl.g, lbl.b, 0);
            ctx->drawSetTextPos(lbl.x, lbl.y);
            ctx->drawPrintText(lbl.text.c_str(), (int)lbl.text.size());
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
// DrawHazardFrame -- the Reset switch's red-and-black hazard stripe
//
// Alternating 'seg'-px filled segments walk the perimeter clockwise from
// the top-left corner. A stand-in for a diagonal hazard stripe that would
// need art; the alternation and the colours read as "danger" without any.
// =====================================================================
void CSkillTreeView::DrawHazardFrame(CInventoryPanel* ctx, const IRect& r, int seg, int alpha) const
{
    static constexpr int kThick = 3;
    int i = 0;

    auto stripe = [&](int x, int y, int w, int h)
    {
        if ((i++ % 2) == 0)
            ctx->drawSetColor(210, 30, 20, alpha);
        else
            ctx->drawSetColor(15, 15, 15, alpha);
        ctx->drawFilledRect(x, y, x + w, y + h);
    };

    for (int x = r.x; x < r.x + r.w; x += seg)
        stripe(x, r.y, std::min(seg, r.x + r.w - x), kThick);
    for (int x = r.x; x < r.x + r.w; x += seg)
        stripe(x, r.y + r.h - kThick, std::min(seg, r.x + r.w - x), kThick);
    for (int y = r.y; y < r.y + r.h; y += seg)
        stripe(r.x, y, kThick, std::min(seg, r.y + r.h - y));
    for (int y = r.y; y < r.y + r.h; y += seg)
        stripe(r.x + r.w - kThick, y, kThick, std::min(seg, r.y + r.h - y));
}

// =====================================================================
// HandleMousePress
//
// A press only records where the gesture started; it does not act. Whether
// it becomes a click or a drag is decided once the cursor has had a chance
// to move -- see HandleMouseMove and HandleMouseRelease (SKILL_PANEL.md
// "View": "a press that moves under 4 px is a click").
// =====================================================================
bool CSkillTreeView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY)
{
    (void)ctx;
    m_bMouseDown = true;
    m_bDragging  = false;
    m_dragStartMouseX = localX;
    m_dragStartMouseY = localY;
    m_dragStartPanX   = m_panX;
    m_dragStartPanY   = m_panY;
    return true;
}

// =====================================================================
// HandleMouseRelease
//
// Everything HandleMousePress used to do on the way down -- the Reset
// switch, then a node unlock -- now happens here on the way up, and only
// when the press never crossed the drag threshold: a completed drag is not
// also a click.
// =====================================================================
bool CSkillTreeView::HandleMouseRelease(CInventoryPanel* ctx, int localX, int localY)
{
    (void)ctx; (void)localX; (void)localY;

    const bool bWasDragging = m_bDragging;
    m_bMouseDown = false;
    m_bDragging  = false;

    if (bWasDragging)
        return true; // the drag consumed the gesture; no click follows

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
//
// Also where a held press is promoted to a drag: once the cursor has moved
// past the click threshold, the pan follows it for the rest of the press,
// and hovering is suspended (there is nothing to hover while panning).
// =====================================================================
void CSkillTreeView::HandleMouseMove(int localX, int localY)
{
    m_iHoverX = localX;
    m_iHoverY = localY;

    if (m_bMouseDown)
    {
        const int dx = localX - m_dragStartMouseX;
        const int dy = localY - m_dragStartMouseY;

        if (!m_bDragging &&
            (std::abs(dx) > k_DragClickThreshold || std::abs(dy) > k_DragClickThreshold))
            m_bDragging = true;

        if (m_bDragging)
        {
            m_panX = m_dragStartPanX + dx;
            m_panY = m_dragStartPanY + dy;
            ClampPan(m_lastW, m_lastH);
            RememberPan(m_panX, m_panY);
            m_iHoverNode = -1;
            return;
        }
    }

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
