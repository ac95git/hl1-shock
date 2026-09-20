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
extern cvar_t item_debug;

// Records: the unread glow -- shell thickness on a loose document, halo
// scale on a brush one, 0 on either turning that form off -- and how far
// the player may drift before the reader shuts.
extern cvar_t record_glow_shell;
extern cvar_t record_glow_halo;
extern cvar_t record_read_range;

// Skill Tree economy
extern cvar_t skill_points_start;
extern cvar_t skill_reset_tokens_start;
// Debugging aid: while set, the whole tree is held.  See game.cpp.
extern cvar_t skill_unlock_all;

// Skill effects
extern cvar_t skill_health_bonus;
extern cvar_t skill_armor_ratio_scale;
extern cvar_t skill_fall_damage_scale;
extern cvar_t skill_battery_bonus;
extern cvar_t skill_melee_reach_scale;
extern cvar_t skill_melee_force_scale;
extern cvar_t skill_melee_speed_scale;
extern cvar_t skill_stat_melee_damage;
extern cvar_t skill_stat_max_health;
extern cvar_t skill_stat_max_armor;
extern cvar_t skill_backstab_bonus_scale;
extern cvar_t cleave_cooldown;
extern cvar_t cleave_arc_dot;
extern cvar_t cleave_radius;
extern cvar_t cleave_damage_scale;
extern cvar_t cleave_swing_time;
extern cvar_t katana_swing_time_scale;
extern cvar_t katana_wave_swing_time_scale;
extern cvar_t katana_wave_damage;
extern cvar_t katana_wave_range;
extern cvar_t katana_wave_full_range;
extern cvar_t katana_wave_speed;
extern cvar_t katana_wave_cost;
extern cvar_t katana_wave_blade_damage;
extern cvar_t deposit_arc_period;
extern cvar_t deposit_arc_warn;
extern cvar_t deposit_arc_damage;
extern cvar_t deposit_arc_radius;
extern cvar_t panther_spot_cone;
extern cvar_t panther_spot_dwell;
extern cvar_t panther_stalk_crawl_dist;
extern cvar_t panther_growl_pitch;
extern cvar_t panther_growl_volume;
extern cvar_t panther_growl_interval;
extern cvar_t panther_leap_min;
extern cvar_t panther_leap_max;
extern cvar_t panther_leap_speed;
extern cvar_t panther_leap_cooldown;
extern cvar_t panther_leap_windup;
extern cvar_t panther_debug;
extern cvar_t skill_weapon_damage_scale;
extern cvar_t skill_reload_time_scale;
extern cvar_t skill_marksman_scale;
extern cvar_t skill_stat_bullet_damage;
extern cvar_t skill_draw_time_scale;
extern cvar_t skill_demolitions_scale;
extern cvar_t skill_demolitions_resist_scale;
extern cvar_t skill_headhunter_scale;
extern cvar_t skill_swap_surge_scale;
extern cvar_t skill_swap_surge_window;
extern cvar_t skill_swap_surge_cooldown;
extern cvar_t skill_stat_healing;
extern cvar_t skill_leech_fraction;
extern cvar_t skill_overheal_cap;
extern cvar_t skill_overheal_decay;
extern cvar_t skill_energy_damage_scale;
extern cvar_t skill_stat_energy_damage;
extern cvar_t skill_energy_efficiency_scale;
extern cvar_t skill_insulation_scale;
extern cvar_t skill_overdraw_armor_per_uranium;
extern cvar_t skill_overdraw_floor;
extern cvar_t skill_overdraw_damage_scale;
extern cvar_t skill_ricochet_chance;

// Overdraw's armour drain: defined in dlls/player_skills.cpp.  Declared
// here rather than in player_skills.h so the egon and the katana -- which
// already include this header under #ifndef CLIENT_DLL for the cvars above
// -- reach it with no new include.
class CBasePlayer;
void OverdrawSpendArmor(CBasePlayer* pPlayer, int uraniumSpent);
// Last Stand, the Medical major, and Glass Cannon, the keystone that arms it
// permanently (docs/SKILL_TREE.md, "The keystone").  First guesses.
extern cvar_t skill_last_stand_invuln;
extern cvar_t skill_last_stand_cooldown;
extern cvar_t skill_last_stand_low_health;
extern cvar_t skill_last_stand_heal_scale;
extern cvar_t skill_glass_cannon_max_health;

// The Dash and the Shinobi Route.  First guesses.
extern cvar_t dash_speed;
extern cvar_t dash_time;
extern cvar_t dash_recharge;
extern cvar_t skill_dash_reach_scale;
extern cvar_t skill_dash_recovery;
extern cvar_t skill_stat_dash_recovery;

