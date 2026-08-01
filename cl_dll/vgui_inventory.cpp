#include <VGUI_LineBorder.h>
#include <VGUI_App.h>
#include <VGUI_Cursor.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"
#include "vgui_helpers.h"
#include "vgui_loadtga.h"
#include "vgui_SpectatorPanel.h"
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "vgui_inventory.h"
#include <string>
#include <VGUI_Label.h>
#include <VGUI_Button.h>
#include <VGUI_ActionSignal.h>
#include "ammohistory.h"
#include <vector>
#include <cstdio>

using namespace vgui;
extern bool g_iVisibleMouse;
extern void IN_ResetRelativeMouseState();

static void LoadInvItemSprite(InventoryItemEntry& entry);

// =====================================================================
// LAYOUT CONSTANTS
// =====================================================================
static constexpr int INV_HEADER_H       = 64;
static constexpr int INV_LEFT_COL_W     = 240;
static constexpr int INV_MARGIN         = 8;
static constexpr int INV_SECTION_GAP    = 6;
static constexpr int INV_GRID_COLS      = 11;
static constexpr int INV_GRID_ROWS      = 8;
static constexpr int INV_GRID_PADDING   = 4;

static const InvCellWidthEntry k_cellWidthTable[] =
{
    { "item_healthkit", 1 },
    { "item_battery",   1 },
    { "item_antidote",  1 },
    { "item_security",  1 },
};

static int GetItemCellWidth(const std::string& classname)
{
    for (auto& e : k_cellWidthTable)
        if (classname == e.classname) return e.cellWidth;
    return 1;
}

// =====================================================================
// CInventoryCloseAction
// =====================================================================
void CInventoryCloseAction::actionPerformed(vgui::Panel*)
{
    if (m_pPanel) m_pPanel->Close();
}

// =====================================================================
// CInventoryPanel helpers
// =====================================================================
int CInventoryPanel::SlotCellWidth(int slotIdx) const
{
    if (slotIdx < (int)m_weaponList.size()) return InvCellWidthEntry::WeaponCellWidth;
    int afterWeapons = slotIdx - (int)m_weaponList.size();
    if (afterWeapons < (int)m_inventoryItems.size())
        return GetItemCellWidth(m_inventoryItems[afterWeapons].classname);
    return 1;
}

int CInventoryPanel::SlotNaturalCell(int slotIdx) const
{
    int cell = 0;
    for (int i = 0; i < slotIdx; ++i) cell += SlotCellWidth(i);
    return cell;
}

void CInventoryPanel::GetGridOrigin(int& x0, int& y0) const
{
    x0 = INV_MARGIN + INV_LEFT_COL_W + INV_MARGIN;
    y0 = INV_MARGIN + INV_HEADER_H + INV_MARGIN;
}

// =====================================================================
// CTX_MENU
// =====================================================================
static const int CTX_MENU_WIDTH  = 100;
static const int CTX_MENU_HEIGHT = 52;
static const int CTX_BTN_HEIGHT  = 22;
static const int CTX_BTN_MARGIN  = 4;

CInventoryContextMenu::CInventoryContextMenu(CInventoryPanel* pOwner, int wide, int tall)
    : Panel(0, 0, wide, tall), m_pOwner(pOwner), m_iItemIndex(-1)
{
    setVisible(false);
    setPos(-wide, -tall);
    setBgColor(0, 0, 0, 200);

    vgui::Font* ctxFont = nullptr;
    if (gViewPort)
    {
        CSchemeManager* pSchemes = gViewPort->GetSchemeManager();
        if (pSchemes)
        {
            SchemeHandle_t h = pSchemes->getSchemeHandle("Scoreboard Small Text");
            ctxFont = pSchemes->getFont(h);
        }
    }

    int btnW = wide - CTX_BTN_MARGIN * 2;
    int yPos = CTX_BTN_MARGIN;

    m_pUseButton = new Button("Equip", CTX_BTN_MARGIN, yPos, btnW, CTX_BTN_HEIGHT);
    m_pUseButton->setParent(this);
    m_pUseButton->setContentAlignment(Label::a_center);
    m_pUseButton->addActionSignal(new CInventoryMenuAction(pOwner, this, CInventoryMenuAction::ACT_USE));
    m_pUseButton->setPaintBackgroundEnabled(false);
    m_pUseButton->setBgColor(0, 0, 0, 255);
    m_pUseButton->setFgColor(Scheme::sc_primary1);
    if (ctxFont) m_pUseButton->setFont(ctxFont);

    yPos += CTX_BTN_HEIGHT + 2;

    m_pDropButton = new Button("Drop", CTX_BTN_MARGIN, yPos, btnW, CTX_BTN_HEIGHT);
    m_pDropButton->setParent(this);
    m_pDropButton->setContentAlignment(Label::a_center);
    m_pDropButton->addActionSignal(new CInventoryMenuAction(pOwner, this, CInventoryMenuAction::ACT_DROP));
    m_pDropButton->setPaintBackgroundEnabled(false);
    m_pDropButton->setBgColor(0, 0, 0, 255);
    m_pDropButton->setFgColor(Scheme::sc_primary1);
    if (ctxFont) m_pDropButton->setFont(ctxFont);
}

