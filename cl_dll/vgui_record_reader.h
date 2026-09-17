#pragma once

#include <VGUI_Font.h>
#include <string>
#include <vector>

#include "records.h"
#include "vgui_skilltree.h" // TextWidth, WrapToWidth -- shared, not copied

// =====================================================================
// CRecordReaderView
//   The reader: a Record's title, its source line and its body, drawn
//   into a rectangle.
//
//   ONE reader serves the world and the tab (docs/ROADMAP.md, "Pillar 1:
//   Records").  It is a plain helper rather than a panel, like
//   CStatusView, so the world's reader panel and the Records tab draw
//   exactly the same thing with exactly the same code -- a second
//   implementation would drift, and a document that read differently
//   depending on where you opened it would be worse than no tab at all.
//
//   Paint is a template on the host panel only because VGUI1's drawing
//   methods are protected: each host declares this class a friend, and
//   each gets its own instantiation.  Nothing else about it is generic.
// =====================================================================
class CRecordReaderView
{
public:
	void SetRecord(int id)
	{
		if (m_iRecordId != id)
			m_iScroll = 0;
		m_iRecordId = id;
	}

	int Record() const { return m_iRecordId; }

	void Clear()
	{
		m_iRecordId = k_RecordIdNone;
		m_iScroll = 0;
	}

	// Lines past the bottom of the last paint, so a document longer than
	// its box is still readable. Records are meant to be short, so this is
	// a safety net rather than a feature.
	void ScrollBy(int lines)
	{
		m_iScroll += lines;
		if (m_iScroll < 0)
			m_iScroll = 0;
		if (m_iScroll > m_iMaxScroll)
			m_iScroll = m_iMaxScroll;
	}

	template <class THost>
	void Paint(THost* ctx, int x0, int y0, int w, int h,
		vgui::Font* smallFont, vgui::Font* titleFont);

private:
	int m_iRecordId = k_RecordIdNone;
	int m_iScroll = 0;
	int m_iMaxScroll = 0; // set by the last paint, which is the only thing that knows
};

