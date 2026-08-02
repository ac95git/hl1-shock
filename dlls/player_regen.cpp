#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "player_regen.h"
#include "player_skills.h"
#include "game.h"
#include <algorithm>

// The beat both regenerators land on.  Fixed rather than a cvar: the rates are
// tunable, and a tunable interval as well would let the two multiply into
// something nobody chose.
static constexpr float k_RegenTickInterval = 0.25f;

// =====================================================================
// Save/restore
// =====================================================================
static TYPEDESCRIPTION g_RegenSaveData[] =
{
	DEFINE_FIELD(CPlayerRegen, m_flNextTick,    FIELD_TIME),
	DEFINE_FIELD(CPlayerRegen, m_flHealthAccum, FIELD_FLOAT),
	DEFINE_FIELD(CPlayerRegen, m_flArmorAccum,  FIELD_FLOAT),
};

bool RegenSave(CPlayerRegen& regen, CSave& save)
{
	return save.WriteFields("REGEN", &regen, g_RegenSaveData, ARRAYSIZE(g_RegenSaveData));
}

bool RegenRestore(CPlayerRegen& regen, CRestore& restore)
{
	return restore.ReadFields("REGEN", &regen, g_RegenSaveData, ARRAYSIZE(g_RegenSaveData));
}

// =====================================================================
// Clear
// =====================================================================
void CPlayerRegen::Clear()
{
	m_flNextTick    = 0;
	m_flHealthAccum = 0;
	m_flArmorAccum  = 0;
}

// =====================================================================
// Accumulate whole points from a fractional rate.
//
// Returns how many whole points are due this tick, leaving the
// remainder in 'accum' for next time.
// =====================================================================
static int TakeWholePoints(float& accum, float ratePerSecond)
{
	if (ratePerSecond <= 0.0f)
		return 0;

	accum += ratePerSecond * k_RegenTickInterval;

	const int whole = (int)accum;
	if (whole > 0)
		accum -= (float)whole;

	return whole;
}

// =====================================================================
// Think
// =====================================================================
void CPlayerRegen::Think(CBasePlayer* pPlayer)
{
	if (!pPlayer || !pPlayer->IsAlive())
		return;

	const bool bHealth = pPlayer->m_skills.HasSkill(ESkillId::HealthRegen);
	const bool bArmor  = pPlayer->m_skills.HasSkill(ESkillId::BatteryRegen);

	if (!bHealth && !bArmor)
	{
		// Nothing to do -- but drop the carried fractions, so a Skill that is
		// reset and later re-bought does not pay out a point it never earned.
		m_flHealthAccum = 0;
		m_flArmorAccum  = 0;
		return;
	}

	if (gpGlobals->time < m_flNextTick)
		return;

	m_flNextTick = gpGlobals->time + k_RegenTickInterval;

	if (bHealth)
	{
		if (pPlayer->pev->health < pPlayer->pev->max_health)
		{
			const int gained = TakeWholePoints(m_flHealthAccum, skill_health_regen_rate.value);
			if (gained > 0)
			{
				pPlayer->pev->health =
					std::min(pPlayer->pev->health + (float)gained, pPlayer->pev->max_health);
			}
		}
		else
		{
			// At full health there is nothing to carry, and carrying it would
			// dump a burst of healing the moment the player takes a scratch.
			m_flHealthAccum = 0;
		}
	}

	// Armour regenerates only for a suit that exists to hold it.
	if (bArmor && pPlayer->HasSuit())
	{
		const float maxArmor = (float)PlayerMaxArmor(pPlayer);

		if (pPlayer->pev->armorvalue < maxArmor)
		{
			const int gained = TakeWholePoints(m_flArmorAccum, skill_battery_regen_rate.value);
			if (gained > 0)
				pPlayer->pev->armorvalue = std::min(pPlayer->pev->armorvalue + (float)gained, maxArmor);
		}
		else
		{
			m_flArmorAccum = 0;
		}
	}
}