void CInventoryContextMenu::Show(int x, int y, int itemIndex, EInventoryItemType itemType)
{
    m_iItemIndex = itemIndex;
    m_eItemType  = itemType;
    switch (itemType)
    {
    case EInventoryItemType::Weapon:
        m_pUseButton->setText("Equip");
        m_pUseButton->setVisible(true);
        m_pDropButton->setVisible(true);
        break;
    case EInventoryItemType::Medkit:
    case EInventoryItemType::Battery:
        m_pUseButton->setText("Use");
        m_pUseButton->setVisible(true);
        m_pDropButton->setVisible(true);
        break;
    case EInventoryItemType::Ammo:
        m_pUseButton->setVisible(false);
        m_pDropButton->setVisible(true);
        break;
    case EInventoryItemType::Junk:
        m_pUseButton->setVisible(false);
        m_pDropButton->setVisible(true);
        break;
    }
    setPos(x, y);
    setVisible(true);
}

void CInventoryContextMenu::Hide()
{
    setVisible(false);
    m_iItemIndex = -1;
    int w, h; getSize(w, h);
    setPos(-w, -h);
}

bool CInventoryContextMenu::HandleClick(int panelLocalX, int panelLocalY)
{
    if (!isVisible()) return false;
    int menuX, menuY; getPos(menuX, menuY);
    int localX = panelLocalX - menuX, localY = panelLocalY - menuY;
    int menuW, menuH; getSize(menuW, menuH);
    if (localX < 0 || localX >= menuW || localY < 0 || localY >= menuH) return false;

    if (m_pUseButton && m_pUseButton->isVisible())
    {
        int bx, by, bw, bh; m_pUseButton->getBounds(bx, by, bw, bh);
        if (localX >= bx && localX < bx + bw && localY >= by && localY < by + bh)
        { m_pUseButton->doClick(); return true; }
    }
    if (m_pDropButton && m_pDropButton->isVisible())
    {
        int bx, by, bw, bh; m_pDropButton->getBounds(bx, by, bw, bh);
        if (localX >= bx && localX < bx + bw && localY >= by && localY < by + bh)
        { m_pDropButton->doClick(); return true; }
    }
    return false;
}

void CInventoryContextMenu::paintBackground()
{
    int w, h; getSize(w, h);
    drawSetColor(0, 0, 0, 56);
    drawFilledRect(0, 0, w, h);
    drawSetColor(255, 170, 0, 80);
    drawOutlinedRect(0, 0, w, h);
}

// =====================================================================
// CInventoryMenuAction
// =====================================================================
CInventoryMenuAction::CInventoryMenuAction(CInventoryPanel* pOwner, CInventoryContextMenu* pMenu, Action action)
    : m_pOwner(pOwner), m_pMenu(pMenu), m_action(action) {}

