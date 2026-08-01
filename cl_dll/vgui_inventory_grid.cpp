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
static constexpr int INV_GRID_PADDING = 4;

// =====================================================================
// Sprite lookup for an Item Type.
//
// The sprite NAME comes from the shared table, so client and server
// cannot disagree about what an item is -- only about how it looks.
// =====================================================================
struct ItemSprite { HSPRITE hSprite = 0; Rect rc = {}; };

static ItemSprite GetItemSprite(int itemTypeId)
{
	ItemSprite out;

	const ItemTypeDef* def = GetItemType(itemTypeId);
	if (!def || !def->spriteName)
		return out;

	int idx = gHUD.GetSpriteIndex(def->spriteName);
	if (idx < 0)
		return out;

	out.hSprite = gHUD.GetSprite(idx);
	out.rc = gHUD.GetSpriteRect(idx);
	return out;
}

// Tint per Item Type, so a medkit still reads as a medkit at a glance.
static void GetItemTint(int itemTypeId, int& r, int& g, int& b)
{
	switch (static_cast<EItemTypeId>(itemTypeId))
	{
	case EItemTypeId::Medkit:  r = 50;  g = 200; b = 80;  break;
	case EItemTypeId::Battery: r = 80;  g = 160; b = 255; break;
	case EItemTypeId::Keycard: r = 230; g = 200; b = 90;  break;
	default:                   r = 180; g = 180; b = 180; break;
	}
}

// =====================================================================
// Geometry
// =====================================================================
CInventoryGridView::IRect CInventoryGridView::CellRect(int col, int row, int cellWidth) const
{
	IRect r;
	r.x = m_x0 + col * m_cellStep;
	r.y = m_y0 + row * m_cellStep;
	r.w = cellWidth * m_cellSize + (cellWidth - 1) * INV_GRID_PADDING;
	r.h = m_cellSize;
	return r;
}

int CInventoryGridView::EntryAt(int localX, int localY) const
{
	for (int i = (int)m_itemRects.size() - 1; i >= 0; --i)
	{
		const IRect& r = m_itemRects[i];
		if (r.w <= 0 || r.h <= 0)
			continue;
		if (localX >= r.x && localX < r.x + r.w &&
			localY >= r.y && localY < r.y + r.h)
			return i;
	}
	return -1;
}

void CInventoryGridView::CancelDrag()
{
	m_bDragging = false;
	m_draggedIndex = -1;
}

