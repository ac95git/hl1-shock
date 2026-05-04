#pragma once

#include <vector>
#include <string>
#include <VGUI_Font.h>

class CInventoryPanel;

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
    bool        bUnlocked   = false;
    bool        bAvailable  = false;     // prereq met AND player has enough points
};

// =====================================================================
// CSkillTreeView
//   Plain C++ helper — not a VGUI panel.
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
    // ctx  – owning CInventoryPanel (friend; gives access to draw methods)
    void Paint(CInventoryPanel* ctx,
               int x0, int y0, int areaW, int areaH,
               vgui::Font* smallFont, vgui::Font* titleFont);

    // Input – returns true if consumed.
    bool HandleMousePress(CInventoryPanel* ctx, int localX, int localY);

    // Tooltip text for the node under the cursor, or nullptr.
    const char* GetTooltip(int localX, int localY) const;

    int GetSkillPoints() const { return m_iSkillPoints; }

private:
    struct IRect { int x, y, w, h; };

    // Populate m_nodeRects from the current layout.
    void RebuildRects(int x0, int y0, int areaW, int areaH);

    std::vector<SkillNode> m_nodes;
    std::vector<IRect>     m_nodeRects;   // parallel to m_nodes, set in Paint()
    int                    m_iSkillPoints = 0;

    // Cached layout geometry
    int m_lastX0 = 0, m_lastY0 = 0, m_lastW = 0, m_lastH = 0;

    static constexpr int k_NodeW      = 72;
    static constexpr int k_NodeH      = 52;
    static constexpr int k_ColStep    = 96;
    static constexpr int k_RowStep    = 72;
    static constexpr int k_ConnRadius = 3;  // half-width of connector lines
};