void CInventoryMenuAction::actionPerformed(vgui::Panel* panel)
{
    if (!m_pOwner || !m_pMenu) return;
    int idx = m_pMenu->GetItemIndex();
    char cmd[128];
    switch (m_action)
    {
    case ACT_USE:
    {
        EInventoryItemType itemType = m_pMenu->GetItemType();
        if (itemType == EInventoryItemType::Weapon)
        {
            const char* weapon = m_pOwner->GetWeaponName(idx);
            if (!weapon || weapon[0] == '\0') break;
            snprintf(cmd, sizeof(cmd), "use %s\n", weapon);
            gEngfuncs.pfnClientCmd(cmd);
            m_pOwner->CloseContextMenu();
            m_pOwner->Close();
            return;
        }
        else
        {
            const InventoryItemEntry* entry = m_pOwner->GetInventoryItem(idx);
            if (!entry || entry->classname.empty()) break;
            snprintf(cmd, sizeof(cmd), "inv_use %s\n", entry->classname.c_str());
            gEngfuncs.pfnClientCmd(cmd);
            m_pOwner->CloseContextMenu();
            return;
        }
    }
    case ACT_DROP:
    {
        EInventoryItemType itemType = m_pMenu->GetItemType();
        if (itemType == EInventoryItemType::Weapon)
        {
            const char* weapon = m_pOwner->GetWeaponName(idx);
            if (!weapon || weapon[0] == '\0') break;
            snprintf(cmd, sizeof(cmd), "drop %s\n", weapon);
        }
        else
        {
            const InventoryItemEntry* entry = m_pOwner->GetInventoryItem(idx);
            if (!entry || entry->classname.empty()) break;
            snprintf(cmd, sizeof(cmd), "drop %s\n", entry->classname.c_str());
        }
        gEngfuncs.pfnClientCmd(cmd);
        m_pOwner->CloseContextMenu();
        return;
    }
    }
    m_pOwner->CloseContextMenu();
}

// =====================================================================
// CInventoryPanel constructor
// =====================================================================
CInventoryPanel::CInventoryPanel(int x, int y, int wide, int tall)
    : Panel(x, y, wide, tall)
{
    setVisible(false);
    setBgColor(0, 0, 0, 96);

    m_pSmallFont = nullptr;
    m_pTitleFont = nullptr;

    const int closeBtnSz = INV_HEADER_H - 6;
    const int closeBtnOffsetX = 4;
    int labelX = INV_MARGIN + 8;
    int labelY = INV_MARGIN + (INV_HEADER_H - 20) / 2;
    int labelW = wide - INV_MARGIN * 2 - closeBtnSz - 8;
    m_pLabel = new Label("INVENTORY", labelX, labelY, labelW, 20);

    if (gViewPort)
    {
        CSchemeManager* pSchemes = gViewPort->GetSchemeManager();
        if (pSchemes)
        {
            SchemeHandle_t hTitle = pSchemes->getSchemeHandle("Scoreboard Title Text");
            SchemeHandle_t hSmall = pSchemes->getSchemeHandle("Scoreboard Small Text");
            m_pTitleFont = pSchemes->getFont(hTitle);
            m_pSmallFont = pSchemes->getFont(hSmall);
            if (m_pTitleFont) m_pLabel->setFont(m_pTitleFont);
        }
    }
    m_pLabel->setPaintBackgroundEnabled(false);
    m_pLabel->setFgColor(255, 200, 60, 0);
    m_pLabel->setContentAlignment(vgui::Label::a_west);
	m_pLabel->setText("Inventory");
    m_pLabel->setParent(this);

    m_pCloseButton = new Button("", wide - INV_MARGIN - closeBtnSz - closeBtnOffsetX, INV_MARGIN + 3, closeBtnSz, closeBtnSz);
    m_pCloseButton->setParent(this);
    m_pCloseButton->setPaintBackgroundEnabled(false);
    m_pCloseButton->setPaintEnabled(false);
    m_pCloseButton->setContentAlignment(Label::a_center);
    m_pCloseButton->addActionSignal(new CInventoryCloseAction(this));
    m_pCloseButton->setBgColor(0, 0, 0, 255);
    m_pCloseButton->setFgColor(255, 80, 80, 0);
    if (m_pTitleFont) m_pCloseButton->setFont(m_pTitleFont);

    SetWeaponNames(nullptr, 0);

    m_pContextMenu = new CInventoryContextMenu(this, CTX_MENU_WIDTH, CTX_MENU_HEIGHT);
    m_pContextMenu->setParent(this);

    m_HitTestPanel.setBgColor(0, 0, 0, 255);
    m_HitTestPanel.setParent(this);
    m_HitTestPanel.setBounds(0, 0, wide, tall);
    m_HitTestPanel.addInputSignal(this);

    m_eActiveTab = EInventoryTab::Inventory;
    for (int i = 0; i < k_NumNavBtns; ++i) m_navBtnRects[i] = {};
}

