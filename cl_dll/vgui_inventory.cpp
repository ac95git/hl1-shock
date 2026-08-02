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
#include <algorithm>
#include <cstdio>

using namespace vgui;
extern bool g_iVisibleMouse;
extern void IN_ResetRelativeMouseState();

// =====================================================================
// LAYOUT CONSTANTS
//
// The Grid's own dimensions are NOT here -- they come from the server
// with every sync, because the number of Rows changes as the player
// earns them. See docs/adr/0003-fixed-grid-width-rows-only-growth.md.
// =====================================================================
static constexpr int INV_HEADER_H    = 64;
static constexpr int INV_LEFT_COL_W  = 240;
static constexpr int INV_MARGIN      = 8;
static constexpr int INV_SECTION_GAP = 6;

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
void CInventoryPanel::GetGridOrigin(int& x0, int& y0) const
{
    x0 = INV_MARGIN + INV_LEFT_COL_W + INV_MARGIN;
    y0 = INV_MARGIN + INV_HEADER_H + INV_MARGIN;
}

// Classname of a weapon, for the engine's own "use <weapon>" command.
static const char* WeaponClassnameFromId(int weaponId)
{
    WEAPON* w = gWR.GetWeapon(weaponId);
    if (!w || w->iId == 0 || w->szName[0] == '\0')
        return nullptr;
    return w->szName;
}

// =====================================================================
// CTX_MENU
// =====================================================================
static const int CTX_MENU_WIDTH  = 100;
static const int CTX_BTN_HEIGHT  = 22;
static const int CTX_BTN_MARGIN  = 4;
static const int CTX_BTN_GAP     = 2;
static const int CTX_MENU_BUTTONS = 3;

// The menu is sized to the buttons it actually shows; this is the tallest it
// can get, used to keep it on screen before the layout is known.
static const int CTX_MENU_MAX_HEIGHT =
    CTX_BTN_MARGIN * 2 + CTX_MENU_BUTTONS * CTX_BTN_HEIGHT + (CTX_MENU_BUTTONS - 1) * CTX_BTN_GAP;

CInventoryContextMenu::CInventoryContextMenu(CInventoryPanel* pOwner, int wide, int tall)
    : Panel(0, 0, wide, tall), m_pOwner(pOwner), m_iEntryIndex(-1)
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

    const int btnW = wide - CTX_BTN_MARGIN * 2;

    auto makeButton = [&](const char* label, CInventoryMenuAction::Action action) -> Button*
    {
        // Positions are set in Show(), which lays out only the visible buttons.
        Button* btn = new Button(label, CTX_BTN_MARGIN, CTX_BTN_MARGIN, btnW, CTX_BTN_HEIGHT);
        btn->setParent(this);
        btn->setContentAlignment(Label::a_center);
        btn->addActionSignal(new CInventoryMenuAction(pOwner, this, action));
        btn->setPaintBackgroundEnabled(false);
        btn->setBgColor(0, 0, 0, 255);
        btn->setFgColor(Scheme::sc_primary1);
        if (ctxFont) btn->setFont(ctxFont);
        return btn;
    };

    m_pUseButton     = makeButton("Equip",    CInventoryMenuAction::ACT_USE);
    m_pDropOneButton = makeButton("Drop",     CInventoryMenuAction::ACT_DROP_ONE);
    m_pDropAllButton = makeButton("Drop all", CInventoryMenuAction::ACT_DROP_ALL);
}

void CInventoryContextMenu::Show(int x, int y, int entryIndex, EEntryKind kind, int id, int count)
{
    m_iEntryIndex = entryIndex;
    m_eKind = kind;
    m_iId = id;

    bool showUse = false;
    if (kind == EEntryKind::Weapon)
    {
        m_pUseButton->setText("Equip");
        showUse = true;
    }
    else
    {
        // Only consumables do anything when used; a keycard is carried, not used.
        // Read from the shared Item Type table rather than listed here, so a new
        // item cannot ship without its Use button.
        const ItemTypeDef* def = GetItemType(id);
        showUse = (def && def->usable);
        m_pUseButton->setText("Use");
    }

    // A Stack can shed one item or all of them; a single item just drops.
    const bool isStack = (count > 1);
    m_pDropOneButton->setText(isStack ? "Drop 1" : "Drop");

    Button* order[CTX_MENU_BUTTONS] = { m_pUseButton, m_pDropOneButton, m_pDropAllButton };
    const bool visible[CTX_MENU_BUTTONS] = { showUse, true, isStack };

    // Laid out over the visible buttons only, so a hidden option leaves no gap.
    int yPos = CTX_BTN_MARGIN;
    int shown = 0;
    for (int i = 0; i < CTX_MENU_BUTTONS; ++i)
    {
        order[i]->setVisible(visible[i]);
        if (!visible[i])
            continue;

        order[i]->setBounds(CTX_BTN_MARGIN, yPos, CTX_MENU_WIDTH - CTX_BTN_MARGIN * 2, CTX_BTN_HEIGHT);
        yPos += CTX_BTN_HEIGHT + CTX_BTN_GAP;
        ++shown;
    }

    const int height = (shown > 0)
        ? (yPos - CTX_BTN_GAP + CTX_BTN_MARGIN)
        : (CTX_BTN_MARGIN * 2);

    setSize(CTX_MENU_WIDTH, height);
    setPos(x, y);
    setVisible(true);
}

