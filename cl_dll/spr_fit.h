#pragma once

#include "hud.h"
#include "cl_util.h"
#include <algorithm>

// =====================================================================
// SPR_DrawFitted / SPR_DrawFittedClipped
//
// The one scaled HUD sprite draw, wrapped so its traps live in one place.
// Every other HUD draw is native size, and HUD sprites are
// resolution-bucketed (the same icon is 44px at 640 and 88px at 1280), so
// anything that sizes its box from the layout rather than from the art has
// to come through here. The Inventory Grid's Footprints, the Status tab's
// Slots and the Skill Tree's nodes all do.
//
// Fits the sprite rect into the box, keeping its aspect, centred. The caller
// has already called SPR_Set with the tint; the blend pair is raw GL, see
// SPR_BLEND_* in cl_util.h -- (ONE, ONE) is the HUD's additive look,
// (SRC_ALPHA, ONE_MINUS_SRC_ALPHA) is alpha-blended colour art.
//
// THE MODEL OF SPR_DrawGeneric, settled 2026-09-16 after four observations
// that each looked like a separate trap and are one rule:
//
//   The frame is scaled to the requested width and height. The rect is in
//   PIXELS OF THAT SCALED FRAME, not of the sprite. The cut is drawn at the
//   position given, at the rect's own size. A rect edge past the request is
//   clamped to it.
//
// - 2026-09-14: a 340x90 icon on a 512x128 sheet, asked for at 340x90 with
//   its native rect, came out at two thirds. The rect was clamped to the
//   request, so the whole sheet was drawn at 340 wide and the icon took
//   340/512 of it.
// - 2026-09-14: asked for more than native size, the gauss and egon icons
//   lost their right edge and bottom, which read as "the engine does not
//   magnify". The native rect cut only that many pixels of the magnified
//   frame. Magnification is untested under the model and the fit is still
//   capped at native size; lift the cap only after measuring it.
// - 2026-09-16: SPR_EnableScissor around the draw, inside a VGUI paint,
//   drew nothing at all even when the clip contained the whole draw. Still
//   unexplained; never scissor from a paint.
// - 2026-09-16: a sub-rect in sprite pixels, drawn at the visible position,
//   drifted out of its node and drew past the field. In scaled pixels its
//   offset was too small and its extent too large.
//
// So the rect is scaled with the frame, and clipping to an edge is plain
// arithmetic in the scaled space: the visible span of the fitted icon is
// the rect to cut and the place to draw it. SPR_DrawFittedClipped does
// that; SPR_DrawFitted is it with no clip. Both agree with the old draw
// wherever the old draw was right (a rect at 0,0 covering the icon, at or
// below native size), which is every icon in the 1280 and 2560 buckets.
//
// Returns where the icon landed and what was asked of the engine, so a
// caller with a debug readout can print the same numbers it drew with.
// =====================================================================
struct SprFitDraw
{
	int x = 0, y = 0, w = 0, h = 0; // where the whole fitted icon lands, before any clip
	int reqW = 0, reqH = 0;         // the scaled-frame size handed to the engine
	int visX = 0, visY = 0, visW = 0, visH = 0; // the part actually drawn; zero if none
};

// The fit alone: where the icon would land, and the frame size to ask for.
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

// The fit, drawn only where it overlaps the clip rect (screen space, the
// same space the box is in). Nothing is asked of the scissor: the visible
// span is cut from the scaled frame and drawn where it is visible.
inline SprFitDraw SPR_DrawFittedClipped(HSPRITE hspr, const Rect& rc,
	int boxX, int boxY, int boxW, int boxH,
	int clipX, int clipY, int clipW, int clipH,
	int srcBlend, int dstBlend)
{
	SprFitDraw out = SPR_FitRect(hspr, rc, boxX, boxY, boxW, boxH);
	if (out.w <= 0 || out.h <= 0)
		return out;

	const int visX0 = std::max(out.x, clipX);
	const int visY0 = std::max(out.y, clipY);
	const int visX1 = std::min(out.x + out.w, clipX + clipW);
	const int visY1 = std::min(out.y + out.h, clipY + clipH);
	if (visX1 <= visX0 || visY1 <= visY0)
		return out; // entirely outside the clip

	// The icon's rect in the scaled frame: its native corner scaled by the
	// same factor the frame was, its size the fitted size exactly, so the
	// cut and the fit can never disagree by a rounding.
	const int   sprW  = rc.right - rc.left;
	const float scale = (float)out.w / sprW;
	const int   left  = (int)(rc.left * scale + 0.5f);
	const int   top   = (int)(rc.top * scale + 0.5f);

	// The visible span, as an offset into that rect, is the cut.
	Rect cut;
	cut.left   = left + (visX0 - out.x);
	cut.top    = top + (visY0 - out.y);
	cut.right  = left + (visX1 - out.x);
	cut.bottom = top + (visY1 - out.y);

	// The engine clamps an edge past the request to the request, which would
	// silently widen the cut; keep the request large enough to hold it.
	out.reqW = std::max(out.reqW, cut.right);
	out.reqH = std::max(out.reqH, cut.bottom);

	SPR_DrawGeneric(0, visX0, visY0, &cut, srcBlend, dstBlend, out.reqW, out.reqH);

	out.visX = visX0;
	out.visY = visY0;
	out.visW = visX1 - visX0;
	out.visH = visY1 - visY0;
	return out;
}

inline SprFitDraw SPR_DrawFitted(HSPRITE hspr, const Rect& rc,
	int boxX, int boxY, int boxW, int boxH,
	int srcBlend, int dstBlend)
{
	// No clip: a clip rect the size of any screen.
	return SPR_DrawFittedClipped(hspr, rc, boxX, boxY, boxW, boxH,
		-(1 << 20), -(1 << 20), 1 << 21, 1 << 21, srcBlend, dstBlend);
}
