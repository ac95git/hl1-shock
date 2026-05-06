#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_inventory_grid.h"
#include "ammohistory.h"
#include <VGUI_App.h>
#include <VGUI_Cursor.h>
#include <algorithm>
#include <cstring>

using namespace vgui;

// =====================================================================
// Layout constants (must match the ones in vgui_inventory.cpp)
// =====================================================================
static constexpr int INV_GRID_COLS    = 11;
static constexpr int INV_GRID_ROWS    = 8;
static constexpr int INV_GRID_PADDING = 4;
static constexpr int INV_MARGIN       = 8;
static constexpr int k_weaponCellWidth = 3;

static int GetItemCellWidthGrid(const std::string& classname)
{
    // 1-cell items
    static const char* k_oneCellNames[] = {
        "item_healthkit", "item_battery", "item_antidote", "item_security"
    };
    for (auto n : k_oneCellNames)
        if (classname == n) return 1;
    return 1;
}

// =====================================================================
// CInventoryGridView
// =====================================================================
CInventoryGridView::CInventoryGridView()
    : m_bDragging(false), m_draggedIndex(-1)
{}

int CInventoryGridView::SlotCellWidth(int slotIdx,
    const std::vector<WEAPON*>& wl,
    const std::vector<InventoryItemEntry>& items) const
{
    if (slotIdx < (int)wl.size()) return k_weaponCellWidth;
    int afterWeapons = slotIdx - (int)wl.size();
    if (afterWeapons < (int)items.size())
        return GetItemCellWidthGrid(items[afterWeapons].classname);
    return 1;
}

int CInventoryGridView::SlotNaturalCell(int slotIdx,
    const std::vector<WEAPON*>& wl,
    const std::vector<InventoryItemEntry>& items) const
{
    int cell = 0;
    for (int i = 0; i < slotIdx; ++i)
        cell += SlotCellWidth(i, wl, items);
    return cell;
}

void CInventoryGridView::CancelDrag()
{
    m_bDragging    = false;
    m_draggedIndex = -1;
}

