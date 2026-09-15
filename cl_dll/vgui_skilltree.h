#pragma once

#include <vector>
#include <string>
#include <VGUI_Font.h>
#include <cstdint>

#include "skill_defs.h"

class CInventoryPanel;

// =====================================================================
// CSkillTreeView
//   Plain C++ helper � not a VGUI panel.
//   CInventoryPanel owns one of these and delegates skill-tree
//   painting and input to it.
//
//   The view reads every Skill's name, description, position, cost,
//   prerequisites and tier straight out of k_SkillDefs.  The only
//   things it is told by the server are which Skills are unlocked and
//   how many Skill Points are unspent -- see
//   docs/adr/0008-skill-definitions-are-shared-not-networked.md.
// =====================================================================
class CSkillTreeView
{
public:
    CSkillTreeView();

    // Apply a server state sync: an unlocked mask (k_SkillMaskBytes long),
    // the player's unspent Skill Points, their banked Reset Tokens, and the
    // open-gates bitmask (one bit per EGate value; docs/SKILL_TREE.md,
    // "Reveal gates").
    void UpdateState(const unsigned char* unlockedMask, int skillPoints, int resetTokens, unsigned char openGates);

    // Drops the Reset button out of its armed state.  Called when the panel
    // closes so a confirmation can never survive being walked away from.
    void CancelResetConfirm() { m_flResetConfirmUntil = 0.0f; }

    // Drops a held press so it cannot resume as a drag after the panel
    // closes or the tab changes out from under it.
    void CancelDrag() { m_bMouseDown = false; m_bDragging = false; }

    // Paint the skill tree into the given area.
    // ctx  � owning CInventoryPanel (friend; gives access to draw methods)
    void Paint(CInventoryPanel* ctx,
               int x0, int y0, int areaW, int areaH,
               vgui::Font* smallFont, vgui::Font* titleFont);

    // Input -- returns true if consumed.
    //
    // A press only records where the gesture started; it does not act.
    // Whether it turns out to be a click (node unlock / Reset) or a drag
    // (the field panning) is decided at release, once we know whether the
    // cursor moved past the click threshold in between (SKILL_PANEL.md
    // "View").
    bool HandleMousePress(CInventoryPanel* ctx, int localX, int localY);
    bool HandleMouseRelease(CInventoryPanel* ctx, int localX, int localY);

    // Hover tracking for the tooltip/context bubble; also where a held
    // press turns into a drag and moves the pan.
    void HandleMouseMove(int localX, int localY);

    // Tooltip text for the node under the cursor, or nullptr.
    const char* GetTooltip(int localX, int localY) const;

    int GetSkillPoints() const { return m_iSkillPoints; }

private:
    struct IRect { int x, y, w, h; };

    // Cached sprite handle for a node icon (loaded lazily in EnsureSprites).
    struct NodeSprite { HSPRITE hSprite = 0; Rect rc = {}; };

    // ---- Hover bubble ----
    //
    // Measured with vgui::Font::getTextSize rather than assumed character
    // widths, so the box fits its content exactly at any font or resolution.
    // One structure carries the finished geometry AND the laid-out lines, so
    // nothing is measured twice or measured differently in two places.
    struct TipLine
    {
        std::string text;
        int r = 220, g = 220, b = 220;
    };

    struct TooltipLayout
    {
        std::string          title;
        std::vector<TipLine> lines;
        int w = 0, h = 0;
        int titleH = 0, lineH = 0, bodyTop = 0;
    };

    // Lays out the bubble for one Skill: name, description, and which
    // prerequisites are still missing.  'maxW' bounds the box; the result is
    // shrunk to its widest line, so nothing is padded out to a fixed size.
    void BuildTooltip(int skillId, vgui::Font* titleFont, vgui::Font* smallFont,
                      int maxW, TooltipLayout& out) const;

    // Rebuild m_nodes from k_SkillDefs.  A Skill is in the tree if it has
    // an id and a name; anything else is a reserved id with no node.
    void RebuildNodeList();

    // Populate m_nodeRects from the current layout, at the pan currently in
    // m_panX/m_panY.  Also where the pan is first set: centred on the Suit
    // the first time this view lays out, or recalled from s_savedPan (see
    // the .cpp) if the panel was closed and reopened on the same map.
    void RebuildRects(int x0, int y0, int areaW, int areaH);