// The Pulse.  Every one of these is a starting guess to be judged in play.
extern cvar_t pulse_window;
extern cvar_t pulse_window_bonus;
extern cvar_t pulse_recharge_hit;
extern cvar_t pulse_recharge_miss;
extern cvar_t pulse_tail_scale;
extern cvar_t pulse_recharge_scale;
extern cvar_t pulse_discharge_scale;
extern cvar_t pulse_discharge_min;
extern cvar_t pulse_discharge_max;
extern cvar_t pulse_deflect_punch;
extern cvar_t pulse_discharge_melee;
extern cvar_t pulse_followup_time;
extern cvar_t pulse_followup_damage;
extern cvar_t pulse_followup_knockback;

// The Defense Matrix (the Juggernaut Route).  See dlls/player_pulse.cpp.
extern cvar_t skill_matrix_hold;
extern cvar_t skill_matrix_duration;
extern cvar_t skill_matrix_cooldown;
extern cvar_t skill_matrix_armor_cost_scale;
extern cvar_t skill_matrix_speed_scale;
extern cvar_t skill_matrix_kill_armor;
extern cvar_t skill_matrix_grant;

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
extern cvar_t suspicion_witness;     // a witness's jump, and the give-up's
extern cvar_t suspicion_floor;       // the permanent floor after either
extern cvar_t disturbance_volume;    // a body's hearing radius, in units
extern cvar_t disturbance_duration;  // and how long it stays in the sound list
extern cvar_t noise_stance_duck;
extern cvar_t noise_stance_walk;
extern cvar_t conceal_angle_edge;
extern cvar_t conceal_dist_far;
extern cvar_t conceal_stance_duck;
extern cvar_t conceal_stance_walk;
extern cvar_t conceal_light_dark;

// The Stealth region (docs/SKILL_TREE.md, "Stealth").  Server-only, like the
// perception cvars they modify: every one of them changes what a monster
// believes or what the server deals, never anything the client predicts.
extern cvar_t skill_stat_concealment;     // each Concealment Stat node: fill this much slower
extern cvar_t skill_soft_step_scale;      // Soft Step: the crouch/walk body noise, scaled again
extern cvar_t skill_nightfall_scale;      // Nightfall: the light term's dark end, scaled down
extern cvar_t skill_slip_away_fraction;   // Slip Away: the share of Suspicion lost on a break of contact
extern cvar_t skill_ambush_spotted_scale; // Ambush: on a victim below Spotted
extern cvar_t skill_ambush_noticed_scale; // Ambush: on a victim below Noticed (replaces, not stacks)
extern cvar_t skill_shroud_scale;         // Shroud: a flat multiplier on the fill for the holder
extern cvar_t skill_phantom_duration;     // Phantom: seconds of silence and speed after an unseen Backstab
extern cvar_t skill_phantom_speed_scale;  // Phantom: the player's maxspeed during it

// The Alien region's Hive nodes (docs/SKILL_TREE.md, "Alien").  The capacity
// bonus is server-side; the replenish and attack-speed scales are read by the
// hivehand on both sides through dlls/skill_tuning.h, since its cadence is
// predicted.  Defined here so the names live in one place.
extern cvar_t skill_hive_capacity_bonus;     // Hive Capacity: extra hornets carried
extern cvar_t skill_hive_replenish_scale;    // Hive Replenish: the regrowth rate, above 1 is faster
extern cvar_t skill_hive_attack_speed_scale; // Hive Attack Speed: the fire interval, below 1 is faster
extern cvar_t skill_stat_hornet_replenish;   // each Hornet Replenish Stat node adds this to the rate

// The alien Module's summon weapon and its ghosts (docs/ROADMAP.md, "The
// summon weapon -- settled").  A summon is resolved entirely on the server --
// the spot search, the cap and the ghost itself -- so these live here; the
// cooldown and Recall also have a reading in dlls/skill_tuning.h, because the
// delay they set is m_flNextPrimaryAttack and the client predicts that.
// Every number is a first guess.
extern cvar_t summon_cooldown;        // seconds between summons, before Recall
extern cvar_t summon_max_ghosts;      // ghosts out at once, before Pack
extern cvar_t summon_ghost_lifetime;  // seconds a ghost stands, before Tether
extern cvar_t skill_pack_bonus;       // Pack: ghosts the cap gains
extern cvar_t skill_tether_scale;     // Tether: multiplies the lifetime
extern cvar_t skill_recall_scale;     // Recall: multiplies the cooldown, below 1 is sooner

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
extern cvar_t debug_schedule;  // the monster under the crosshair: state, squad role, schedule, task, meter; the last kill and Search
extern cvar_t debug_invisible; // no monster can see or hear the player

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
