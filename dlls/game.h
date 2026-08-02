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
extern cvar_t skill_weapon_damage_scale;

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

// Engine Cvars
inline cvar_t* g_psv_gravity;
inline cvar_t* g_psv_aim;
inline cvar_t* g_psv_allow_autoaim;
inline cvar_t* g_footsteps;
inline cvar_t* g_psv_cheats;
