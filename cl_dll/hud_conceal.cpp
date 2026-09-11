//=========================================================
// hud_conceal.cpp
//
// The Concealment readout: an icon in the bottom-left suit cluster, after
// health, armour and the Pulse, tinted by how noticed the player is -- the
// cluster's own dim yellow when hidden, amber when something has noticed them,
// red when something has acquired them.
//
// It sits with the suit readouts rather than on the crosshair so it is read
// the way health and armour are read: a glance at the corner, rather than a
// mark in the middle of the room the player is trying to read.
//
// The sprite is a PLACEHOLDER -- the flashlight's own icon.  See
// docs/ART_DEBT.md, "The Concealment readout -- icon", for what is wrong
// with it and what it should become.
//
// The server sends only threshold crossings; this element runs its own
// transition cue off the client clock.  See dlls/perception.cpp.
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <math.h>

DECLARE_MESSAGE(m_Conceal, Conceal)

// Mirrors EConcealState in dlls/perception.h.
#define CONCEAL_UNSEEN 0
#define CONCEAL_NOTICED 1
#define CONCEAL_SPOTTED 2

// Placeholder -- docs/ART_DEBT.md.  Whatever replaces it must exist in EVERY
// resolution block of sprites/hud.txt: GetSpriteIndex returns -1 for a missing
// name, and GetSpriteRect does not check the index it is given.
static const char* const k_ConcealSprite = "flash_full";

// Amber for Noticed, red for Spotted.  A different HUE rather than a brighter
// amber, so the two read apart at the edge of vision without being looked at.
#define CONCEAL_NOTICED_R 255
#define CONCEAL_NOTICED_G 190
#define CONCEAL_NOTICED_B 70

#define CONCEAL_SPOTTED_R 255
#define CONCEAL_SPOTTED_G 70
#define CONCEAL_SPOTTED_B 50

// Noticed eases in; Spotted blinks.  Different SHAPES of cue rather than
// different strengths of the same one, so the escalation registers even when
// the player only catches it out of the corner of an eye.
#define CONCEAL_NOTICED_CUE_TIME 0.35f
#define CONCEAL_SPOTTED_CUE_TIME 0.6f
#define CONCEAL_SPOTTED_BLINK 0.2f // one on/off period; three blinks in the cue

bool CHudConceal::Init()
{
	HOOK_MESSAGE(Conceal);

	m_iState = CONCEAL_UNSEEN;
	m_flStateStart = 0;

	// Lets the readout be turned off outright, for a player who would rather
	// read the room unaided -- and for judging whether it is carrying the
	// mechanic or merely decorating it.
	m_pCvarEnable = CVAR_CREATE("hud_conceal", "1", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return true;
}

bool CHudConceal::VidInit()
{
	const Rect& rcSuit = gHUD.GetSpriteRect(gHUD.GetSpriteIndex("suit_full"));
	m_iSuitWidth = rcSuit.right - rcSuit.left;
	m_iSuitHeight = rcSuit.bottom - rcSuit.top;

	m_hSprite = 0; // resolved lazily in Draw, once the sprites are loaded
	m_prc = &gHUD.GetSpriteRect(gHUD.GetSpriteIndex(k_ConcealSprite));

	return true;
}

void CHudConceal::Reset()
{
	// Calm rather than alarming in the gap: the server re-sends the true state
	// on the next frame anyway (CBasePlayer::ForgetConcealState).
	m_iState = CONCEAL_UNSEEN;
	m_flStateStart = 0;
}

bool CHudConceal::MsgFunc_Conceal(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const int iNew = READ_BYTE();

	if (iNew != m_iState)
	{
		m_iState = iNew;
		m_flStateStart = gHUD.m_flTime;
	}

	return true;
}

bool CHudConceal::Draw(float flTime)
{
	// Same hide flags as the suit readouts beside it.  HIDEHUD_HEALTH is only
	// ever set for observers (dlls/player.cpp), never for a player without the
	// suit, so honouring it does not suppress the pre-suit warning below.
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_HEALTH)) != 0)
		return true;

	if (m_pCvarEnable && m_pCvarEnable->value == 0)
		return true;

	const bool bNoticed = m_iState == CONCEAL_NOTICED;
	const bool bSpotted = m_iState == CONCEAL_SPOTTED;

	// The resting icon lines up with the suit readouts, which only draw with
	// the suit -- without it, a dim icon would float alone in an empty corner.
	// The warnings draw regardless: Concealment applies from the first map,
	// before the player has a suit at all, and an unseen warning is not one.
	if (!bNoticed && !bSpotted && !gHUD.HasSuit())
		return true;

	if (0 == m_hSprite)
		m_hSprite = gHUD.GetSprite(gHUD.GetSpriteIndex(k_ConcealSprite));

	if (0 == m_hSprite || !m_prc)
		return true;

	int r, g, b, a;

	if (bSpotted)
	{
		r = CONCEAL_SPOTTED_R;
		g = CONCEAL_SPOTTED_G;
		b = CONCEAL_SPOTTED_B;
		a = 255;
	}
	else if (bNoticed)
	{
		r = CONCEAL_NOTICED_R;
		g = CONCEAL_NOTICED_G;
		b = CONCEAL_NOTICED_B;
		a = 170;
	}
	else
	{
		// Unseen rests in the cluster's own colour and brightness, so it reads
		// as one of the suit readouts until something changes.
		UnpackRGB(r, g, b, RGB_YELLOWISH);
		a = MIN_ALPHA;
	}

	// ---- The transition cue -----------------------------------------------
	const float flSince = flTime - m_flStateStart;

	if (bNoticed && flSince >= 0 && flSince < CONCEAL_NOTICED_CUE_TIME)
	{
		// Soft: a brief lift to full that eases back to resting.
		const float flEase = 1.0f - flSince / CONCEAL_NOTICED_CUE_TIME;
		a += (int)((255 - a) * flEase);
	}
	else if (bSpotted && flSince >= 0 && flSince < CONCEAL_SPOTTED_CUE_TIME)
	{
		// Hard: blinks, so it registers as something happening rather than
		// something being true.
		if (fmod(flSince, CONCEAL_SPOTTED_BLINK) > CONCEAL_SPOTTED_BLINK * 0.5f)
			a = 60;
	}

	if (a > 255)
		a = 255;

	ScaleColors(r, g, b, a);

	// ---- Layout -----------------------------------------------------------
	//
	// Same baseline as CHudBattery and CHudPulse.  Vertically CENTRED on the
	// suit icon rather than sharing its top edge, because the placeholder is
	// a different height from the readouts it sits beside.
	const int y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	const int iconH = m_prc->bottom - m_prc->top;
	const int iconY = (y - m_iSuitHeight / 6) + (m_iSuitHeight - iconH) / 2;

	// Starts where the Pulse ends, with the same half-sprite gap the Pulse
	// leaves after the armour number, so it cannot drift if the Pulse moves.
	const int x = gHUD.m_Pulse.RightEdge() + m_iSuitWidth / 2;

	SPR_Set(m_hSprite, r, g, b);
	SPR_DrawAdditive(0, x, iconY, m_prc);

	return true;
}
