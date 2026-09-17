#pragma once

#include <string>
#include <vector>

#include "record_defs.h"

// =====================================================================
// records.txt, and the client's mirror of the found-set.
//
// A Record's text is read HERE and nowhere else.  It never crosses the
// wire: a user message caps at 192 bytes, and prose gets rewritten
// constantly, which must not cost a rebuild of two DLLs.  So the server
// sends a bitmask and the client owns every word (docs/ROADMAP.md,
// "Pillar 1: Records").
//
// The file is the repo's records.txt, copied to the mod directory by
// hand like sprites/hud.txt.  Format -- keyed blocks in the manner of
// titles.txt, line-based rather than tokenised so that prose needs no
// quoting and no escaping:
//
//   // a comment line
//   west_access_notice
//   {
//       id        1
//       category  Notice
//       title     Sector C Access Notice
//       source    Printed notice, west corridor
//       revocable 0
//       body
//       The first line of the body.
//
//       A blank line is a paragraph break, and *stars* mark the one
//       emphasis, drawn in the suit's colour -- codes, and *[REDACTED]*.
//   }
//
// Everything after the lone `body` line is taken verbatim until the
// closing brace, so a body may contain braces, quotes and slashes.
// =====================================================================

struct RecordDef
{
	int id = k_RecordIdNone; // the bit; the whole of what the server knows
	std::string stringId;    // the block's name, for the console and for mappers
	std::string category;    // the tab groups by this, in first-appearance order
	std::string title;
	std::string source;      // one line: "Medical file, Level 3 infirmary"
	bool revocable = false;  // Guidance only; nothing reads it until slice 3
	std::vector<std::string> body; // raw lines, blank lines kept as breaks
};

// The Guidance category is pinned to the top of the tab rather than
// ordered with the rest, and is the only category anything is ever
// revoked from.  Matched case-insensitively, since it is typed by hand.
inline constexpr const char* k_GuidanceCategory = "Guidance";

class CRecordFile
{
public:
	// Loads records.txt once.  Safe to call every frame.
	void EnsureLoaded();

	// Re-reads it from disk.  Bound to a console command so prose can be
	// rewritten and judged without leaving the game.
	void Reload();

	const std::vector<RecordDef>& All() const { return m_records; }

	// nullptr for an id records.txt does not hold -- which the reader draws
	// as "record not found", because only the client can tell.
	const RecordDef* ById(int id) const;

	// Every category that has at least one Record, Guidance first and the
	// rest in the order the file introduces them.
	const std::vector<std::string>& Categories() const { return m_categories; }

	// ---- The found-set, as the server last said ----

	void SetMask(const unsigned char* mask);

	bool Found(int id) const { return RecordMaskGet(m_found, id); }

	bool AnyFound() const;

	// Found ids in the order they were found, for as far as this session
	// watched them arrive; ids already set when the first mask landed --
	// a loaded save -- keep file order behind them.  The honest
	// approximation of "found order": the server sends a set, not a list,
	// and a real order would be 512 saved sequence numbers.
	const std::vector<int>& FoundOrder() const { return m_foundOrder; }

private:
	void Parse(const char* data, int length);

	std::vector<RecordDef> m_records;
	std::vector<std::string> m_categories;

	unsigned char m_found[k_RecordMaskBytes] = {};
	std::vector<int> m_foundOrder;
	bool m_bHaveMask = false;

	bool m_bLoaded = false;
};

extern CRecordFile gRecords;

// Registers records_reload. Called once from CHud::Init.
void RecordsInit();

// =====================================================================
// One line of a body, split into the runs the reader draws: plain text,
// and the *emphasised* spans that take the suit's colour.  An unclosed
// star is drawn as a literal star rather than swallowing the rest of the
// document.
// =====================================================================
struct RecordRun
{
	std::string text;
	bool emphasis = false;
};

void SplitRecordEmphasis(const std::string& line, std::vector<RecordRun>& runs);
