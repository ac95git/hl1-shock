/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// flashlight.cpp
//
// implementation of CHudFlashlight class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h" // dlight_t, CL_AllocDlight -- the Night Vision eye light

#include <string.h>
#include <stdio.h>



DECLARE_MESSAGE(m_Flash, FlashBat)
DECLARE_MESSAGE(m_Flash, Flashlight)

#define BAT_NAME "sprites/%d_Flashlight.spr"

bool CHudFlashlight::Init()
{
	m_fFade = 0;
	m_fOn = false;
	m_iMode = 0;

	HOOK_MESSAGE(Flashlight);
	HOOK_MESSAGE(FlashBat);

	// Night Vision (docs/SKILL_TREE.md, "The Night Vision Module"): the
	// overlay's colour is fixed (green, as Opposing Force's own), alpha and
	// the dlight's radius are cvars so they can be judged in play.
	m_pCvarNvOverlay = CVAR_CREATE("nv_overlay", "160", FCVAR_ARCHIVE);
	m_pCvarNvLightRadius = CVAR_CREATE("nv_light_radius", "700", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return true;
}

void CHudFlashlight::Reset()
{
	m_fFade = 0;
	m_fOn = false;
	m_iMode = 0;
}

bool CHudFlashlight::VidInit()
{
	int HUD_flash_empty = gHUD.GetSpriteIndex("flash_empty");
	int HUD_flash_full = gHUD.GetSpriteIndex("flash_full");
	int HUD_flash_beam = gHUD.GetSpriteIndex("flash_beam");

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	// Loaded by path, not through hud.txt: adapted from Opposing Force's own
	// Night Vision (docs/SKILL_TREE.md, "The Night Vision Module"); the file
	// is sprites/of_nv_b.spr in both the repo and the install.
	m_hNightVisionOverlay = LoadSprite("sprites/of_nv_b.spr");

	return true;
}

bool CHudFlashlight::MsgFunc_FlashBat(const char* pszName, int iSize, void* pbuf)
{


	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	return true;
}

bool CHudFlashlight::MsgFunc_Flashlight(const char* pszName, int iSize, void* pbuf)
{

	BEGIN_READ(pbuf, iSize);
	m_fOn = READ_BYTE() != 0;
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;
	m_iMode = READ_BYTE(); // 0 flashlight, 1 night vision (dlls/UserMessages.cpp)

	return true;
}

bool CHudFlashlight::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_FLASHLIGHT | HIDEHUD_ALL)) != 0)
		return true;

	int r, g, b, x, y, a;
	Rect rc;

	if (!gHUD.HasSuit())
		return true;

	if (m_fOn)
		a = 225;
	else
		a = MIN_ALPHA;

	if (m_flBat < 0.20)
		UnpackRGB(r, g, b, RGB_REDISH);
	else
		UnpackRGB(r, g, b, RGB_SUIT);

	ScaleColors(r, g, b, a);

	y = (m_prc1->bottom - m_prc2->top) / 2;
	x = ScreenWidth - m_iWidth - m_iWidth / 2;

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y, m_prc1);

	if (m_fOn)
	{ // draw the flashlight beam
		x = ScreenWidth - m_iWidth / 2;

		SPR_Set(m_hBeam, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prcBeam);

		// Night Vision draws in place of the beam: no light touches the
		// world (docs/SKILL_TREE.md, "The Night Vision Module"), so the
		// dark only reads through these two purely client effects.
		if (m_iMode == 1)
		{
			DrawNightVision();
		}
	}

	// draw the flashlight energy level
	x = ScreenWidth - m_iWidth - m_iWidth / 2;
	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = *m_prc2;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, r, g, b);
		SPR_DrawAdditive(0, x + iOffset, y, &rc);
	}


	return true;
}

// Night Vision (docs/SKILL_TREE.md, "The Night Vision Module"): the overlay
// technique is Opposing Force's own (E:\Projects\halflife-op4-updated\cl_dll\
// flashlight.cpp, drawNightVision) -- a noise sprite re-picked at random each
// frame and tiled across the screen with an additive draw, plus an eye-level
// dlight so the dark is actually visible. Both are purely client-side: the
// server never learns about either, so a monster is exactly as blind to the
// player as it would be with nothing on.
void CHudFlashlight::DrawNightVision()
{
	if (m_pCvarNvOverlay->value > 0.0f && m_hNightVisionOverlay != 0)
	{
		static int lastFrame = 0;

		const int frames = SPR_Frames(m_hNightVisionOverlay);
		int frameIndex = frames > 0 ? rand() % frames : 0;
		if (frames > 1 && frameIndex == lastFrame)
			frameIndex = (frameIndex + 1) % frames;
		lastFrame = frameIndex;

		// Green, fixed; nv_overlay is the alpha, faked the way every other
		// additive sprite in this file is -- scale the colour, not a blend.
		int r = 20, g = 255, b = 90;
		ScaleColors(r, g, b, static_cast<int>(m_pCvarNvOverlay->value));
		SPR_Set(m_hNightVisionOverlay, r, g, b);

		const int width = SPR_Width(m_hNightVisionOverlay, frameIndex);
		const int height = SPR_Height(m_hNightVisionOverlay, frameIndex);

		if (width > 0 && height > 0)
		{
			Rect drawingRect;
			for (int tx = 0; tx < ScreenWidth; tx += width)
			{
				drawingRect.left = 0;
				drawingRect.right = tx + width >= ScreenWidth ? ScreenWidth - tx : width;

				for (int ty = 0; ty < ScreenHeight; ty += height)
				{
					drawingRect.top = 0;
					drawingRect.bottom = ty + height >= ScreenHeight ? ScreenHeight - ty : height;

					SPR_DrawAdditive(frameIndex, tx, ty, &drawingRect);
				}
			}
		}
	}

	// The eye light. Keyed by the local player's index, like the progression
	// pickups' own dlights (cl_dll/entity.cpp, ProgressionLight) -- refreshed
	// every frame at the same slot rather than stacked, and it dies with the
	// frame that stops asking for it, so it needs no cleanup when the Module
	// is off or the player disconnects.
	if (m_pCvarNvLightRadius->value > 0.0f)
	{
		cl_entity_t* player = gEngfuncs.GetLocalPlayer();
		if (player != nullptr)
		{
			dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(player->index);
			if (dl != nullptr)
			{
				memcpy(dl->origin, gHUD.m_vecOrigin, sizeof(Vector));
				dl->radius = m_pCvarNvLightRadius->value;
				dl->color.r = 20;
				dl->color.g = 255;
				dl->color.b = 90;
				dl->die = gEngfuncs.GetClientTime() + 0.1f;
				dl->decay = 0.0f;
			}
		}
	}
}
