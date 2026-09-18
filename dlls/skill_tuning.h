#pragma once

// =====================================================================
// Skill tuning values that BOTH DLLs read.
//
// The cvars themselves are defined and registered in game.cpp, which is
// server-only.  Weapon files compile into the client as well, for
// prediction, so they cannot name those symbols -- but they can ask the
// engine for the value by name: CVAR_GET_POINTER goes through
// g_engfuncs, which HUD_InitClientWeapons points at the engine's own
// cvar functions (cl_dll/hl/hl_weapons.cpp).  Single-player runs a
// listen server, so the game DLL and the client share one cvar registry
// and read the same number.
//
// Only knobs a PREDICTED value depends on belong here.  A Skill the
// server alone resolves -- damage, health, armour -- should keep reading
// its cvar_t from game.h under #ifndef CLIENT_DLL, because there is
// nothing for the client to agree with.
//
// The fallback is the NEUTRAL value, not the cvar's default: if the
// lookup ever fails, the client predicts as though the Skill were not
// held, which is exactly what it did before it could see Skills at all.
// That keeps this file free of a second copy of every default that would
// then have to be kept in step with game.cpp.
// =====================================================================
struct CSkillTuning
{
	const char* name;     // cvar to look up
	float       neutral;  // value that makes the Skill a no-op
	cvar_t*     cached;   // resolved on first use; cvars are never unregistered

	float Value()
	{
		if (!cached)
			cached = CVAR_GET_POINTER(name);
		return cached ? cached->value : neutral;
	}
};

// Melee Reach multiplies the 32-unit swing trace.  The client traces too --
// its copy picks which swing animation plays -- so it needs the same number or
// an unlocked player sees a miss animation for a hit that landed.
inline CSkillTuning g_tuneMeleeReach = {"skill_melee_reach_scale", 1.0f, nullptr};

// Melee Speed multiplies the swing delay, which is m_flNextPrimaryAttack and
// predicted.  Neutral is the weapon's own rate.
inline CSkillTuning g_tuneMeleeSpeed = {"skill_melee_speed_scale", 1.0f, nullptr};

// Fast Reload scales the reload delay in CBasePlayerWeapon::DefaultReload.
// m_flNextAttack is predicted frame to frame, so the two sides disagreeing
// here is visible as a hitch at the end of every reload.
inline CSkillTuning g_tuneReloadTime = {"skill_reload_time_scale", 1.0f, nullptr};

// Quick Draw scales the draw delay in CBasePlayerWeapon::DefaultDeploy, which
// has a copy on each side (dlls/weapons.cpp, cl_dll/hl/hl_weapons.cpp) and
// sets m_flNextAttack, predicted.  Same rule as Fast Reload.
inline CSkillTuning g_tuneDrawTime = {"skill_draw_time_scale", 1.0f, nullptr};

// The Gauss Katana's two swing times, as multiples of the crowbar's: the
// slash's and the wave's.  Not Skills, but the same rule applies:
// m_flNextPrimaryAttack is predicted, so the client has to see the same
// number.  Neutral means "as fast as the crowbar".
inline CSkillTuning g_tuneKatanaSwing = {"katana_swing_time_scale", 1.0f, nullptr};
inline CSkillTuning g_tuneKatanaWaveSwing = {"katana_wave_swing_time_scale", 1.0f, nullptr};

// The Carbon Pickaxe's swing time, on the same rule: a multiple of the
// crowbar's, predicted, neutral meaning "as fast as the crowbar".
inline CSkillTuning g_tunePickaxeSwing = {"pickaxe_swing_time_scale", 1.0f, nullptr};

// The recovery after a Cleave swing, in seconds, so the swipe animation plays
// whole (CCrowbar::CleaveRecovery).  Predicted like every swing delay.
// Neutral is 0, which the swing reads as "the stock delay".
inline CSkillTuning g_tuneCleaveSwingTime = {"cleave_swing_time", 0.0f, nullptr};

// The katana's wave spends uranium, and whether the right click swings at all
// depends on having it, so the check runs on both sides.  The cost, then the
// Skill that divides it.  Neutral for the cost is free, which on a failed
// lookup is the safe side: the client never refuses a swing the server
// allows.  Neutral for Energy Efficiency is no discount.
inline CSkillTuning g_tuneKatanaWaveCost = {"katana_wave_cost", 0.0f, nullptr};
inline CSkillTuning g_tuneEnergyEfficiency = {"skill_energy_efficiency_scale", 1.0f, nullptr};

// The Alien region's Hive nodes (docs/SKILL_TREE.md, "Alien").  Hive
// Attack Speed multiplies the hivehand's fire interval, which sets
// m_flNextPrimaryAttack / m_flNextSecondaryAttack and is predicted like
// Melee Speed.  Hive Replenish and the Hornet Replenish Stat nodes scale
// the regrowth rate in CHgun::Reload; that loop is server-only today
// (ammo regrowth is not itself predicted), but they read through here
// rather than game.h so the reads cannot disagree if that changes.
// skill_hive_capacity_bonus stays server-side -- see PlayerHornetMaxCarry
// in dlls/player_skills.cpp -- because the ceiling it raises is never
// something the client predicts.
inline CSkillTuning g_tuneHiveAttackSpeed = {"skill_hive_attack_speed_scale", 1.0f, nullptr};
inline CSkillTuning g_tuneHiveReplenish = {"skill_hive_replenish_scale", 1.0f, nullptr};
inline CSkillTuning g_tuneHornetReplenishStat = {"skill_stat_hornet_replenish", 0.0f, nullptr};

// The summon's cooldown and Recall, which shortens it.  The summon itself is
// the server's alone -- the spot, the cap, the ghost -- but the delay it costs
// is m_flNextPrimaryAttack, so the client has to set the same one or the
// weapon hitches at the end of every cooldown.  Neutral for the cooldown is 0,
// the safe side: on a failed lookup the client never refuses a summon the
// server allows.  Neutral for Recall is no discount.
inline CSkillTuning g_tuneSummonCooldown = {"summon_cooldown", 0.0f, nullptr};
inline CSkillTuning g_tuneRecallScale = {"skill_recall_scale", 1.0f, nullptr};