// =====================================================================
// Paint
// =====================================================================
void CInventoryGridView::Paint(
    CInventoryPanel* ctx,
    int x0, int y0, int areaW, int areaH,
    const std::vector<WEAPON*>& weaponList,
    const std::vector<const char*>& weaponNames,
    std::vector<int>& weaponOffsetX,
    std::vector<int>& weaponOffsetY,
    std::vector<InventoryItemEntry>& inventoryItems,
    std::vector<int>& invOffsetX,
    std::vector<int>& invOffsetY,
    std::vector<AmmoGridEntry>& ammoEntries)
{
    m_itemRects.clear();

    // Compute cell size from available area
    int cellSize = std::min(
        (areaW - (INV_GRID_COLS - 1) * INV_GRID_PADDING) / INV_GRID_COLS,
        (areaH - (INV_GRID_ROWS - 1) * INV_GRID_PADDING) / INV_GRID_ROWS);
    if (cellSize < 1) cellSize = 1;
    int cellStepX = cellSize + INV_GRID_PADDING;
    int cellStepY = cellSize + INV_GRID_PADDING;
    int gridW = INV_GRID_COLS * cellSize + (INV_GRID_COLS - 1) * INV_GRID_PADDING;
    int gridH = INV_GRID_ROWS * cellSize + (INV_GRID_ROWS - 1) * INV_GRID_PADDING;

    // Cache for drag helpers
    m_x0 = x0; m_y0 = y0;
    m_cellSize = cellSize;
    m_cellStepX = cellStepX; m_cellStepY = cellStepY;

    // ---- Grid background ----
    ctx->drawSetColor(10, 10, 10, 60);
    ctx->drawFilledRect(x0, y0, x0 + gridW, y0 + gridH);

    // Grid lines
    ctx->drawSetColor(200, 200, 200, 160);
    for (int c = 0; c <= INV_GRID_COLS; ++c)
    {
        int vx = (c < INV_GRID_COLS) ? x0 + c * cellStepX : x0 + gridW;
        ctx->drawFilledRect(vx, y0, vx + 1, y0 + gridH);
    }
    for (int r = 0; r <= INV_GRID_ROWS; ++r)
    {
        int hy = (r < INV_GRID_ROWS) ? y0 + r * cellStepY : y0 + gridH;
        ctx->drawFilledRect(x0, hy, x0 + gridW, hy + 1);
    }

    // Outer border
    ctx->drawSetColor(255, 170, 0, 60);
    ctx->drawOutlinedRect(x0, y0, x0 + gridW, y0 + gridH);

    int rr, gg, bb;

    // ---- WEAPONS ----
    for (int idx = 0; idx < (int)weaponList.size(); ++idx)
    {
        WEAPON* p = weaponList[idx];
        int cw  = SlotCellWidth(idx, weaponList, inventoryItems);
        int nat = SlotNaturalCell(idx, weaponList, inventoryItems);
        if (nat + cw > INV_GRID_COLS * INV_GRID_ROWS) break;

        int col   = nat % INV_GRID_COLS, row = nat / INV_GRID_COLS;
        int baseX = x0 + col * cellStepX, baseY = y0 + row * cellStepY;
        int ox    = (idx < (int)weaponOffsetX.size()) ? weaponOffsetX[idx] : 0;
        int oy    = (idx < (int)weaponOffsetY.size()) ? weaponOffsetY[idx] : 0;
        int wx    = baseX + ox, wy = baseY + oy;
        int slotW = cw * cellSize + (cw - 1) * INV_GRID_PADDING;
        int slotH = cellSize;

        HSPRITE hspr = 0; Rect rc = {};
        if (p)
        {
            if (p->hActive && (p->rcActive.right - p->rcActive.left) > 0) { hspr = p->hActive; rc = p->rcActive; }
            else { hspr = p->hInactive; rc = p->rcInactive; }
        }
        int sprW = rc.right - rc.left, sprH = rc.bottom - rc.top;

        if (m_bDragging && idx == m_draggedIndex)
        {
            IRect rect; rect.x = wx; rect.y = wy; rect.w = slotW; rect.h = slotH;
            m_itemRects.push_back(rect);
            continue;
        }

        if (!gWR.HasAmmo(p)) { UnpackRGB(rr, gg, bb, RGB_REDISH);    ScaleColors(rr, gg, bb, 128); }
        else                 { UnpackRGB(rr, gg, bb, RGB_YELLOWISH); ScaleColors(rr, gg, bb, 192); }

        ctx->drawSetColor(30, 30, 30, 80);
        ctx->drawFilledRect(wx, wy, wx + slotW, wy + slotH);
        ctx->drawSetColor(255, 170, 0, 80);
        ctx->drawOutlinedRect(wx, wy, wx + slotW, wy + slotH);

        if (p && hspr && sprW > 0 && sprH > 0)
        {
            SPR_Set(hspr, rr, gg, bb);
            SPR_DrawAdditive(0, wx + (slotW - sprW) / 2, wy + (slotH - sprH) / 2, &rc);
        }

        IRect rect; rect.x = wx; rect.y = wy; rect.w = slotW; rect.h = slotH;
        m_itemRects.push_back(rect);
    }

    // ---- INVENTORY ITEMS ----
    {
        while ((int)invOffsetX.size() < (int)inventoryItems.size())
        { invOffsetX.push_back(0); invOffsetY.push_back(0); }

        for (int ii = 0; ii < (int)inventoryItems.size(); ++ii)
        {
            const InventoryItemEntry& entry = inventoryItems[ii];
            if (entry.count <= 0) continue;

            int slotIdx = (int)weaponList.size() + ii;
            int cw  = SlotCellWidth(slotIdx, weaponList, inventoryItems);
            int nat = SlotNaturalCell(slotIdx, weaponList, inventoryItems);
            if (nat >= INV_GRID_COLS * INV_GRID_ROWS) break;

            int col = nat % INV_GRID_COLS, row = nat / INV_GRID_COLS;
            int bX  = x0 + col * cellStepX, bY = y0 + row * cellStepY;
            int ox  = invOffsetX[ii], oy2 = invOffsetY[ii];
            int slotW = cw * cellSize + (cw - 1) * INV_GRID_PADDING;
            int slotH = cellSize;
            int cx2 = bX + ox + 2, cy2 = bY + oy2 + 2;
            int icw = slotW - 4, ich = slotH - 4;

            if (m_bDragging && m_draggedIndex == slotIdx)
            {
                IRect rect; rect.x = cx2; rect.y = cy2; rect.w = icw; rect.h = ich;
                m_itemRects.push_back(rect);
                continue;
            }

            int ir, ig, ib;
            switch (entry.type)
            {
            case EInventoryItemType::Medkit:  ir = 50;  ig = 200; ib = 80;  break;
            case EInventoryItemType::Battery: ir = 80;  ig = 160; ib = 255; break;
            default:                          ir = 180; ig = 180; ib = 180; break;
            }
            FillRGBA(cx2, cy2, icw, ich, ir, ig, ib, 100);
            ctx->drawSetColor(ir, ig, ib, 60);
            ctx->drawOutlinedRect(cx2, cy2, cx2 + icw, cy2 + ich);

            if (entry.hSprite && (entry.rc.right - entry.rc.left) > 0 && (entry.rc.bottom - entry.rc.top) > 0)
            {
                int sprW2 = entry.rc.right - entry.rc.left, sprH2 = entry.rc.bottom - entry.rc.top;
                SPR_Set(entry.hSprite, ir, ig, ib);
                SPR_DrawAdditive(0, cx2 + (icw - sprW2) / 2, cy2 + (ich - sprH2) / 2, &entry.rc);
            }

            if (entry.count > 0 && ctx->m_pSmallFont)
            {
                vgui::Font* smallFont = ctx->m_pSmallFont;
                char countStr[8]; snprintf(countStr, sizeof(countStr), "x%d", entry.count);
                int textLen = (int)strlen(countStr);
                int charW = 6, charH = 10;
                int textW = textLen * charW;
                FillRGBA(cx2 + icw - textW - 3, cy2 + ich - charH - 2, textW + 2, charH + 1, 0, 0, 0, 180);
                ctx->drawSetTextFont(smallFont);
                ctx->drawSetTextColor(255, 220, 50, 255);
                ctx->drawSetTextPos(cx2 + icw - textW - 2, cy2 + ich - charH - 1);
                ctx->drawPrintText(countStr, textLen);
            }

            IRect rect; rect.x = cx2; rect.y = cy2; rect.w = icw; rect.h = ich;
            m_itemRects.push_back(rect);
        }
    }

    // ---- AMMO ----
    {
        ammoEntries.clear();
        bool seen[MAX_AMMO_TYPES] = {};
        for (WEAPON* w : weaponList)
        {
            if (!w || w->iId == 0) continue;
            if (w->iAmmoType >= 0 && w->iAmmoType < MAX_AMMO_TYPES && !seen[w->iAmmoType])
                if (w->hAmmo && (w->rcAmmo.right - w->rcAmmo.left) > 0)
                { seen[w->iAmmoType] = true; ammoEntries.push_back({ w->iAmmoType, w->hAmmo, w->rcAmmo, w->iMax1 }); }
            if (w->iAmmo2Type >= 0 && w->iAmmo2Type < MAX_AMMO_TYPES && !seen[w->iAmmo2Type])
                if (w->hAmmo2 && (w->rcAmmo2.right - w->rcAmmo2.left) > 0)
                { seen[w->iAmmo2Type] = true; ammoEntries.push_back({ w->iAmmo2Type, w->hAmmo2, w->rcAmmo2, w->iMax2 }); }
        }

        int ammoStartSlot = (int)weaponList.size() + (int)inventoryItems.size();
        vgui::Font* smallFont = ctx->m_pSmallFont;

        for (int ai = 0; ai < (int)ammoEntries.size(); ++ai)
        {
            const AmmoGridEntry& ae = ammoEntries[ai];
            int nat = SlotNaturalCell(ammoStartSlot + ai, weaponList, inventoryItems);
            if (nat >= INV_GRID_COLS * INV_GRID_ROWS) break;

            int col = nat % INV_GRID_COLS, row = nat / INV_GRID_COLS;
            int bX = x0 + col * cellStepX, bY = y0 + row * cellStepY;
            int cx2 = bX + 2, cy2 = bY + 2;
            int icw = cellSize - 4, ich = cellSize - 4;

            int count = gWR.CountAmmo(ae.ammoType);
            if (count > 0) { UnpackRGB(rr, gg, bb, RGB_YELLOWISH); ScaleColors(rr, gg, bb, 200); }
            else           { UnpackRGB(rr, gg, bb, RGB_REDISH);    ScaleColors(rr, gg, bb, 140); }

            FillRGBA(cx2, cy2, icw, ich, rr / 4, gg / 4, bb / 4, 140);
            ctx->drawSetColor(rr, gg, bb, 60);
            ctx->drawOutlinedRect(cx2, cy2, cx2 + icw, cy2 + ich);

            if (ae.hSpr && (ae.rc.right - ae.rc.left) > 0)
            {
                int sprW2 = ae.rc.right - ae.rc.left, sprH2 = ae.rc.bottom - ae.rc.top;
                int barH = 5, sprAreaH = ich - barH - 3;
                SPR_Set(ae.hSpr, rr, gg, bb);
                SPR_DrawAdditive(0, cx2 + (icw - sprW2) / 2, cy2 + (sprAreaH - sprH2) / 2, &ae.rc);

                int barX = cx2 + 2, barY = cy2 + ich - barH - 1, barW = icw - 4;
                int dr, dg, db; UnpackRGB(dr, dg, db, RGB_YELLOWISH);
                FillRGBA(barX, barY, barW, barH, dr, dg, db, 50);
                if (ae.iMax > 0 && count > 0)
                {
                    float frac = std::min(1.0f, (float)count / (float)ae.iMax);
                    int fw = std::max(1, (int)(barW * frac));
                    int fr, fg, fb;
                    if (frac > 0.5f)       UnpackRGB(fr, fg, fb, RGB_GREENISH);
                    else if (frac > 0.25f) UnpackRGB(fr, fg, fb, RGB_YELLOWISH);
                    else                   UnpackRGB(fr, fg, fb, RGB_REDISH);
                    FillRGBA(barX, barY, fw, barH, fr, fg, fb, 220);
                }
            }

            if (smallFont)
            {
                char countStr[8]; snprintf(countStr, sizeof(countStr), "%d", count);
                int textLen = (int)strlen(countStr);
                int charW = 6, charH = 10;
                int textW = textLen * charW;
                FillRGBA(cx2 + icw - textW - 3, cy2 + 2, textW + 2, charH + 1, 0, 0, 0, 180);
                ctx->drawSetTextFont(smallFont);
                ctx->drawSetTextColor(rr, gg, bb, 255);
                ctx->drawSetTextPos(cx2 + icw - textW - 2, cy2 + 3);
                ctx->drawPrintText(countStr, textLen);
            }
        }
    }

    // ---- Draw dragged item on top ----
    if (m_bDragging && m_draggedIndex >= 0)
    {
        int curX = 0, curY = 0;
        App::getInstance()->getCursorPos(curX, curY);
        int dx = curX - m_dragStartX, dy = curY - m_dragStartY;

        bool isDraggingWeapon = (m_draggedIndex < (int)weaponList.size());
        bool isDraggingInv    = !isDraggingWeapon &&
                                (m_draggedIndex < (int)weaponList.size() + (int)inventoryItems.size());

        int dragCw   = SlotCellWidth(m_draggedIndex, weaponList, inventoryItems);
        int dragNat  = SlotNaturalCell(m_draggedIndex, weaponList, inventoryItems);
        int dragCol  = dragNat % INV_GRID_COLS, dragRow = dragNat / INV_GRID_COLS;
        int dragSlotW = dragCw * cellSize + (dragCw - 1) * INV_GRID_PADDING;
        int dragSlotH = cellSize;

        if (isDraggingWeapon)
        {
            size_t idx = (size_t)m_draggedIndex;
            if (idx < weaponOffsetX.size())
            { weaponOffsetX[idx] = m_origOffsetX + dx; weaponOffsetY[idx] = m_origOffsetY + dy; }
            WEAPON* p = weaponList[idx];
            if (p)
            {
                Rect rc = (p->hActive && (p->rcActive.right - p->rcActive.left) > 0) ? p->rcActive : p->rcInactive;
                HSPRITE h = (p->hActive && (p->rcActive.right - p->rcActive.left) > 0) ? p->hActive : p->hInactive;
                int bX2 = x0 + dragCol * cellStepX, bY2 = y0 + dragRow * cellStepY;
                int wx2 = bX2 + weaponOffsetX[idx], wy2 = bY2 + weaponOffsetY[idx];
                if (!gWR.HasAmmo(p)) { UnpackRGB(rr, gg, bb, RGB_REDISH); ScaleColors(rr, gg, bb, 128); }
                else                 { UnpackRGB(rr, gg, bb, RGB_YELLOWISH); ScaleColors(rr, gg, bb, 192); }
                ctx->drawSetColor(255, 255, 255, 180);
                ctx->drawOutlinedRect(wx2 - 2, wy2 - 2, wx2 + dragSlotW + 2, wy2 + dragSlotH + 2);
                const int outlinesz = 2;
                for (int oy2 = -outlinesz; oy2 <= outlinesz; ++oy2)
                    for (int ox2 = -outlinesz; ox2 <= outlinesz; ++ox2)
                        if (ox2 || oy2) SPR_DrawAdditive(0, wx2 + ox2, wy2 + oy2, &rc);
                SPR_Set(h, rr, gg, bb);
                SPR_DrawAdditive(0, wx2, wy2, &rc);
            }
        }
        else if (isDraggingInv)
        {
            int ii = m_draggedIndex - (int)weaponList.size();
            if (ii >= 0 && ii < (int)invOffsetX.size())
            { invOffsetX[ii] = m_origOffsetX + dx; invOffsetY[ii] = m_origOffsetY + dy; }

            const InventoryItemEntry& entry = inventoryItems[ii];
            int bX2 = x0 + dragCol * cellStepX, bY2 = y0 + dragRow * cellStepY;
            int ox2 = invOffsetX[ii], oy2 = invOffsetY[ii];
            int cx2 = bX2 + ox2 + 2, cy2 = bY2 + oy2 + 2;
            int icw = dragSlotW - 4, ich = dragSlotH - 4;
            int ir, ig, ib;
            switch (entry.type)
            {
            case EInventoryItemType::Medkit:  ir = 50;  ig = 200; ib = 80;  break;
            case EInventoryItemType::Battery: ir = 80;  ig = 160; ib = 255; break;
            default:                          ir = 180; ig = 180; ib = 180; break;
            }
            ctx->drawSetColor(255, 255, 255, 220);
            ctx->drawOutlinedRect(cx2 - 2, cy2 - 2, cx2 + icw + 2, cy2 + ich + 2);
            FillRGBA(cx2, cy2, icw, ich, ir, ig, ib, 140);
            ctx->drawSetColor(ir, ig, ib, 220);
            ctx->drawOutlinedRect(cx2, cy2, cx2 + icw, cy2 + ich);
            if (entry.hSprite && (entry.rc.right - entry.rc.left) > 0)
            {
                int sprW2 = entry.rc.right - entry.rc.left, sprH2 = entry.rc.bottom - entry.rc.top;
                SPR_Set(entry.hSprite, ir, ig, ib);
                SPR_DrawAdditive(0, cx2 + (icw - sprW2) / 2, cy2 + (ich - sprH2) / 2, &entry.rc);
            }
        }
    }
}

