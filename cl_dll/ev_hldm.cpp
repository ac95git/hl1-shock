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
#include "hud.h"
#include "cl_util.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "com_weapons.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

extern engine_studio_api_t IEngineStudio;

static int tracerCount[MAX_PLAYERS];

#include "pm_shared.h"

void V_PunchAxis(int axis, float punch);
void VectorAngles(const float* forward, float* angles);

extern cvar_t* cl_lw;
extern cvar_t* r_decals;

static inline bool EV_HLDM_IsBSPModel(physent_t* pe)
{
	return pe != nullptr && (pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP);
}

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
float EV_HLDM_PlayTextureSound(int idx, pmtrace_t* ptr, float* vecSrc, float* vecEnd, int iBulletType)
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	cl_entity_t* cl_entity = NULL;
	float fvol;
	float fvolbar;
	const char* rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char* pTextureName;
	char texname[64];
	char szbuffer[64];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace(ptr);

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if (entity == 0)
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char*)gEngfuncs.pEventAPI->EV_TraceTexture(ptr->ent, vecSrc, vecEnd);

		if (pTextureName)
		{
			strcpy(texname, pTextureName);
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}

			// '}}'
			strcpy(szbuffer, pTextureName);
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// get texture type
			chTextureType = PM_FindTextureType(szbuffer);
		}
	}
	else
	{
		// JoshA: Look up the entity and find the EFLAG_FLESH_SOUND flag.
		// This broke at some point then TF:C added prediction.
		//
		// It used to use Classify of pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE
		// to determine what sound to play, but that's server side and isn't available on the client
		// and got lost in the translation to that.
		// Now the server will replicate that state via an eflag.
		cl_entity = gEngfuncs.GetEntityByIndex(entity);

		if (cl_entity && !!(cl_entity->curstate.eflags & EFLAG_FLESH_SOUND))
			chTextureType = CHAR_TEX_FLESH;
	}

	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE:
		fvol = 0.9;
		fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL:
		fvol = 0.9;
		fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:
		fvol = 0.9;
		fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:
		fvol = 0.5;
		fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE:
		fvol = 0.9;
		fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:
		fvol = 0.8;
		fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH:
		fvol = 0.9;
		fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD:
		fvol = 0.9;
		fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8;
		fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound
		fvol = 1.0;
		fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound(0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0, cnt - 1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0, 0xf));
	return fvolbar;
}

char* EV_HLDM_DamageDecal(physent_t* pe)
{
	static char decalname[32];
	int idx;

	if (pe->rendermode == kRenderTransAlpha)
		return nullptr;

	if (pe->classnumber == 1)
	{
		idx = gEngfuncs.pfnRandomLong(0, 2);
		sprintf(decalname, "{break%i", idx + 1);
	}
	else if (pe->rendermode != kRenderNormal)
	{
		sprintf(decalname, "{bproof1");
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong(0, 4);
		sprintf(decalname, "{shot%i", idx + 1);
	}
	return decalname;
}

