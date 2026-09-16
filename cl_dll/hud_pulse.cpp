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
DECLARE_MESSAGE(m_Pulse, Matrix)

// Mirrors EPulseState in dlls/player_pulse.h.
#define PULSE_READY 0
#define PULSE_SHIELD 1
#define PULSE_RECHARGING 2

// Mirrors EMatrixState in dlls/player_pulse.h.
#define MATRIX_NONE 0
#define MATRIX_READY 1
#define MATRIX_UP 2
#define MATRIX_COOLDOWN 3

// The Pulse draws in the suit's colour like everything else.
//
// It used to have a private cyan, for one reason: it shares the armour's
// sprite and sat right beside it, so colour was the only thing telling two
// identical icons apart.  A cyan HUD would have erased that difference
// anyway, so the readout gives the colour up and leans on its charge bar,
// which the armour has no equivalent of.  The real fix is a sprite of its
// own -- docs/ART_DEBT.md, "The Pulse -- readout icon".

bool CHudPulse::Init()
{
	HOOK_MESSAGE(Pulse);
	HOOK_MESSAGE(Matrix);

	m_iState = PULSE_READY;
	m_flStateStart = 0;
	m_flStateEnd = 0;
	m_iMatrixState = MATRIX_NONE;
	m_flMatrixStateStart = 0;
	m_flMatrixStateEnd = 0;

	// Alpha of the screen tint while a Shield stands; 0 turns it off.
	m_pCvarTint = CVAR_CREATE("hud_pulse_tint", "48", FCVAR_ARCHIVE);

	// The Defense Matrix's edge tint: the alpha of the outermost band, and the
	// bands' total depth as a fraction of the screen's height.  0 turns it off.
	m_pCvarMatrixTint = CVAR_CREATE("hud_matrix_tint", "110", FCVAR_ARCHIVE);
	m_pCvarMatrixTintWidth = CVAR_CREATE("hud_matrix_tint_width", "0.12", FCVAR_ARCHIVE);

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
	m_iMatrixState = MATRIX_NONE;
	m_flMatrixStateStart = 0;
	m_flMatrixStateEnd = 0;
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

// The Defense Matrix, on change only, like the Pulse: the state and how long
// it lasts, and the bar runs off the client's clock from there.
bool CHudPulse::MsgFunc_Matrix(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iMatrixState = READ_BYTE();
	const float flDuration = READ_BYTE() / 10.0f;

	m_flMatrixStateStart = gHUD.m_flTime;
	m_flMatrixStateEnd = gHUD.m_flTime + flDuration;

	return true;
}

bool CHudPulse::MatrixUp() const
{
	return m_iMatrixState == MATRIX_UP;
}

bool CHudPulse::MatrixBarShown() const
{
	return m_iMatrixState != MATRIX_NONE;
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

	int x = IconX() + spriteW + spriteW / 4 + BarWidth(spriteW);

	// The Matrix bar, when the player has one to show.
	if (MatrixBarShown())
		x += spriteW / 4 + BarWidth(spriteW);

	return x;
}

//=========================================================
// The Defense Matrix's edge tint: bands in the suit's colour along all four
// edges, strongest at the edge and fading inward, so the screen reads as
// framed while the Matrix stands and the middle stays clear to fight in.
// FillRGBA draws flat rectangles, so the fade is stepped -- a handful of
// bands is enough at the alphas involved.  The top and bottom bands run the
// full width and the side bands fill between them, so no corner is painted
// twice.
//=========================================================
void CHudPulse::DrawMatrixTint() const
{
	if (!m_pCvarMatrixTint || m_pCvarMatrixTint->value <= 0)
		return;

	const int aEdge = (int)V_min(m_pCvarMatrixTint->value, 255.0f);
	const float flFrac = m_pCvarMatrixTintWidth ? m_pCvarMatrixTintWidth->value : 0.12f;
	const int depth = (int)(ScreenHeight * V_max(0.0f, V_min(flFrac, 0.5f)));
	if (depth <= 0)
		return;

	int r, g, b;
	UnpackRGB(r, g, b, RGB_SUIT);

	constexpr int k_Bands = 6;
	const int bandH = V_max(1, depth / k_Bands);

	for (int i = 0; i < k_Bands; ++i)
	{
		// Outermost band at full alpha, each one inward a step fainter.
		const int a = aEdge * (k_Bands - i) / k_Bands;
		if (a <= 0)
			continue;

		const int inset = i * bandH;

		// Top and bottom, full width.
		FillRGBA(0, inset, ScreenWidth, bandH, r, g, b, a);
		FillRGBA(0, ScreenHeight - inset - bandH, ScreenWidth, bandH, r, g, b, a);

		// Left and right, between them.
		const int sideY = inset + bandH;
		const int sideH = ScreenHeight - 2 * sideY;
		if (sideH > 0)
		{
			FillRGBA(inset, sideY, bandH, sideH, r, g, b, a);
			FillRGBA(ScreenWidth - inset - bandH, sideY, bandH, sideH, r, g, b, a);
		}
	}
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

		int tr, tg, tb;
		UnpackRGB(tr, tg, tb, RGB_SUIT);

		FillRGBA(0, 0, ScreenWidth, ScreenHeight, tr, tg, tb, a);
	}

	// The Defense Matrix frames the screen while it stands; same draw-order
	// reasoning as the Shield's tint above.
	if (m_iMatrixState == MATRIX_UP)
		DrawMatrixTint();

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

	int r, g, b;
	UnpackRGB(r, g, b, RGB_SUIT);

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

	// ---- The Matrix bar --------------------------------------------------
	//
	// Right of the Pulse's bar, only for a player who holds the Skill.  The
	// same vocabulary as the Pulse's: full in the suit's colour when ready,
	// white and draining while it stands, dim and refilling on cooldown.
	if (MatrixBarShown())
	{
		int mr, mg, mb;
		UnpackRGB(mr, mg, mb, RGB_SUIT);

		float flMatrixFill = 1.0f;
		int ma = 192;

		const float flSpan = m_flMatrixStateEnd - m_flMatrixStateStart;
		const float flElapsed = (flSpan > 0) ? (flTime - m_flMatrixStateStart) / flSpan : 1.0f;
		const float flClamped = V_max(0.0f, V_min(flElapsed, 1.0f));

		switch (m_iMatrixState)
		{
		case MATRIX_UP:
			// White, and what is left of the six seconds drains away.
			mr = mg = mb = 255;
			flMatrixFill = 1.0f - flClamped;
			ma = 255;
			break;

		case MATRIX_COOLDOWN:
			flMatrixFill = flClamped;
			ma = 100;
			break;

		case MATRIX_READY:
		default:
			break;
		}

		const int mbarX = barX + barW + spriteW / 4;

		FillRGBA(mbarX, barY, barW, barH, 24, 24, 24, 100);

		const int mfillH = (int)(barH * flMatrixFill);
		if (mfillH > 0)
			FillRGBA(mbarX, barY + (barH - mfillH), barW, mfillH, mr, mg, mb, ma);
	}

	return true;
}
