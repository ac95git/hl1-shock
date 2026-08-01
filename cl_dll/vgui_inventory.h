#ifndef INVENTORYPANEL_H
#define INVENTORYPANEL_H

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>
#include <VGUI_ActionSignal.h>
#include <vector>
#include <string>
#include "ammohistory.h" // for WEAPON
#include "inventory_defs.h"
#include "vgui_inventory_grid.h"
#include "vgui_skilltree.h"

class CInventoryPanel;

// ------------------------------------------------------------------
// Which right-hand view is active
// ------------------------------------------------------------------
enum class EInventoryTab { Inventory, Upgrades };

// ------------------------------------------------------------------
// InvEntryView
//   The client's copy of one Entry.  Purely what the server said --
//   the client never invents, moves or removes one of these.
//   See docs/adr/0004-the-server-owns-the-inventory.md.
// ------------------------------------------------------------------
struct InvEntryView
{
    int kind  = 0; // EEntryKind
    int id    = 0; // WeaponId or EItemTypeId
    int count = 0;
    int col   = 0;
    int row   = 0;

    EEntryKind Kind() const { return static_cast<EEntryKind>(kind); }
    bool IsWeapon() const { return Kind() == EEntryKind::Weapon; }
    bool IsItem()   const { return Kind() == EEntryKind::Item; }
};

// ------------------------------------------------------------------
// Ammo readout entry.  Ammo is not in the Grid (ADR-0001); it is
// collected each frame and listed in the left column instead.
// ------------------------------------------------------------------
struct AmmoReadoutEntry
{
    int     ammoType = -1;
    HSPRITE hSpr     = 0;
    Rect    rc       = {};
    int     iMax     = 0;
};

// ------------------------------------------------------------------
// Context menu shown on right-click
// ------------------------------------------------------------------
class CInventoryContextMenu : public vgui::Panel
{
public:
    CInventoryContextMenu(CInventoryPanel* pOwner, int wide, int tall);

    void Show(int x, int y, int entryIndex, EEntryKind kind, int id);
    void Hide();

    int        GetEntryIndex() const { return m_iEntryIndex; }
    EEntryKind GetEntryKind()  const { return m_eKind; }
    int        GetEntryId()    const { return m_iId; }

    bool HandleClick(int panelLocalX, int panelLocalY);
    virtual void paintBackground();

private:
    CInventoryPanel* m_pOwner;
    vgui::Button*    m_pUseButton;
    vgui::Button*    m_pDropButton;
    int              m_iEntryIndex = -1;
    EEntryKind       m_eKind       = EEntryKind::Empty;
    int              m_iId         = 0;
};

// ------------------------------------------------------------------
// ActionSignal for context-menu buttons
// ------------------------------------------------------------------
class CInventoryMenuAction : public vgui::ActionSignal
{
public:
    enum Action { ACT_USE, ACT_DROP };

    CInventoryMenuAction(CInventoryPanel* pOwner, CInventoryContextMenu* pMenu, Action action);
    void actionPerformed(vgui::Panel* panel) override;

private:
    CInventoryPanel*       m_pOwner;
    CInventoryContextMenu* m_pMenu;
    Action                 m_action;
};

// ------------------------------------------------------------------
// ActionSignal that simply closes the inventory panel
// ------------------------------------------------------------------
class CInventoryCloseAction : public vgui::ActionSignal
{
public:
    explicit CInventoryCloseAction(CInventoryPanel* pPanel) : m_pPanel(pPanel) {}
    void actionPerformed(vgui::Panel*) override;
private:
    CInventoryPanel* m_pPanel;
};

// ------------------------------------------------------------------
class CInventoryPanel : public vgui::Panel, public vgui::CDefaultInputSignal
{
    // View helpers need access to protected draw methods
    friend class CInventoryGridView;
    friend class CSkillTreeView;

private:
    vgui::Label*  m_pLabel;
    vgui::Button* m_pCloseButton = nullptr;
    vgui::Font*   m_pSmallFont   = nullptr;
    vgui::Font*   m_pTitleFont   = nullptr;

    // ---- Inventory state, as told by the server ----
    std::vector<InvEntryView> m_entries;
    int m_gridWidth      = INV_GRID_WIDTH;
    int m_gridRows       = 1;
    int m_gridRowsToDraw = 1;

    // Ammo readout, rebuilt each paint from the weapon list.
    std::vector<AmmoReadoutEntry> m_ammoReadout;

    // ---- Tab state ----
    EInventoryTab m_eActiveTab = EInventoryTab::Inventory;
    struct IRect { int x; int y; int w; int h; };
    static constexpr int k_NumNavBtns = 2;
    IRect m_navBtnRects[k_NumNavBtns] = {};

    // ---- Sub-views ----
    CInventoryGridView m_gridView;
    CSkillTreeView     m_skillTreeView;

    CInventoryContextMenu* m_pContextMenu;

    class HitTestPanel : public Panel
    {
    public:
        void internalMousePressed(vgui::MouseCode code) override;
        void internalMouseReleased(vgui::MouseCode code) override;
    };
    CInventoryPanel::HitTestPanel m_HitTestPanel;

public:
    CInventoryPanel(int x, int y, int wide, int tall);

    void SetText(const char* text);
    void Open();
    void Close();
    void Initialize();

    // Applies one chunk of a server sync.  'reset' starts a fresh list.
    void UpdateInventory(bool reset, int gridWidth, int rows, int rowsToDraw,
                         const InvEntryView* entries, int count);

    const std::vector<InvEntryView>& GetEntries() const { return m_entries; }
    const InvEntryView* GetEntry(int index) const;

    int GridWidth()      const { return m_gridWidth; }
    int GridRows()       const { return m_gridRows; }
    int GridRowsToDraw() const { return m_gridRowsToDraw; }

    // Skill-tree data update (call from UserMessage handler)
    void UpdateSkillTree(const SkillNode* nodes, int count, int skillPoints)
    { m_skillTreeView.UpdateNodes(nodes, count, skillPoints); }

    void CloseContextMenu();

    virtual void paintBackground();
    virtual void paint();

    virtual void mousePressed(vgui::MouseCode code, vgui::Panel* panel) override;
    virtual void mouseReleased(vgui::MouseCode code, vgui::Panel* panel) override;
    virtual void cursorMoved(int x, int y, vgui::Panel* panel) override;

private:
    void GetGridOrigin(int& x0, int& y0) const;
    void RebuildAmmoReadout();
};

#endif
