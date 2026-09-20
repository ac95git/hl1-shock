#include "hud.h"
#include "cl_util.h"
#include "vgui_inventory.h"
#include "vgui_inventory_grid.h"
#include "vgui_deferred_text.h"
#include "spr_fit.h"
#include "ammohistory.h"
#include <VGUI_App.h>
#include <VGUI_Cursor.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace vgui;

// =====================================================================
// Layout constants
//
// The Grid is a lattice of 1px lines on a fixed pitch. A Cell is the
// space between two lines; a footprint sits inside its Cell(s) inset by the
// same amount on every side, so the gap to the line is equal left and
// right, top and bottom. The lines are the geometry -- footprints are
// derived from them, never the other way round.
// =====================================================================
static constexpr int INV_GRID_LINE  = 1; // thickness of a grid line
static constexpr int INV_CELL_INSET = 2; // gap between a footprint and the line on each side

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
// Footprint art
//
// A footprint's sprite is fitted to the footprint, not drawn at its native size:
// HUD sprites come in resolution buckets (170x45 at 640, 340x90 at 1280,
// 510x135 at 2560) and the footprint's size follows the screen, so a native
// draw only fits at the one resolution the art was picked against. The
// fit keeps the sprite's aspect and leaves a margin proportional to the
// footprint, so the art is resolution-independent even though the sprite is not.
// =====================================================================
// The whole first frame of a sprite as a rect. Named fields on purpose:
// Rect is laid out left, right, top, bottom, and a positional initialiser
// in the x, y, w, h order one expects silently yields a zero-width rect.
static Rect FullFrame(HSPRITE hspr)
{
	Rect rc;
	rc.left   = 0;
	rc.top    = 0;
	rc.right  = SPR_Width(hspr, 0);
	rc.bottom = SPR_Height(hspr, 0);
	return rc;
}

static void DrawFootprintSprite(HSPRITE hspr, const Rect& rc, int r, int g, int b,
	const CInventoryGridView::IRect& footprint, bool alphaBlend)
{
	const int sprW = rc.right - rc.left;
	const int sprH = rc.bottom - rc.top;
	if (!hspr || sprW <= 0 || sprH <= 0 || footprint.w <= 0 || footprint.h <= 0)
		return;

	SPR_Set(hspr, r, g, b);

	if (CVAR_GET_FLOAT("inv_icon_fit") == 0.0f)
	{
		// Native size, through the engine's own draw for the sprite's kind:
		// holes for alphatest art, additive for the HUD's. This is the
		// control when the scaled draw is in question.
		const int nx = footprint.x + (footprint.w - sprW) / 2;
		const int ny = footprint.y + (footprint.h - sprH) / 2;
		if (alphaBlend)
			SPR_DrawHoles(0, nx, ny, &rc);
		else
			SPR_DrawAdditive(0, nx, ny, &rc);
		return;
	}

	// Margin as a share of the footprint's height, so it scales with the footprint.
	// inv_icon_pad is that share; the icon's own render adds a little more.
	const float padFrac = std::max(0.0f, std::min(0.4f, CVAR_GET_FLOAT("inv_icon_pad")));
	const int pad  = std::max(1, (int)(footprint.h * padFrac + 0.5f));
	const int boxW = std::max(1, footprint.w - 2 * pad);
	const int boxH = std::max(1, footprint.h - 2 * pad);

	int src = SPR_BLEND_ONE, dst = SPR_BLEND_ONE;
	if (alphaBlend)
	{
		src = SPR_BLEND_SRC_ALPHA;
		dst = SPR_BLEND_ONE_MINUS_SRC_ALPHA;
	}

	// The frame-over-rect correction the scaled draw needs lives in
	// SPR_DrawFitted, shared with the Skill Tree's nodes.
	const SprFitDraw d = SPR_DrawFitted(hspr, rc, footprint.x + pad, footprint.y + pad, boxW, boxH, src, dst);

	if (CVAR_GET_FLOAT("inv_icon_debug") != 0.0f)
	{
		// Each distinct sprite once per second: enough to read, not a flood.
		static float s_windowEnd = 0.0f;
		static int   s_seen[32];
		static int   s_nSeen = 0;
		if (gHUD.m_flTime >= s_windowEnd)
		{
			s_windowEnd = gHUD.m_flTime + 1.0f;
			s_nSeen = 0;
		}
		bool seen = false;
		for (int i = 0; i < s_nSeen; ++i)
			if (s_seen[i] == (int)hspr) { seen = true; break; }
		if (!seen && s_nSeen < 32)
		{
			s_seen[s_nSeen++] = (int)hspr;
			gEngfuncs.Con_Printf("inv_icon draw: h%d rc %d,%d-%d,%d frame %dx%d at %d,%d size %dx%d req %dx%d blend %x/%x\n",
				(int)hspr, rc.left, rc.top, rc.right, rc.bottom, SPR_Width(hspr, 0), SPR_Height(hspr, 0),
				d.x, d.y, d.w, d.h, d.reqW, d.reqH, src, dst);
		}
	}
}

