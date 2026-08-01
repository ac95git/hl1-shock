#ifndef INVENTORYPANEL_H
#define INVENTORYPANEL_H

#include <VGUI_Panel.h>
#include <VGUI_Label.h>
#include <VGUI_Button.h>
#include <VGUI_ActionSignal.h>
#include <vector>
#include <string>
#include "ammohistory.h" // for WEAPON
#include "vgui_inventory_grid.h"
#include "vgui_skilltree.h"

class CInventoryPanel;

// ------------------------------------------------------------------
// Which right-hand view is active
// ----------------------------------------------------------------__
enum class EInventoryTab { Inventory, Upgrades };

// ------------------------------------------------------------------
// Item type -- controls context menu buttons and actions
// ------------------------------------------------------------------
enum class EInventoryItemType
{
    Weapon,
    Ammo,
    Medkit,
    Battery,
    Junk,
};

struct InvCellWidthEntry
{
    const char* classname;
    int         cellWidth;
    static constexpr int WeaponCellWidth = 3;
};

// ------------------------------------------------------------------
// Non-weapon inventory item tracked on the client
// ------------------------------------------------------------------
struct InventoryItemEntry
{
    EInventoryItemType type        = EInventoryItemType::Junk;
    std::string        classname;
    std::string        displayName;
    int                count       = 0;
    HSPRITE            hSprite     = 0;
    Rect               rc          = {};
};

// ------------------------------------------------------------------
// Ammo entry used for grid display (collected each frame)
// ------------------------------------------------------------------
struct AmmoGridEntry
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

    void Show(int x, int y, int itemIndex, EInventoryItemType itemType);
    void Hide();

    int                GetItemIndex() const { return m_iItemIndex; }
    EInventoryItemType GetItemType()  const { return m_eItemType; }
    int                GetWeaponIndex() const { return m_iItemIndex; }

    bool HandleClick(int panelLocalX, int panelLocalY);
    virtual void paintBackground();

private:
    CInventoryPanel*   m_pOwner;
    vgui::Button*      m_pUseButton;
    vgui::Button*      m_pDropButton;
    int                m_iItemIndex  = -1;
    EInventoryItemType m_eItemType   = EInventoryItemType::Weapon;
};

// ------------------------------------------------------------------
// ActionSignal for context-menu buttons
// ------------------------------------------------------------------
class CInventoryMenuAction : public vgui::ActionSignal
{
public:
    enum Action { ACT_EQUIP, ACT_USE = ACT_EQUIP, ACT_DROP };

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
    vgui::Label*             m_pLabel;
    vgui::Button*            m_pCloseButton  = nullptr;
    vgui::Font*              m_pSmallFont    = nullptr;
    vgui::Font*              m_pTitleFont    = nullptr;
    std::vector<const char*> m_weaponNames;
    std::vector<WEAPON*>     m_weaponList;

    std::vector<InventoryItemEntry> m_inventoryItems;

    // Ammo entries collected each frame for grid display
    std::vector<AmmoGridEntry> m_ammoGridEntries;

    // Grid item offsets (owned here so context-menu actions can still read them)
    std::vector<int> m_weaponOffsetX;
    std::vector<int> m_weaponOffsetY;
    std::vector<int> m_invOffsetX;
    std::vector<int> m_invOffsetY;
    std::vector<int> m_ammoOffsetX;
    std::vector<int> m_ammoOffsetY;

    // Simple hit-rect used for items (populated each frame by the grid view)
    struct IRect { int x; int y; int w; int h; };
    std::vector<IRect> m_weaponRects;

    // ---- Tab state ----
    EInventoryTab m_eActiveTab = EInventoryTab::Inventory;
    // Nav-button rects populated each paint; used for click-detection
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

    void        SetWeaponNames(const char** names, int count);
    const char* GetWeaponName(int index) const;

    void                      UpdateInventoryItem(int itemId, int count);
    const InventoryItemEntry* GetInventoryItem(int index) const;

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
    // Kept here because SetWeaponNames still needs them to restore saved grid cells
    int SlotCellWidth(int slotIdx) const;
    int SlotNaturalCell(int slotIdx) const;
    void GetGridOrigin(int& x0, int& y0) const;
};

#endif