//=========================================================
// vgui_records.cpp
//
// The Records tab, and the reader as it appears when a Record is read in
// the world.  Both draw through the one CRecordReaderView
// (cl_dll/vgui_record_reader.h), so a document reads identically wherever
// it is opened.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_inventory.h"
#include "vgui_records.h"
#include "vgui_SchemeManager.h"

#include <algorithm>

using namespace vgui;

namespace
{
constexpr int k_RowPad = 4;
constexpr int k_ListMaxW = 300;

// A title too long for the list column is cut with an ellipsis rather than
// wrapped: the list is an index, and a two-line entry in it reads as two
// Records. The reader beside it shows the title whole.
std::string FitToWidth(vgui::Font* font, const std::string& text, int maxW)
{
	if (!font || TextWidth(font, text) <= maxW)
		return text;

	const std::string ellipsis = "...";
	const int room = maxW - TextWidth(font, ellipsis);
	if (room <= 0)
		return ellipsis;

	std::string out;
	for (size_t i = 0; i < text.size(); ++i)
	{
		out.push_back(text[i]);
		if (TextWidth(font, out) > room)
		{
			out.pop_back();
			break;
		}
	}
	return out + ellipsis;
}
} // namespace

// =====================================================================
// CRecordsView
// =====================================================================
void CRecordsView::BuildRows(int x0, int y0, int listW, int lineH, vgui::Font* smallFont)
{
	m_rows.clear();

	const int rowH = lineH + k_RowPad;
	int y = y0 + 8;

	for (const std::string& category : gRecords.Categories())
	{
		// A category with nothing found in it is not shown at all: an empty
		// heading tells the player a category exists, which is half a spoiler.
		std::vector<int> ids;
		for (int id : gRecords.FoundOrder())
		{
			const RecordDef* rec = gRecords.ById(id);
			if (rec && rec->category == category)
				ids.push_back(id);
		}
		if (ids.empty())
			continue;

		ListRow head;
		head.rc = {x0 + 8, y, listW - 16, rowH};
		head.id = k_RecordIdNone;
		head.text = FitToWidth(smallFont, category, head.rc.w - 8);
		m_rows.push_back(head);
		y += rowH;

		for (int id : ids)
		{
			const RecordDef* rec = gRecords.ById(id);
			if (!rec)
				continue; // records.txt was reloaded out from under the set

			ListRow row;
			row.rc = {x0 + 16, y, listW - 24, rowH};
			row.id = id;
			row.text = FitToWidth(smallFont,
				!rec->title.empty() ? rec->title : rec->stringId, row.rc.w - 8);
			m_rows.push_back(row);
			y += rowH;
		}

		y += 4;
	}
}

