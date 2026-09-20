// The Shield, drawn in first person.
//
// Replaces the flat full-screen rectangle CHudPulse used to fill while a Shield
// stood (hud_pulse.cpp, hud_pulse_tint -- now defaulted to 0 and kept only as a
// fallback).  Andrei, 2026-09-20: "instead of 'seeing blue' like the current
// functionality, a Shield rapidly forming around you and vanishing is much
// cooler."  The motion is the whole point; the colour and the (absent) texture
// are not.  docs/ROADMAP.md, "The Shield in first person", has every decision.
//
// Vocabulary: this is the **Shield** (CONTEXT.md), whose Avoid list includes
// bubble, barrier and forcefield.  The geometry below really is a sphere, so
// the temptation is constant.  Resist it in comments and commit messages.
//
// The shape.  A sphere centred on the eye, so the player is INSIDE it.  That is
// not decoration: CPlayerPulse::WouldNegate takes no direction at all, so a
// shotgun in the back is negated exactly as one in the face.  Anything drawn in
// front of the player would teach a facing rule the game never rewards.
//
// The motion.  The geometry never moves.  What travels is the boundary between
// "Shield" and "nothing", spreading from the crosshair out to full coverage and
// then retreating the same way -- one bright edge out, the same edge back.  The
// retreat winks out at the crosshair, which is the player's "you are
// unprotected now" cue, delivered where their eyes already are.  A trailing-edge
// sweep looked better and was rejected for lying: it empties the centre of the
// view while the Shield is still mechanically up.
//
// The sweep is a FIXED duration at both ends; a longer window (Pulse Window,
// 0.25s -> 0.40s) buys hold time only.  Sweep speed is a property of the suit,
// not of the player's build, and an entrance that looks identical every time is
// what makes it readable as "the press registered".
//
// The falloff is AUTHORED, not physical.  It was argued during the design as
// "what the inside of a sphere looks like, oblique at the edges" -- that
// reasoning is wrong and is corrected here so nobody tunes against it: from the
// exact centre of a sphere every surface element is face-on, and the path length
// through a thin shell is the same in every direction.  There is no fresnel to
// reproduce.  The centre is kept clear because the player needs to see what they
// are shooting during the one quarter-second that matters, and that is the whole
// justification.  pulse_shield_spread and _falloff shape it by eye.
//
// Raw GL rather than the triangle API, which is the reverse of what the design
// doc first assumed, for two reasons that only became clear in the writing.  v1
// has no texture at all, so pTriAPI's one real service -- binding a sprite -- is
// not wanted; and turning the depth test OFF is the entire point of where this
// draws, which the API gives no way to do.  Hardware renderer only, like the
// katana trail (katana_trail.cpp) and the blade's cooling; the software renderer
// has no GL context and draws nothing, which is exactly why hud_pulse_tint was
// kept rather than deleted.
//
// No depth test, so it never clips.  shaft1 and minemap are tight corridors
// where walls sit nearer than any useful radius, and a Shield sliced open by the
// wall it is protecting you from reads as a bug rather than as physics.  The
// accepted cost is that it draws over the viewmodel -- and the dividend is that
// the weapon is washed in the suit's colour on every press, on every weapon,
// which is a free preview of the energised-gear treatment the Defense Matrix is
// queued to get.

#include "PlatformHeaders.h"
#include <GL/gl.h>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "r_studioint.h"

#include "pulse_shield.h"

#include <cmath>

extern engine_studio_api_t IEngineStudio;

// The real view origin and angles, from view.cpp:70.  gHUD.m_vecOrigin is the
// player's, which is not where the camera is -- and this sphere is centred on
// the camera, so the difference would show as the whole pattern sitting off
// centre.
extern Vector v_origin;
extern Vector v_angles;

