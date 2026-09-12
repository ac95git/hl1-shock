//=========================================================
// player_infusion.cpp
//
// The Infusion.  See player_infusion.h and docs/PILLARS.md pillar 3.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "player_infusion.h"
#include "UserMessages.h"
#include "game.h"

//=========================================================
// How often a tick lands.
//
// Deliberately a constant and not a cvar: it is coupled to the
// accumulator below and there is nothing to learn from tuning it.
// 0.25s is what makes the health readout climb steadily rather
// than jump -- at one tick a second an Infusion is indistinguishable
// from someone eating medkits.
//
// A power of two, so the repeated += below stays exact and the last
// tick of a whole-second duration lands precisely on the end time.
//=========================================================
static constexpr float k_InfusionTickInterval = 0.25f;

// Placeholder assets -- all stock Half-Life, pending the mod's own.
// See docs/ART_DEBT.md.
//
// medshot4 is the health charger's hypodermic hiss, which reads as an
// injection and is deliberately NOT the medkit's smallmedkit1: the two items
// must not sound alike.  medshotno1 is the stock "cannot heal you" sound and
// its whole job is this.
static const char* const k_InfusionSoundUse    = "items/medshot4.wav";
static const char* const k_InfusionSoundDenied = "items/medshotno1.wav";

// The sentence is literally "hiss, morphine_shot".  Silent for a player
// without the suit, which is fine -- the healing is not suit hardware.
static const char* const k_InfusionSuitSentence = "!HEV_HEAL7";

// The same sprite the Health Syringe uses in the Grid, on purpose: the player
// sees a syringe in the Grid, uses it, and a syringe appears at the screen
// edge.  Defined in the mod's sprites/hud.txt.
static const char* const k_InfusionIconSprite = "item_syringe";

// Green, to sit apart from the yellowish damage tiles beside the health
// readout -- those mean something is being done TO you.
static constexpr int k_InfusionIconR = 0;
static constexpr int k_InfusionIconG = 255;
static constexpr int k_InfusionIconB = 0;

//=========================================================
// Local save/restore descriptor table
//=========================================================
static TYPEDESCRIPTION g_InfusionSaveData[] =
{
	DEFINE_FIELD(CPlayerInfusion, m_bActive,    FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerInfusion, m_flEndTime,  FIELD_TIME),
	DEFINE_FIELD(CPlayerInfusion, m_flNextTick, FIELD_TIME),
	DEFINE_FIELD(CPlayerInfusion, m_flAccum,    FIELD_FLOAT),
};

bool InfusionSave(CPlayerInfusion& infusion, CSave& save)
{
	return save.WriteFields("INFUSION", &infusion, g_InfusionSaveData, ARRAYSIZE(g_InfusionSaveData));
}

bool InfusionRestore(CPlayerInfusion& infusion, CRestore& restore)
{
	return restore.ReadFields("INFUSION", &infusion, g_InfusionSaveData, ARRAYSIZE(g_InfusionSaveData));
}

//=========================================================
// InfusionPrecache
//=========================================================
void InfusionPrecache()
{
	PRECACHE_SOUND(k_InfusionSoundUse);
	PRECACHE_SOUND(k_InfusionSoundDenied);
}

//=========================================================
// Tuning readers.  Both numbers are starting guesses to be judged
// in play, which is why they are cvars and not constants.
//=========================================================
static float InfusionDurationFor(const CBasePlayer* pPlayer)
{
	float duration = infusion_duration.value;

	if (pPlayer->m_skills.HasSkill(ESkillId::MedExpert))
		duration += infusion_duration_bonus.value;

	return duration;
}

//=========================================================
// CPlayerInfusion::SetIcon
//
// Drives CHudStatusIcons, which the SDK shipped complete and never
// wired up -- gmsgStatusIcon had no sender anywhere in dlls/ before
// this.  Enabling takes a colour; disabling does not.
//=========================================================
void CPlayerInfusion::SetIcon(CBasePlayer* pPlayer, bool on)
{
	m_bIconSent = on;

	if (gmsgStatusIcon == 0)
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
	WRITE_BYTE(on ? 1 : 0);
	WRITE_STRING(k_InfusionIconSprite);
	if (on)
	{
		WRITE_BYTE(k_InfusionIconR);
		WRITE_BYTE(k_InfusionIconG);
		WRITE_BYTE(k_InfusionIconB);
	}
	MESSAGE_END();
}

