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
// The third trap: the engine's sprite scissor cannot clip this draw. Inside
// a VGUI paint, SPR_EnableScissor around SPR_DrawGeneric drew NOTHING, even
// with a clip rect that contained the whole draw (measured 2026-09-16 on the
// Skill Tree's nodes: the readout showed every icon landing inside its clip,
// and none appeared until the scissor was bypassed). So a draw that must
// stop at an edge clips itself: SPR_DrawFittedClipped intersects the fitted
// rect with the clip and hands the engine the matching sub-rect of the
// sprite at the same scale, which is exactly what the scissor should have
// done.
//
// Returns where the rect landed and what was asked of the engine, so a
// caller with a debug readout can print the same numbers it drew with.
// =====================================================================
struct SprFitDraw
{
	int x = 0, y = 0, w = 0, h = 0; // where the sprite rect was drawn, before any clip
	int reqW = 0, reqH = 0;         // the whole-frame size handed to the engine
};

// The fit alone: where the rect would land, and what to ask of the engine.
inline SprFitDraw SPR_FitRect(HSPRITE hspr, const Rect& rc, int boxX, int boxY, int boxW, int boxH)
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
	return out;
}

inline SprFitDraw SPR_DrawFitted(HSPRITE hspr, const Rect& rc,
	int boxX, int boxY, int boxW, int boxH,
	int srcBlend, int dstBlend)
{
	const SprFitDraw out = SPR_FitRect(hspr, rc, boxX, boxY, boxW, boxH);
	if (out.w <= 0 || out.h <= 0)
		return out;

	SPR_DrawGeneric(0, out.x, out.y, &rc, srcBlend, dstBlend, out.reqW, out.reqH);
	return out;
}

// The same fit, drawn only where it overlaps the clip rect (screen space,
// the same space the box is in). The part outside is cut from the sprite
// rect rather than from the screen, so nothing is asked of the scissor.
inline SprFitDraw SPR_DrawFittedClipped(HSPRITE hspr, const Rect& rc,
	int boxX, int boxY, int boxW, int boxH,
	int clipX, int clipY, int clipW, int clipH,
	int srcBlend, int dstBlend)
{
	const SprFitDraw out = SPR_FitRect(hspr, rc, boxX, boxY, boxW, boxH);
	if (out.w <= 0 || out.h <= 0)
		return out;

	const int visX0 = std::max(out.x, clipX);
	const int visY0 = std::max(out.y, clipY);
	const int visX1 = std::min(out.x + out.w, clipX + clipW);
	const int visY1 = std::min(out.y + out.h, clipY + clipH);
	if (visX1 <= visX0 || visY1 <= visY0)
		return out; // entirely outside the clip

	if (visX0 == out.x && visY0 == out.y && visX1 == out.x + out.w && visY1 == out.y + out.h)
	{
		SPR_DrawGeneric(0, out.x, out.y, &rc, srcBlend, dstBlend, out.reqW, out.reqH);
		return out;
	}

	// Map the visible screen span back onto the sprite rect at the draw's
	// scale. The far edges round up so a sliver is never lost to truncation;
	// the rect is then cut out of the frame at the same scale it was fitted
	// at, so the visible part lands exactly where the whole would have.
	const int   sprW  = rc.right - rc.left;
	const int   sprH  = rc.bottom - rc.top;
	const float scale = std::max((float)out.w / sprW, (float)out.h / sprH);
	Rect sub;
	sub.left   = rc.left + (int)((visX0 - out.x) / scale);
	sub.top    = rc.top  + (int)((visY0 - out.y) / scale);
	sub.right  = std::min(rc.right,  rc.left + (int)((visX1 - out.x) / scale + 0.999f));
	sub.bottom = std::min(rc.bottom, rc.top  + (int)((visY1 - out.y) / scale + 0.999f));
	if (sub.right <= sub.left || sub.bottom <= sub.top)
		return out;

	SPR_DrawGeneric(0, visX0, visY0, &sub, srcBlend, dstBlend, out.reqW, out.reqH);
	return out;
}