void CInventoryPanel::SetText(const char* text) { m_pLabel->setText(text); }

void CInventoryPanel::Open()
{
    for (auto& entry : m_inventoryItems)
        LoadInvItemSprite(entry);

    SetWeaponNames(nullptr, 0);

    m_HitTestPanel.setVisible(true);
    setVisible(true);
    g_iVisibleMouse = true;
    App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow));

    int centerX = ScreenWidth / 2;
    int centerY = ScreenHeight / 2;
    App::getInstance()->setCursorPos(centerX, centerY);

    IN_ResetRelativeMouseState();
}

void CInventoryPanel::Close()
{
    CloseContextMenu();
    m_gridView.CancelDrag();
    setVisible(false);
    if (gViewPort) gViewPort->UpdateCursorState();
}

void CInventoryPanel::Initialize() { setBgColor(0, 0, 0, 96); }

// Accessors used by context-menu action handlers
const char* CInventoryPanel::GetWeaponName(int index) const
{
    if (index < 0 || index >= (int)m_weaponNames.size()) return nullptr;
    return m_weaponNames[index];
}

void CInventoryPanel::CloseContextMenu()
{
    if (m_pContextMenu) m_pContextMenu->Hide();
}

// =====================================================================
// Inventory item template table
// =====================================================================
static const InventoryItemEntry k_invItemTemplates[] =
{
    {},
    { EInventoryItemType::Medkit,  "item_healthkit", "Medkit",  0, 0, {} },
    { EInventoryItemType::Junk,    "item_antidote",  "Antidote",0, 0, {} },
    { EInventoryItemType::Junk,    "item_security",  "Keycard", 0, 0, {} },
    { EInventoryItemType::Battery, "item_battery",   "Battery", 0, 0, {} },
};
static constexpr int k_numInvItemTemplates = (int)(sizeof(k_invItemTemplates) / sizeof(k_invItemTemplates[0]));

static void LoadInvItemSprite(InventoryItemEntry& entry)
{
    if (entry.hSprite != 0) return;

    const char* hudSpriteName = nullptr;
    if (entry.classname == "item_healthkit")  hudSpriteName = "item_healthkit";
    else if (entry.classname == "item_battery") hudSpriteName = "item_battery";
    if (!hudSpriteName) return;

    int idx = gHUD.GetSpriteIndex(hudSpriteName);
    if (idx < 0) return;
    entry.hSprite = gHUD.GetSprite(idx);
    entry.rc      = gHUD.GetSpriteRect(idx);
}

void CInventoryPanel::UpdateInventoryItem(int itemId, int count)
{
    if (itemId <= 0 || itemId >= k_numInvItemTemplates) return;
    const std::string& targetClassname = k_invItemTemplates[itemId].classname;

    for (auto it = m_inventoryItems.begin(); it != m_inventoryItems.end(); ++it)
    {
        if (it->classname == targetClassname)
        {
            if (count <= 0)
            {
                int idx = (int)(it - m_inventoryItems.begin());
                m_inventoryItems.erase(it);
                if (idx < (int)m_invOffsetX.size())
                { m_invOffsetX.erase(m_invOffsetX.begin() + idx); m_invOffsetY.erase(m_invOffsetY.begin() + idx); }
            }
            else
            {
                it->count = count;
                LoadInvItemSprite(*it);
            }
            return;
        }
    }
    if (count > 0)
    {
        InventoryItemEntry newEntry = k_invItemTemplates[itemId];
        newEntry.count = count;
        LoadInvItemSprite(newEntry);
        m_inventoryItems.push_back(std::move(newEntry));
        m_invOffsetX.push_back(0);
        m_invOffsetY.push_back(0);
    }
}

const InventoryItemEntry* CInventoryPanel::GetInventoryItem(int index) const
{
    int invIdx = index - (int)m_weaponList.size();
    if (invIdx < 0 || invIdx >= (int)m_inventoryItems.size()) return nullptr;
    return &m_inventoryItems[invIdx];
}

