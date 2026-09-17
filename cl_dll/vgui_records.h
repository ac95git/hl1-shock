#pragma once

#include <VGUI_Font.h>
#include <VGUI_Panel.h>
#include <string>
#include <vector>

#include "records.h"
#include "vgui_record_reader.h"

class CInventoryPanel;

// =====================================================================
// CRecordsView
//   The fourth tab: the found Records on the left, grouped by category
//   with Guidance pinned on top, and the reader on the right.
//
//   A plain C++ helper like CStatusView, owned by CInventoryPanel.
// =====================================================================
class CRecordsView
{
public:
	void Paint(CInventoryPanel* ctx,
		int x0, int y0, int areaW, int areaH,
		vgui::Font* smallFont, vgui::Font* titleFont);

	void HandleMousePress(CInventoryPanel* ctx, int localX, int localY);
	void HandleMouseMove(int localX, int localY)
	{
		m_iHoverX = localX;
		m_iHoverY = localY;
	}

	// Opening a Record from the world leaves the tab showing that same
	// Record, so a player who then opens the panel is already on the page
	// they were just reading.
	void ShowRecord(int id) { m_reader.SetRecord(id); }

private:
	struct IRect
	{
		int x, y, w, h;
	};
	struct ListRow
	{
		IRect rc = {0, 0, 0, 0};
		int id = k_RecordIdNone; // k_RecordIdNone: a category heading
		std::string text;
	};

	void BuildRows(int x0, int y0, int listW, int lineH, vgui::Font* smallFont);

	CRecordReaderView m_reader;
	std::vector<ListRow> m_rows;
	int m_iHoverX = -1;
	int m_iHoverY = -1;
};

// =====================================================================
// CRecordReaderPanel
//   The reader as it appears when a Record is read in the WORLD.
//
//   It takes no input: no cursor, no buttons, nothing to click.  That is
//   the point -- reading happens in real time in an unsafe world
//   (docs/ROADMAP.md, "Pillar 1: Records"), so the player keeps their
//   mouse, keeps moving and keeps looking while they read, and damage
//   shuts the page.  The server owns whether it is open; this only draws.
//
//   It is off to the left so that the crosshair, the Prompt under it and
//   the bottom-right readouts stay clear.
// =====================================================================
class CRecordReaderPanel : public vgui::Panel
{
	friend class CRecordReaderView;

public:
	CRecordReaderPanel(int x, int y, int wide, int tall);

	// id == k_RecordIdNone shuts it.
	void ShowRecord(int id);

	void paintBackground() override;
	void paint() override;

private:
	CRecordReaderView m_reader;
	vgui::Font* m_pSmallFont = nullptr;
	vgui::Font* m_pTitleFont = nullptr;
};