// =====================================================================
// Inventory icons
//
// Full-colour art of the item as it is seen in the world, one file per
// classname at sprites/inv/<classname>.spr: alphatest, one size, drawn
// untinted and alpha-blended. The suit colour stays on the chrome; the
// thing in the footprint keeps its own colour. An Entry with no icon file
// falls back to its HUD sprite, tinted and additive as before.
//
// Lookups are cached per sync rather than per session: the engine frees
// client sprites on a map change, so a handle must not outlive the
// Entries it was loaded for. Misses are cached too, so a missing file
// costs one failed load per sync, not one per frame.
// =====================================================================
HSPRITE CInventoryGridView::IconFor(const char* classname)
{
	if (!classname || !*classname)
		return 0;

	for (const IconCacheEntry& e : m_iconCache)
		if (strcmp(e.classname, classname) == 0)
			return e.hSprite;

	char path[128];
	snprintf(path, sizeof(path), "sprites/inv/%s.spr", classname);

	IconCacheEntry e{};
	strncpy(e.classname, classname, sizeof(e.classname) - 1);

	// SPR_Load prints an engine error for a file that is not there, and
	// most Entries have no icon yet. COM_LoadFile walks the same search
	// path the sprite loader does and is silent on a miss, so it is the
	// existence check: a plain fopen against the game directory is not,
	// because the working directory under Steam is not the game folder.
	int length = 0;
	bool exists = false;
	if (byte* data = gEngfuncs.COM_LoadFile(path, 5, &length))
	{
		gEngfuncs.COM_FreeFile(data);
		exists = true;
		e.hSprite = SPR_Load(path);
	}
	m_iconCache.push_back(e);

	if (CVAR_GET_FLOAT("inv_icon_debug") != 0.0f)
	{
		gEngfuncs.Con_Printf("inv_icon: %s -> %s, handle %d, %dx%d, frames %d\n", path,
			exists ? "found" : "not found", (int)e.hSprite,
			e.hSprite ? SPR_Width(e.hSprite, 0) : 0,
			e.hSprite ? SPR_Height(e.hSprite, 0) : 0,
			e.hSprite ? SPR_Frames(e.hSprite) : 0);
	}
	return e.hSprite;
}

void CInventoryGridView::ResetIconCache()
{
	m_iconCache.clear();
}