namespace
{
// Mirrors EPulseState in dlls/player_pulse.h, as hud_pulse.cpp does.
constexpr int kPulseShield = 1;

constexpr float kPi = 3.14159265358979323846f;

// Tessellation.  Fixed rather than cvar'd: the only thing it changes is how
// polygonal the travelling edge looks, and 48x24 is already past the point
// where that is visible.  1225 vertices, recoloured once a frame.
constexpr int kSegments = 48; // around
constexpr int kRings = 24;	  // pole to pole
constexpr int kVerts = (kSegments + 1) * (kRings + 1);

// Nominal radius.  Deliberately NOT a cvar: with the depth test off and the
// sphere centred on the eye, the radius has no visual effect whatsoever -- only
// the direction of each vertex matters.  It exists so the geometry is real
// world-space geometry rather than a screen-space trick, and so that turning the
// depth test back on one day would do something sensible.
constexpr float kRadius = 32.0f;

struct cvar_s* g_pCvarEnable = nullptr;
struct cvar_s* g_pCvarSweep = nullptr;
struct cvar_s* g_pCvarAlpha = nullptr;
struct cvar_s* g_pCvarEdge = nullptr;
struct cvar_s* g_pCvarEdgeWidth = nullptr;
struct cvar_s* g_pCvarSpread = nullptr;
struct cvar_s* g_pCvarFalloff = nullptr;
struct cvar_s* g_pCvarCentre = nullptr;
struct cvar_s* g_pCvarFlare = nullptr;
struct cvar_s* g_pCvarLive = nullptr;

bool g_bRegistered = false;

// ---- State -------------------------------------------------------------

bool g_bUp = false;		  // a Shield stands
float g_flStart = 0;	  // client time it went up
float g_flDuration = 0;	  // how long the server said it would stand
Vector g_vecLatched;	  // view forward at the press, for pulse_shield_live 0

// The deflect flare, in vanilla's four quadrants.  See PulseShield_Deflect.
float g_flFront = 0;
float g_flRear = 0;
float g_flLeft = 0;
float g_flRight = 0;

// Unit sphere directions, built once.  World axes, so the poles are up and down
// in the world -- which is free with no texture, and is the reason the texture
// entry in docs/ART_DEBT.md flags pole distortion as a problem for later.
Vector g_dirs[kVerts];
bool g_bBuilt = false;

void BuildSphere()
{
	if (g_bBuilt)
		return;
	g_bBuilt = true;

	for (int i = 0; i <= kRings; i++)
	{
		const float theta = kPi * (float)i / (float)kRings;
		const float st = sinf(theta);
		const float ct = cosf(theta);

		for (int j = 0; j <= kSegments; j++)
		{
			const float phi = 2.0f * kPi * (float)j / (float)kSegments;
			g_dirs[i * (kSegments + 1) + j] = Vector(st * cosf(phi), st * sinf(phi), ct);
		}
	}
}

void RegisterCvars()
{
	if (g_bRegistered)
		return;
	g_bRegistered = true;

	// Every number here is a first guess to be judged in play, which is why they
	// are cvars -- pulse_ring_scale's precedent (dlls/game.cpp).
	g_pCvarEnable = CVAR_CREATE("pulse_shield", "1", FCVAR_ARCHIVE);
	// How long the edge takes to cross the view, each way.
	//
	// 0.5 on Andrei's call after the first sighting, up from 0.08.  Note what
	// that does at the default window: the sweep is clamped below to half the
	// window's length, and half of 0.25s is 0.125s, so a default Pulse now
	// spends its ENTIRE life travelling -- out for half, back for half, with no
	// hold at all.  The "fixed sweep, variable hold" decision
	// (docs/ROADMAP.md) is thereby inert at this value: everything scales with
	// the window again, which was the option it was chosen over.  Left as asked
	// because it is judged by eye and it plainly looks better; the decision is
	// still the right shape if the window ever grows, and dropping this back
	// under half the window restores it.
	g_pCvarSweep = CVAR_CREATE("pulse_shield_sweep", "0.5", FCVAR_ARCHIVE);
	// Brightness of the Shield's body where it is fully dense.
	g_pCvarAlpha = CVAR_CREATE("pulse_shield_alpha", "0.55", FCVAR_ARCHIVE);
	// Brightness of the travelling edge, and its angular width in radians.
	g_pCvarEdge = CVAR_CREATE("pulse_shield_edge", "1.1", FCVAR_ARCHIVE);
	g_pCvarEdgeWidth = CVAR_CREATE("pulse_shield_edge_width", "0.30", FCVAR_ARCHIVE);
	// The angle off the crosshair, in degrees, at which the body reaches full
	// density, and the curve it takes getting there.
	g_pCvarSpread = CVAR_CREATE("pulse_shield_spread", "60", FCVAR_ARCHIVE);
	g_pCvarFalloff = CVAR_CREATE("pulse_shield_falloff", "1.5", FCVAR_ARCHIVE);
	// What is left at the crosshair itself.  0 is perfectly clear.
	g_pCvarCentre = CVAR_CREATE("pulse_shield_centre", "0.08", FCVAR_ARCHIVE);
	// The deflect flare's brightness.
	g_pCvarFlare = CVAR_CREATE("pulse_shield_flare", "1.0", FCVAR_ARCHIVE);
	// 1 keeps the sweep's origin on the live crosshair; 0 latches the direction
	// in world space at the press, so turning during the sweep leaves it where
	// it started.  Andrei took live on the grounds that the Pulse is over too
	// quickly for a turn to matter; this exists so that can be judged rather
	// than argued.  See docs/ROADMAP.md.
	g_pCvarLive = CVAR_CREATE("pulse_shield_live", "1", FCVAR_ARCHIVE);
}

float CvarOr(struct cvar_s* pCvar, float flDefault)
{
	return pCvar ? pCvar->value : flDefault;
}

float Clamp01(float f)
{
	return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
}
} // namespace