void EV_HLDM_GunshotDecalTrace(pmtrace_t* pTrace, char* decalName)
{
	int iRand;

	gEngfuncs.pEfxAPI->R_BulletImpactParticles(pTrace->endpos);

	iRand = gEngfuncs.pfnRandomLong(0, 0x7FFF);
	if (iRand < (0x7fff / 2)) // not every bullet makes a sound.
	{
		switch (iRand % 5)
		{
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 3:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 4:
			gEngfuncs.pEventAPI->EV_PlaySound(-1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}

	// Only decal brush models such as the world etc.
	if (decalName && '\0' != decalName[0] && r_decals->value > 0)
	{
		gEngfuncs.pEfxAPI->R_DecalShoot(
			gEngfuncs.pEfxAPI->Draw_DecalIndex(gEngfuncs.pEfxAPI->Draw_DecalIndexFromName(decalName)),
			gEngfuncs.pEventAPI->EV_IndexFromTrace(pTrace), 0, pTrace->endpos, 0);
	}
}

void EV_HLDM_DecalGunshot(pmtrace_t* pTrace, int iBulletType)
{
	physent_t* pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (EV_HLDM_IsBSPModel(pe))
	{
		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			// smoke and decal
			EV_HLDM_GunshotDecalTrace(pTrace, EV_HLDM_DamageDecal(pe));
			break;
		}
	}
}

void EV_HLDM_CheckTracer(int idx, float* vecSrc, float* end, float* forward, float* right, int iBulletType, int iTracerFreq, int* tracerCount)
{
	int i;
	bool player = idx >= 1 && idx <= gEngfuncs.GetMaxClients();

	if (iTracerFreq != 0 && ((*tracerCount)++ % iTracerFreq) == 0)
	{
		Vector vecTracerSrc;

		if (player)
		{
			Vector offset(0, 0, -4);

			// adjust tracer position for player
			for (i = 0; i < 3; i++)
			{
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
			}
		}
		else
		{
			VectorCopy(vecSrc, vecTracerSrc);
		}

		switch (iBulletType)
		{
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_MONSTER_9MM:
		case BULLET_MONSTER_12MM:
		default:
			EV_CreateTracer(vecTracerSrc, end);
			break;
		}
	}
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_HLDM_FireBullets(int idx, float* forward, float* right, float* up, int cShots, float* vecSrc, float* vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int* tracerCount, float flSpreadX, float flSpreadY)
{
	int i;
	pmtrace_t tr;
	int iShot;

	for (iShot = 1; iShot <= cShots; iShot++)
	{
		Vector vecDir, vecEnd;

		float x, y, z;
		//We randomize for the Shotgun.
		if (iBulletType == BULLET_PLAYER_BUCKSHOT)
		{
			do
			{
				x = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				y = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + x * flSpreadX * right[i] + y * flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		} //But other guns already have their spread randomized in the synched spread.
		else
		{

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + flSpreadX * right[i] + flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		// JoshA: Changed from PM_STUDIO_BOX to PM_NORMAL in prediction code as otherwise if you hit an NPC or player's
		// bounding box but not one of their hitboxes, the shot won't hit on the server but it will
		// play a hit sound on the client and not make a decal (as if it hit the NPC/player).
		// We should mirror the way the server does the test here as close as possible.
		//
		// I initially thought I was just fixing some stupid Half-Life bug but no,
		// this is *the* root cause of all the ghost shot bad prediction bugs in Half-Life Deathmatch!
		//
		// Also... CStrike was always using PM_NORMAL for all of these so it didn't have the problem.
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

		EV_HLDM_CheckTracer(idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCount);

		// do damage, paint decals
		if (tr.fraction != 1.0)
		{
			switch (iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_MP5:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);
				break;
			case BULLET_PLAYER_BUCKSHOT:

				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_357:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			}
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}

//======================
//	    GLOCK START
//======================
void EV_FireGlock1(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 0);

		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

void EV_FireGlock2(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	bool empty;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	empty = 0 != args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 0);

		V_PunchAxis(0, -2.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	   GLOCK END
//======================

//======================
//	  SHOTGUN START
//======================
void EV_FireShotGunDouble(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	int j;
	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE2, 0);
		V_PunchAxis(0, -10.0);
	}

	for (j = 0; j < 2; j++)
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dbarrel1.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.17365, 0.04362);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
	}
}

void EV_FireShotGunSingle(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE, 0);

		V_PunchAxis(0, -5.0);
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0x1f));

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (gEngfuncs.GetMaxClients() > 1)
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.04362);
	}
	else
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], 0.08716, 0.08716);
	}
}
//======================
//	   SHOTGUN END
//======================

//======================
//	    MP5 START
//======================
void EV_FireMP5(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector ShellVelocity;
	Vector ShellOrigin;
	int shell;
	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl"); // brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_FIRE1 + gEngfuncs.pfnRandomLong(0, 2), 0);

		V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-2, 2));
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 2, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}

// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireMP52(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_LAUNCH, 0);
		V_PunchAxis(0, -10);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
	}
}
//======================
//		 MP5 END
//======================

//======================
//	   PHYTON START
//	     ( .357 )
//======================
void EV_FirePython(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	Vector vecSrc, vecAiming;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		// Python uses different body in multiplayer versus single player
		bool multiplayer = gEngfuncs.GetMaxClients() != 1;

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(PYTHON_FIRE1, multiplayer ? 1 : 0);

		V_PunchAxis(0, -10.0);
	}

	switch (gEngfuncs.pfnRandomLong(0, 1))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot1.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot2.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
		break;
	}

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_357, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	    PHYTON END
//	     ( .357 )
//======================

//======================
//	   GAUSS START
//======================
void EV_SpinGauss(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	pitch = args->iparam1;

	iSoundState = 0 != args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch);
}

/*
==============================
EV_StopPreviousGauss

==============================
*/
void EV_StopPreviousGauss(int idx)
{
	// Make sure we don't have a gauss spin event in the queue for this guy
	gEngfuncs.pEventAPI->EV_KillEvents(idx, "events/gaussspin.sc");
	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_WEAPON, "ambience/pulsemachine.wav");
}

extern float g_flApplyVel;

