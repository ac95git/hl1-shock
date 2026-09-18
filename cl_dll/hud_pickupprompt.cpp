//=========================================================
// hud_pickupprompt.cpp
//
// The Prompt: the text under the crosshair naming what the player is
// looking at and what a use press does to it.  The Pickup Prompt, for
// what can be taken, is its oldest case; since 2026-09-17 every ordinary
// usable entity gets one too (docs/ROADMAP.md, Pillar 1: Records).
//
// The server decides what is being looked at and sends only
// (kind, id, class) -- the text is resolved here from the shared Item Type
// and Prompt class tables, so no strings cross the wire. See
// docs/PILLARS.md and
// docs/adr/0002-two-identity-spaces-for-weapons-and-items.md.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "ammohistory.h"
#include "inventory_defs.h"
#include "prompt_defs.h"
#include "suit_defs.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_PickupPrompt, PickupHint)

bool CHudPickupPrompt::Init()
{
	HOOK_MESSAGE(PickupHint);

	m_iKind = 0;
	m_iId = 0;
	m_iClass = 0;

	gHUD.AddHudElem(this);
	return true;
}

bool CHudPickupPrompt::VidInit()
{
	return true;
}

void CHudPickupPrompt::Reset()
{
	m_iKind = 0;
	m_iId = 0;
	m_iClass = 0;
	m_iFlags &= ~HUD_ACTIVE;
}

bool CHudPickupPrompt::MsgFunc_PickupHint(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iKind = READ_BYTE();
	m_iId = READ_BYTE();
	m_iClass = READ_BYTE();

	// All zero means the player is no longer looking at anything a use press
	// would act on.
	if (m_iKind == 0 && m_iClass == 0)
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;

	return true;
}

//=========================================================
// Turns a weapon classname into something worth reading:
// "weapon_shotgun" -> "Shotgun".
//
// Weapons have no display name on the client -- only their classname, via
// gWR -- and inventing a second name table just for this would be exactly
// the client/server duplication ADR-0002 exists to avoid.
//=========================================================
static bool PrettyWeaponName(int weaponId, char* out, int outSize)
{
	WEAPON* w = gWR.GetWeapon(weaponId);
	if (!w || w->iId == 0 || w->szName[0] == '\0')
		return false;

	const char* src = w->szName;
	if (strncmp(src, "weapon_", 7) == 0)
		src += 7;

	if (src[0] == '\0')
		return false;

	int i = 0;
	for (; src[i] != '\0' && i < outSize - 1; ++i)
	{
		char c = src[i];
		out[i] = (c == '_') ? ' ' : c;
	}
	out[i] = '\0';

	if (out[0] >= 'a' && out[0] <= 'z')
		out[0] = (char)(out[0] - 'a' + 'A');

	return true;
}

//=========================================================
// "[E] Press": the action behind the key +use is really bound to, looked
// up every time so a rebind never shows a stale key.  The engine matches a
// binding with its '+' already stripped, so the lookup is for "use".
//=========================================================
static void FormatAction(const char* action, char* out, int outSize)
{
	const char* key = gEngfuncs.Key_LookupBinding("use");

	char keyBuf[32];
	snprintf(keyBuf, sizeof(keyBuf), "%s", (key && key[0]) ? key : "unbound");
	for (char* c = keyBuf; *c != '\0'; ++c)
		*c = (char)toupper((unsigned char)*c);

	snprintf(out, outSize, "[%s] %s", keyBuf, action);
}

bool CHudPickupPrompt::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_WEAPONS)) != 0)
		return true;

	if (m_iKind == 0 && m_iClass == 0)
		return true;

	char nameBuf[64];
	const char* name = nullptr;
	const char* action = k_PromptTakeAction;

	// A state line replaces the key and action entirely: what is in the way,
	// rather than an offer the press will not honour.
	const char* state = nullptr;

	if (m_iKind == 0)
	{
		// Used, not taken. A class may have no title: the action stands alone.
		const PromptClassDef& def = GetPromptClass(m_iClass);
		name = def.title;
		action = def.action;
		state = def.state;
	}
	else if (static_cast<EEntryKind>(m_iKind) == EEntryKind::Weapon)
	{
		if (PrettyWeaponName(m_iId, nameBuf, sizeof(nameBuf)))
			name = nameBuf;
	}
	else if (static_cast<EEntryKind>(m_iKind) == EEntryKind::Pickup)
	{
		// Taken but never carried, so the Prompt's own table names it.
		name = GetPromptClass(m_iId).title;
	}
	else if (static_cast<EEntryKind>(m_iKind) == EEntryKind::Suit)
	{
		// Which of the three is on the floor -- the whole point of prompting
		// for a suit rather than letting the player walk into it.
		name = GetSuitVariant(m_iId).displayName;
	}
	else
	{
		const ItemTypeDef* def = GetItemType(m_iId);
		if (def && def->displayName)
			name = def->displayName;
	}

	// A pickup that cannot be named is not prompted for, as before.
	if ((!action && !state) || (!name && m_iKind != 0))
		return true;

	// Centred under the crosshair. Two lines: what it is, and either what a
	// use press does to it or what is stopping one.
	char hint[64];
	if (state)
		snprintf(hint, sizeof(hint), "%s", state);
	else
		FormatAction(action, hint, sizeof(hint));

	int nameW = 0, nameH = 0;
	int hintW = 0, hintH = 0;
	if (name)
		GetConsoleStringSize(name, &nameW, &nameH);
	GetConsoleStringSize(hint, &hintW, &hintH);

	const int centreX = ScreenWidth / 2;
	int y = ScreenHeight / 2 + 32;

	if (name)
	{
		DrawConsoleString(centreX - nameW / 2, y, name);
		y += nameH + 2;
	}
	DrawConsoleString(centreX - hintW / 2, y, hint);

	return true;
}
