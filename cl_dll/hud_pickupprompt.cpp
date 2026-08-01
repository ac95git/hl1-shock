//=========================================================
// hud_pickupprompt.cpp
//
// The Pickup Prompt: the text under the crosshair naming what the
// player is looking at and can take.
//
// The server decides what is being looked at and sends only (kind, id) --
// the display name is resolved here from the shared Item Type table, so no
// strings cross the wire. See docs/PILLARS.md and
// docs/adr/0002-two-identity-spaces-for-weapons-and-items.md.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "ammohistory.h"
#include "inventory_defs.h"
#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_PickupPrompt, PickupHint)

bool CHudPickupPrompt::Init()
{
	HOOK_MESSAGE(PickupHint);

	m_iKind = 0;
	m_iId = 0;

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
	m_iFlags &= ~HUD_ACTIVE;
}

bool CHudPickupPrompt::MsgFunc_PickupHint(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iKind = READ_BYTE();
	m_iId = READ_BYTE();

	// (0,0) means the player is no longer looking at anything takeable.
	if (m_iKind == 0)
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

bool CHudPickupPrompt::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_WEAPONS)) != 0)
		return true;

	if (m_iKind == 0)
		return true;

	char nameBuf[64];
	const char* name = nullptr;

	if (static_cast<EEntryKind>(m_iKind) == EEntryKind::Weapon)
	{
		if (PrettyWeaponName(m_iId, nameBuf, sizeof(nameBuf)))
			name = nameBuf;
	}
	else
	{
		const ItemTypeDef* def = GetItemType(m_iId);
		if (def && def->displayName)
			name = def->displayName;
	}

	if (!name)
		return true;

	// Centred under the crosshair. Two lines: what it is, and how to take it.
	static const char* k_hint = "Press USE to take";

	int nameW = 0, nameH = 0;
	int hintW = 0, hintH = 0;
	GetConsoleStringSize(name, &nameW, &nameH);
	GetConsoleStringSize(k_hint, &hintW, &hintH);

	const int centreX = ScreenWidth / 2;
	const int topY = ScreenHeight / 2 + 32;

	DrawConsoleString(centreX - nameW / 2, topY, name);
	DrawConsoleString(centreX - hintW / 2, topY + nameH + 2, k_hint);

	return true;
}