//=========================================================
// PulseShield_Reset
//=========================================================
void PulseShield_Reset()
{
	g_bUp = false;
	g_flStart = 0;
	g_flDuration = 0;
	g_flFront = g_flRear = g_flLeft = g_flRight = 0;
}

//=========================================================
// PulseShield_SetState
//
// The Shield only exists for PULSE_SHIELD.  Every other state -- recharging,
// ready, no Module at all -- ends it immediately rather than letting it play
// out, because the server has already said it is down.
//=========================================================
void PulseShield_SetState(int iState, float flDuration)
{
	if (iState != kPulseShield)
	{
		g_bUp = false;
		return;
	}

	g_bUp = true;
	g_flStart = gHUD.m_flTime;
	g_flDuration = flDuration > 0.0f ? flDuration : 0.25f;

	// Latch the look direction for pulse_shield_live 0.  Captured whether or not
	// that cvar is on, so flipping it mid-game cannot find a stale vector.
	Vector forward, right, up;
	AngleVectors(v_angles, forward, right, up);
	g_vecLatched = forward;

	// A new Shield starts with a clean compass.  Flares belong to the window
	// they were struck in.
	g_flFront = g_flRear = g_flLeft = g_flRight = 0;
}

//=========================================================
// PulseShield_Deflect
//
// A blow the Shield turned away, from gmsgPulseHit.
//
// This is CHudHealth::CalcDamageDirection (cl_dll/health.cpp:238) reimplemented
// deliberately rather than called, because the vanilla one writes into the
// health element's own members and those drive the pain trapezoids.  Sharing
// them would make a parried hit draw the damage compass, which is the exact
// signal this effect exists to keep separate.
//
// The maths is otherwise vanilla's, Andrei's call, including the parts that look
// like accidents and are not:
//
//   - the 0.3 threshold, below which a quadrant does not register at all
//   - V_max accumulation, so several hits inside one window stack instead of
//     replacing each other.  That is correct here: the window never closes
//     early (CPlayerPulse::Think), so a burst from two directions genuinely is
//     several deflects in one Shield
//   - inside 50 units, ALL FOUR quadrants light at once, so a melee deflect
//     floods the whole Shield rather than picking a side.  A claw at arm's
//     length is not "from the left"
//
// One deliberate departure: vanilla stops drawing a quadrant below 0.4 and
// snaps it to zero (health.cpp:311, :324).  That is right for a sprite, which
// is on or off; it would be a visible pop on a gradient.  The flare here fades
// all the way out instead.  The decay RATE is vanilla's.
//
// Note the trap in the original: at health.cpp:262-263 `front` holds the RIGHT
// dot and `side` holds the FORWARD dot.  The names are backwards and the
// behaviour is correct.  Named honestly here.
//=========================================================
void PulseShield_Deflect(const Vector& vecFrom)
{
	if (!g_bUp)
		return;

	Vector vecDir = vecFrom - v_origin;
	const float flDist = vecDir.Length();

	if (flDist <= 0.0f)
	{
		// No direction at all -- the server reports a hit with no inflictor from
		// the player's own origin, and vanilla reads that as everywhere at once.
		g_flFront = g_flRear = g_flLeft = g_flRight = 1.0f;
		return;
	}

	vecDir = vecDir.Normalize();

	Vector forward, right, up;
	AngleVectors(v_angles, forward, right, up);

	const float flForward = DotProduct(vecDir, forward);
	const float flRight = DotProduct(vecDir, right);

	if (flDist <= 50.0f)
	{
		g_flFront = g_flRear = g_flLeft = g_flRight = 1.0f;
		return;
	}

	if (flForward > 0.3f)
		g_flFront = V_max(g_flFront, flForward);
	else if (-flForward > 0.3f)
		g_flRear = V_max(g_flRear, -flForward);

	if (flRight > 0.3f)
		g_flRight = V_max(g_flRight, flRight);
	else if (-flRight > 0.3f)
		g_flLeft = V_max(g_flLeft, -flRight);
}