void CInventoryPanel::SetWeaponNames(const char** names, int count)
{
    m_weaponNames.clear();
    m_weaponList.clear();
    m_weaponRects.clear();

    if (names == nullptr || count == 0)
    {
        for (int i = 0; i < MAX_WEAPONS; ++i)
        {
            WEAPON* w = gWR.GetWeapon(i);
            if (!w || w->iId == 0 || w->szName[0] == '\0') continue;
            if (!gHUD.HasWeapon(w->iId)) continue;
            m_weaponNames.push_back(w->szName);
            m_weaponList.push_back(w);
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            const char* n = names[i];
            if (!n || n[0] == '\0') continue;
            m_weaponNames.push_back(n);
            for (int j = 0; j < MAX_WEAPONS; j++)
            {
                WEAPON* w = gWR.GetWeapon(j);
                if (w && w->iId != 0 && strcmp(w->szName, n) == 0) { m_weaponList.push_back(w); break; }
            }
        }
    }

    m_weaponOffsetX.assign(m_weaponList.size(), 0);
    m_weaponOffsetY.assign(m_weaponList.size(), 0);
    m_ammoOffsetX.clear();
    m_ammoOffsetY.clear();

    // Restore saved grid positions
    {
        int x0, y0; GetGridOrigin(x0, y0);
        int panelW = 0, panelH = 0; getSize(panelW, panelH);
        int gridAreaW = panelW - x0 - INV_MARGIN;
        int gridAreaH = panelH - y0 - INV_MARGIN;
        int cellSize  = std::min(
            (gridAreaW - (INV_GRID_COLS - 1) * INV_GRID_PADDING) / INV_GRID_COLS,
            (gridAreaH - (INV_GRID_ROWS - 1) * INV_GRID_PADDING) / INV_GRID_ROWS);
        if (cellSize < 1) cellSize = 1;
        int cellStepX = cellSize + INV_GRID_PADDING;
        int cellStepY = cellSize + INV_GRID_PADDING;

        for (int i = 0; i < (int)m_weaponList.size(); ++i)
        {
            WEAPON* w = m_weaponList[i];
            if (!w || w->iId <= 0) continue;
            int targetCell = gWR.GetGridCell(w->iId);
            if (targetCell < 0 || targetCell >= INV_GRID_COLS * INV_GRID_ROWS) continue;
            int naturalCell = SlotNaturalCell(i);
            int origCol = naturalCell % INV_GRID_COLS, origRow = naturalCell / INV_GRID_COLS;
            int tCol    = targetCell % INV_GRID_COLS,   tRow   = targetCell / INV_GRID_COLS;
            m_weaponOffsetX[i] = x0 + tCol * cellStepX - (x0 + origCol * cellStepX);
            m_weaponOffsetY[i] = y0 + tRow * cellStepY - (y0 + origRow * cellStepY);
        }
    }
}

// =====================================================================
// paint / paintBackground
// =====================================================================
void CInventoryPanel::paint() {}

