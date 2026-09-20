// The Shield, drawn in first person.
//
// See pulse_shield.cpp for what it is and why it is shaped this way, and
// docs/ROADMAP.md, "The Shield in first person", for the decisions behind it.
//
// Vocabulary: this is the **Shield** (CONTEXT.md).  Not a bubble, not a
// barrier, not a forcefield -- those are on that entry's Avoid list, and the
// temptation here is real because the thing genuinely is a sphere.

#pragma once

// Fed from CHudPulse's message handlers, which already parse the wire.
//
// SetState takes an EPulseState and the duration the server sent.  The Shield
// only cares about PULSE_SHIELD; every other state ends it.
void PulseShield_SetState(int iState, float flDuration);

// A blow the Shield turned away, from gmsgPulseHit.  vecFrom is where it came
// from, in world coordinates -- the same origin gmsgDamage reports.
void PulseShield_Deflect(const Vector& vecFrom);

// Clears everything.  Called from CHudPulse::Reset, so a level change or a
// respawn cannot leave a Shield standing on screen.
void PulseShield_Reset();

// Drawn from HUD_DrawTransparentTriangles (cl_dll/tri.cpp), after the world and
// after the viewmodel.  A no-op when no Shield stands.
void PulseShield_Draw();