// =====================================================================
// Paint
// =====================================================================
void CInventoryGridView::Paint(
	CInventoryPanel* ctx,
	int x0, int y0, int areaW, int areaH,
	const std::vector<InvEntryView>& entries,
	int gridWidth, int rows, int rowsToDraw)
{
	m_itemRects.clear();
	m_itemRects.resize(entries.size());

	if (gridWidth < 1)  gridWidth = INV_GRID_WIDTH;
	if (rowsToDraw < 1) rowsToDraw = 1;
	if (rows < 0)       rows = 0;

	// Cell size is driven by the FULL drawn Grid, including Rows the player
	// has not earned yet, so unlocking a Row never re-flows the layout.
	int cellSize = std::min(
		(areaW - (gridWidth - 1) * INV_GRID_PADDING) / gridWidth,
		(areaH - (rowsToDraw - 1) * INV_GRID_PADDING) / rowsToDraw);
	if (cellSize < 1)
		cellSize = 1;

	m_x0 = x0;
	m_y0 = y0;
	m_cellSize = cellSize;
	m_cellStep = cellSize + INV_GRID_PADDING;
	m_gridWidth = gridWidth;
	m_rows = rows;
	m_rowsToDraw = rowsToDraw;

	const int gridW = gridWidth * cellSize + (gridWidth - 1) * INV_GRID_PADDING;
	const int gridH = rowsToDraw * cellSize + (rowsToDraw - 1) * INV_GRID_PADDING;

	// ---- Grid background ----
	ctx->drawSetColor(10, 10, 10, 60);
	ctx->drawFilledRect(x0, y0, x0 + gridW, y0 + gridH);

	// ---- Locked Rows ----
	// Drawn but visibly dead, so the player can see the space still to earn.
	if (rowsToDraw > rows)
	{
		const int lockedY = y0 + rows * m_cellStep;
		ctx->drawSetColor(0, 0, 0, 140);
		ctx->drawFilledRect(x0, lockedY, x0 + gridW, y0 + gridH);
	}

	// ---- Grid lines ----
	for (int c = 0; c <= gridWidth; ++c)
	{
		int vx = (c < gridWidth) ? x0 + c * m_cellStep : x0 + gridW;
		ctx->drawSetColor(200, 200, 200, 160);
		ctx->drawFilledRect(vx, y0, vx + 1, y0 + gridH);
	}
	for (int r = 0; r <= rowsToDraw; ++r)
	{
		int hy = (r < rowsToDraw) ? y0 + r * m_cellStep : y0 + gridH;
		// The boundary between earned and locked Rows is drawn brighter.
		if (r == rows && rowsToDraw > rows)
			ctx->drawSetColor(255, 170, 0, 140);
		else
			ctx->drawSetColor(200, 200, 200, 160);
		ctx->drawFilledRect(x0, hy, x0 + gridW, hy + 1);
	}

	// ---- Outer border ----
	ctx->drawSetColor(255, 170, 0, 60);
	ctx->drawOutlinedRect(x0, y0, x0 + gridW, y0 + gridH);

	// ---- Entries ----
	struct DeferredCountLabel
	{
		int  textX, textY;
		char text[8];
		int  textLen;
	};
	std::vector<DeferredCountLabel> deferredCountLabels;

	for (int i = 0; i < (int)entries.size(); ++i)
	{
		const InvEntryView& e = entries[i];

		const int cellWidth = InvEntryCellWidth(e.Kind(), e.id);
		if (cellWidth < 1)
			continue;

		IRect rect = CellRect(e.col, e.row, cellWidth);

		// The dragged Entry follows the cursor instead of its Cell. Its
		// server-side position does not change until the move is accepted.
		if (m_bDragging && i == m_draggedIndex)
		{
			rect.x = m_mouseX - m_dragGrabX;
			rect.y = m_mouseY - m_dragGrabY;
		}

		m_itemRects[i] = rect;

		if (e.IsWeapon())
		{
			WEAPON* p = gWR.GetWeapon(e.id);

			HSPRITE hspr = 0;
			Rect rc = {};
			if (p)
			{
				if (p->hActive && (p->rcActive.right - p->rcActive.left) > 0)
				{
					hspr = p->hActive;
					rc = p->rcActive;
				}
				else
				{
					hspr = p->hInactive;
					rc = p->rcInactive;
				}
			}

			int rr, gg, bb;
			if (!gWR.HasAmmo(p)) { UnpackRGB(rr, gg, bb, RGB_REDISH);    ScaleColors(rr, gg, bb, 128); }
			else                 { UnpackRGB(rr, gg, bb, RGB_YELLOWISH); ScaleColors(rr, gg, bb, 192); }

			ctx->drawSetColor(30, 30, 30, 80);
			ctx->drawFilledRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
			ctx->drawSetColor(255, 170, 0, 80);
			ctx->drawOutlinedRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);

			const int sprW = rc.right - rc.left;
			const int sprH = rc.bottom - rc.top;
			if (hspr && sprW > 0 && sprH > 0)
			{
				SPR_Set(hspr, rr, gg, bb);
				SPR_DrawAdditive(0,
					rect.x + (rect.w - sprW) / 2,
					rect.y + (rect.h - sprH) / 2, &rc);
			}
		}
		else if (e.IsItem())
		{
			const int ix = rect.x + 2, iy = rect.y + 2;
			const int iw = rect.w - 4, ih = rect.h - 4;

			int ir, ig, ib;
			GetItemTint(e.id, ir, ig, ib);

			FillRGBA(ix, iy, iw, ih, ir, ig, ib, 100);
			ctx->drawSetColor(ir, ig, ib, 60);
			ctx->drawOutlinedRect(ix, iy, ix + iw, iy + ih);

			ItemSprite spr = GetItemSprite(e.id);
			const int sprW = spr.rc.right - spr.rc.left;
			const int sprH = spr.rc.bottom - spr.rc.top;
			if (spr.hSprite && sprW > 0 && sprH > 0)
			{
				SPR_Set(spr.hSprite, ir, ig, ib);
				SPR_DrawAdditive(0, ix + (iw - sprW) / 2, iy + (ih - sprH) / 2, &spr.rc);
			}

			// Only Stacks are worth labelling; a lone item needs no "x1".
			if (e.count > 1 && ctx->m_pSmallFont)
			{
				DeferredCountLabel label{};
				snprintf(label.text, sizeof(label.text), "x%d", e.count);
				label.textLen = (int)strlen(label.text);

				const int charW = 6, charH = 10;
				const int textW = label.textLen * charW;
				label.textX = ix + iw - textW - 2;
				label.textY = iy + ih - charH - 1;
				deferredCountLabels.push_back(label);
			}
		}
	}

	// ---- Stack counts, drawn last so nothing overlaps them ----
	if (ctx->m_pSmallFont)
	{
		ctx->drawSetTextFont(ctx->m_pSmallFont);
		ctx->drawSetTextColor(255, 220, 50, 0);

		for (const DeferredCountLabel& label : deferredCountLabels)
		{
			ctx->drawSetTextPos(label.textX, label.textY);
			for (int c = 0; c < label.textLen; ++c)
				ctx->drawPrintChar(label.text[c]);
		}
	}
}