void EV_FireGauss(event_args_t* args)
{
	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;
	float flDamage = args->fparam1;

	bool m_fPrimaryFire = 0 != args->bparam1;
	Vector vecSrc;
	Vector vecDest;
	edict_t* pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	bool fHasPunched = false;
	bool fFirstBeam = true;
	int nMaxHits = 10;
	physent_t* pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	Vector up, right, forward;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	if (0 != args->bparam2)
	{
		EV_StopPreviousGauss(idx);
		return;
	}

	//	Con_Printf( "Firing gauss with %f\n", flDamage );
	EV_GetGunPosition(args, vecSrc, origin);

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	AngleVectors(angles, forward, right, up);

	VectorMA(vecSrc, 8192, forward, vecDest);

	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, -2.0);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GAUSS_FIRE2, 0);

		if (!m_fPrimaryFire)
			g_flApplyVel = flDamage;
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecDest, PM_NORMAL, -1, &tr);

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if (0 != tr.allsolid)
			break;

		if (fFirstBeam)
		{
			if (EV_IsLocal(idx))
			{
				// Add muzzle flash to current weapon model
				EV_MuzzleFlash();
			}
			fFirstBeam = false;

			gEngfuncs.pEfxAPI->R_BeamEntPoint(
				idx | 0x1000,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				(m_fPrimaryFire ? 128.0 : flDamage) / 255.0,
				0,
				0,
				0,
				(m_fPrimaryFire ? 255 : 255) / 255.0,
				(m_fPrimaryFire ? 128 : 255) / 255.0,
				(m_fPrimaryFire ? 0 : 255) / 255.0);
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc,
				tr.endpos,
				m_iBeam,
				0.1,
				m_fPrimaryFire ? 1.0 : 2.5,
				0.0,
				(m_fPrimaryFire ? 128.0 : flDamage) / 255.0,
				0,
				0,
				0,
				(m_fPrimaryFire ? 255 : 255) / 255.0,
				(m_fPrimaryFire ? 128 : 255) / 255.0,
				(m_fPrimaryFire ? 0 : 255) / 255.0);
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		if (pEntity == NULL)
			break;

		if (EV_HLDM_IsBSPModel(pEntity))
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct(tr.plane.normal, forward);

			if (n < 0.5) // 60 degrees
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				Vector r;

				VectorMA(forward, 2.0 * n, tr.plane.normal, r);

				flMaxFrac = flMaxFrac - tr.fraction;

				VectorCopy(r, forward);

				VectorMA(tr.endpos, 8.0, forward, vecSrc);
				VectorMA(vecSrc, 8192.0, forward, vecDest);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT);

				Vector fwd;
				VectorAdd(tr.endpos, tr.plane.normal, fwd);

				gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
					255, 100);

				// lose energy
				if (n == 0)
				{
					n = 0.1;
				}

				flDamage = flDamage * (1 - n);
			}
			else
			{
				// tunnel
				EV_HLDM_DecalGunshot(&tr, BULLET_MONSTER_12MM);

				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

				// limit it to one hole punch
				if (fHasPunched)
				{
					break;
				}
				fHasPunched = true;

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (!m_fPrimaryFire)
				{
					Vector start;

					VectorMA(tr.endpos, 8.0, forward, start);

					// Store off the old count
					gEngfuncs.pEventAPI->EV_PushPMStates();

					// Now add in all of the players.
					gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

					gEngfuncs.pEventAPI->EV_SetTraceHull(2);
					gEngfuncs.pEventAPI->EV_PlayerTrace(start, vecDest, PM_NORMAL, -1, &beam_tr);

					if (0 == beam_tr.allsolid)
					{
						Vector delta;
						float n;

						// trace backwards to find exit point

						gEngfuncs.pEventAPI->EV_PlayerTrace(beam_tr.endpos, tr.endpos, PM_NORMAL, -1, &beam_tr);

						VectorSubtract(beam_tr.endpos, tr.endpos, delta);

						n = Length(delta);

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;
							flDamage -= n;

							// absorption balls
							{
								Vector fwd;
								VectorSubtract(tr.endpos, forward, fwd);
								gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
									255, 100);
							}

							//////////////////////////////////// WHAT TO DO HERE
							// CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

							EV_HLDM_DecalGunshot(&beam_tr, BULLET_MONSTER_12MM);

							gEngfuncs.pEfxAPI->R_TempSprite(beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);

							// balls
							{
								Vector fwd;
								VectorSubtract(beam_tr.endpos, forward, fwd);
								gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.3), 0.1, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 200,
									255, 40);
							}

							VectorAdd(beam_tr.endpos, forward, vecSrc);
						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if (m_fPrimaryFire)
					{
						// slug doesn't punch through ever with primary
						// fire, so leave a little glowy bit and make some balls
						gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT);

						{
							Vector fwd;
							VectorAdd(tr.endpos, tr.plane.normal, fwd);
							gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat(10, 20) / 100.0, 100,
								255, 200);
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			VectorAdd(tr.endpos, forward, vecSrc);
		}
	}
}
//======================
//	   GAUSS END
//======================