    // Keeps the board's edges no further than one step past the field's
    // edges, or centres a board smaller than the field. Called after every
    // pan change, including the initial centering.
    void ClampPan(int areaW, int areaH);

    // Intersects 'r' with 'field'; returns false (leaving 'out' untouched) if
    // they do not overlap at all, so the caller can skip a node that has
    // panned entirely out of view instead of asking the engine to draw or
    // scissor a degenerate rect.
    static bool IntersectRect(const IRect& r, const IRect& field, IRect& out);

    // The Reset switch's red-and-black hazard frame: alternating filled
    // segments walking the perimeter. 'alpha' drives the blink once armed.
    void DrawHazardFrame(CInventoryPanel* ctx, const IRect& r, int seg, int alpha) const;

    // ---- The suit's circuit (docs/SKILL_PANEL.md "The board") ----
    //
    // Painted in this literal order -- substrate, washes, traces, then the
    // node frames themselves -- each a small helper rather than folding
    // further into Paint(). All take the field rect so every fill and
    // outline clips the same way the rest of the view already does.
    void DrawSubstrate(CInventoryPanel* ctx, const IRect& field) const;
    void DrawWashes(CInventoryPanel* ctx, const IRect& field) const;
    void DrawTraces(CInventoryPanel* ctx, const IRect& field) const;

    // One trace, between the node at 'idA' and its right or down neighbour
    // 'idB' (DrawTraces only ever calls it that way, so the two are always
    // orthogonally adjacent and the line is always axis-aligned).
    void DrawTrace(CInventoryPanel* ctx, const IRect& field, int idA, int idB, bool horizontal) const;

    // The frame for one node: fill, outline(s), pins, and -- for the Suit --
    // the inner die. 'r' is the node's own unclipped rect (pins are placed
    // from it and clipped individually, as the icon's scissor already is);
    // 'c' is 'r' pre-clipped to the field, for the fill and outline.
    void DrawNodeFrame(CInventoryPanel* ctx, const IRect& r, const IRect& c, const IRect& field,
                        ENodeTier tier, bool bHeld, bool bAvailable,
                        int fillR, int fillG, int fillB, int lineR, int lineG, int lineB) const;

    // The pins sticking out of a frame: 'count' per side, on the left/right
    // sides if 'sidesLR', the top/bottom sides if 'sidesTB'. Tier alone
    // decides both (SKILL_MAP.md "Frames"), so DrawNodeFrame is the only
    // caller.
    void DrawNodePins(CInventoryPanel* ctx, const IRect& r, const IRect& field,
                       int count, bool sidesLR, bool sidesTB,
                       int pinR, int pinG, int pinB, int alpha) const;

    // Lazily load HUD sprites for each node (no-op if already loaded).
    void EnsureSprites();

    bool IsUnlocked(int skillId) const { return SkillMaskGet(m_unlockedMask, skillId); }

    // Prereqs met, not already unlocked, and affordable.  A display state
    // the client derives; the server validates unlocks independently.
    bool IsAvailable(int skillId) const;

    // A node behind a closed gate: drawn as a blank pad, "No signal" in the
    // tooltip, and never available (ADR-0012, "Hidden means impassable").
    bool IsHidden(int skillId) const;

    // Skill ids present in the tree, in k_SkillDefs order.
    std::vector<int>         m_nodes;
    std::vector<IRect>       m_nodeRects;    // parallel to m_nodes, set in Paint()
    std::vector<NodeSprite>  m_nodeSprites;  // parallel to m_nodes, loaded lazily

    // Reverse lookup, id -> index into m_nodes/m_nodeRects, or -1. The
    // traces need a node's screen rect starting from its neighbour's id
    // (SkillIdAtCell gives ids, not indices), and this is the one place that
    // mapping is built, in step with m_nodes (RebuildNodeList).
    std::vector<int>         m_idToNodeIndex;

    unsigned char            m_unlockedMask[k_SkillMaskBytes] = {};
    unsigned char            m_openGates = 0; // one bit per EGate value, from gmsgSkillTree
    int                      m_iSkillPoints = 0;
    int                      m_iResetTokens = 0;
    int                      m_iHoverNode = -1;
    int                      m_iHoverX = -1;
    int                      m_iHoverY = -1;

