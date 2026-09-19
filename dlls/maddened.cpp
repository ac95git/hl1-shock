// monster_maddened -- the maddened miner (docs/ROADMAP.md, "The cult and the
// maddened", grilled 2026-09-19).
//
// A miner who heard the voice.  A melee human on the base AI's chase-and-
// swing, the zombie's shape with a man's body: models/maddened.mdl is the
// player's own model from the SDK sources, built by maddened_build.py
// (E:\CustomAssets\scripts), with the pick as a bodygroup on the hand.  He
// walks, always -- the run is the cult's, later.  He is everyone's enemy
// (CLASS_MADDENED: the player's, the soldiers', Xen's) and his own kind's
// ally, which the cult will share.  The default Perception Profile for now,
// no Disturbances, backstabbable: three swings from the front, one from
// behind, which is the stealth lesson of the first hour.
//
// The "suited" keyvalue picks the body: some of the maddened still wear the
// suit and some do not, so the player learns nothing false about who wears
// one.  It sets the head (bare or helmet) and the skin (overalls or suit)
// together; the pick is always in the hand.
//
// Sounds are stand-ins (docs/ART_DEBT.md, "shaft1"): the alien slave's words
// as his mutter, so what the player heard down the old workings is him and
// it is his tell in the dark; the crowbar's miss and body hit pitched down;
// Barney's pain and death.
//
// Server only.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "skill.h"

// The QC's event on the frame the pick lands.
#define MADDENED_AE_SWING 1

// Bodygroups, in the QC's order: body (one), head (bare, helmet), weapon
// (blank, pick).  pev->body is head + 2 * weapon.
#define MADDENED_HEAD_BARE 0
#define MADDENED_HEAD_HELMET 1
#define MADDENED_WEAPON_PICK 2

#define MADDENED_SKIN_SUIT 0
#define MADDENED_SKIN_OVERALLS 1

// The mutter: every 4-8 s while nothing is happening.
#define MADDENED_MUTTER_MIN 4.0f
#define MADDENED_MUTTER_MAX 8.0f

class CMaddened : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void SetYawSpeed() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void PrescheduleThink() override;

	void PainSound() override;
	void DeathSound() override;
	void IdleSound() override;

	// No range attacks, ever.
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	static const char* pMutterSounds[];
	static const char* pHitSounds[];
	static const char* pMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

private:
	bool m_bSuited = false;
	float m_flNextMutter = 0.0f;
};

LINK_ENTITY_TO_CLASS(monster_maddened, CMaddened);

TYPEDESCRIPTION CMaddened::m_SaveData[] =
{
	DEFINE_FIELD(CMaddened, m_bSuited, FIELD_BOOLEAN),
	DEFINE_FIELD(CMaddened, m_flNextMutter, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CMaddened, CBaseMonster);

const char* CMaddened::pMutterSounds[] =
{
	"aslave/slv_word1.wav",
	"aslave/slv_word2.wav",
	"aslave/slv_word4.wav",
	"aslave/slv_word5.wav",
	"aslave/slv_word7.wav",
};

const char* CMaddened::pHitSounds[] =
{
	"weapons/cbar_hitbod1.wav",
	"weapons/cbar_hitbod2.wav",
	"weapons/cbar_hitbod3.wav",
};

const char* CMaddened::pMissSounds[] =
{
	"weapons/cbar_miss1.wav",
};

const char* CMaddened::pPainSounds[] =
{
	"barney/ba_pain1.wav",
	"barney/ba_pain2.wav",
	"barney/ba_pain3.wav",
};

const char* CMaddened::pDeathSounds[] =
{
	"barney/ba_die1.wav",
	"barney/ba_die2.wav",
	"barney/ba_die3.wav",
};

bool CMaddened::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "suited"))
	{
		m_bSuited = atoi(pkvd->szValue) != 0;
		return true;
	}
	return CBaseMonster::KeyValue(pkvd);
}

int CMaddened::Classify()
{
	return CLASS_MADDENED;
}

void CMaddened::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

void CMaddened::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/maddened.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.maddenedHealth;
	pev->view_ofs = VEC_VIEW;
	m_flFieldOfView = 0.5f;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;

	pev->body = (m_bSuited ? MADDENED_HEAD_HELMET : MADDENED_HEAD_BARE) + MADDENED_WEAPON_PICK;
	pev->skin = m_bSuited ? MADDENED_SKIN_SUIT : MADDENED_SKIN_OVERALLS;

	m_flNextMutter = gpGlobals->time + RANDOM_FLOAT(1.0f, MADDENED_MUTTER_MAX);

	MonsterInit();
}

void CMaddened::Precache()
{
	PRECACHE_MODEL("models/maddened.mdl");

	PRECACHE_SOUND_ARRAY(pMutterSounds);
	PRECACHE_SOUND_ARRAY(pHitSounds);
	PRECACHE_SOUND_ARRAY(pMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
}

//=========================================================
// The mutter runs on its own clock rather than the AI's idle-sound roll,
// which fires about every ten seconds and only in IDLE.  He mutters while
// he walks at you too; it stops when he swings and when he dies.
//=========================================================
void CMaddened::PrescheduleThink()
{
	if (gpGlobals->time >= m_flNextMutter && IsAlive() && m_Activity != ACT_MELEE_ATTACK1)
	{
		IdleSound();
		m_flNextMutter = gpGlobals->time + RANDOM_FLOAT(MADDENED_MUTTER_MIN, MADDENED_MUTTER_MAX);
	}
}

void CMaddened::IdleSound()
{
	// Low and close: a man half-saying words, not a vortigaunt at full voice.
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pMutterSounds), 0.5, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 10));
}

void CMaddened::PainSound()
{
	if (RANDOM_LONG(0, 2) == 0)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 9));
}

void CMaddened::DeathSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 9));
}

//=========================================================
// The swing lands on the QC's event.  The trace is the zombie's 70; the reach
// the AI attacks from is the base 64.
//=========================================================
void CMaddened::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case MADDENED_AE_SWING:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.maddenedDmgSwing, DMG_CLUB);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.x = 6;
				pHurt->pev->punchangle.z = RANDOM_LONG(0, 1) ? 10 : -10;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 60;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pHitSounds), 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 8));
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pMissSounds), 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 8));
		}
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}