//======================
//	   CROWBAR START
//======================
int g_iSwing;

//Only predict the miss sounds, hit sounds are still played
//server side, so players don't get the wrong idea.
void EV_Crowbar(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	//Play Swing sound
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM);

	if (EV_IsLocal(idx))
	{
		switch ((g_iSwing++) % 3)
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 0);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK2MISS, 0);
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK3MISS, 0);
			break;
		}
	}
}

//======================
//	  KATANA ARC START
//======================
// The Gauss Katana's swing throws a crescent -- a ")" standing in front of
// the player, bulging in the direction of the cut -- that flies forward and
// burns the first wall it meets.  Where it lands it does what the gauss gun
// does on a wall hit (EV_FireGauss above): the ordinary gunshot decal with
// the gauss glow fading over six seconds on top.  Visual only.  Nothing here
// deals damage; the swing's own trace does that, server-side.
//
// The crescent is a temp entity with no model and a per-frame callback.  Each
// frame it moves, redraws itself as a chain of short beams, and traces its
// centre and its two tips forward; the first of those to reach a wall ends
// it.  Every knob is a client cvar registered in hud.cpp, because every one
// of them is a first guess to be dialled in by eye.
//
// Storage in the tempent, since it has no fields of its own:
//   entity.baseline.vuser1  forward (unit)
//   entity.baseline.vuser2  angles, roll included, for right/up
//   entity.baseline.fuser1  speed      entity.baseline.fuser2  radius
//   entity.baseline.fuser3  sweep, degrees of circle the crescent spans
//   entity.baseline.fuser4  lean, degrees the belly turns from +right toward +forward
//   entity.baseline.vuser3  birthplace     entity.baseline.scale  range, for the fade
// Until when the katana's blade is HOT.  Set here on the swing; read every
// frame in view.cpp, which picks the viewmodel's hot skin family while it
// holds.  The hot blade is a texture with the fullbright flag, not a drawn
// effect: two earlier attempts, a beam entity between attachments and quads
// drawn by the studio renderer, were both rejected in play.
float g_flKatanaHotEnd = 0.0f;

static bool EV_KatanaArcIsWall(pmtrace_t* tr)
{
	if (tr == nullptr || tr->fraction >= 1.0f || tr->allsolid)
		return false;
	physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr->ent);
	return pe != nullptr && EV_HLDM_IsBSPModel(pe);
}

// The burn: a line of glows, one per point of the crescent, each traced onto
// the wall along the flight and left to fade over six seconds.  Neighbouring
// glows overlap, so the wall shows the crescent's silhouette along its
// flight -- a vertical line at lean 90 or 270, the ")" itself at lean 0.
// No gunshot decals: they read as bullet holes, and this is not gunfire.
//
// Every trace starts PULLED BACK from its point, because by the frame the
// leading point registers a wall it has already moved a few units into it,
// and a trace that starts inside a solid finds nothing.  The first build
// traced from the points themselves and only the trailing tips ever burned.
static void EV_KatanaArcBurnLine(Vector* pts, int count, const Vector& forward, float back, float reach, int iGlow, float fade)
{
	for (int i = 0; i < count; i++)
	{
		Vector from, to;
		VectorMA(pts[i], -back, forward, from);
		VectorMA(pts[i], reach, forward, to);
		pmtrace_t* tr = gEngfuncs.PM_TraceLine(from, to, PM_TRACELINE_PHYSENTSONLY, 2, -1);
		if (!EV_KatanaArcIsWall(tr))
			continue;
		Vector hit;
		VectorMA(tr->endpos, 1.0f, tr->plane.normal, hit); // a hair off the surface so the glow is not half inside it
		const bool anchor = (i == 0 || i == count - 1 || i == count / 2);
		const float size = (anchor ? 0.6f : 0.4f) * fade;
		gEngfuncs.pEfxAPI->R_TempSprite(hit, vec3_origin, size, iGlow, kRenderGlow, kRenderFxNoDissipation, 0.8f * fade, 6.0f * fade, FTENT_FADEOUT);
		if (anchor)
		{
			Vector fwd;
			VectorAdd(hit, tr->plane.normal, fwd);
			gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, hit, fwd, iGlow, 3, 0.1f,
				gEngfuncs.pfnRandomFloat(10.0f, 20.0f) / 100.0f, 60.0f, 255, 100.0f);
		}
	}
}

