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
    // the player's unspent Skill Points, and their banked Reset Tokens.
    void UpdateState(const unsigned char* unlockedMask, int skillPoints, int resetTokens);

    // Drops the Reset button out of its armed state.  Called when the panel
    // closes so a confirmation can never survive being walked away from.
    void CancelResetConfirm() { m_flResetConfirmUntil = 0.0f; }

    // Paint the skill tree into the given area.
    // ctx  � owning CInventoryPanel (friend; gives access to draw methods)
    void Paint(CInventoryPanel* ctx,
               int x0, int y0, int areaW, int areaH,
               vgui::Font* smallFont, vgui::Font* titleFont);

    // Input � returns true if consumed.
    bool HandleMousePress(CInventoryPanel* ctx, int localX, int localY);

    // Hover tracking for the tooltip/context bubble.
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

    // Populate m_nodeRects from the current layout.
    void RebuildRects(int x0, int y0, int areaW, int areaH);

    // Lazily load HUD sprites for each node (no-op if already loaded).
    void EnsureSprites();

    bool IsUnlocked(int skillId) const { return SkillMaskGet(m_unlockedMask, skillId); }

    // Prereqs met, not already unlocked, and affordable.  A display state
    // the client derives; the server validates unlocks independently.
    bool IsAvailable(int skillId) const;

    // Skill ids present in the tree, in k_SkillDefs order.
    std::vector<int>         m_nodes;
    std::vector<IRect>       m_nodeRects;    // parallel to m_nodes, set in Paint()
    std::vector<NodeSprite>  m_nodeSprites;  // parallel to m_nodes, loaded lazily

    unsigned char            m_unlockedMask[k_SkillMaskBytes] = {};
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

    // The tree is laid out to FIT the area rather than at fixed pixel steps.
    // Seven columns at the full step is 728px against a tree area of
    // panelW - 264, so a hardcoded step hangs off the sides of anything but a
    // wide screen -- and RebuildRects clamps the centering offset at zero, so
    // the overflow was silently clipped rather than visibly wrong.
    //
    // One uniform scale drives step and node size together, so the tree
    // compresses in proportion instead of nodes colliding as gaps shrink.
    float m_scale   = 1.0f;
    int   m_colStep = 96;
    int   m_rowStep = 96;

    int NodeW(ENodeTier tier) const;
    int NodeH(ENodeTier tier) const;

    // Node dimensions at full scale, indexed by ENodeTier. The layout owns
    // the node size and the icon is fitted into it (SPR_DrawFitted), so the
    // art has no say here: HUD sprites are resolution-bucketed -- the same
    // icon is 44px at 640 and 88px at 1280 -- and until 2026-09-14 the node
    // was sized FROM the largest sprite, which is why the tree needed
    // scaling down to fit anything narrower than ~1600px.
    //
    // Square, since nothing is printed on a node under the matrix design
    // (docs/SKILL_TREE.md).
    static constexpr int k_TierNodeW[(int)ENodeTier::_Count] = { 32, 44, 54, 64 };
    static constexpr int k_TierNodeH[(int)ENodeTier::_Count] = { 32, 44, 54, 64 };
    // Gap between the icon and the node's border, at full scale.
    static constexpr int k_IconPad = 4;
    // Accent stripe height per tier
    static constexpr int k_TierStripeH[(int)ENodeTier::_Count] = { 1, 2, 3, 4 };
    // Border thickness per tier (1=single outline, 2=double outline inset 1px)
    static constexpr int k_TierBorderW[(int)ENodeTier::_Count] = { 1, 1, 1, 2 };

    // The grid step, both axes. At 64 the Majors touched (2026-09-14, from
    // a capture); 96 leaves a Stat node's width between two Majors, and a
    // 180-node grid at 15x12 still fits a 1450-wide tree area at ~0.9.
    // skilltree_step overrides it for judging by eye; 0 means this.
    static constexpr int k_Step = 96;
    int m_step = k_Step; // as last read from the cvar, part of the cached layout
    // Nothing is printed on a node (every node costs one, docs/SKILL_TREE.md)
    // and the icon scales with it, so the only floor is the one that keeps a
    // Stat node from vanishing. Below it the tree clips instead of shrinking.
    static constexpr float k_MinScale = 0.3f;
    static constexpr int k_ConnRadius = 3;  // half-width of connector lines
};