//=========================================================
// CPlayerInfusion::Clear
//
// No icon-off message is sent here on purpose.  Clear() runs from
// Spawn(), which sets m_fInitHUD and so triggers a ResetHUD -- and
// ResetHUD wipes CHudStatusIcons on the client anyway.  Sending one
// would be talking to a client that is about to throw the answer away.
//=========================================================
void CPlayerInfusion::Clear(CBasePlayer* pPlayer)
{
	m_bActive    = false;
	m_flEndTime  = 0;
	m_flNextTick = 0;
	m_flAccum    = 0;
	m_bIconSent  = false;
}

//=========================================================
// CPlayerInfusion::TryStart
//
// One Infusion at a time.  A second Syringe is refused rather than
// extending or refreshing, which is what stops a Stack being emptied
// in one press and keeps the on/off icon sufficient -- there is no
// remaining-time the player would need to see to decide.
// See docs/adr/0007-the-infusion-is-one-at-a-time.md.
//=========================================================
bool CPlayerInfusion::TryStart(CBasePlayer* pPlayer)
{
	if (m_bActive)
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_InfusionSoundDenied, 1.0, ATTN_NORM);
		return false;
	}

	const float duration = InfusionDurationFor(pPlayer);
	if (duration <= 0)
		return false;

	m_bActive    = true;
	m_flEndTime  = gpGlobals->time + duration;
	m_flNextTick = gpGlobals->time + k_InfusionTickInterval;
	m_flAccum    = 0;

	SetIcon(pPlayer, true);

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_InfusionSoundUse, 1.0, ATTN_NORM);
	pPlayer->SetSuitUpdate((char*)k_InfusionSuitSentence, false, SUIT_NEXT_IN_30SEC);

	return true;
}

//=========================================================
// CPlayerInfusion::Think
//
// Nothing interrupts an Infusion -- not damage, not being on fire.
// It ends when its timer ends, and that predictability is the point:
// the Syringe is a promise of N health over N seconds, taken before
// the fight rather than during it.
//=========================================================
void CPlayerInfusion::Think(CBasePlayer* pPlayer)
{
	if (!m_bActive)
		return;

	// Death ends it. TakeHealth already refuses a corpse (takedamage is
	// DAMAGE_NO by then), so this changes no healing -- it is here so the icon
	// does not sit on screen counting down over a dead player.
	if (!pPlayer->IsAlive())
	{
		m_bActive = false;
		m_flAccum = 0;
		SetIcon(pPlayer, false);
		return;
	}

	// The client wipes its icon list on every ResetHUD, so re-assert the icon
	// when ForgetSentIcon() has told us that happened.  Without this a save
	// loaded mid-Infusion heals invisibly.
	if (!m_bIconSent)
		SetIcon(pPlayer, true);

	const float now = gpGlobals->time;

	// Bounded by the duration rather than by real time, so a long stall cannot
	// spin here -- at worst this runs duration/interval times.
	while (now >= m_flNextTick && m_flNextTick <= m_flEndTime)
	{
		m_flAccum += infusion_rate.value * k_InfusionTickInterval;

		const int whole = (int)m_flAccum;
		if (whole > 0)
		{
			m_flAccum -= (float)whole;

			// TakeHealth refuses at max_health, so ticks that land on a full
			// health bar are simply lost.  Deliberate: a Syringe may be used
			// at full health, and the waste is the cost of using it early.
			pPlayer->TakeHealth((float)whole, DMG_GENERIC);
		}

		m_flNextTick += k_InfusionTickInterval;
	}

	if (now >= m_flEndTime)
	{
		m_bActive = false;
		m_flAccum = 0;
		SetIcon(pPlayer, false);
	}
}

//=========================================================
// InfusionUseSyringe
//=========================================================
bool InfusionUseSyringe(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return false;

	return pPlayer->m_infusion.TryStart(pPlayer);
}
