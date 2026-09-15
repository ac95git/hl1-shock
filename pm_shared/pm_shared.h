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
// pm_shared.h
//

#pragma once

#include "Platform.h"

struct playermove_s;

void PM_Init(playermove_s* ppmove);
void PM_Move(playermove_s* ppmove, qboolean server);
char PM_FindTextureType(const char* name);

/**
*	@brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
*/
bool PM_GetHullBounds(int hullnumber, float* mins, float* maxs);

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
#define OBS_NONE 0
#define OBS_CHASE_LOCKED 1
#define OBS_CHASE_FREE 2
#define OBS_ROAMING 3
#define OBS_IN_EYE 4
#define OBS_MAP_FREE 5
#define OBS_MAP_CHASE 6

// The Dash (docs/PILLARS.md, pillar 3).  An impulse rather than a button bit
// for the Pulse's reason: usercmd_t.buttons has no spare bits, and the impulse
// rides the same per-tick packet, so the movement code sees it and predicts it.
#define DASH_IMPULSE 151

// Physinfo keys the server writes (CBasePlayer::DashThink) and the movement
// code and the HUD read.  The server owns the charges; the movement code only
// asks whether one is ready.  The burst itself runs off pmove->fuser1, the
// milliseconds left, which the client predicts.
#define DASH_KEY_READY "dsc"  // charges ready now; 0 without the Module
#define DASH_KEY_MAX "dsn"	  // charge ceiling; 0 without the Module
#define DASH_KEY_SPEED "dsv"  // burst speed, units per second
#define DASH_KEY_TIME "dst"	  // burst length, milliseconds
#define DASH_KEY_RECHARGE "dsr" // seconds for one charge to come back; the HUD's fill
#define DASH_KEY_AIR "dsa"	  // 1 with the Air Dash: the Dash works in the air, along the aim

// Placeholder, precached in ClientPrecache -- docs/ART_DEBT.md, "The Dash -- sound".
// Not the crowbar's miss: the Pulse already borrows that one.
#define DASH_SOUND "zombie/claw_miss2.wav"

extern playermove_s* pmove;

inline bool g_CheckForPlayerStuck = false;