void CRecordsView::Paint(CInventoryPanel* ctx,
	int x0, int y0, int areaW, int areaH,
	vgui::Font* smallFont, vgui::Font* titleFont)
{
	gRecords.EnsureLoaded();

	int sr, sg, sb, dr, dg, db, lr, lg, lb, orr, ogg, obb;
	UnpackRGB(sr, sg, sb, RGB_SUIT);
	UnpackRGB(dr, dg, db, RGB_SUIT_DIM);
	UnpackRGB(lr, lg, lb, RGB_SUIT_LIT);
	UnpackRGB(orr, ogg, obb, RGB_SUIT_OFF);

	const int lineH = smallFont ? smallFont->getTall() : 10;
	const int listW = std::min(std::max(areaW / 3, 160), k_ListMaxW);
	const int gap = 10;

	// ---- The list ----
	ctx->drawSetColor(10, 10, 10, 60);
	ctx->drawFilledRect(x0, y0, x0 + listW, y0 + areaH);
	ctx->drawSetColor(sr, sg, sb, 80);
	ctx->drawOutlinedRect(x0, y0, x0 + listW, y0 + areaH);

	BuildRows(x0, y0, listW, lineH, smallFont);

	struct RLabel
	{
		int x, y;
		std::string text;
		int r, g, b;
	};
	std::vector<RLabel> labels;

	if (m_rows.empty())
	{
		labels.push_back({x0 + 12, y0 + 12, "Nothing read yet.", orr, ogg, obb});
	}

	for (const ListRow& row : m_rows)
	{
		const bool bHeading = (row.id == k_RecordIdNone);
		const bool bSelected = (!bHeading && row.id == m_reader.Record());
		const bool bHover = (!bHeading && m_iHoverX >= row.rc.x && m_iHoverX < row.rc.x + row.rc.w &&
							 m_iHoverY >= row.rc.y && m_iHoverY < row.rc.y + row.rc.h);

		if (bSelected)
		{
			ctx->drawSetColor(dr, dg, db, 60);
			ctx->drawFilledRect(row.rc.x, row.rc.y, row.rc.x + row.rc.w, row.rc.y + row.rc.h);
		}
		else if (bHover)
		{
			ctx->drawSetColor(dr, dg, db, 140);
			ctx->drawFilledRect(row.rc.x, row.rc.y, row.rc.x + row.rc.w, row.rc.y + row.rc.h);
		}

		const int tx = row.rc.x + 4;
		const int ty = row.rc.y + k_RowPad / 2;

		if (bHeading)
		{
			labels.push_back({tx, ty, row.text, sr, sg, sb});
			ctx->drawSetColor(sr, sg, sb, 170);
			ctx->drawFilledRect(row.rc.x, row.rc.y + row.rc.h - 1, row.rc.x + row.rc.w, row.rc.y + row.rc.h);
		}
		else if (bSelected)
		{
			labels.push_back({tx, ty, row.text, lr, lg, lb});
		}
		else
		{
			labels.push_back({tx, ty, row.text, 190, 190, 185});
		}
	}

	if (smallFont)
	{
		ctx->drawSetTextFont(smallFont);
		for (const RLabel& lbl : labels)
		{
			ctx->drawSetTextColor(lbl.r, lbl.g, lbl.b, 0);
			ctx->drawSetTextPos(lbl.x, lbl.y);
			ctx->drawPrintText(lbl.text.c_str(), (int)lbl.text.size());
		}
	}

	// ---- The reader ----
	const int readerX = x0 + listW + gap;
	m_reader.Paint(ctx, readerX, y0, areaW - listW - gap, areaH, smallFont, titleFont);
}

void CRecordsView::HandleMousePress(CInventoryPanel* ctx, int localX, int localY)
{
	for (const ListRow& row : m_rows)
	{
		if (row.id == k_RecordIdNone)
			continue;
		if (localX >= row.rc.x && localX < row.rc.x + row.rc.w &&
			localY >= row.rc.y && localY < row.rc.y + row.rc.h)
		{
			m_reader.SetRecord(row.id);
			return;
		}
	}
}

// =====================================================================
// CRecordReaderPanel
// =====================================================================
CRecordReaderPanel::CRecordReaderPanel(int x, int y, int wide, int tall)
	: Panel(x, y, wide, tall)
{
	setVisible(false);
	setPaintBackgroundEnabled(false);

	if (gViewPort)
	{
		CSchemeManager* pSchemes = gViewPort->GetSchemeManager();
		if (pSchemes)
		{
			SchemeHandle_t hTitle = pSchemes->getSchemeHandle("Scoreboard Title Text");
			SchemeHandle_t hSmall = pSchemes->getSchemeHandle("Scoreboard Small Text");
			m_pTitleFont = pSchemes->getFont(hTitle);
			m_pSmallFont = pSchemes->getFont(hSmall);
		}
	}
}

void CRecordReaderPanel::ShowRecord(int id)
{
	if (id == k_RecordIdNone)
	{
		m_reader.Clear();
		setVisible(false);
		return;
	}

	m_reader.SetRecord(id);
	setVisible(true);
}

void CRecordReaderPanel::paintBackground()
{
	// The reader draws its own frame, so the panel has no background of its
	// own to fill; anything here would show as a second box behind it.
}

void CRecordReaderPanel::paint()
{
	if (m_reader.Record() == k_RecordIdNone)
		return;

	gRecords.EnsureLoaded();

	int wide = 0, tall = 0;
	getSize(wide, tall);
	m_reader.Paint(this, 0, 0, wide, tall, m_pSmallFont, m_pTitleFont);
}
