#pragma once

// ---------------------------------------------------------
// The Pulse key on the wire.
//
// The Pulse is an impulse, not a button bit: usercmd_t.buttons is an
// unsigned short and common/in_buttons.h already spends all 16 usable
// bits (docs/TECH_DEBT.md).  An impulse rides the same per-tick packet,
// so its timing is a button's, but it is edge-triggered and carries no
// release -- and the Defense Matrix (the Juggernaut Route) needs to know
// how long the key is HELD.  So the key is a +pulse / -pulse pair on the
// client (cl_dll/input.cpp) that sends one impulse on the press and a
// second on the release; the server (CPlayerPulse::OnPress / OnRelease)
// times the hold between them.  A bare "impulse 150" bind still works
// as a tap: it fires the Shield and can never raise the Matrix.
//
// Compiled into both DLLs, so the two sides cannot disagree.
// ---------------------------------------------------------
#define PULSE_IMPULSE 150         // the key went down: the tap's Shield, and the start of a hold
#define PULSE_RELEASE_IMPULSE 152 // the key came up: the end of a hold