//=========================================================
// PulseShield_Draw
//=========================================================
void PulseShield_Draw()
{
	RegisterCvars();

	if (!g_bUp)
		return;
	if (CvarOr(g_pCvarEnable, 1.0f) <= 0.0f)
		return;
	// Hardware only: the software renderer has no GL context.  hud_pulse_tint is
	// the fallback there, which is why it was kept rather than deleted.
	if (0 == IEngineStudio.IsHardware())
		return;
	// Same hide rules the readout obeys (CHudPulse::Draw).
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_HEALTH)) != 0)
		return;

	const float flNow = gHUD.m_flTime;
	const float flElapsed = flNow - g_flStart;

	if (flElapsed < 0.0f)
	{
		// The clock went backwards -- a load, or a level change.  Nothing sane
		// to draw.
		g_bUp = false;
		return;
	}

	// The window may run slightly past what the server predicted before the next
	// state message lands; hold full coverage rather than flickering off.
	const float flDuration = g_flDuration;

	// Sweep time, clamped so a short window cannot ask for an entrance and an
	// exit longer than the window itself.
	float flSweep = V_max(0.01f, CvarOr(g_pCvarSweep, 0.08f));
	if (flSweep * 2.0f > flDuration)
		flSweep = flDuration * 0.5f;

	const float flEdgeWidth = V_max(0.01f, CvarOr(g_pCvarEdgeWidth, 0.30f));

	// Full coverage is PI plus the edge's width, not PI.
	//
	// The fill ramps in across the edge width -- (front - angle) / width -- so a
	// boundary parked at exactly PI leaves the antipode, the point directly
	// OPPOSITE the sweep's origin, at zero fill: a soft hole one edge-width
	// across that never closes.  Live, the origin tracks the view and the hole
	// stays behind the player forever; latched, turning around puts it in plain
	// sight.  Overshooting by the edge width closes it, and costs nothing --
	// there is no geometry past PI for the extra travel to light.
	const float flFull = kPi + flEdgeWidth;

	// Where the boundary is, as an angle from the sweep's origin: 0 is nothing,
	// flFull is full coverage.  Out over the first sweep, held, then back.
	float flFront;
	if (flElapsed < flSweep)
		flFront = flFull * (flElapsed / flSweep);
	else if (flElapsed < flDuration - flSweep)
		flFront = flFull;
	else if (flElapsed < flDuration)
		flFront = flFull * ((flDuration - flElapsed) / flSweep);
	else
		flFront = 0.0f;

	if (flFront <= 0.0f)
		return;

	BuildSphere();

	Vector forward, right, up;
	AngleVectors(v_angles, forward, right, up);

	// Live follows the crosshair; latched stays where the press put it.
	const Vector vecOrigin = CvarOr(g_pCvarLive, 1.0f) > 0.0f ? forward : g_vecLatched;

	int sr, sg, sb;
	UnpackRGB(sr, sg, sb, RGB_SUIT);
	const float flSuitR = (float)sr;
	const float flSuitG = (float)sg;
	const float flSuitB = (float)sb;

	const float flAlpha = CvarOr(g_pCvarAlpha, 0.55f);
	const float flEdge = CvarOr(g_pCvarEdge, 1.1f);
	const float flCentre = Clamp01(CvarOr(g_pCvarCentre, 0.08f));
	const float flFalloff = V_max(0.01f, CvarOr(g_pCvarFalloff, 1.5f));
	const float flFlare = CvarOr(g_pCvarFlare, 1.0f);

	// The body reaches full density this far off the crosshair.  Expressed as
	// 1 - cos so it can be compared against a dot product without a trig call
	// per vertex.
	const float flSpreadDeg = V_max(5.0f, CvarOr(g_pCvarSpread, 60.0f));
	const float flSpreadRef = V_max(0.01f, 1.0f - cosf(flSpreadDeg * kPi / 180.0f));

	// Colours, one pass over the vertices.
	static float s_colours[kVerts][3];

	for (int v = 0; v < kVerts; v++)
	{
		const Vector& d = g_dirs[v];

		// Angle from the sweep's origin decides fill and edge.
		float flDotOrigin = DotProduct(d, vecOrigin);
		flDotOrigin = flDotOrigin < -1.0f ? -1.0f : (flDotOrigin > 1.0f ? 1.0f : flDotOrigin);
		const float flAng = acosf(flDotOrigin);

		// Filled behind the boundary, ramping in across the edge's width so the
		// transition is not a polygon.
		const float flFill = Clamp01((flFront - flAng) / flEdgeWidth);

		// The travelling edge itself, brightest exactly on the boundary.
		float flEdgeAmt = Clamp01(1.0f - fabsf(flAng - flFront) / flEdgeWidth);
		flEdgeAmt *= flEdgeAmt;

		// The body's authored falloff: clear at the crosshair, dense away from
		// it.  See the file header -- this is readability, not optics.
		const float flOff = Clamp01((1.0f - DotProduct(d, forward)) / flSpreadRef);
		const float flBody = flCentre + (1.0f - flCentre) * powf(flOff, flFalloff);

		float flIntensity = flFill * flBody * flAlpha + flFill * flEdgeAmt * flEdge;

		// The deflect flare: a broad cosine lobe per quadrant, summed.  Each
		// lobe covers a hemisphere, which is what makes this a region of the
		// Shield lighting up rather than a patch appearing on it.
		float flHit = 0.0f;
		if (flFlare > 0.0f)
		{
			const float fwdDot = DotProduct(d, forward);
			const float rgtDot = DotProduct(d, right);

			if (fwdDot > 0.0f)
				flHit += g_flFront * fwdDot;
			else
				flHit += g_flRear * -fwdDot;

			if (rgtDot > 0.0f)
				flHit += g_flRight * rgtDot;
			else
				flHit += g_flLeft * -rgtDot;

			flHit *= flFlare * flFill;
		}

		// White-hot: the flare adds the Shield's own hue at low intensity and
		// pushes toward white as it climbs.  Luminance is the only axis that
		// reads against every Suit Variant -- a fixed accent colour picked to
		// pop against blue can vanish against amber.
		const float flWhite = Clamp01(flHit);
		const float flHitR = (flSuitR * (1.0f - flWhite) + 255.0f * flWhite) * flHit;
		const float flHitG = (flSuitG * (1.0f - flWhite) + 255.0f * flWhite) * flHit;
		const float flHitB = (flSuitB * (1.0f - flWhite) + 255.0f * flWhite) * flHit;

		s_colours[v][0] = (flSuitR * flIntensity + flHitR) / 255.0f;
		s_colours[v][1] = (flSuitG * flIntensity + flHitG) / 255.0f;
		s_colours[v][2] = (flSuitB * flIntensity + flHitB) / 255.0f;
	}

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); // purely additive: the colour carries everything
	glDisable(GL_DEPTH_TEST);	 // never clipped by the room, and over the viewmodel
	glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE); // seen from inside, so the winding is backwards
	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
	for (int i = 0; i < kRings; i++)
	{
		for (int j = 0; j < kSegments; j++)
		{
			const int a = i * (kSegments + 1) + j;
			const int b = a + 1;
			const int c = b + (kSegments + 1);
			const int e = a + (kSegments + 1);
			const int quad[4] = {a, b, c, e};

			for (int k = 0; k < 4; k++)
			{
				const int idx = quad[k];
				glColor3fv(s_colours[idx]);
				const Vector p = v_origin + g_dirs[idx] * kRadius;
				glVertex3fv(p);
			}
		}
	}
	glEnd();

	glPopAttrib();

	// Decay the flare at vanilla's rate (health.cpp:308, m_flTimeDelta * 2),
	// but all the way to zero rather than snapping off at 0.4 the way a sprite
	// does.
	const float flFade = (float)gHUD.m_flTimeDelta * 2.0f;
	g_flFront = V_max(0.0f, g_flFront - flFade);
	g_flRear = V_max(0.0f, g_flRear - flFade);
	g_flLeft = V_max(0.0f, g_flLeft - flFade);
	g_flRight = V_max(0.0f, g_flRight - flFade);
}
