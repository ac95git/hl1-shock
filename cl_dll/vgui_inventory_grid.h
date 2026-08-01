#pragma once

#include <vector>
#include "inventory_defs.h"

struct InvEntryView;
class CInventoryPanel;

// =====================================================================
// CInventoryGridView
//   Plain C++ helper, not a VGUI panel.  CInventoryPanel owns one and
//   delegates Grid painting and input to it; draw calls are forwarded
//   back through the owner pointer (friend relationship).
//
//   This view OWNS NOTHING.  It draws the Entries the server sent and
//   turns drags into inv_move requests -- it never decides where
//   anything sits.  See docs/adr/0004-the-server-owns-the-inventory.md.
// =====================================================================
class CInventoryGridView
{
public:
	CInventoryGridView() = default;

	void Paint(CInventoryPanel* ctx,
		int x0, int y0, int areaW, int areaH,
		const std::vector<InvEntryView>& entries,
		int gridWidth, int rows, int rowsToDraw);

	// Input -- return true if the event was consumed.
	bool HandleMousePress(CInventoryPanel* ctx, int localX, int localY,
		const std::vector<InvEntryView>& entries);
	bool HandleMouseRelease(CInventoryPanel* ctx, int localX, int localY,
		const std::vector<InvEntryView>& entries);
	void HandleMouseMove(int localX, int localY);

	// Hit rectangles, rebuilt every Paint. Index matches the entry list.
	struct IRect { int x, y, w, h; };
	const std::vector<IRect>& GetItemRects() const { return m_itemRects; }

	// Entry index under a point, or -1.
	int EntryAt(int localX, int localY) const;

	bool IsDragging()   const { return m_bDragging; }
	int  DraggedIndex() const { return m_draggedIndex; }
	void CancelDrag();

private:
	// Pixel rect of an Entry sitting at (col,row) and 'cellWidth' wide.
	IRect CellRect(int col, int row, int cellWidth) const;

	std::vector<IRect> m_itemRects;

	bool m_bDragging    = false;
	int  m_draggedIndex = -1;

	// Where inside the Entry the cursor grabbed it, so the Entry does not
	// snap its corner to the pointer when a drag starts.
	int m_dragGrabX = 0, m_dragGrabY = 0;
	int m_mouseX = 0, m_mouseY = 0;

	// Geometry, cached at the start of each Paint.
	int m_x0 = 0, m_y0 = 0;
	int m_cellSize  = 1;
	int m_cellStep  = 1;
	int m_gridWidth = INV_GRID_WIDTH;
	int m_rows = 0;
	int m_rowsToDraw = 0;
};
