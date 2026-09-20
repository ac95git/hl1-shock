// vgui_deferred_text.h -- the one type for the VGUI draw-order workaround.
//
// Text drawn from a VGUI paint before a later SPR_DrawAdditive is overwritten
// by the sprite (docs/TECH_DEBT.md, "VGUI Draw Order: All Sprites, Then All
// Text").  The cause is not understood; the workaround is to collect every
// piece of text during the sprite pass and draw it all at the end of the
// paint.  Until 2026-09-20 each panel re-invented that as a local struct and
// vector; this is the same thing once, so a new panel copies one line instead
// of a pattern it might get wrong.
//
// Flush is a template on the panel type because the VGUI1 draw calls are
// protected members of vgui::Panel: the panel must name CDeferredText a
// friend, as CInventoryPanel does for its views, and the pointer passed must
// be of that panel's own type, not vgui::Panel*.  It leaves the text font and
// colour set to what it was given.
#pragma once

#include <cstring>
#include <vector>

namespace vgui
{
class Font;
}

class CDeferredText
{
public:
	static constexpr int k_MaxLen = 48;

	// Copies the text, truncated to k_MaxLen - 1 characters.
	void Add(int x, int y, const char* pszText)
	{
		Entry e{};
		e.x = x;
		e.y = y;
		strncpy(e.text, pszText, sizeof(e.text) - 1);
		e.text[sizeof(e.text) - 1] = '\0';
		e.len = (int)strlen(e.text);
		m_entries.push_back(e);
	}

	bool Empty() const { return m_entries.empty(); }

	void Clear() { m_entries.clear(); }

	// Draws everything collected, in order, with one font and colour; the
	// caller has already established that the sprite pass is over.
	template <class TPanel>
	void Flush(TPanel* pPanel, vgui::Font* pFont, int r, int g, int b, int a)
	{
		if (m_entries.empty() || pFont == nullptr)
			return;

		pPanel->drawSetTextFont(pFont);
		pPanel->drawSetTextColor(r, g, b, a);

		for (const Entry& e : m_entries)
		{
			pPanel->drawSetTextPos(e.x, e.y);
			pPanel->drawPrintText(e.text, e.len);
		}
	}

private:
	struct Entry
	{
		int x, y;
		char text[k_MaxLen];
		int len;
	};
	std::vector<Entry> m_entries;
};
