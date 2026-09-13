#pragma once

#include "hud.h"
#include "cl_util.h"
#include <algorithm>

// =====================================================================
// SPR_DrawFitted
//
// The one scaled HUD sprite draw, wrapped so its traps live in one place.
// Every other HUD draw is native size, and HUD sprites are
// resolution-bucketed (the same icon is 44px at 640 and 88px at 1280), so
// anything that sizes its box from the layout rather than from the art has
// to come through here. The Inventory Grid's Footprints and the Skill Tree's
// nodes both do.
//
// Fits the sprite rect into the box, keeping its aspect, centred. The caller
// has already called SPR_Set with the tint; the blend pair is raw GL, see
// SPR_BLEND_* in cl_util.h -- (ONE, ONE) is the HUD's additive look,
// (SRC_ALPHA, ONE_MINUS_SRC_ALPHA) is alpha-blended colour art.
//
// The trap: the width and height SPR_DrawGeneric takes are the size to
// draw the WHOLE sprite frame at, and the rect is then cut out of that at
// the same scale. A 340x90 icon on a 512x128 sheet asked for at 340x90
// comes out at two thirds. The request is scaled up by frame-over-rect so
// the rect itself lands at the fitted size.
//
// The second trap: the engine shrinks but does not magnify. Asked for a
// size larger than the sprite's own frame it draws the frame's worth and
// clips the rest (measured 2026-09-14: the gauss and egon node icons, the
// only two magnified, lost their right edge and bottom). So the fit is
// capped at native size -- a sprite smaller than its box sits centred in it
// at 1:1. Art for a fitted box therefore has to be made at least as large
// as the box will ever be; shrinking is free, growing is not possible.
//
// Returns where the rect landed and what was asked of the engine, so a
// caller with a debug readout can print the same numbers it drew with.
// =====================================================================
struct SprFitDraw
{
	int x = 0, y = 0, w = 0, h = 0; // where the sprite rect was drawn
	int reqW = 0, reqH = 0;         // the whole-frame size handed to the engine
};

inline SprFitDraw SPR_DrawFitted(HSPRITE hspr, const Rect& rc,
	int boxX, int boxY, int boxW, int boxH,
	int srcBlend, int dstBlend)
{
	SprFitDraw out;

	const int sprW = rc.right - rc.left;
	const int sprH = rc.bottom - rc.top;
	if (!hspr || sprW <= 0 || sprH <= 0 || boxW <= 0 || boxH <= 0)
		return out;

	const float scale = std::min(1.0f, std::min((float)boxW / sprW, (float)boxH / sprH));
	out.w = std::max(1, (int)(sprW * scale + 0.5f));
	out.h = std::max(1, (int)(sprH * scale + 0.5f));

	const int frameW = std::max(sprW, SPR_Width(hspr, 0));
	const int frameH = std::max(sprH, SPR_Height(hspr, 0));
	out.reqW = std::max(1, (int)((float)out.w * frameW / sprW + 0.5f));
	out.reqH = std::max(1, (int)((float)out.h * frameH / sprH + 0.5f));

	out.x = boxX + (boxW - out.w) / 2;
	out.y = boxY + (boxH - out.h) / 2;

	SPR_DrawGeneric(0, out.x, out.y, &rc, srcBlend, dstBlend, out.reqW, out.reqH);
	return out;
}