static void EV_KatanaArcThink(struct tempent_s* ent, float frametime, float currenttime)
{
	const Vector forward = ent->entity.baseline.vuser1;
	const float speed = ent->entity.baseline.fuser1;
	const float sweep = ent->entity.baseline.fuser3;

	Vector right, up, f;
	AngleVectors(ent->entity.baseline.vuser2, f, right, up);

	const Vector prev = ent->entity.origin;
	Vector origin;
	VectorMA(prev, speed * frametime, forward, origin);
	ent->entity.origin = origin;

	// It is a wave, and waves die out: over the flight it shrinks and dims
	// to nothing, so running out of range never looks like a cut.  The range
	// is long enough that a wall usually comes first.
	const Vector start = ent->entity.baseline.vuser3;
	const float range = ent->entity.baseline.scale;
	const float progress = range > 0.0f ? (origin - start).Length() / range : 1.0f;
	const float fade = 1.0f - (progress < 1.0f ? progress : 1.0f);
	if (fade <= 0.02f)
	{
		ent->die = currenttime;
		return;
	}
	const float radius = ent->entity.baseline.fuser2 * fade;

	const int iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
	const int iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	// The ")" : a circular arc, tips at the ends, centred on the line of
	// flight so the belly bulges as far one way as the tips reach the other.
	// The bulge points along `bulge`: +right for a crescent facing the player,
	// leaned toward +forward (katana_arc_lean) for one that leads with its
	// belly like a thrown blade.  The first build put the tips ON the line of
	// flight and the belly a whole radius off to the side; the crescent was
	// seen sitting beside where the player looked, not in front.
	const int segments = 10;
	const float half = sweep * 0.5f * (M_PI / 180.0f);
	const float lean = ent->entity.baseline.fuser4 * (M_PI / 180.0f);
	Vector bulge;
	VectorScale(right, cosf(lean), bulge);
	VectorMA(bulge, sinf(lean), forward, bulge);
	const float centre = radius * (1.0f + cosf(half)) * 0.5f; // midway between belly and tips
	Vector pts[segments + 1];
	for (int i = 0; i <= segments; i++)
	{
		const float theta = -half + (2.0f * half) * (float)i / (float)segments;
		Vector p;
		VectorMA(origin, radius * cosf(theta) - centre, bulge, p);
		VectorMA(p, radius * sinf(theta), up, p);
		pts[i] = p;
	}
	for (int i = 0; i < segments; i++)
	{
		// Bright at the belly, thinner toward the tips.  The gauss gun's
		// primary-fire orange, 255/128/0.
		const float t = fabsf((float)i / (float)segments - 0.5f) * 2.0f; // 0 belly .. 1 tip
		gEngfuncs.pEfxAPI->R_BeamPoints(pts[i], pts[i + 1], iBeam,
			0.06f, (4.0f - 2.5f * t) * fade, 0.15f, (1.0f - 0.4f * t) * fade, 0.0f, 0, 0.0f,
			1.0f, 0.5f, 0.0f);
	}

	// Did the belly or either tip cross a WALL this frame?  Anything else --
	// a monster, the player -- is flown through: the crescent is light.
	// A floor or ceiling is not a wall either: a tip that meets one is
	// scraped along it, leaving a glow where it touched, and the wave flies
	// on.  Aiming down at a headcrab used to end the wave on the first floor
	// tile its lower tip touched.
	Vector probes[3] = {pts[segments / 2], pts[0], pts[segments]};
	for (Vector& to : probes)
	{
		Vector from;
		VectorMA(to, -speed * frametime, forward, from);
		pmtrace_t* tr = gEngfuncs.PM_TraceLine(from, to, PM_TRACELINE_PHYSENTSONLY, 2, -1);
		if (EV_KatanaArcIsWall(tr) && fabsf(tr->plane.normal[2]) > 0.7f)
		{
			Vector scrape;
			VectorMA(tr->endpos, 1.0f, tr->plane.normal, scrape);
			gEngfuncs.pEfxAPI->R_TempSprite(scrape, vec3_origin, 0.3f * fade, iGlow, kRenderGlow, kRenderFxNoDissipation, 0.6f * fade, 3.0f * fade, FTENT_FADEOUT);
			continue;
		}
		if (EV_KatanaArcIsWall(tr))
		{
			// Points are spread along the flight by up to the crescent's whole
			// depth: the leading ones are a step into the wall, the trailing
			// ones that far short of it.  Pull every trace back by the depth
			// plus this frame's step, and reach the same distance forward.
			const float depth = 2.0f * centre * fabsf(sinf(lean)) + speed * frametime + 8.0f;
			EV_KatanaArcBurnLine(pts, segments + 1, forward, depth, depth, iGlow, fade);
			ent->die = currenttime; // spent
			return;
		}
	}
}