void CInventoryContextMenu::Hide()
{
    setVisible(false);
    m_iEntryIndex = -1;
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

    Button* order[CTX_MENU_BUTTONS] = { m_pUseButton, m_pDropOneButton, m_pDropAllButton };
    for (Button* btn : order)
    {
        if (!btn || !btn->isVisible())
            continue;

        int bx, by, bw, bh; btn->getBounds(bx, by, bw, bh);
        if (localX >= bx && localX < bx + bw && localY >= by && localY < by + bh)
        {
            btn->doClick();
            return true;
        }
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
//
// Every verb is a REQUEST. The panel changes nothing itself; the server
// validates, acts, and syncs back.
// =====================================================================
CInventoryMenuAction::CInventoryMenuAction(CInventoryPanel* pOwner, CInventoryContextMenu* pMenu, Action action)
    : m_pOwner(pOwner), m_pMenu(pMenu), m_action(action) {}

void CInventoryMenuAction::actionPerformed(vgui::Panel* panel)
{
    if (!m_pOwner || !m_pMenu) return;

    const int index = m_pMenu->GetEntryIndex();
    const EEntryKind kind = m_pMenu->GetEntryKind();
    const int id = m_pMenu->GetEntryId();

    if (index < 0)
    {
        m_pOwner->CloseContextMenu();
        return;
    }

    char cmd[128];

    if (m_action == ACT_USE)
    {
        if (kind == EEntryKind::Weapon)
        {
            // Equipping goes through Half-Life's own weapon selection.
            const char* classname = WeaponClassnameFromId(id);
            if (classname)
            {
                snprintf(cmd, sizeof(cmd), "use %s\n", classname);
                gEngfuncs.pfnClientCmd(cmd);
                m_pOwner->CloseContextMenu();
                m_pOwner->Close();
                return;
            }
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "inv_use %d %d %d\n", index, (int)kind, id);
            gEngfuncs.pfnClientCmd(cmd);
        }
    }
    else if (m_action == ACT_DROP_ONE)
    {
        snprintf(cmd, sizeof(cmd), "inv_drop %d %d %d\n", index, (int)kind, id);
        gEngfuncs.pfnClientCmd(cmd);
    }
    else if (m_action == ACT_DROP_ALL)
    {
        snprintf(cmd, sizeof(cmd), "inv_dropall %d %d %d\n", index, (int)kind, id);
        gEngfuncs.pfnClientCmd(cmd);
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

    // Constructed at its tallest; Show() resizes it to whatever options apply.
    m_pContextMenu = new CInventoryContextMenu(this, CTX_MENU_WIDTH, CTX_MENU_MAX_HEIGHT);
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
    // Ask for a fresh copy rather than trusting whatever we last saw --
    // the panel may have been closed across a level change.
    gEngfuncs.pfnClientCmd("inv_sync\n");

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
    // An armed Reset must not still be armed when the panel is opened again.
    m_skillTreeView.CancelResetConfirm();
    setVisible(false);
    if (gViewPort) gViewPort->UpdateCursorState();
}

void CInventoryPanel::Initialize() { setBgColor(0, 0, 0, 96); }

void CInventoryPanel::CloseContextMenu()
{
    if (m_pContextMenu) m_pContextMenu->Hide();
}

// =====================================================================
// Server sync
// =====================================================================
void CInventoryPanel::UpdateInventory(bool reset, int gridWidth, int rows, int rowsToDraw,
                                      const InvEntryView* entries, int count)
{
    if (reset)
    {
        m_entries.clear();
        // A drag in flight refers to an index that is about to change
        // meaning, so abandon it rather than move the wrong thing.
        m_gridView.CancelDrag();
        CloseContextMenu();
    }

    m_gridWidth      = (gridWidth > 0) ? gridWidth : INV_GRID_WIDTH;
    m_gridRows       = (rows > 0) ? rows : 1;
    m_gridRowsToDraw = (rowsToDraw > m_gridRows) ? rowsToDraw : m_gridRows;

    for (int i = 0; i < count; ++i)
        m_entries.push_back(entries[i]);
}

const InvEntryView* CInventoryPanel::GetEntry(int index) const
{
    if (index < 0 || index >= (int)m_entries.size()) return nullptr;
    return &m_entries[index];
}

// =====================================================================
// Ammo readout
//
// Ammo is not in the Grid (ADR-0001) but is still worth seeing, so it is
// listed in the left column. Rebuilt each paint from the carried weapons.
// =====================================================================
void CInventoryPanel::RebuildAmmoReadout()
{
    m_ammoReadout.clear();

    bool seen[MAX_AMMO_TYPES] = {};

    auto consider = [&](int ammoType, HSPRITE hSpr, const Rect& rc, int iMax)
    {
        if (ammoType < 0 || ammoType >= MAX_AMMO_TYPES) return;
        if (seen[ammoType]) return;
        if (!hSpr || (rc.right - rc.left) <= 0) return;

        seen[ammoType] = true;
        m_ammoReadout.push_back({ ammoType, hSpr, rc, iMax });
    };

    for (const InvEntryView& e : m_entries)
    {
        if (!e.IsWeapon()) continue;

        WEAPON* w = gWR.GetWeapon(e.id);
        if (!w || w->iId == 0) continue;

        consider(w->iAmmoType, w->hAmmo, w->rcAmmo, w->iMax1);
        consider(w->iAmmo2Type, w->hAmmo2, w->rcAmmo2, w->iMax2);
    }
}

// =====================================================================
// paint / paintBackground
// =====================================================================
void CInventoryPanel::paint() {}

void CInventoryPanel::paintBackground()
{
    Panel::paintBackground();
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

    // Text drawn between sprite passes gets overwritten by the sprites that
    // follow it -- a known quirk of this VGUI draw path. Everything textual is
    // collected here and flushed at the very end, after all sprite work.
    struct DeferredText
    {
        int  x, y;
        char text[48];
        int  len;
    };
    std::vector<DeferredText> deferredText;

    // ----------------------------------------------------------------
    // LEFT COLUMN
    // ----------------------------------------------------------------
    {
        int colX = INV_MARGIN;
        int colY = INV_MARGIN + INV_HEADER_H + INV_MARGIN;
        int colW = INV_LEFT_COL_W;
        int colH = panelH - colY - INV_MARGIN;

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

        // --- Panel 2: AMMO RESERVES ---
        int p2Y = colY + panelH1 + INV_SECTION_GAP;

        drawSetColor(15, 15, 15, 60);
        drawFilledRect(colX, p2Y, colX + colW, p2Y + panelH2);
        drawSetColor(100, 200, 255, 80);
        drawOutlinedRect(colX, p2Y, colX + colW, p2Y + panelH2);
        drawSetColor(100, 200, 255, 0);
        drawFilledRect(colX, p2Y, colX + colW, p2Y + 2);

        RebuildAmmoReadout();

        {
            // Row height follows the sprite rather than a fixed guess: HUD ammo
            // icons vary a lot in height, and a fixed row lets the tall ones
            // bleed into the row below. The icon column is a fixed width so the
            // counts line up in a column regardless of icon width.
            static constexpr int AMMO_ICON_COL_W = 34;
            static constexpr int AMMO_ROW_PAD    = 6;
            static constexpr int AMMO_TEXT_H     = 10;

            const int listBottom = p2Y + panelH2 - 4;
            int rowY = p2Y + 8;

            for (const AmmoReadoutEntry& ae : m_ammoReadout)
            {
                const int sprW = ae.rc.right - ae.rc.left;
                const int sprH = ae.rc.bottom - ae.rc.top;

                const int contentH = std::max(sprH, AMMO_TEXT_H);
                const int rowH = contentH + AMMO_ROW_PAD;

                if (rowY + rowH > listBottom)
                    break;

                if (ae.hSpr && sprW > 0 && sprH > 0)
                {
                    // Left-aligned inside the icon column, clipped by the column
                    // rather than allowed to run under the text.
                    const int iconX = colX + 8 + std::max(0, (AMMO_ICON_COL_W - sprW) / 2);
                    SPR_Set(ae.hSpr, 120, 200, 255);
                    SPR_DrawAdditive(0, iconX, rowY + (contentH - sprH) / 2, &ae.rc);
                }

                if (m_pSmallFont)
                {
                    DeferredText label{};
                    snprintf(label.text, sizeof(label.text), "%d / %d",
                        gWR.CountAmmo(ae.ammoType), ae.iMax);
                    label.len = (int)strlen(label.text);
                    label.x = colX + 8 + AMMO_ICON_COL_W + 8;
                    label.y = rowY + (contentH - AMMO_TEXT_H) / 2;
                    deferredText.push_back(label);
                }

                rowY += rowH;
            }
        }
    }

    // ----------------------------------------------------------------
    // RIGHT COLUMN - delegate to the active view
    // ----------------------------------------------------------------
    int x0, y0;
    GetGridOrigin(x0, y0);
    int areaW = panelW - x0 - INV_MARGIN;
    int areaH = panelH - y0 - INV_MARGIN;
    drawSetTextPos(0, 0);

    if (m_eActiveTab == EInventoryTab::Inventory)
    {
        m_gridView.Paint(this, x0, y0, areaW, areaH,
            m_entries, m_gridWidth, m_gridRows, m_gridRowsToDraw);
    }
    else
    {
        m_skillTreeView.Paint(this, x0, y0, areaW, areaH,
            m_pSmallFont, m_pTitleFont);
    }

    // Flush the ammo readout text. Collected earlier, drawn here so the grid's
    // sprite pass above cannot overwrite it.
    if (m_pSmallFont && !deferredText.empty())
    {
        drawSetTextFont(m_pSmallFont);
        drawSetTextColor(200, 230, 255, 0);
        for (const DeferredText& label : deferredText)
        {
            drawSetTextPos(label.x, label.y);
            drawPrintText(label.text, label.len);
        }
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

            const int index = m_gridView.EntryAt(localx, localy);
            const InvEntryView* e = GetEntry(index);
            if (e && m_pContextMenu)
            {
                int panelW = 0, panelH = 0; getSize(panelW, panelH);
                int menuX = std::min(localx, panelW - CTX_MENU_WIDTH);
                int menuY = std::min(localy, panelH - CTX_MENU_MAX_HEIGHT);
                m_pContextMenu->Show(menuX, menuY, index, e->Kind(), e->id, e->count);
            }
        }
        return;
    }

    if (code != MOUSE_LEFT)
        return;

    // ---- Context menu takes the click if it is open over that spot ----
    if (m_pContextMenu && m_pContextMenu->isVisible())
    {
        if (m_pContextMenu->HandleClick(localx, localy))
            return;
        CloseContextMenu();
    }

    // ---- Nav buttons ----
    for (int nb = 0; nb < k_NumNavBtns; ++nb)
    {
        const IRect& r = m_navBtnRects[nb];
        if (r.w <= 0 || r.h <= 0) continue;
        if (localx >= r.x && localx < r.x + r.w && localy >= r.y && localy < r.y + r.h)
        {
            m_eActiveTab = (nb == 0) ? EInventoryTab::Inventory : EInventoryTab::Upgrades;
            m_gridView.CancelDrag();
            m_skillTreeView.HandleMouseMove(-1, -1);
            m_skillTreeView.CancelResetConfirm();
            return;
        }
    }

    // ---- Close button ----
    if (m_pCloseButton)
    {
        int bx, by, bw, bh;
        m_pCloseButton->getBounds(bx, by, bw, bh);
        if (localx >= bx && localx < bx + bw && localy >= by && localy < by + bh)
        {
            Close();
            return;
        }
    }

    // ---- Active view ----
    if (m_eActiveTab == EInventoryTab::Inventory)
        m_gridView.HandleMousePress(this, localx, localy, m_entries);
    else
        m_skillTreeView.HandleMousePress(this, localx, localy);
}

void CInventoryPanel::mouseReleased(vgui::MouseCode code, vgui::Panel* panel)
{
    if (code != MOUSE_LEFT) return;

    int cx = 0, cy = 0; getApp()->getCursorPos(cx, cy);
    int ax = 0, ay = 0, bx2 = 0, by2 = 0; getAbsExtents(ax, ay, bx2, by2);
    int localx = cx - ax, localy = cy - ay;

    if (m_eActiveTab == EInventoryTab::Inventory)
        m_gridView.HandleMouseRelease(this, localx, localy, m_entries);
}

void CInventoryPanel::cursorMoved(int x, int y, vgui::Panel* panel)
{
    // Deliberately ignores the x/y handed in: VGUI delivers those local to the
    // signalling panel, while mousePressed/mouseReleased derive their own from
    // the cursor. Mixing the two conventions offset the dragged Entry by the
    // panel's origin. All three handlers now measure the same way.
    int cx = 0, cy = 0; getApp()->getCursorPos(cx, cy);
    int ax = 0, ay = 0, bx2 = 0, by2 = 0; getAbsExtents(ax, ay, bx2, by2);
    int localx = cx - ax, localy = cy - ay;

    if (m_eActiveTab == EInventoryTab::Inventory)
        m_gridView.HandleMouseMove(localx, localy);
    else
        m_skillTreeView.HandleMouseMove(localx, localy);
}
