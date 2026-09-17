//=========================================================
// records.cpp
//
// Reading records.txt, and mirroring the found-set the server sends.
//
// The parser is line-based on purpose.  COM_ParseFile tokenises and eats
// newlines, which is right for a scheme file and wrong for prose: a body
// is paragraphs, and paragraphs are made of the blank lines a tokeniser
// throws away.  So a block's header is `key rest-of-line` and its body is
// verbatim, which also means no quoting rules and no escapes -- a body
// can contain braces, quotes and slashes and nothing has to know.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "records.h"

#include <cctype>
#include <cstring>

CRecordFile gRecords;

// records_reload: prose is rewritten far more often than code, and the
// whole reason the text is a data file is that it should not cost a
// rebuild -- it should not cost a map load either.
static void Records_Reload()
{
	gRecords.Reload();
	gEngfuncs.Con_Printf("records.txt reloaded: %d Record(s).\n", (int)gRecords.All().size());
}

void RecordsInit()
{
	gEngfuncs.pfnAddCommand("records_reload", Records_Reload);
}

namespace
{

const char* const k_RecordsFile = "records.txt";

std::string Trim(const std::string& s)
{
	size_t a = 0;
	size_t b = s.size();
	while (a < b && std::isspace((unsigned char)s[a]) != 0)
		++a;
	while (b > a && std::isspace((unsigned char)s[b - 1]) != 0)
		--b;
	return s.substr(a, b - a);
}

bool EqualsNoCase(const std::string& a, const char* b)
{
	if (!b)
		return false;
	size_t i = 0;
	for (; i < a.size() && b[i] != '\0'; ++i)
	{
		if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
			return false;
	}
	return i == a.size() && b[i] == '\0';
}

// Splits the file into lines, keeping empty ones: inside a body they are
// the paragraph breaks.
void SplitLines(const char* data, int length, std::vector<std::string>& lines)
{
	lines.clear();

	std::string cur;
	for (int i = 0; i < length && data[i] != '\0'; ++i)
	{
		const char c = data[i];
		if (c == '\r')
			continue;
		if (c == '\n')
		{
			lines.push_back(cur);
			cur.clear();
			continue;
		}
		cur.push_back(c);
	}
	lines.push_back(cur);
}

} // namespace

//=========================================================
// SplitRecordEmphasis
//
// One emphasis marker, as designed: a star opens it and a star closes it.
// An unclosed star is a typo, and a typo should cost the author one
// visible star rather than the colour of everything after it.
//=========================================================
void SplitRecordEmphasis(const std::string& line, std::vector<RecordRun>& runs)
{
	runs.clear();

	size_t pos = 0;
	while (pos < line.size())
	{
		const size_t open = line.find('*', pos);
		if (open == std::string::npos)
		{
			runs.push_back({line.substr(pos), false});
			return;
		}

		const size_t close = line.find('*', open + 1);
		if (close == std::string::npos)
		{
			// Unclosed: the rest is plain, star and all.
			runs.push_back({line.substr(pos), false});
			return;
		}

		if (open > pos)
			runs.push_back({line.substr(pos, open - pos), false});

		if (close > open + 1)
			runs.push_back({line.substr(open + 1, close - open - 1), true});

		pos = close + 1;
	}
}