// ---------------------------------------------------------------------
template <class THost>
void CRecordReaderView::Paint(THost* ctx, int x0, int y0, int w, int h,
	vgui::Font* smallFont, vgui::Font* titleFont)
{
	int sr, sg, sb, dr, dg, db, lr, lg, lb, orr, ogg, obb;
	UnpackRGB(sr, sg, sb, RGB_SUIT);
	UnpackRGB(dr, dg, db, RGB_SUIT_DIM);
	UnpackRGB(lr, lg, lb, RGB_SUIT_LIT);
	UnpackRGB(orr, ogg, obb, RGB_SUIT_OFF);

	ctx->drawSetColor(8, 8, 8, 40);
	ctx->drawFilledRect(x0, y0, x0 + w, y0 + h);
	ctx->drawSetColor(sr, sg, sb, 90);
	ctx->drawOutlinedRect(x0, y0, x0 + w, y0 + h);

	const int pad = 12;
	const int lineH = smallFont ? smallFont->getTall() : 10;
	const int titleH = titleFont ? titleFont->getTall() : lineH;
	const int innerW = w - pad * 2;

	// Every label is collected and drawn after the fills
	// (docs/TECH_DEBT.md, "VGUI Draw Order").
	struct RLabel
	{
		int x, y;
		std::string text;
		int r, g, b;
		bool title;
	};
	std::vector<RLabel> labels;

	int y = y0 + pad;

	if (m_iRecordId == k_RecordIdNone)
	{
		labels.push_back({x0 + pad, y, "No Record selected.", orr, ogg, obb, false});
	}
	else
	{
		const RecordDef* rec = gRecords.ById(m_iRecordId);
		if (!rec)
		{
			// The server can hold a bit for a Record this copy of
			// records.txt does not describe -- a stale install, a cut
			// document. Say so rather than drawing an empty page.
			char buf[64];
			snprintf(buf, sizeof(buf), "Record %d not found.", m_iRecordId);
			labels.push_back({x0 + pad, y, buf, 200, 90, 90, false});
			labels.push_back({x0 + pad, y + lineH + 4, "records.txt does not describe it.", orr, ogg, obb, false});
		}
		else
		{
			// ---- Title ----
			// Wrapped rather than clipped: a Record's title is a real
			// document's heading -- "Restricted Access -- West Corridor" --
			// and shortening titles to fit a box would be the box deciding
			// how the fiction is written.
			std::vector<std::string> titleLines;
			WrapToWidth(titleFont ? titleFont : smallFont,
				(rec->title.empty() ? rec->stringId : rec->title).c_str(), innerW, titleLines);

			for (const std::string& tl : titleLines)
			{
				labels.push_back({x0 + pad, y, tl, lr, lg, lb, true});
				y += titleH;
			}
			y += 2;

			// ---- Source: where the document physically is ----
			// Wrapped too, for the same reason.
			if (!rec->source.empty())
			{
				std::vector<std::string> sourceLines;
				WrapToWidth(smallFont, rec->source.c_str(), innerW, sourceLines);
				for (const std::string& sl : sourceLines)
				{
					labels.push_back({x0 + pad, y, sl, orr, ogg, obb, false});
					y += lineH;
				}
				y += 2;
			}

			// ---- Rule ----
			ctx->drawSetColor(sr, sg, sb, 140);
			ctx->drawFilledRect(x0 + pad, y + 3, x0 + w - pad, y + 4);
			y += 10;

			// ---- Body ----
			// Wrapped once per paint. A Record is a page of prose, so this
			// is a few dozen measurements, not a budget worth caching.
			struct BodyLine
			{
				std::vector<RecordRun> runs;
			};
			std::vector<BodyLine> drawn;

			std::vector<std::string> wrapped;
			std::vector<RecordRun> runs;

			for (const std::string& src : rec->body)
			{
				if (src.find_first_not_of(" \t") == std::string::npos)
				{
					drawn.push_back(BodyLine()); // a paragraph break
					continue;
				}

				// Wrapped with its markup in place, then each resulting
				// line split into runs. The cost is that a *span* broken
				// across a wrap reads as two literal stars -- so an
				// emphasised code belongs in the middle of a line, not at
				// the end of a long one. Worth it: the alternative is a
				// wrapper that understands runs, for prose that is meant to
				// be a page long.
				WrapToWidth(smallFont, src.c_str(), innerW, wrapped);
				for (const std::string& wl : wrapped)
				{
					BodyLine bl;
					SplitRecordEmphasis(wl, bl.runs);
					drawn.push_back(bl);
				}
			}

			// ---- Scroll ----
			const int footerH = lineH + 6;
			const int avail = (y0 + h - pad - footerH) - y;
			const int visible = (lineH > 0) ? avail / lineH : 0;
			m_iMaxScroll = (int)drawn.size() - visible;
			if (m_iMaxScroll < 0)
				m_iMaxScroll = 0;
			if (m_iScroll > m_iMaxScroll)
				m_iScroll = m_iMaxScroll;

			for (int i = m_iScroll; i < (int)drawn.size() && (i - m_iScroll) < visible; ++i)
			{
				int x = x0 + pad;
				for (const RecordRun& run : drawn[i].runs)
				{
					if (run.text.empty())
						continue;
					// Emphasis takes the suit's colour; the prose is the
					// paper's grey, so a code reads as the suit lifting it
					// out rather than as the author shouting.
					if (run.emphasis)
						labels.push_back({x, y, run.text, sr, sg, sb, false});
					else
						labels.push_back({x, y, run.text, 200, 200, 195, false});
					x += TextWidth(smallFont, run.text);
				}
				y += lineH;
			}

			// ---- Footer: the suit's own voice ----
			const int footY = y0 + h - pad - lineH;
			if (m_iMaxScroll > 0)
			{
				char buf[64];
				snprintf(buf, sizeof(buf), "Registered  --  %d more line(s) below",
					m_iMaxScroll - m_iScroll);
				labels.push_back({x0 + pad, footY, buf, orr, ogg, obb, false});
			}
			else
			{
				labels.push_back({x0 + pad, footY, "Registered in suit memory", orr, ogg, obb, false});
			}
		}
	}

	// ---- All text, last ----
	for (const RLabel& lbl : labels)
	{
		vgui::Font* font = lbl.title ? (titleFont ? titleFont : smallFont) : smallFont;
		if (!font)
			continue;
		ctx->drawSetTextFont(font);
		ctx->drawSetTextColor(lbl.r, lbl.g, lbl.b, 0);
		ctx->drawSetTextPos(lbl.x, lbl.y);
		ctx->drawPrintText(lbl.text.c_str(), (int)lbl.text.size());
	}
}
