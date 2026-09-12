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
// Layout constants
//
// The Grid is a lattice of 1px lines on a fixed pitch. A Cell is the
// space between two lines; a tile sits inside its Cell(s) inset by the
// same amount on every side, so the gap to the line is equal left and
// right, top and bottom. The lines are the geometry -- tiles are
// derived from them, never the other way round.
// =====================================================================
static constexpr int INV_GRID_LINE  = 1; // thickness of a grid line
static constexpr int INV_CELL_INSET = 2; // gap between a tile and the line on each side

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
// Tile art
//
// A tile's sprite is fitted to the tile, not drawn at its native size:
// HUD sprites come in resolution buckets (170x45 at 640, 340x90 at 1280,
// 510x135 at 2560) and the tile's size follows the screen, so a native
// draw only fits at the one resolution the art was picked against. The
// fit keeps the sprite's aspect and leaves a margin proportional to the
// tile, so the art is resolution-independent even though the sprite is not.
// =====================================================================
static void DrawTileSprite(HSPRITE hspr, const Rect& rc, int r, int g, int b,
	const CInventoryGridView::IRect& tile)
{
	const int sprW = rc.right - rc.left;
	const int sprH = rc.bottom - rc.top;
	if (!hspr || sprW <= 0 || sprH <= 0 || tile.w <= 0 || tile.h <= 0)
		return;

	SPR_Set(hspr, r, g, b);

	if (CVAR_GET_FLOAT("inv_icon_fit") == 0.0f)
	{
		SPR_DrawAdditive(0,
			tile.x + (tile.w - sprW) / 2,
			tile.y + (tile.h - sprH) / 2, &rc);
		return;
	}

	// Margin as a share of the tile's height, so it scales with the tile.
	const int pad  = std::max(1, tile.h / 10);
	const int boxW = std::max(1, tile.w - 2 * pad);
	const int boxH = std::max(1, tile.h - 2 * pad);

	const float scale = std::min((float)boxW / sprW, (float)boxH / sprH);
	const int w = std::max(1, (int)(sprW * scale + 0.5f));
	const int h = std::max(1, (int)(sprH * scale + 0.5f));

	int src = SPR_BLEND_ONE, dst = SPR_BLEND_ONE;
	if (CVAR_GET_FLOAT("inv_icon_blend") != 0.0f)
	{
		src = SPR_BLEND_SRC_ALPHA;
		dst = SPR_BLEND_ONE_MINUS_SRC_ALPHA;
	}

	// The width and height SPR_DrawGeneric takes are the size to draw the
	// WHOLE sprite frame at; the rect is then cut out of that at the same
	// scale. A weapon icon is a 340x90 rect on a 512x128 sheet, so asking
	// for the rect's own size draws it at two thirds. Scale the request up
	// by frame-over-rect so the rect itself lands at (w, h).
	const int frameW = std::max(sprW, SPR_Width(hspr, 0));
	const int frameH = std::max(sprH, SPR_Height(hspr, 0));
	const int reqW = std::max(1, (int)((float)w * frameW / sprW + 0.5f));
	const int reqH = std::max(1, (int)((float)h * frameH / sprH + 0.5f));

	SPR_DrawGeneric(0,
		tile.x + (tile.w - w) / 2,
		tile.y + (tile.h - h) / 2, &rc, src, dst, reqW, reqH);
}