// =====================================================================
// HandleMousePress
// =====================================================================
bool CInventoryGridView::HandleMousePress(
    CInventoryPanel* ctx, int localX, int localY,
    std::vector<const char*>& weaponNames,
    std::vector<WEAPON*>& weaponList,
    std::vector<int>& weaponOffsetX,
    std::vector<int>& weaponOffsetY,
    std::vector<InventoryItemEntry>& inventoryItems,
    std::vector<int>& invOffsetX,
    std::vector<int>& invOffsetY)
{
    for (size_t i = 0; i < m_itemRects.size(); ++i)
    {
        const IRect& r = m_itemRects[i];
        if (localX < r.x || localX >= r.x + r.w || localY < r.y || localY >= r.y + r.h)
            continue;

        if (i < weaponNames.size())
        {
            const char* weapon = weaponNames[i];
            if (!weapon || weapon[0] == '\0') return true;
            m_bDragging = true; m_draggedIndex = (int)i;
            App::getInstance()->getCursorPos(m_dragStartX, m_dragStartY);
            m_origOffsetX = (i < weaponOffsetX.size()) ? weaponOffsetX[i] : 0;
            m_origOffsetY = (i < weaponOffsetY.size()) ? weaponOffsetY[i] : 0;
        }
        else
        {
            int ii = (int)i - (int)weaponList.size();
            if (ii >= 0 && ii < (int)inventoryItems.size())
            {
                m_bDragging = true; m_draggedIndex = (int)i;
                App::getInstance()->getCursorPos(m_dragStartX, m_dragStartY);
                m_origOffsetX = (ii < (int)invOffsetX.size()) ? invOffsetX[ii] : 0;
                m_origOffsetY = (ii < (int)invOffsetY.size()) ? invOffsetY[ii] : 0;
            }
        }

        vgui::Cursor* hand = App::getInstance()->getScheme()->getCursor(Scheme::scu_hand);
        App::getInstance()->setCursorOveride(hand ? hand : App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow));
        return true;
    }
    return false;
}