//=========================================================
// Parse
//=========================================================
void CRecordFile::Parse(const char* data, int length)
{
	m_records.clear();
	m_categories.clear();

	std::vector<std::string> lines;
	SplitLines(data, length, lines);

	enum class EState
	{
		Outside,    // between blocks: looking for a block name
		WantBrace,  // have a name, waiting for '{'
		Header,     // inside a block, reading key/value lines
		Body,       // inside a block, taking lines verbatim
	};

	EState state = EState::Outside;
	RecordDef cur;
	int blockLine = 0;
	bool bBadBlock = false;

	// Drops a block on the floor loudly. The rest of the file still loads:
	// one malformed Record should not cost every other one.
	auto fail = [&](int lineNo, const char* why)
	{
		gEngfuncs.Con_Printf("records.txt(%d): %s -- skipping block '%s'\n",
			lineNo, why, cur.stringId.empty() ? "?" : cur.stringId.c_str());
		bBadBlock = true;
	};

	for (size_t i = 0; i < lines.size(); ++i)
	{
		const int lineNo = (int)i + 1;
		const std::string& raw = lines[i];

		if (state == EState::Body)
		{
			if (Trim(raw) == "}")
			{
				if (!bBadBlock)
				{
					// Trailing blank lines are the author's formatting, not
					// the document's.
					while (!cur.body.empty() && Trim(cur.body.back()).empty())
						cur.body.pop_back();

					if (!RecordIdValid(cur.id))
						fail(blockLine, "id is missing or out of range");
					else if (ById(cur.id) != nullptr)
						fail(blockLine, "id is already used by another Record");
				}

				if (!bBadBlock)
					m_records.push_back(cur);

				cur = RecordDef();
				bBadBlock = false;
				state = EState::Outside;
				continue;
			}

			cur.body.push_back(raw);
			continue;
		}

		const std::string line = Trim(raw);

		if (line.empty() || (line.size() >= 2 && line[0] == '/' && line[1] == '/'))
			continue;

		switch (state)
		{
		case EState::Outside:
			cur = RecordDef();
			cur.stringId = line;
			blockLine = lineNo;
			bBadBlock = false;
			state = EState::WantBrace;
			break;

		case EState::WantBrace:
			if (line != "{")
			{
				fail(lineNo, "expected '{'");
				state = EState::Outside;
			}
			else
			{
				state = EState::Header;
			}
			break;

		case EState::Header:
		{
			if (line == "}")
			{
				fail(blockLine, "block has no body");
				state = EState::Outside;
				cur = RecordDef();
				bBadBlock = false;
				break;
			}

			if (EqualsNoCase(line, "body"))
			{
				state = EState::Body;
				break;
			}

			const size_t sp = line.find_first_of(" \t");
			if (sp == std::string::npos)
			{
				fail(lineNo, "expected 'key value'");
				break;
			}

			const std::string key = line.substr(0, sp);
			const std::string value = Trim(line.substr(sp + 1));

			if (EqualsNoCase(key, "id"))
				cur.id = atoi(value.c_str());
			else if (EqualsNoCase(key, "category"))
				cur.category = value;
			else if (EqualsNoCase(key, "title"))
				cur.title = value;
			else if (EqualsNoCase(key, "source"))
				cur.source = value;
			else if (EqualsNoCase(key, "revocable"))
				cur.revocable = atoi(value.c_str()) != 0;
			else
				gEngfuncs.Con_Printf("records.txt(%d): unknown key '%s', ignored\n", lineNo, key.c_str());
			break;
		}

		default:
			break;
		}
	}

	if (state != EState::Outside)
		gEngfuncs.Con_Printf("records.txt: file ends inside block '%s'\n", cur.stringId.c_str());

	// Categories in first-appearance order, with Guidance pinned on top --
	// it is the advisor speaking, not something found.
	for (const RecordDef& rec : m_records)
	{
		if (rec.category.empty())
			continue;

		bool seen = false;
		for (const std::string& c : m_categories)
		{
			if (EqualsNoCase(c, rec.category.c_str()))
			{
				seen = true;
				break;
			}
		}
		if (!seen)
			m_categories.push_back(rec.category);
	}

	for (size_t i = 0; i < m_categories.size(); ++i)
	{
		if (!EqualsNoCase(m_categories[i], k_GuidanceCategory))
			continue;
		std::string guidance = m_categories[i];
		m_categories.erase(m_categories.begin() + i);
		m_categories.insert(m_categories.begin(), guidance);
		break;
	}

	gEngfuncs.Con_DPrintf("records.txt: %d Record(s) in %d categor(ies)\n",
		(int)m_records.size(), (int)m_categories.size());
}

//=========================================================
// Loading
//=========================================================
void CRecordFile::EnsureLoaded()
{
	if (m_bLoaded)
		return;
	Reload();
}

void CRecordFile::Reload()
{
	// Set first: a missing or broken file must not make every frame try
	// again, which would hammer the filesystem for as long as the game runs.
	m_bLoaded = true;

	m_records.clear();
	m_categories.clear();

	int length = 0;
	byte* data = gEngfuncs.COM_LoadFile((char*)k_RecordsFile, 5, &length);
	if (!data)
	{
		gEngfuncs.Con_Printf("records.txt not found; no Records will be readable.\n");
		return;
	}

	Parse((const char*)data, length);
	gEngfuncs.COM_FreeFile(data);

	// A reload leaves the found-set standing -- it is the server's, not the
	// file's -- but the ORDER was built against the old file and may name
	// ids the new one does not describe. Rebuild it in file order, which is
	// what a fresh mask would have given anyway.
	m_foundOrder.clear();
	for (const RecordDef& rec : m_records)
	{
		if (RecordMaskGet(m_found, rec.id))
			m_foundOrder.push_back(rec.id);
	}
}

const RecordDef* CRecordFile::ById(int id) const
{
	if (!RecordIdValid(id))
		return nullptr;

	for (const RecordDef& rec : m_records)
	{
		if (rec.id == id)
			return &rec;
	}
	return nullptr;
}

//=========================================================
// The found-set
//=========================================================
bool CRecordFile::AnyFound() const
{
	for (int i = 0; i < k_RecordMaskBytes; ++i)
	{
		if (m_found[i] != 0)
			return true;
	}
	return false;
}

void CRecordFile::SetMask(const unsigned char* mask)
{
	if (!mask)
		return;

	EnsureLoaded();

	// Newly-set bits are appended in file order, so within one session the
	// tab really does list Records in the order they were found. The first
	// mask of a session -- a new game, or a loaded save -- arrives whole, so
	// everything in it falls back to file order, which is the honest limit
	// of a set that carries no order.
	const bool bFirst = !m_bHaveMask;
	if (bFirst)
		m_foundOrder.clear();

	for (const RecordDef& rec : m_records)
	{
		const bool wasFound = !bFirst && RecordMaskGet(m_found, rec.id);
		const bool isFound = RecordMaskGet(mask, rec.id);
		if (isFound && !wasFound)
			m_foundOrder.push_back(rec.id);
	}

	// A revoke (Guidance, slice 3) takes an id back out of the order too.
	for (size_t i = 0; i < m_foundOrder.size();)
	{
		if (!RecordMaskGet(mask, m_foundOrder[i]))
			m_foundOrder.erase(m_foundOrder.begin() + i);
		else
			++i;
	}

	memcpy(m_found, mask, k_RecordMaskBytes);
	m_bHaveMask = true;
}
