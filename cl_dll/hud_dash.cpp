//=========================================================
// hud_dash.cpp
//
// The Dash readout: one vertical bar per charge, in the suit cluster after
// the Concealment icon, drawn like the Pulse's charge bar.  A ready charge is
// full; the charge coming back fills bottom-up; the rest are empty.
//
// No message: the server writes the charges into physinfo for the movement
// code anyway (CBasePlayer::DashSync), and physinfo reaches the client with
// every update.  The fill runs off the client clock from the recharge time,
// the way CHudPulse runs its Recharge.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "pm_shared.h"

#include <stdlib.h>

// Throwaway diagnostic: print the raw physinfo values the readout draws
// from, so a bar that draws wrong can be blamed on the string or on the
// draw rather than guessed at (2026-09-16: a bar that repeated to the
// screen's edge and never drained).  Delete with the bug.
static void DashDebug()
{
	static const char* rgszKeys[] = {DASH_KEY_READY, DASH_KEY_MAX, DASH_KEY_SPEED, DASH_KEY_TIME, DASH_KEY_RECHARGE, DASH_KEY_AIR, "phs", "slj"};
	for (const char* pszKey : rgszKeys)
		gEngfuncs.Con_Printf("physinfo %s = \"%s\"\n", pszKey, gEngfuncs.PhysInfo_ValueForKey(pszKey));
}

bool CHudDash::Init()
{
	m_iLastReady = -1;
	m_flRefillStart = 0;

	gEngfuncs.pfnAddCommand("dash_debug", DashDebug);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return true;
}

bool CHudDash::VidInit()
{
	const Rect& rcSuit = gHUD.GetSpriteRect(gHUD.GetSpriteIndex("suit_full"));
	m_iSuitWidth = rcSuit.right - rcSuit.left;
	m_iSuitHeight = rcSuit.bottom - rcSuit.top;
	return true;
}

void CHudDash::Reset()
{
	m_iLastReady = -1;
	m_flRefillStart = 0;
}

bool CHudDash::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_HEALTH)) != 0)
		return true;

	if (!gHUD.HasSuit())
		return true;

	const int iMax = atoi(gEngfuncs.PhysInfo_ValueForKey(DASH_KEY_MAX));
	if (iMax <= 0)
		return true; // no Module

	const int iReady = atoi(gEngfuncs.PhysInfo_ValueForKey(DASH_KEY_READY));
	const float flRecharge = (float)atof(gEngfuncs.PhysInfo_ValueForKey(DASH_KEY_RECHARGE));

	// The server refills one charge at a time and a spent charge does not
	// restart the one already coming back, so the refill clock restarts only
	// when a charge arrives, or when the first one is spent from full.
	if (iReady != m_iLastReady)
	{
		if (m_iLastReady < 0 || iReady > m_iLastReady || m_iLastReady >= iMax)
			m_flRefillStart = flTime;
		m_iLastReady = iReady;
	}

	int r, g, b;
	UnpackRGB(r, g, b, RGB_SUIT);

	const int barW = (m_iSuitWidth / 5) > 3 ? (m_iSuitWidth / 5) : 3;
	const int barH = m_iSuitHeight;
	const int y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	const int barY = y - m_iSuitHeight / 6;

	int x = gHUD.m_Conceal.RightEdge() + m_iSuitWidth / 2;

	for (int i = 0; i < iMax; ++i)
	{
		FillRGBA(x, barY, barW, barH, 24, 24, 24, 100);

		float flFill = 0;
		int a = 192;

		if (i < iReady)
		{
			flFill = 1;
		}
		else if (i == iReady)
		{
			flFill = (flRecharge > 0) ? (flTime - m_flRefillStart) / flRecharge : 1;
			if (flFill < 0)
				flFill = 0;
			if (flFill > 1)
				flFill = 1;
			a = 100;
		}

		const int fillH = (int)(barH * flFill);
		if (fillH > 0)
			FillRGBA(x, barY + (barH - fillH), barW, fillH, r, g, b, a);

		x += barW * 2;
	}

	return true;
}
