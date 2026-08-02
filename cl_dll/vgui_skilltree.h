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

    // The tree is laid out to FIT the area rather than at fixed pixel steps.
    // Seven columns at the full step is 700px against a tree area of
    // panelW - 264, so a hardcoded step hangs off the sides of anything but a
    // wide screen -- and RebuildRects clamps the centering offset at zero, so
    // the overflow was silently clipped rather than visibly wrong.
    //
    // One uniform scale drives step and node size together, so the tree
    // compresses in proportion instead of nodes colliding as gaps shrink.
    float m_scale   = 1.0f;
    int   m_colStep = 100;
    int   m_rowStep = 152;

    // Node size is derived from the icons, not fixed.
    //
    // HUD sprites are resolution-bucketed assets sized for the HUD -- the same
    // icon is 44px at 640 and 88px at 1280 -- and there is no scaled sprite
    // draw in the HUD API (SPR_DrawAdditive is native size only, as the
    // Inventory Grid also found). So a node with fixed pixel dimensions either
    // clips its icon on big screens or wastes space on small ones. Sizing the
    // node from the sprites instead works at every bucket, and will keep
    // working when the placeholder icons are replaced.
    int m_baseNodeW[3] = { 60, 74, 88 };
    int m_baseNodeH[3] = { 44, 54, 64 };

    int NodeW(ENodeTier tier) const;
    int NodeH(ENodeTier tier) const;

    // Fills m_baseNodeW/H from the largest loaded sprite. No-op until sprites
    // are available, so an early paint does not lock in undersized nodes.
    void RebuildNodeMetrics();

    // Minimum node dimensions at full scale, indexed by ENodeTier. The floor,
    // not the answer -- icons enlarge these.
    static constexpr int k_TierNodeW[3] = { 60, 74, 88 };
    static constexpr int k_TierNodeH[3] = { 44, 54, 64 };
    // Accent stripe height per tier
    static constexpr int k_TierStripeH[3] = { 2, 3, 4 };
    // Border thickness per tier (1=single outline, 2=double outline inset 1px)
    static constexpr int k_TierBorderW[3] = { 1, 1, 2 };

    // Gaps added around the widest/tallest node to get the grid step. The row
    // gap is generous because connectors route through it.
    static constexpr int k_ColGap = 16;
    static constexpr int k_RowGap = 76;
    // Room reserved inside a node, below the icon, for the cost.
    static constexpr int k_CostRoom = 12;
    // Below this the nodes are too small to carry an icon and a cost, so the
    // tree stops shrinking and clips instead. An honest limit rather than a
    // silent one.
    static constexpr float k_MinScale = 0.55f;
    static constexpr int k_ConnRadius = 3;  // half-width of connector lines
};