void CInventoryPanel::paintBackground()
{
    Panel::paintBackground();
    m_weaponRects.clear();
    drawSetTextPos(0, 0);

    int panelW = 0, panelH = 0;
    getSize(panelW, panelH);

    // ----------------------------------------------------------------
    // HEADER BAR
    // ----------------------------------------------------------------
    {
        int hx1 = INV_MARGIN, hy1 = INV_MARGIN;
        int hx2 = panelW - INV_MARGIN, hy2 = INV_MARGIN + INV_HEADER_H;

        drawSetColor(20, 20, 20, 30);
        drawFilledRect(hx1, hy1, hx2, hy2);
        drawSetColor(255, 170, 0, 80);
        drawOutlinedRect(hx1, hy1, hx2, hy2);
        drawFilledRect(hx1, hy2 - 2, hx2, hy2);
        drawSetColor(255, 170, 0, 0);
        drawFilledRect(hx1, hy1, hx1 + 3, hy2);

        if (m_pCloseButton)
        {
            int bx, by, bw, bh;
            m_pCloseButton->getBounds(bx, by, bw, bh);
            drawSetColor(40, 10, 10, 60);
            drawFilledRect(bx, by, bx + bw, by + bh);
            drawSetColor(220, 60, 60, 40);
            drawOutlinedRect(bx, by, bx + bw, by + bh);
            drawSetColor(255, 100, 100, 100);
            drawFilledRect(bx + 1, by + 1, bx + bw - 1, by + 2);
        }
    }

    // ----------------------------------------------------------------
    // LEFT COLUMN
    // ----------------------------------------------------------------
    {
        int colX = INV_MARGIN;
        int colY = INV_MARGIN + INV_HEADER_H + INV_MARGIN;
        int colW = INV_LEFT_COL_W;
        int colH = panelH - colY - INV_MARGIN;

        // Diagnostic layout: make nav area taller so overdraw around tabs is easier to inspect.
        int panelH1 = (colH * 2) / 3;
        int panelH2 = colH - panelH1 - INV_SECTION_GAP;

        // --- Panel 1: NAV TABS ---
        int p1Y = colY;
        static constexpr int NAV_BTN_H   = 76;
        static constexpr int NAV_BTN_GAP = 4;

        drawSetColor(15, 15, 15, 60);
        drawFilledRect(colX, p1Y, colX + colW, p1Y + panelH1);
        drawSetColor(255, 170, 0, 80);
        drawOutlinedRect(colX, p1Y, colX + colW, p1Y + panelH1);
        drawSetColor(255, 170, 0, 0);
        drawFilledRect(colX, p1Y, colX + colW, p1Y + 2);

        struct NavEntry { const char* label; EInventoryTab tab; };
        NavEntry navBtns[k_NumNavBtns] = {
            { "Inventory", EInventoryTab::Inventory },
            { "Upgrades",  EInventoryTab::Upgrades  }
        };

        int btnY = p1Y + 6;
        for (int nb = 0; nb < k_NumNavBtns; ++nb)
        {
            bool active = (m_eActiveTab == navBtns[nb].tab);
            int btnX  = colX + 6;
            int btnW2 = colW - 12;
            int btnH  = NAV_BTN_H;

            m_navBtnRects[nb] = { btnX, btnY, btnW2, btnH };

            if (active)
            {
                drawSetColor(200, 130, 0, 30);
                drawFilledRect(btnX, btnY, btnX + btnW2, btnY + btnH);
                drawSetColor(255, 170, 0, 40);
                drawOutlinedRect(btnX, btnY, btnX + btnW2, btnY + btnH);
                drawSetColor(255, 170, 0, 0);
                drawFilledRect(btnX, btnY, btnX + 3, btnY + btnH);
            }
            else
            {
                drawSetColor(30, 30, 30, 120);
                drawFilledRect(btnX, btnY, btnX + btnW2, btnY + btnH);
                drawSetColor(120, 100, 60, 140);
                drawOutlinedRect(btnX, btnY, btnX + btnW2, btnY + btnH);
            }

            btnY += btnH + NAV_BTN_GAP;
        }

        // --- Panel 2: ITEMS side panel ---
        int p2Y = colY + panelH1 + INV_SECTION_GAP;

        drawSetColor(15, 15, 15, 60);
        drawFilledRect(colX, p2Y, colX + colW, p2Y + panelH2);
        drawSetColor(100, 200, 255, 80);
        drawOutlinedRect(colX, p2Y, colX + colW, p2Y + panelH2);
        drawSetColor(100, 200, 255, 0);
        drawFilledRect(colX, p2Y, colX + colW, p2Y + 2);

        const InventoryItemEntry* pLongjump = nullptr;
        for (const auto& entry : m_inventoryItems)
        {
            if (entry.count <= 0) continue;
            if (entry.classname.find("longjump") != std::string::npos ||
                entry.displayName.find("Longjump") != std::string::npos)
            { pLongjump = &entry; break; }
        }
        if (pLongjump)
        {
            int tileX = colX + 6, tileY = p2Y + 6;
            int tileW = colW - 12, tileH = panelH2 - 12;
            drawSetColor(40, 40, 40, 140);
            drawFilledRect(tileX, tileY, tileX + tileW, tileY + tileH);
            if (pLongjump->hSprite && (pLongjump->rc.right - pLongjump->rc.left) > 0)
            {
                int sprW = pLongjump->rc.right - pLongjump->rc.left;
                int sprH = pLongjump->rc.bottom - pLongjump->rc.top;
                int maxW = tileW - 12, maxH = tileH - 12;
                float scale = 1.0f;
                if (sprW > maxW || sprH > maxH)
                    scale = std::min((float)maxW / (float)sprW, (float)maxH / (float)sprH);
                int drawX = tileX + (tileW - (int)(sprW * scale)) / 2;
                int drawY = tileY + (tileH - (int)(sprH * scale)) / 2;
                SPR_Set(pLongjump->hSprite, 220, 220, 220);
                SPR_DrawAdditive(0, drawX, drawY, &pLongjump->rc);
            }
        }

    }

    // ----------------------------------------------------------------
    // RIGHT COLUMN � delegate to the active view
    // ----------------------------------------------------------------
    int x0, y0;
    GetGridOrigin(x0, y0);
    int areaW = panelW - x0 - INV_MARGIN;
    int areaH = panelH - y0 - INV_MARGIN;
    drawSetTextPos(0, 0);

    if (m_eActiveTab == EInventoryTab::Inventory)
    {
        m_gridView.Paint(this, x0, y0, areaW, areaH,
            m_weaponList, m_weaponNames,
            m_weaponOffsetX, m_weaponOffsetY,
            m_inventoryItems, m_invOffsetX, m_invOffsetY,
            m_ammoGridEntries, m_ammoOffsetX, m_ammoOffsetY);

        // Mirror grid item rects for context-menu hit-testing
        for (auto& r : m_gridView.GetItemRects())
            m_weaponRects.push_back({ r.x, r.y, r.w, r.h });
    }
    else
    {
        m_skillTreeView.Paint(this, x0, y0, areaW, areaH,
            m_pSmallFont, m_pTitleFont);
    }

    // Draw nav labels after sprite-heavy sections to avoid stale text cursor state
    // affecting later sprite draws inside the same frame.
    if (m_pSmallFont)
    {
        static const char* navLabels[k_NumNavBtns] = { "Inventory", "Upgrades" };
        static constexpr int NAV_BTN_PAD = 6;

        for (int nb = 0; nb < k_NumNavBtns; ++nb)
        {
            const IRect& r = m_navBtnRects[nb];
            bool active = (m_eActiveTab == ((nb == 0) ? EInventoryTab::Inventory : EInventoryTab::Upgrades));
            int nameLen = (int)strlen(navLabels[nb]);

            vgui::Font* navFont = m_pTitleFont ? m_pTitleFont : m_pSmallFont;
            drawSetTextFont(navFont);
            if (active) drawSetTextColor(255, 200, 60, 0);
            else        drawSetTextColor(160, 140, 100, 0);

            int textH = m_pTitleFont ? 18 : 10;
            drawSetTextPos(r.x + NAV_BTN_PAD + 4, r.y + (r.h - textH) / 2);
            drawPrintText(navLabels[nb], nameLen);
        }
    }

    // Draw close button text last so it cannot leak text cursor state into sprite passes.
    if (m_pCloseButton && m_pTitleFont)
    {
        int bx, by, bw, bh;
        m_pCloseButton->getBounds(bx, by, bw, bh);
        drawSetTextFont(m_pTitleFont);
        drawSetTextColor(255, 80, 80, 0);
        int charW = 5, charH = 5;
        drawSetTextPos(bx + (bw - charW) / 2 - 1, by + (bh - charH) / 2 - 1);
        drawPrintText("x", 1);
    }

    drawSetTextPos(0, 0);

}