// =====================================================================
// Geometry
// =====================================================================
CInventoryGridView::IRect CInventoryGridView::CellRect(int col, int row, int cellWidth) const
{
	// The line at the Cell's near edge occupies the boundary pixel; the footprint
	// starts past it and the inset, and stops the same inset short of the
	// far line. A footprint spanning several Cells covers the interior lines.
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
	// The Entry under the cursor, judged against last paint's rects before
	// they are rebuilt. A frame of lag on hover is invisible; a hover
	// computed from rects that do not exist yet is not.
	const int hoverIndex = m_bDragging ? m_draggedIndex : EntryAt(m_mouseX, m_mouseY);

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
	// A pitch has to hold a line, two insets and at least one footprint pixel.
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
	// Stack counts are collected during the sprite pass and drawn after it
	// (vgui_deferred_text.h; docs/TECH_DEBT.md, the VGUI draw-order rule).
	CDeferredText deferredCountLabels;

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

		// A full-colour icon sits straight on the lattice with no box under
		// it, the way System Shock 2 lays its weapons over the grid. Its
		// footprint -- how many Cells it takes -- shows only when the player
		// points at it or is carrying it, as a suit-coloured outline. Entries
		// still on a HUD sprite keep their dark box, because an additive
		// silhouette needs one to read at all.
		const bool hovered = (i == hoverIndex);
		auto drawFootprint = [&](int r_, int g_, int b_, int fillA, int lineA)
		{
			ctx->drawSetColor(r_, g_, b_, fillA);
			ctx->drawFilledRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
			ctx->drawSetColor(r_, g_, b_, lineA);
			ctx->drawOutlinedRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
		};

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

			const bool hasAmmo = gWR.HasAmmo(p);
			const HSPRITE hIcon = IconFor(p ? p->szName : nullptr);

			int rr, gg, bb;
			if (!hasAmmo) { UnpackRGB(rr, gg, bb, RGB_REDISH);    ScaleColors(rr, gg, bb, 128); }
			else          { UnpackRGB(rr, gg, bb, RGB_SUIT);      ScaleColors(rr, gg, bb, 192); }

			if (hIcon)
			{
				// With a full-colour icon the "no ammo" red cannot go on the
				// art, which is untinted by design, so it goes on the footprint
				// instead, and it shows at rest: it is a warning, not a cue.
				if (!hasAmmo)
				{
					int er, eg, eb;
					UnpackRGB(er, eg, eb, RGB_REDISH);
					drawFootprint(er, eg, eb, 210, hovered ? 40 : 100);
				}
				else if (hovered)
				{
					drawFootprint(sr, sg, sb, 210, 40);
				}
			}
			else
			{
				ctx->drawSetColor(30, 30, 30, 80);
				ctx->drawFilledRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
				ctx->drawSetColor(sr, sg, sb, hovered ? 30 : 80);
				ctx->drawOutlinedRect(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
			}

			if (hIcon)
				DrawFootprintSprite(hIcon, FullFrame(hIcon), 255, 255, 255, rect, true);
			else
				DrawFootprintSprite(hspr, rc, rr, gg, bb, rect,
					CVAR_GET_FLOAT("inv_icon_blend") != 0.0f);
		}
		else if (e.IsItem())
		{
			// Same footprint rect as a weapon: the inset is the Cell's, not the kind's.
			const int ix = rect.x, iy = rect.y;
			const int iw = rect.w, ih = rect.h;

			int ir, ig, ib;
			GetItemTint(e.id, ir, ig, ib);

			const ItemTypeDef* def = GetItemType(e.id);
			const HSPRITE hIcon = IconFor(def ? def->classname : nullptr);
			if (hIcon)
			{
				if (hovered)
					drawFootprint(sr, sg, sb, 210, 40);
				DrawFootprintSprite(hIcon, FullFrame(hIcon), 255, 255, 255, rect, true);
			}
			else
			{
				FillRGBA(ix, iy, iw, ih, ir, ig, ib, 100);
				ctx->drawSetColor(ir, ig, ib, hovered ? 20 : 60);
				ctx->drawOutlinedRect(ix, iy, ix + iw, iy + ih);

				ItemSprite spr = GetItemSprite(e.id);
				DrawFootprintSprite(spr.hSprite, spr.rc, ir, ig, ib, rect,
					CVAR_GET_FLOAT("inv_icon_blend") != 0.0f);
			}

			// Only Stacks are worth labelling; a lone item needs no "x1".
			if (e.count > 1 && ctx->m_pSmallFont)
			{
				char text[8];
				snprintf(text, sizeof(text), "x%d", e.count);

				// Measured, not guessed: a guessed height put the label
				// below the footprint on any font taller than it.
				int textW = 0, textH = 0;
				ctx->m_pSmallFont->getTextSize(text, textW, textH);
				deferredCountLabels.Add(ix + iw - textW - 2, iy + ih - textH - 1, text);
			}
		}
	}

	// ---- Stack counts, drawn last so nothing overlaps them ----
	deferredCountLabels.Flush(ctx, ctx->m_pSmallFont, lr, lg, lb, 0);
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
	// so it lands where it looks like it will land. The corner is the footprint's,
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
