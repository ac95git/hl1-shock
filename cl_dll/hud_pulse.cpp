//=========================================================
// hud_pulse.cpp
//
// The Pulse readiness readout: a suit icon beside the armour one, with a
// vertical charge bar next to it, plus a screen tint while a Shield stands.
//
// The server sends only state CHANGES, each carrying how long the new
// state lasts. This element runs the fill off the client's own clock
// from that duration, so a Recharge costs three messages rather than
// one every frame. See dlls/player_pulse.cpp.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_Pulse, Pulse)

// Mirrors EPulseState in dlls/player_pulse.h.
#define PULSE_READY 0
#define PULSE_SHIELD 1
#define PULSE_RECHARGING 2

// Cyan, so the Pulse icon is not mistaken for the armour icon sitting
// immediately to its left in the same shape.
#define PULSE_COLOR_R 64
#define PULSE_COLOR_G 200
#define PULSE_COLOR_B 255

bool CHudPulse::Init()
{
	HOOK_MESSAGE(Pulse);

	m_iState = PULSE_READY;
	m_flStateStart = 0;
	m_flStateEnd = 0;

	// Alpha of the screen tint while a Shield stands; 0 turns it off.
	m_pCvarTint = CVAR_CREATE("hud_pulse_tint", "48", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return true;
}

bool CHudPulse::VidInit()
{
	const int suitFull = gHUD.GetSpriteIndex("suit_full");

	m_hSprite = 0; // resolved lazily in Draw, once the sprites are loaded
	m_prc = &gHUD.GetSpriteRect(suitFull);
	m_iHeight = m_prc->bottom - m_prc->top;

	return true;
}

void CHudPulse::Reset()
{
	// A reset player is armed. The server re-sends the true state on the next
	// frame anyway (CPlayerPulse::ForgetSentState), so this is only what shows
	// in the gap.
	m_iState = PULSE_READY;
	m_flStateStart = 0;
	m_flStateEnd = 0;
}

bool CHudPulse::MsgFunc_Pulse(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iState = READ_BYTE();
	const float flDuration = READ_BYTE() / 10.0f;

	m_flStateStart = gHUD.m_flTime;
	m_flStateEnd = gHUD.m_flTime + flDuration;

	return true;
}

//=========================================================
// Layout, shared with anything that sits after the Pulse.
//
// The armour readout starts at 3 * spriteW, then spends its own sprite width
// plus three digits. The Pulse icon sits immediately after that, and its
// charge bar immediately after the icon.
//=========================================================
int CHudPulse::IconX() const
{
	if (!m_prc)
		return 0;

	const int spriteW = m_prc->right - m_prc->left;

	return 3 * spriteW + spriteW
		+ gHUD.GetHudNumberWidth(100, 3, DHN_DRAWZERO)
		+ spriteW / 2;
}

int CHudPulse::BarWidth(int spriteW)
{
	return (spriteW / 5) > 3 ? (spriteW / 5) : 3;
}

int CHudPulse::RightEdge() const
{
	if (!m_prc)
		return 0;

	const int spriteW = m_prc->right - m_prc->left;

	return IconX() + spriteW + spriteW / 4 + BarWidth(spriteW);
}

bool CHudPulse::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_HEALTH)) != 0)
		return true;

	// The Pulse is suit hardware. Without the suit the player does not have it,
	// so there is nothing to show.
	if (!gHUD.HasSuit())
		return true;

	// ---- Screen tint ----------------------------------------------------
	//
	// Drawn first, and this element is deliberately registered FIRST in
	// CHud::Init so that every other readout draws on top of the tint rather
	// than under it. AddHudElem appends, so Init order is draw order.
	if (m_iState == PULSE_SHIELD && m_pCvarTint && m_pCvarTint->value > 0)
	{
		int a = (int)m_pCvarTint->value;
		if (a > 255)
			a = 255;

		FillRGBA(0, 0, ScreenWidth, ScreenHeight,
			PULSE_COLOR_R, PULSE_COLOR_G, PULSE_COLOR_B, a);
	}

	// ---- Icon and charge bar --------------------------------------------
	//
	// Geometry mirrors CHudBattery so the two readouts share a baseline.
	if (0 == m_hSprite)
		m_hSprite = gHUD.GetSprite(gHUD.GetSpriteIndex("suit_full"));

	if (0 == m_hSprite || !m_prc)
		return true;

	const int spriteW = m_prc->right - m_prc->left;
	const int iOffset = (m_prc->bottom - m_prc->top) / 6;

	const int y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

	const int x = IconX();

	int r = PULSE_COLOR_R;
	int g = PULSE_COLOR_G;
	int b = PULSE_COLOR_B;

	// How full the bar is, and how loud the whole readout is.
	float flFill = 1.0f;
	int a = 255;

	switch (m_iState)
	{
	case PULSE_SHIELD:
		// White while the Shield stands -- a different colour, not just a
		// brighter one, so it reads at a glance mid-fight.
		r = g = b = 255;
		flFill = 1.0f;
		a = 255;
		break;

	case PULSE_RECHARGING:
	{
		const float flSpan = m_flStateEnd - m_flStateStart;
		flFill = (flSpan > 0) ? (flTime - m_flStateStart) / flSpan : 1.0f;
		if (flFill < 0)
			flFill = 0;
		if (flFill > 1)
			flFill = 1;
		a = 100;
		break;
	}

	case PULSE_READY:
	default:
		flFill = 1.0f;
		a = 192;
		break;
	}

	int sr = r, sg = g, sb = b;
	ScaleColors(sr, sg, sb, a);

	SPR_Set(m_hSprite, sr, sg, sb);
	SPR_DrawAdditive(0, x, y - iOffset, m_prc);

	// Vertical charge bar, immediately right of the icon, filling bottom-up.
	const int barW = BarWidth(spriteW);
	const int barX = x + spriteW + spriteW / 4;
	const int barY = y - iOffset;
	const int barH = m_iHeight;

	FillRGBA(barX, barY, barW, barH, 24, 24, 24, 100);

	const int fillH = (int)(barH * flFill);
	if (fillH > 0)
		FillRGBA(barX, barY + (barH - fillH), barW, fillH, r, g, b, a);

	return true;
}