// =====================================================================
// Input
//
// A drag never moves anything locally.  On release the view asks the
// server to move the Entry and waits; the next sync is what actually
// changes the picture.
// =====================================================================
bool CInventoryGridView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY,
	const std::vector<InvEntryView>& entries)
{
	m_mouseX = localX;
	m_mouseY = localY;

	const int index = EntryAt(localX, localY);
	if (index < 0 || index >= (int)entries.size())
		return false;

	m_bDragging = true;
	m_draggedIndex = index;
	m_dragGrabX = localX - m_itemRects[index].x;
	m_dragGrabY = localY - m_itemRects[index].y;
	return true;
}

void CInventoryGridView::HandleMouseMove(int localX, int localY)
{
	m_mouseX = localX;
	m_mouseY = localY;
}

bool CInventoryGridView::HandleMouseRelease(CInventoryPanel* ctx, int localX, int localY,
	const std::vector<InvEntryView>& entries)
{
	if (!m_bDragging)
		return false;

	const int index = m_draggedIndex;
	m_bDragging = false;
	m_draggedIndex = -1;

	if (index < 0 || index >= (int)entries.size())
		return false;

	const InvEntryView& e = entries[index];

	const int cellWidth = InvEntryCellWidth(e.Kind(), e.id);
	if (cellWidth < 1)
		return false;

	// Snap by the Entry's top-left corner, offset by where it was grabbed,
	// so it lands where it looks like it will land.
	const int cornerX = localX - m_dragGrabX;
	const int cornerY = localY - m_dragGrabY;

	// Round to the nearest Cell rather than truncating, so a half-Cell
	// overhang snaps forwards instead of always backwards.
	int col = (cornerX - m_x0 + m_cellStep / 2) / m_cellStep;
	int row = (cornerY - m_y0 + m_cellStep / 2) / m_cellStep;

	col = std::max(0, std::min(col, m_gridWidth - cellWidth));
	row = std::max(0, std::min(row, std::max(0, m_rows - 1)));

	// Unchanged position: not worth a round trip.
	if (col == e.col && row == e.row)
		return true;

	char cmd[128];
	snprintf(cmd, sizeof(cmd), "inv_move %d %d %d %d %d\n", index, e.kind, e.id, col, row);
	gEngfuncs.pfnClientCmd(cmd);
	return true;
}
