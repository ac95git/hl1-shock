#pragma once

#include <vector>
#include <string>
#include "ammohistory.h" // WEAPON

struct InventoryItemEntry;
struct AmmoGridEntry;
class CInventoryPanel;
class CInventoryContextMenu;

// =====================================================================
// CInventoryGridView
//   Plain C++ helper — not a VGUI panel.
//   CInventoryPanel owns one of these and delegates all grid
//   painting and input to it.  Draw calls are forwarded back
//   through the owner pointer (friend relationship).
// =====================================================================
class CInventoryGridView
{
public:
    CInventoryGridView();

    // Called by CInventoryPanel::Open() after weapon list is rebuilt
    void SetWeaponData(
        const std::vector<const char*>& weaponNames,
        const std::vector<WEAPON*>& weaponList,
        std::vector<int>& weaponOffsetX,
        std::vector<int>& weaponOffsetY);

    // Paint the full grid area (weapons, items, ammo).
    // ctx   – the owning panel (draw calls are protected members of Panel)
    // x0/y0 – top-left of the grid area
    // areaW/areaH – available pixel size
    void Paint(CInventoryPanel* ctx,
               int x0, int y0, int areaW, int areaH,
               const std::vector<WEAPON*>& weaponList,
               const std::vector<const char*>& weaponNames,
               std::vector<int>& weaponOffsetX,
               std::vector<int>& weaponOffsetY,
               std::vector<InventoryItemEntry>& inventoryItems,
               std::vector<int>& invOffsetX,
               std::vector<int>& invOffsetY,
               std::vector<AmmoGridEntry>& ammoEntries);

    // Input – return true if the event was consumed
    bool HandleMousePress  (CInventoryPanel* ctx, int localX, int localY,
                            std::vector<const char*>& weaponNames,
                            std::vector<WEAPON*>& weaponList,
                            std::vector<int>& weaponOffsetX,
                            std::vector<int>& weaponOffsetY,
                            std::vector<InventoryItemEntry>& inventoryItems,
                            std::vector<int>& invOffsetX,
                            std::vector<int>& invOffsetY);

    bool HandleMouseRelease(CInventoryPanel* ctx, int localX, int localY,
                            std::vector<const char*>& weaponNames,
                            std::vector<WEAPON*>& weaponList,
                            std::vector<int>& weaponOffsetX,
                            std::vector<int>& weaponOffsetY,
                            std::vector<InventoryItemEntry>& inventoryItems,
                            std::vector<int>& invOffsetX,
                            std::vector<int>& invOffsetY);

    void HandleMouseMove   (CInventoryPanel* ctx,
                            std::vector<int>& weaponOffsetX,
                            std::vector<int>& weaponOffsetY,
                            const std::vector<WEAPON*>& weaponList,
                            std::vector<int>& invOffsetX,
                            std::vector<int>& invOffsetY);

    // Hit-rect list, populated every Paint() call.
    // Entry i maps to weaponList[i] for i < weaponList.size(),
    // else inventoryItems[i - weaponList.size()].
    struct IRect { int x, y, w, h; };
    const std::vector<IRect>& GetItemRects() const { return m_itemRects; }

    // Drag state queries (used by context-menu handler)
    bool IsDragging()      const { return m_bDragging; }
    int  DraggedIndex()    const { return m_draggedIndex; }

    void CancelDrag();

private:
    // Layout helpers (mirrors the statics in vgui_inventory.cpp)
    int  SlotCellWidth (int slotIdx, const std::vector<WEAPON*>& wl,
                        const std::vector<InventoryItemEntry>& items) const;
    int  SlotNaturalCell(int slotIdx, const std::vector<WEAPON*>& wl,
                         const std::vector<InventoryItemEntry>& items) const;

    std::vector<IRect> m_itemRects;

    bool m_bDragging    = false;
    int  m_draggedIndex = -1;
    int  m_dragStartX   = 0, m_dragStartY  = 0;
    int  m_origOffsetX  = 0, m_origOffsetY = 0;

    // Cached grid geometry (set at the start of each Paint call)
    int m_x0 = 0, m_y0 = 0;
    int m_cellSize  = 1;
    int m_cellStepX = 1, m_cellStepY = 1;
};