void EV_KatanaArc(event_args_t* args)
{
	const int idx = args->entindex;
	Vector origin, angles, vecSrc, forward, right, up;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	if (gEngfuncs.pfnGetCvarFloat("katana_arc") <= 0.0f)
		return;
	const float range = gEngfuncs.pfnGetCvarFloat("katana_arc_range");
	const float radius = gEngfuncs.pfnGetCvarFloat("katana_arc_radius");
	const float speed = gEngfuncs.pfnGetCvarFloat("katana_arc_speed");
	const float sweep = gEngfuncs.pfnGetCvarFloat("katana_arc_sweep");
	const float roll = gEngfuncs.pfnGetCvarFloat("katana_arc_roll");
	const float lean = gEngfuncs.pfnGetCvarFloat("katana_arc_lean");
	if (range <= 0.0f || radius <= 0.0f || speed <= 0.0f)
		return;

	EV_GetGunPosition(args, vecSrc, origin);
	AngleVectors(angles, forward, right, up);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON,
		gEngfuncs.pfnRandomLong(0, 1) != 0 ? "weapons/electro4.wav" : "weapons/electro5.wav",
		0.35, ATTN_NORM, 0, 90 + gEngfuncs.pfnRandomLong(0, 20));

	// The blade lights up.  Two parts: a dynamic light at the hand, which the
	// studio renderer folds into the viewmodel's lighting so the hands and the
	// blade themselves go orange, and a beam along the edge between the
	// viewmodel's two attachments, grip and tip -- for the local player the
	// engine maps a player attachment onto the viewmodel, which is how the
	// egon's beam starts at its gun.  Other players have no attachments on the
	// crowbar's p_ model, so the edge is local only.
	const float lightLife = gEngfuncs.pfnGetCvarFloat("katana_glow_light");
	if (lightLife > 0.0f)
	{
		Vector at;
		VectorMA(vecSrc, 14.0f, forward, at);
		VectorMA(at, 6.0f, right, at);
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
		if (dl != nullptr)
		{
			VectorCopy(at, dl->origin);
			dl->radius = 200.0f;
			dl->color.r = 255;
			dl->color.g = 128;
			dl->color.b = 0;
			dl->die = gEngfuncs.GetClientTime() + lightLife;
			dl->decay = dl->radius / lightLife;
		}
	}
	// The blade goes hot: view.cpp switches the viewmodel to its hot skin
	// family while this clock holds.  All the event does is start it.
	const float hotLife = gEngfuncs.pfnGetCvarFloat("katana_glow_hot");
	if (hotLife > 0.0f && EV_IsLocal(idx))
		g_flKatanaHotEnd = gEngfuncs.GetClientTime() + hotLife;

	// Born past the blade, every part of it ahead of the player: a leaned
	// crescent trails its tips behind its centre by up to its whole depth, so
	// the centre starts that much further out.  Tilted to the cut, the tilt
	// alternating so two swings in a row read as two different cuts.
	const float half = sweep * 0.5f * (M_PI / 180.0f);
	const float depth = radius * (1.0f + cosf(half)) * 0.5f * fabsf(sinf(lean * (M_PI / 180.0f)));
	Vector start;
	VectorMA(vecSrc, 20.0f + depth, forward, start);
	VectorMA(start, -4.0f, up, start);
	Vector tilted = angles;
	tilted.z += (gEngfuncs.pfnRandomLong(0, 1) != 0 ? roll : -roll);

	TEMPENTITY* arc = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(start);
	if (arc == nullptr)
		return;
	arc->flags = FTENT_NOMODEL | FTENT_CLIENTCUSTOM | FTENT_PERSIST;
	arc->callback = EV_KatanaArcThink;
	arc->die = gEngfuncs.GetClientTime() + range / speed;
	arc->entity.baseline.vuser1 = forward;
	arc->entity.baseline.vuser2 = tilted;
	arc->entity.baseline.fuser1 = speed;
	arc->entity.baseline.fuser2 = radius;
	arc->entity.baseline.fuser3 = sweep;
	arc->entity.baseline.fuser4 = lean;
	arc->entity.baseline.vuser3 = start; // where it was born, for the fade
	arc->entity.baseline.scale = range;
	arc->entity.baseline.origin = Vector(0, 0, 0); // it moves itself
}
//======================
//	   KATANA ARC END
//======================
//======================
//	   CROWBAR END
//======================

//======================
//	  CROSSBOW START
//======================
//=====================
// EV_BoltCallback
// This function is used to correct the origin and angles
// of the bolt, so it looks like it's stuck on the wall.
//=====================
void EV_BoltCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	ent->entity.origin = ent->entity.baseline.vuser1;
	ent->entity.angles = ent->entity.baseline.vuser2;
}