// =====================================================================
// HitTestPanel
// =====================================================================
void CInventoryPanel::HitTestPanel::internalMousePressed(MouseCode code)
{
    for (int i = 0; i < _inputSignalDar.getCount(); i++)
        _inputSignalDar[i]->mousePressed(code, this);
}

void CInventoryPanel::HitTestPanel::internalMouseReleased(MouseCode code)
{
    for (int i = 0; i < _inputSignalDar.getCount(); i++)
        _inputSignalDar[i]->mouseReleased(code, this);
}

// =====================================================================
// Mouse input
// =====================================================================
void CInventoryPanel::mousePressed(vgui::MouseCode code, vgui::Panel* panel)
{
    int cx = 0, cy = 0; getApp()->getCursorPos(cx, cy);
    int ax = 0, ay = 0, bx2 = 0, by2 = 0; getAbsExtents(ax, ay, bx2, by2);
    int localx = cx - ax, localy = cy - ay;

    // ---- Right-click: context menu (inventory tab only) ----
    if (code == MOUSE_RIGHT)
    {
        if (m_eActiveTab == EInventoryTab::Inventory)
        {
            CloseContextMenu();
            for (size_t i = 0; i < m_weaponRects.size(); ++i)
            {
                IRect& r = m_weaponRects[i];
                if (localx >= r.x && localx < r.x + r.w && localy >= r.y && localy < r.y + r.h)
                {
                    if (m_pContextMenu)
                    {
                        int panelW = 0, panelH = 0; getSize(panelW, panelH);
                        int menuX = std::min(localx, panelW - CTX_MENU_WIDTH);
                        int menuY = std::min(localy, panelH - CTX_MENU_HEIGHT);
                        EInventoryItemType ctxType = EInventoryItemType::Weapon;
                        if (i >= m_weaponList.size())
                        {
                            if (i < m_weaponList.size() + m_inventoryItems.size())
                            {
                                const InventoryItemEntry* e = GetInventoryItem((int)i);
                                if (e) ctxType = e->type;
                            }
                            else if (i < m_weaponList.size() + m_inventoryItems.size() + m_ammoGridEntries.size())
                            {
                                ctxType = EInventoryItemType::Ammo;
                            }
                        }
                        m_pContextMenu->Show(menuX, menuY, (int)i, ctxType);
                    }
                    return;
                }
            }
            CloseContextMenu();
        }
        return;
    }

    if (code != MOUSE_LEFT) return;

    // ---- Close button ----
    if (m_pCloseButton)
    {
        int bx, by, bw, bh; m_pCloseButton->getBounds(bx, by, bw, bh);
        if (localx >= bx && localx < bx + bw && localy >= by && localy < by + bh)
        { Close(); return; }
    }

    // ---- Nav tab buttons ----
    for (int nb = 0; nb < k_NumNavBtns; ++nb)
    {
        IRect& r = m_navBtnRects[nb];
        if (localx >= r.x && localx < r.x + r.w && localy >= r.y && localy < r.y + r.h)
        {
            EInventoryTab newTab = (nb == 0) ? EInventoryTab::Inventory : EInventoryTab::Upgrades;
            if (m_eActiveTab != newTab)
            {
                m_eActiveTab = newTab;
                CloseContextMenu();
                m_gridView.CancelDrag();
                m_skillTreeView.HandleMouseMove(-1, -1);
            }
            return;
        }
    }

    // ---- Context menu (inventory tab) ----
    if (m_eActiveTab == EInventoryTab::Inventory && m_pContextMenu && m_pContextMenu->isVisible())
    {
        if (m_pContextMenu->HandleClick(localx, localy)) return;
        CloseContextMenu(); return;
    }

    // ---- Delegate to active view ----
    if (m_eActiveTab == EInventoryTab::Inventory)
    {
        m_gridView.HandleMousePress(this, localx, localy,
            m_weaponNames, m_weaponList,
            m_weaponOffsetX, m_weaponOffsetY,
            m_inventoryItems, m_invOffsetX, m_invOffsetY,
            m_ammoGridEntries, m_ammoOffsetX, m_ammoOffsetY);
    }
    else
    {
        m_skillTreeView.HandleMousePress(this, localx, localy);
    }
}