// =====================================================================
// Geometry
// =====================================================================
CInventoryGridView::IRect CInventoryGridView::CellRect(int col, int row, int cellWidth) const
{
	// The line at the Cell's near edge occupies the boundary pixel; the tile
	// starts past it and the inset, and stops the same inset short of the
	// far line. A tile spanning several Cells covers the interior lines.
	IRect r;
	r.x = m_x0 + col * m_cellStep + INV_GRID_LINE + INV_CELL_INSET;
	r.y = m_y0 + row * m_cellStep + INV_GRID_LINE + INV_CELL_INSET;
	r.w = cellWidth * m_cellStep - INV_GRID_LINE - 2 * INV_CELL_INSET;
	r.h = m_cellStep - INV_GRID_LINE - 2 * INV_CELL_INSET;
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

	// The pitch is driven by the FULL drawn Grid, including Rows the player
	// has not earned yet, so unlocking a Row never re-flows the layout. The
	// closing line on the far side is the one extra pixel beyond the pitches.
	int cellStep = std::min(
		(areaW - INV_GRID_LINE) / gridWidth,
		(areaH - INV_GRID_LINE) / rowsToDraw);
	// A pitch has to hold a line, two insets and at least one tile pixel.
	const int minStep = 2 * INV_GRID_LINE + 2 * INV_CELL_INSET + 1;
	if (cellStep < minStep)
		cellStep = minStep;

	const int gridW = gridWidth * cellStep + INV_GRID_LINE;
	const int gridH = rowsToDraw * cellStep + INV_GRID_LINE;

	// An integer pitch never fills the area exactly. The remainder goes on
	// the left, so the Grid's right edge sits on the same line as the header
	// above it, and the top stays level with the nav column beside it. The
	// gap between the column and the Grid absorbs the difference, where it
	// reads as a gutter rather than as a misalignment.
	x0 += std::max(0, areaW - gridW);

	m_x0 = x0;
	m_y0 = y0;
	m_cellStep = cellStep;
	m_gridWidth = gridWidth;
	m_rows = rows;
	m_rowsToDraw = rowsToDraw;

	// The Grid is suit equipment, so its frame is the suit's colour -- what
	// used to be the HUD's amber throughout. The reds below are not: an empty
	// weapon reads red whatever the player is wearing.
	int sr, sg, sb, lr, lg, lb;
	UnpackRGB(sr, sg, sb, RGB_SUIT);
	UnpackRGB(lr, lg, lb, RGB_SUIT_LIT);

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
	// One line per pitch boundary, the last one closing the far edge.
	for (int c = 0; c <= gridWidth; ++c)
	{
		const int vx = x0 + c * m_cellStep;
		ctx->drawSetColor(200, 200, 200, 160);
		ctx->drawFilledRect(vx, y0, vx + INV_GRID_LINE, y0 + gridH);
	}
	for (int r = 0; r <= rowsToDraw; ++r)
	{
		const int hy = y0 + r * m_cellStep;
		// The boundary between earned and locked Rows is drawn brighter.
		if (r == rows && rowsToDraw > rows)
			ctx->drawSetColor(sr, sg, sb, 140);
		else
			ctx->drawSetColor(200, 200, 200, 160);
		ctx->drawFilledRect(x0, hy, x0 + gridW, hy + INV_GRID_LINE);
	}

	// ---- Outer border ----
	ctx->drawSetColor(sr, sg, sb, 60);
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
			else                 { UnpackRGB(rr, gg, bb, RGB_SUIT);      ScaleColors(rr, gg, bb, 192); }

			ctx->drawSetColor(30, 30, 30, 80);
			ctx->drawFilledRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
			ctx->drawSetColor(sr, sg, sb, 80);
			ctx->drawOutlinedRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);

			DrawTileSprite(hspr, rc, rr, gg, bb, rect);
		}
		else if (e.IsItem())
		{
			// Same tile rect as a weapon: the inset is the Cell's, not the kind's.
			const int ix = rect.x, iy = rect.y;
			const int iw = rect.w, ih = rect.h;

			int ir, ig, ib;
			GetItemTint(e.id, ir, ig, ib);

			FillRGBA(ix, iy, iw, ih, ir, ig, ib, 100);
			ctx->drawSetColor(ir, ig, ib, 60);
			ctx->drawOutlinedRect(ix, iy, ix + iw, iy + ih);

			ItemSprite spr = GetItemSprite(e.id);
			DrawTileSprite(spr.hSprite, spr.rc, ir, ig, ib, rect);

			// Only Stacks are worth labelling; a lone item needs no "x1".
			if (e.count > 1 && ctx->m_pSmallFont)
			{
				DeferredCountLabel label{};
				snprintf(label.text, sizeof(label.text), "x%d", e.count);
				label.textLen = (int)strlen(label.text);

				// Measured, not guessed: a guessed height put the label
				// below the tile on any font taller than it.
				int textW = 0, textH = 0;
				ctx->m_pSmallFont->getTextSize(label.text, textW, textH);
				label.textX = ix + iw - textW - 2;
				label.textY = iy + ih - textH - 1;
				deferredCountLabels.push_back(label);
			}
		}
	}

	// ---- Stack counts, drawn last so nothing overlaps them ----
	if (ctx->m_pSmallFont)
	{
		ctx->drawSetTextFont(ctx->m_pSmallFont);
		ctx->drawSetTextColor(lr, lg, lb, 0);

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
	// so it lands where it looks like it will land. The corner is the tile's,
	// which sits a line and an inset past the Cell boundary; take that off so
	// the rounding below is measured from the boundary itself.
	const int cornerX = localX - m_dragGrabX - INV_GRID_LINE - INV_CELL_INSET;
	const int cornerY = localY - m_dragGrabY - INV_GRID_LINE - INV_CELL_INSET;

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
