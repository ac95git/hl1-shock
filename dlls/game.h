/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#pragma once

extern void GameDLLInit( void );
void GameDLLShutdown();


extern cvar_t	displaysoundlist;

// multiplayer server rules
extern cvar_t fragsleft;
extern cvar_t timeleft;
extern cvar_t teamplay;
extern cvar_t fraglimit;
extern cvar_t timelimit;
extern cvar_t friendlyfire;
extern cvar_t falldamage;
extern cvar_t weaponstay;
extern cvar_t forcerespawn;
extern cvar_t flashlight;
extern cvar_t aimcrosshair;
extern cvar_t decalfrequency;
extern cvar_t teamlist;
extern cvar_t teamoverride;
extern cvar_t defaultteam;
extern cvar_t allowmonsters;
extern cvar_t allow_spectators;
extern cvar_t mp_chattime;

extern cvar_t sv_allowbunnyhopping;

extern cvar_t sv_busters;

// Inventory Grid sizing
extern cvar_t inv_rows_start;
extern cvar_t inv_rows_max;

// Skill Tree economy
extern cvar_t skill_points_start;
extern cvar_t skill_reset_tokens_start;

// Skill effects
extern cvar_t skill_health_bonus;
extern cvar_t skill_armor_ratio_scale;
extern cvar_t skill_fall_damage_scale;
extern cvar_t skill_health_regen_rate;
extern cvar_t skill_battery_regen_rate;
extern cvar_t skill_battery_bonus;
extern cvar_t skill_crowbar_range_scale;
extern cvar_t skill_crowbar_damage_scale;
extern cvar_t katana_swing_time_scale;
extern cvar_t skill_weapon_damage_scale;
extern cvar_t skill_reload_time_scale;

// The Pulse.  Every one of these is a starting guess to be judged in play.
extern cvar_t pulse_window;
extern cvar_t pulse_window_bonus;
extern cvar_t pulse_recharge_hit;
extern cvar_t pulse_recharge_miss;
extern cvar_t pulse_recharge_scale;
extern cvar_t pulse_discharge_scale;
extern cvar_t pulse_discharge_min;
extern cvar_t pulse_discharge_max;
extern cvar_t pulse_ring_style;
extern cvar_t pulse_ring_scale;
extern cvar_t pulse_deflect_punch;
extern cvar_t pulse_discharge_melee;
extern cvar_t pulse_followup_time;
extern cvar_t pulse_followup_damage;
extern cvar_t pulse_followup_knockback;

// The Infusion.  Named for the mechanic rather than the Health Syringe, so a
// later source of one does not inherit syringe-flavoured cvar names.
extern cvar_t infusion_rate;
extern cvar_t infusion_duration;
extern cvar_t infusion_duration_bonus;

// The Backstab.  Server-only: the multiplier is applied where crowbar damage is
// computed, inside the #ifndef CLIENT_DLL guard, so the client never predicts
// it and these do not belong in skill_tuning.h.
extern cvar_t backstab_damage_scale;
extern cvar_t backstab_arc_dot;

// Concealment and Suspicion.  Server-only -- the client predicts nothing about
// what a monster believes, so these do not belong in skill_tuning.h.
// See docs/PERCEPTION.md and dlls/perception.h.
extern cvar_t suspicion_enable;
extern cvar_t suspicion_fill;
extern cvar_t suspicion_drain;
extern cvar_t suspicion_notice;
extern cvar_t suspicion_acquire;
extern cvar_t noise_stance_duck;
extern cvar_t noise_stance_walk;
extern cvar_t conceal_angle_edge;
extern cvar_t conceal_dist_far;
extern cvar_t conceal_stance_duck;
extern cvar_t conceal_stance_walk;
extern cvar_t conceal_light_dark;

// ---------------------------------------------------------
// Damage debug readout.  THROWAWAY DIAGNOSTIC -- delete the cvar and both
// functions once the numbers are settled.
//
// Every damage number in this mod is a first guess (docs/PILLARS.md) and a
// multiplier the player cannot see is a multiplier nobody can tune.  With
// debug_damage set, each hit the PLAYER lands prints centred on screen:
// what was hit, how the figure was arrived at, what actually landed, and
// the health either side of it.
//
// Defined in combat.cpp, next to the damage code they report on.
// ---------------------------------------------------------
class CBaseEntity;

extern cvar_t debug_damage;

// Live Suspicion readout -- see dlls/perception.h.  Also throwaway, and it
// shares the screen centre with debug_damage.
extern cvar_t debug_suspicion;

// ---------------------------------------------------------
// Monster aim debug.  THROWAWAY DIAGNOSTIC -- delete it with the post-aggro
// step (docs/ROADMAP.md).
//
// Monster aim comes from m_vecEnemyLKP and never from pev->angles, so a
// monster can shoot through its own back.  Whether that is what is happening
// in a given fight cannot be judged by eye: a monster facing away might be
// shooting exactly where it faces -- correct -- while an unseen squadmate is
// the one hitting the player.  This prints, per shot, what is needed to tell
// those apart.
//
// Defined in combat.cpp, beside the other damage diagnostics.
// ---------------------------------------------------------
class CBaseMonster;

extern cvar_t debug_monster_aim;

// Called from CBaseMonster::ShootAtEnemy with the direction it is about to
// return.  Rate limited internally -- a grunt fires faster than anyone reads.
void DebugMonsterAimShot(CBaseMonster* pMonster, const Vector& vecShootDir);

// Called where a shot lands on the PLAYER, so the readout can name who
// actually hit them rather than whoever happened to be on screen.
void DebugMonsterAimNoteHit(entvars_t* pevAttacker);

// Stash a breakdown for the next report, printf-style.  Called by a weapon
// that still holds its individual multipliers as separate numbers -- by the
// time damage reaches TakeDamage they have all been folded into one float.
void DebugDamageDetail(const char* fmt, ...);

// Called where damage actually lands, with the figure that arrived.
void DebugDamageReport(CBaseEntity* pVictim, entvars_t* pevAttacker, float flDamage, float flHealthBefore);

// Engine Cvars
inline cvar_t* g_psv_gravity;
inline cvar_t* g_psv_aim;
inline cvar_t* g_psv_allow_autoaim;
inline cvar_t* g_footsteps;
inline cvar_t* g_psv_cheats;