// =====================================================================
// HandleMouseRelease
// =====================================================================
bool CInventoryGridView::HandleMouseRelease(
    CInventoryPanel* ctx, int /*localX*/, int /*localY*/,
    std::vector<const char*>& weaponNames,
    std::vector<WEAPON*>& weaponList,
    std::vector<int>& weaponOffsetX,
    std::vector<int>& weaponOffsetY,
    std::vector<InventoryItemEntry>& inventoryItems,
    std::vector<int>& invOffsetX,
    std::vector<int>& invOffsetY)
{
    if (!m_bDragging || m_draggedIndex < 0) return false;

    int cx2, cy2; App::getInstance()->getCursorPos(cx2, cy2);
    int dx = cx2 - m_dragStartX, dy = cy2 - m_dragStartY;
    const int clickThreshold = 6;
    bool isDraggingWeapon = (m_draggedIndex < (int)weaponList.size());

    if (abs(dx) <= clickThreshold && abs(dy) <= clickThreshold)
    {
        // Click: equip weapon
        if (isDraggingWeapon && m_draggedIndex < (int)weaponNames.size())
        {
            const char* weapon = weaponNames[m_draggedIndex];
            if (weapon && weapon[0] != '\0')
            {
                char cmd[128]; snprintf(cmd, sizeof(cmd), "use %s\n", weapon);
                gEngfuncs.pfnClientCmd(cmd);
            }
        }
    }
    else
    {
        // Drag: snap to nearest cell
        int x0 = m_x0, y0 = m_y0;
        int cellSize  = m_cellSize;
        int cellStepX = m_cellStepX, cellStepY = m_cellStepY;

        int dragNatural = SlotNaturalCell(m_draggedIndex, weaponList, inventoryItems);
        int origCol = dragNatural % INV_GRID_COLS, origRow = dragNatural / INV_GRID_COLS;
        int origBaseX = x0 + origCol * cellStepX, origBaseY = y0 + origRow * cellStepY;

        int currentOx = isDraggingWeapon
            ? (m_draggedIndex < (int)weaponOffsetX.size() ? weaponOffsetX[m_draggedIndex] : 0)
            : (m_draggedIndex - (int)weaponList.size() < (int)invOffsetX.size() ? invOffsetX[m_draggedIndex - (int)weaponList.size()] : 0);
        int currentOy = isDraggingWeapon
            ? (m_draggedIndex < (int)weaponOffsetY.size() ? weaponOffsetY[m_draggedIndex] : 0)
            : (m_draggedIndex - (int)weaponList.size() < (int)invOffsetY.size() ? invOffsetY[m_draggedIndex - (int)weaponList.size()] : 0);

        int curCenterX = origBaseX + currentOx + cellStepX / 2;
        int curCenterY = origBaseY + currentOy + cellStepY / 2;
        int snapCol = std::max(0, std::min(INV_GRID_COLS - 1, (curCenterX - x0) / cellStepX));
        int snapRow = std::max(0, std::min(INV_GRID_ROWS - 1, (curCenterY - y0) / cellStepY));

        int dragCw = SlotCellWidth(m_draggedIndex, weaponList, inventoryItems);
        if (dragCw > 1 && snapCol + dragCw > INV_GRID_COLS)
            snapCol = INV_GRID_COLS - dragCw;
        int snapCell = snapRow * INV_GRID_COLS + snapCol;

        // Helper lambdas
        auto getSlotCell = [&](int slotIdx) -> int
        {
            int nat2 = SlotNaturalCell(slotIdx, weaponList, inventoryItems);
            int sc = nat2 % INV_GRID_COLS, sr = nat2 / INV_GRID_COLS;
            int sbX = x0 + sc * cellStepX, sbY = y0 + sr * cellStepY;
            bool isW2 = (slotIdx < (int)weaponList.size());
            int sOx = isW2
                ? (slotIdx < (int)weaponOffsetX.size() ? weaponOffsetX[slotIdx] : 0)
                : (slotIdx - (int)weaponList.size() < (int)invOffsetX.size() ? invOffsetX[slotIdx - (int)weaponList.size()] : 0);
            int sOy = isW2
                ? (slotIdx < (int)weaponOffsetY.size() ? weaponOffsetY[slotIdx] : 0)
                : (slotIdx - (int)weaponList.size() < (int)invOffsetY.size() ? invOffsetY[slotIdx - (int)weaponList.size()] : 0);
            int iCx = sbX + sOx + cellStepX / 2, iCy = sbY + sOy + cellStepY / 2;
            return std::max(0, std::min(INV_GRID_ROWS - 1, (iCy - y0) / cellStepY)) * INV_GRID_COLS
                 + std::max(0, std::min(INV_GRID_COLS - 1, (iCx - x0) / cellStepX));
        };

        auto setSlotToCell = [&](int slotIdx, int targetCell)
        {
            int nat2 = SlotNaturalCell(slotIdx, weaponList, inventoryItems);
            int sOrigCol = nat2 % INV_GRID_COLS, sOrigRow = nat2 / INV_GRID_COLS;
            int tCol2 = targetCell % INV_GRID_COLS, tRow2 = targetCell / INV_GRID_COLS;
            int newOx = (x0 + tCol2 * cellStepX) - (x0 + sOrigCol * cellStepX);
            int newOy = (y0 + tRow2 * cellStepY) - (y0 + sOrigRow * cellStepY);
            bool isW2 = (slotIdx < (int)weaponList.size());
            if (isW2)
            { if (slotIdx < (int)weaponOffsetX.size()) { weaponOffsetX[slotIdx] = newOx; weaponOffsetY[slotIdx] = newOy; } }
            else
            { int ii2 = slotIdx - (int)weaponList.size(); if (ii2 < (int)invOffsetX.size()) { invOffsetX[ii2] = newOx; invOffsetY[ii2] = newOy; } }
        };

        if (snapCell != dragNatural)
        {
            int totalSlots = (int)weaponList.size() + (int)inventoryItems.size();
            for (int si = 0; si < totalSlots; ++si)
            {
                if (si == m_draggedIndex) continue;
                if (getSlotCell(si) == snapCell) { setSlotToCell(si, dragNatural); break; }
            }
            setSlotToCell(m_draggedIndex, snapCell);
        }
        else
        {
            setSlotToCell(m_draggedIndex, dragNatural);
        }

        // Persist weapon grid positions
        for (int i = 0; i < (int)weaponList.size(); i++)
        {
            WEAPON* w = weaponList[i];
            if (!w || w->iId <= 0) continue;
            gWR.SetGridCell(w->iId, getSlotCell(i));
        }
    }

    m_bDragging = false;
    m_draggedIndex = -1;
    App::getInstance()->setCursorOveride(App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow));
    App::getInstance()->setMouseCapture(nullptr);
    return true;
}

// =====================================================================
// HandleMouseMove
// =====================================================================
void CInventoryGridView::HandleMouseMove(
    CInventoryPanel* ctx,
    std::vector<int>& weaponOffsetX,
    std::vector<int>& weaponOffsetY,
    const std::vector<WEAPON*>& weaponList,
    std::vector<int>& invOffsetX,
    std::vector<int>& invOffsetY)
{
    if (!m_bDragging || m_draggedIndex < 0) return;

    int cx2, cy2; App::getInstance()->getCursorPos(cx2, cy2);
    int dx = cx2 - m_dragStartX, dy = cy2 - m_dragStartY;

    if (m_draggedIndex < (int)weaponOffsetX.size())
    { weaponOffsetX[m_draggedIndex] = m_origOffsetX + dx; weaponOffsetY[m_draggedIndex] = m_origOffsetY + dy; }
    else
    {
        int ii = m_draggedIndex - (int)weaponList.size();
        if (ii >= 0 && ii < (int)invOffsetX.size())
        { invOffsetX[ii] = m_origOffsetX + dx; invOffsetY[ii] = m_origOffsetY + dy; }
    }
    ctx->repaint();
}