void EV_FireCrossbow2(event_args_t* args)
{
	Vector vecSrc, vecEnd;
	Vector up, right, forward;
	pmtrace_t tr;

	int idx;
	Vector origin;
	Vector angles;
	Vector velocity;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	VectorCopy(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	VectorMA(vecSrc, 8192, forward, vecEnd);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	if (EV_IsLocal(idx))
	{
		if (0 != args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 0);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 0);
	}

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

	//We hit something
	if (tr.fraction < 1.0)
	{
		physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);

		//Not the world, let's assume we hit something organic ( dog, cat, uncle joe, etc ).
		if (!EV_HLDM_IsBSPModel(pe))
		{
			switch (gEngfuncs.pfnRandomLong(0, 1))
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
		}
		//Stick to world but don't stick to glass, it might break and leave the bolt floating. It can still stick to other non-transparent breakables though.
		else if (pe->rendermode == kRenderNormal)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, PITCH_NORM);

			//Not underwater, do some sparks...
			if (gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_WATER)
				gEngfuncs.pEfxAPI->R_SparkShower(tr.endpos);

			Vector vBoltAngles;
			int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("models/crossbow_bolt.mdl");

			VectorAngles(forward, vBoltAngles);

			TEMPENTITY* bolt = gEngfuncs.pEfxAPI->R_TempModel(tr.endpos - forward * 10, Vector(0, 0, 0), vBoltAngles, 5, iModelIndex, TE_BOUNCE_NULL);

			if (bolt)
			{
				bolt->flags |= (FTENT_CLIENTCUSTOM);					 //So it calls the callback function.
				bolt->entity.baseline.vuser1 = tr.endpos - forward * 10; // Pull out a little bit
				bolt->entity.baseline.vuser2 = vBoltAngles;				 //Look forward!
				bolt->callback = EV_BoltCallback;						 //So we can set the angles and origin back. (Stick the bolt to the wall)
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

//TODO: Fully predict the fliying bolt.
void EV_FireCrossbow(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		if (0 != args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 0);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 0);

		V_PunchAxis(0, -2.0);
	}
}
//======================
//	   CROSSBOW END
//======================

//======================
//	    RPG START
//======================
void EV_FireRpg(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM);
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(RPG_FIRE2, 0);

		V_PunchAxis(0, -5.0);
	}
}
//======================
//	     RPG END
//======================

//======================
//	    EGON END
//======================
int g_fireAnims1[] = {EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4};
int g_fireAnims2[] = {EGON_ALTFIRECYCLE};

BEAM* pBeam;
BEAM* pBeam2;

void EV_EgonFire(event_args_t* args)
{
	int idx, iFireMode;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	iFireMode = args->iparam2;
	bool iStartup = 0 != args->bparam1;


	if (iStartup)
	{
		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100);
	}
	else
	{
		//If there is any sound playing already, kill it.
		//This is necessary because multiple sounds can play on the same channel at the same time.
		//In some cases, more than 1 run sound plays when the egon stops firing, in which case only the earliest entry in the list is stopped.
		//This ensures no more than 1 of those is ever active at the same time.
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

		if (iFireMode == FIRE_WIDE)
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125);
		else
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100);
	}

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
		gEngfuncs.pEventAPI->EV_WeaponAnimation(g_fireAnims1[gEngfuncs.pfnRandomLong(0, 3)], 0);

	if (iStartup && EV_IsLocal(idx) && !pBeam && !pBeam2 && 0 != cl_lw->value) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
	{
		Vector vecSrc, vecEnd, angles, forward, right, up;
		pmtrace_t tr;

		cl_entity_t* pl = gEngfuncs.GetEntityByIndex(idx);

		if (pl)
		{
			VectorCopy(gHUD.m_vecAngles, angles);

			AngleVectors(angles, forward, right, up);

			EV_GetGunPosition(args, vecSrc, pl->origin);

			VectorMA(vecSrc, 2048, forward, vecEnd);

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(0, 1);

			// Store off the old count
			gEngfuncs.pEventAPI->EV_PushPMStates();

			// Now add in all of the players.
			gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

			gEngfuncs.pEventAPI->EV_SetTraceHull(2);
			gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

			gEngfuncs.pEventAPI->EV_PopPMStates();

			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex(EGON_BEAM_SPRITE);

			float r = 50.0f;
			float g = 50.0f;
			float b = 125.0f;

			//if ( IEngineStudio.IsHardware() )
			{
				r /= 255.0f;
				g /= 255.0f;
				b /= 255.0f;
			}


			pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b);

			if (pBeam)
				pBeam->flags |= (FBEAM_SINENOISE);

			pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b);
		}
	}
}