    // The Reset button, and its two-stage confirmation.
    //
    // A reset discards an entire playthrough's progression and cannot be
    // undone -- the Token is spent too -- so it deliberately does not share
    // the single-click, no-confirm treatment a node unlock gets.  The second
    // click has a different label in a different colour, so a double-click
    // aimed at the first cannot carry through to the second.
    IRect m_resetBtnRect = {};
    float m_flResetConfirmUntil = 0.0f;   // client time the armed state expires
    static constexpr float k_ResetConfirmTime = 3.0f;
    static constexpr int   k_ResetBtnW = 116;
    static constexpr int   k_ResetBtnH = 16;

    bool ResetArmed() const { return m_flResetConfirmUntil > 0.0f; }

    // ---- Pan: 1:1 always, drag to pan (SKILL_PANEL.md "View") ----
    //
    // The board is drawn at its designed size and never fitted, so it is
    // wider than the field on any screen and the field pans across it
    // instead. m_panX/m_panY is the screen offset of board cell (0,0)'s
    // top-left corner from the field's origin.
    int  m_panX = 0, m_panY = 0;
    bool m_bPanInitialized = false; // set once RebuildRects has centred or recalled a pan

    // A press just records where the gesture started; HandleMouseMove
    // promotes it to a drag once the cursor has moved past the threshold,
    // and HandleMouseRelease decides whether a click follows.
    bool m_bMouseDown = false;
    bool m_bDragging  = false;
    int  m_dragStartMouseX = 0, m_dragStartMouseY = 0;
    int  m_dragStartPanX   = 0, m_dragStartPanY   = 0;
    static constexpr int k_DragClickThreshold = 4; // px

    // Cached layout geometry
    int m_lastX0 = 0, m_lastY0 = 0, m_lastW = 0, m_lastH = 0;

    // The grid the last layout was built on: its extent in cells and where
    // cell (0, 0) starts on screen. Kept so the preview can draw the cells
    // that have no node in them.
    int m_gridCols = 1, m_gridRows = 1;
    int m_gridX0 = 0, m_gridY0 = 0;

    // skilltree_preview_cols / _rows as last read, so a change to either
    // invalidates the cached layout the way a resize does.
    int m_previewCols = 0, m_previewRows = 0;

    // The board is drawn at 1:1 always (SKILL_PANEL.md "View") -- it is
    // never fitted to the area, so m_scale is always 1 and m_colStep /
    // m_rowStep always equal m_step. Kept as members (rather than replaced
    // outright by m_step) because NodeW/NodeH and the icon padding below are
    // still written in terms of a scale, and a fixed 1.0 costs nothing.
    float m_scale   = 1.0f;
    int   m_colStep = 112;
    int   m_rowStep = 112;

    int NodeW(ENodeTier tier) const;
    int NodeH(ENodeTier tier) const;

    // Node dimensions at full scale, indexed by ENodeTier. The layout owns
    // the node size and the icon is fitted into it (SPR_DrawFitted), so the
    // art has no say here: HUD sprites are resolution-bucketed -- the same
    // icon is 44px at 640 and 88px at 1280 -- and until 2026-09-14 the node
    // was sized FROM the largest sprite, which is why the tree needed
    // scaling down to fit anything narrower than ~1600px.
    //
    // Square, since nothing is printed on a node under the board design
    // (docs/SKILL_TREE.md).  The sizes are docs/SKILL_MAP.md's "Sizes":
    // Stat pad, Minor, Medium, Major, and the Suit's processor.
    static constexpr int k_TierNodeW[(int)ENodeTier::_Count] = { 36, 50, 62, 74, 92 };
    static constexpr int k_TierNodeH[(int)ENodeTier::_Count] = { 36, 50, 62, 74, 92 };
    // Gap between the icon and the node's border, at full scale.
    static constexpr int k_IconPad = 4;
    // Border thickness per tier (1=single outline, 2=double outline inset 1px)
    static constexpr int k_TierBorderW[(int)ENodeTier::_Count] = { 1, 1, 1, 2, 2 };
    // Pin size, in pixels: 'Length' runs along the frame's edge, 'Thickness'
    // sticks out perpendicular to it (SKILL_MAP.md "Frames").
    static constexpr int k_PinLength    = 4;
    static constexpr int k_PinThickness = 2;

    // The grid step, both axes: 112 for the 15x15 board at 1:1
    // (docs/SKILL_MAP.md, Sizes).  skilltree_step overrides it for judging
    // by eye; 0 means this.
    static constexpr int k_Step = 112;
    int m_step = k_Step; // as last read from the cvar, part of the cached layout
    static constexpr int k_ConnRadius = 3;  // half-width of connector lines
};
