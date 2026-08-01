#pragma once

#include <vector>
#include <string>
#include <VGUI_Font.h>
#include <cstdint>

class CInventoryPanel;

// =====================================================================
// ENodeTier
//   Mirrors the server-side definition in dlls/player_skills.h.
//   Encoded in bits 2-3 of the flags byte sent via gmsgSkillTree.
// =====================================================================
enum class ENodeTier : uint8_t
{
    Minor  = 0,   // small node  – cheap/root skills
    Medium = 1,   // medium node – mid-tree skills
    Major  = 2,   // large node  – powerful end-tree skills
};

// =====================================================================
// SkillNode
//   Describes one node in the skill tree.  The server sends the full
//   list via a UserMessage; the client stores a copy here.
// =====================================================================
struct SkillNode
{
    int         id          = 0;
    const char* displayName = nullptr;   // points into a string pool
    const char* description = nullptr;
    int         gridCol     = 0;         // column in the skill-tree grid
    int         gridRow     = 0;         // row    in the skill-tree grid
    int         cost        = 1;         // skill-point cost to unlock
    int         prereqId    = -1;        // prerequisite node id, or -1 for root
    ENodeTier   tier        = ENodeTier::Minor;
    bool        bUnlocked   = false;
    bool        bAvailable  = false;     // prereq met AND player has enough points
};

// =====================================================================
// CSkillTreeView
//   Plain C++ helper � not a VGUI panel.
//   CInventoryPanel owns one of these and delegates skill-tree
//   painting and input to it.
// =====================================================================
class CSkillTreeView
{
public:
    CSkillTreeView();

    // Update node data from a server message (or local init for testing).
    // Copies the array; strings must remain valid for the lifetime of the view.
    void UpdateNodes(const SkillNode* nodes, int count, int skillPoints);

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

    // Populate m_nodeRects from the current layout.
    void RebuildRects(int x0, int y0, int areaW, int areaH);

    // Lazily load HUD sprites for each node (no-op if already loaded).
    void EnsureSprites();

    std::vector<SkillNode>   m_nodes;
    std::vector<IRect>       m_nodeRects;    // parallel to m_nodes, set in Paint()
    std::vector<NodeSprite>  m_nodeSprites;  // parallel to m_nodes, loaded lazily
    int                      m_iSkillPoints = 0;
    int                      m_iHoverNode = -1;
    int                      m_iHoverX = -1;
    int                      m_iHoverY = -1;

    // Cached layout geometry
    int m_lastX0 = 0, m_lastY0 = 0, m_lastW = 0, m_lastH = 0;

    // Node dimensions indexed by ENodeTier (Minor=0, Medium=1, Major=2)
    static constexpr int k_TierNodeW[3] = { 60, 74, 88 };
    static constexpr int k_TierNodeH[3] = { 44, 54, 64 };
    // Accent stripe height per tier
    static constexpr int k_TierStripeH[3] = { 2, 3, 4 };
    // Border thickness per tier (1=single outline, 2=double outline inset 1px)
    static constexpr int k_TierBorderW[3] = { 1, 1, 2 };

    static constexpr int k_ColStep    = 100;
    static constexpr int k_RowStep    = 152;
    static constexpr int k_ConnRadius = 3;  // half-width of connector lines
};