void CInventoryPanel::mouseReleased(vgui::MouseCode code, vgui::Panel* panel)
{
    if (code != MOUSE_LEFT) return;

    if (m_eActiveTab == EInventoryTab::Inventory)
    {
        int cx = 0, cy = 0; getApp()->getCursorPos(cx, cy);
        int ax = 0, ay = 0, bx2 = 0, by2 = 0; getAbsExtents(ax, ay, bx2, by2);
        int localx = cx - ax, localy = cy - ay;
        m_gridView.HandleMouseRelease(this, localx, localy,
            m_weaponNames, m_weaponList,
            m_weaponOffsetX, m_weaponOffsetY,
            m_inventoryItems, m_invOffsetX, m_invOffsetY,
            m_ammoGridEntries, m_ammoOffsetX, m_ammoOffsetY);
    }
}

void CInventoryPanel::cursorMoved(int x, int y, vgui::Panel* panel)
{
    if (m_eActiveTab == EInventoryTab::Inventory)
    {
        m_gridView.HandleMouseMove(this,
            m_weaponOffsetX, m_weaponOffsetY, m_weaponList,
            m_inventoryItems,
            m_invOffsetX, m_invOffsetY,
            m_ammoGridEntries, m_ammoOffsetX, m_ammoOffsetY);
    }
    else
    {
        m_skillTreeView.HandleMouseMove(x, y);
    }
}