void EV_EgonStop(event_args_t* args)
{
	int idx;
	Vector origin;

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

	//Only stop the sound if the event was sent by the same source as the owner of the egon.
	//If the local player owns the egon then only the local event should play this sound.
	//If another player owns it, only the server event should play it.
	if (0 != args->iparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100);

	if (EV_IsLocal(idx))
	{
		if (pBeam)
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}


		if (pBeam2)
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}

		// HACK: only reset animation if the Egon is still equipped.
		if (g_CurrentWeaponId == WEAPON_EGON)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(EGON_IDLE1, 0);
		}
	}
}
//======================
//	    EGON END
//======================

//======================
//	   HORNET START
//======================
void EV_HornetGunFire(event_args_t* args)
{
	int idx;
	Vector origin, angles;

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		V_PunchAxis(0, gEngfuncs.pfnRandomLong(0, 2));
		gEngfuncs.pEventAPI->EV_WeaponAnimation(HGUN_SHOOT, 0);
	}

	switch (gEngfuncs.pfnRandomLong(0, 2))
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100);
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100);
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100);
		break;
	}
}
//======================
//	   HORNET END
//======================

//======================
//	   TRIPMINE START
//======================
//We only check if it's possible to put a trip mine
//and if it is, then we play the animation. Server still places it.
void EV_TripmineFire(event_args_t* args)
{
	int idx;
	Vector vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	// Grab predicted result for local player
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	vecSrc = vecSrc + view_ofs;

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + forward * 128, PM_NORMAL, -1, &tr);

	//Hit something solid
	if (tr.fraction < 1.0)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(TRIPMINE_DRAW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   TRIPMINE END
//======================

//======================
//	   SQUEAK START
//======================
void EV_SnarkFire(event_args_t* args)
{
	int idx;
	Vector vecSrc, angles, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy(args->origin, vecSrc);
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	if (0 != args->ducking)
		vecSrc = vecSrc - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc + forward * 20, vecSrc + forward * 64, PM_NORMAL, -1, &tr);

	//Find space to drop the thing.
	if (tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SQUEAK_THROW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   SQUEAK END
//======================

void EV_TrainPitchAdjust(event_args_t* args)
{
	int idx;
	Vector origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	bool stop;

	char sz[256];

	idx = args->entindex;

	VectorCopy(args->origin, origin);

	us_params = (unsigned short)args->iparam1;
	stop = 0 != args->bparam1;

	m_flVolume = (float)(us_params & 0x003f) / 40.0;
	noise = (int)(((us_params) >> 12) & 0x0007);
	pitch = (int)(10.0 * (float)((us_params >> 6) & 0x003f));

	switch (noise)
	{
	case 1:
		strcpy(sz, "plats/ttrain1.wav");
		break;
	case 2:
		strcpy(sz, "plats/ttrain2.wav");
		break;
	case 3:
		strcpy(sz, "plats/ttrain3.wav");
		break;
	case 4:
		strcpy(sz, "plats/ttrain4.wav");
		break;
	case 5:
		strcpy(sz, "plats/ttrain6.wav");
		break;
	case 6:
		strcpy(sz, "plats/ttrain7.wav");
		break;
	default:
		// no sound
		strcpy(sz, "");
		return;
	}

	if (stop)
	{
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, sz);
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch);
	}
}

void EV_VehiclePitchAdjust(event_args_t* args)
{
	int idx;
	Vector origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	bool stop;

	char sz[256];

	idx = args->entindex;

	VectorCopy(args->origin, origin);

	us_params = (unsigned short)args->iparam1;
	stop = 0 != args->bparam1;

	m_flVolume = (float)(us_params & 0x003f) / 40.0;
	noise = (int)(((us_params) >> 12) & 0x0007);
	pitch = (int)(10.0 * (float)((us_params >> 6) & 0x003f));

	switch (noise)
	{
	case 1:
		strcpy(sz, "plats/vehicle1.wav");
		break;
	case 2:
		strcpy(sz, "plats/vehicle2.wav");
		break;
	case 3:
		strcpy(sz, "plats/vehicle3.wav");
		break;
	case 4:
		strcpy(sz, "plats/vehicle4.wav");
		break;
	case 5:
		strcpy(sz, "plats/vehicle6.wav");
		break;
	case 6:
		strcpy(sz, "plats/vehicle7.wav");
		break;
	default:
		// no sound
		strcpy(sz, "");
		return;
	}

	if (stop)
	{
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, sz);
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch);
	}
}

bool EV_TFC_IsAllyTeam(int iTeam1, int iTeam2)
{
	return false;
}
