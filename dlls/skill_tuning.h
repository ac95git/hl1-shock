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
