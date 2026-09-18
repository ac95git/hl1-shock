/***
*
*	The Panthereye -- monster_panthereye.
*
*	Half-Life's cut Panthereye, on the model and sounds HL: Extended ships.
*	Its AI there exists only compiled, so this is written new, from the
*	behaviour settled in docs/ROADMAP.md (Pillar 2: Monsters and bosses,
*	Panthereye) and the v1 scope grilled on 2026-09-18.
*
*	Two modes, and it moves one way between them.  STALKING is what it does
*	with an enemy it has not been seen by: it closes in, running where the
*	player cannot see it and crawling on its belly where they might, growling
*	low the whole time, and claws if it gets into reach.  SPOTTED is the player
*	seeing IT -- the reverse of Suspicion -- or hurting it, or its first slash:
*	from then on it is a combat monster for good, rushing and pouncing.  It
*	never pounces unspotted, so being struck out of nowhere is limited to the
*	claws, and the growl warns before those.
*
*	Alone: a CBaseMonster, not a CSquadMonster.  Always the blue body; the red
*	one is the alpha, which is a later slice.  So are the cover-to-cover
*	search, feeding, the glowing-eye light and freezing when watched.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "player.h"
#include "game.h"
#include "perception.h"
#include "animation.h"

#include <cmath>
#include <cstdio>

//=========================================================
// Monster's Anim Events Go Here -- as the model carries them.
// attack_primary hits on 1 then 2, attack_main_claw on 3,
// attack_simple_claw on 1; crouch_to_jump leaves the ground on 5.
//=========================================================
#define PANTHER_AE_CLAW1 1
#define PANTHER_AE_CLAW2 2
#define PANTHER_AE_CLAW3 3
#define PANTHER_AE_LEAP 5

// crouch_to_jump's event 5 is at frame 11 of a 20 fps sequence: the crouch
// before take-off lasts this long at the model's own rate.  panther_leap_windup
// rescales the sequence against it.
#define PANTHER_NATURAL_WINDUP 0.55f

// "Bip01 Head" in panthereye.mdl, for the spotted test's second point.
#define PANTHER_HEAD_BONE 20

// The model's head hitbox is group 2, which Half-Life counts as chest.
#define PANTHER_MODEL_HEAD_HITGROUP 2

// Half the width of the player's screen, in degrees from the crosshair: fov
// 90 at 16:9 is about 53 each way.  Off screen is where the stalk runs.
#define PANTHER_SCREEN_HALF_ANGLE 55.0f

// Claw reach, the zombie's.
#define PANTHER_CLAW_REACH 70

enum class EPantherGait
{
	Upright,
	ToCrawl, // crouch_to_crawl playing
	Crawl,
	ToStand, // walk_to_stand playing
};

class CPanthereye : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override { return CLASS_ALIEN_PREDATOR; }
	const PerceptionProfile& GetPerceptionProfile() override { return g_ProfilePredator; }

	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void SetActivity(Activity NewActivity) override;
	void PrescheduleThink() override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;

	// The pounce is MELEE_ATTACK1 because that is how the model tags it; the
	// claws are MELEE_ATTACK2.  No ranged attacks.
	bool CheckMeleeAttack1(float flDot, float flDist) override;
	bool CheckMeleeAttack2(float flDot, float flDist) override;
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void IdleSound() override;
	void AlertSound() override;
	void PainSound() override;
	void DeathSound() override;

	void EXPORT LeapTouch(CBaseEntity* pOther);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pAttackSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	bool IsStalking() { return !m_bSpotted && m_hEnemy != NULL && m_MonsterState == MONSTERSTATE_COMBAT; }
	void BecomeSpotted(const char* pszWhy);
	void Claw();
	void PlaySequence(const char* pszName);
	void UpdateGait(bool bWantCrawl);
	bool PlayerSees(CBasePlayer* pPlayer, float flHalfAngle, float* pflAngle, bool* pbLine);
	void DebugReport(CBasePlayer* pPlayer, float flAngle, bool bLine, bool bOnScreen, float flDist);

	// Saved.
	bool m_bSpotted;
	float m_flNextLeap;
	float m_flNextGrowl;

	// Not saved: a restore comes back upright, and the dwell starts over.
	EPantherGait m_Gait;
	float m_flSpotDwell;
	float m_flLastSpotTest;
	float m_flNextDebug;
};

LINK_ENTITY_TO_CLASS(monster_panthereye, CPanthereye);

TYPEDESCRIPTION CPanthereye::m_SaveData[] =
	{
		DEFINE_FIELD(CPanthereye, m_bSpotted, FIELD_BOOLEAN),
		DEFINE_FIELD(CPanthereye, m_flNextLeap, FIELD_TIME),
		DEFINE_FIELD(CPanthereye, m_flNextGrowl, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CPanthereye, CBaseMonster);

const char* CPanthereye::pIdleSounds[] =
	{
		"panthereye/pa_idle1.wav",
		"panthereye/pa_idle2.wav",
		"panthereye/pa_idle4.wav",
};

// The growl.  An idle pitched down, standing in until a real one exists --
// docs/ART_DEBT.md.
static const char* PANTHER_GROWL_SOUND = "panthereye/pa_idle3.wav";

const char* CPanthereye::pAlertSounds[] =
	{
		"panthereye/pa_alert1.wav",
		"panthereye/pa_alert2.wav",
};

const char* CPanthereye::pPainSounds[] =
	{
		"panthereye/pa_pain1.wav",
		"panthereye/pa_pain2.wav",
};

const char* CPanthereye::pDeathSounds[] =
	{
		"panthereye/pa_death1.wav",
};

const char* CPanthereye::pAttackSounds[] =
	{
		"panthereye/pa_attack1.wav",
};

const char* CPanthereye::pAttackHitSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav",
};

const char* CPanthereye::pAttackMissSounds[] =
	{
		"zombie/claw_miss1.wav",
		"zombie/claw_miss2.wav",
};

//=========================================================
// The stalk: close in on the enemy.  The base chase with its own name and
// its own interrupts -- being spotted (SPECIAL1) and being hurt end it, and
// being in claw reach does, but the pounce never can, because
// CheckMeleeAttack1 refuses while unspotted.  How it moves is not here: the
// gait is chosen in SetActivity and PrescheduleThink.
//=========================================================
Task_t tlPantherStalk[] =
	{
		{TASK_SET_FAIL_SCHEDULE, (float)SCHED_COMBAT_FACE},
		{TASK_GET_PATH_TO_ENEMY, (float)0},
		{TASK_RUN_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
};

Schedule_t slPantherStalk[] =
	{
		{tlPantherStalk,
			ARRAYSIZE(tlPantherStalk),
			bits_COND_NEW_ENEMY |
				bits_COND_ENEMY_DEAD |
				bits_COND_CAN_MELEE_ATTACK2 |
				bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_TASK_FAILED |
				bits_COND_SPECIAL1,
			0,
			"PantherStalk"},
};

DEFINE_CUSTOM_SCHEDULES(CPanthereye){
	slPantherStalk,
};

IMPLEMENT_CUSTOM_SCHEDULES(CPanthereye, CBaseMonster);

//=========================================================
// Spawn
//=========================================================
void CPanthereye::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/panthereye.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.panthereyeHealth;
	pev->view_ofs = Vector(0, 0, 40);
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;

	// Body 1 is the blue one.  Red is the alpha's, and is not spawned until the
	// alpha exists, so a red Panthereye always means the tougher one.
	pev->body = 1;

	m_bSpotted = false;
	m_flNextLeap = 0;
	m_flNextGrowl = 0;
	m_Gait = EPantherGait::Upright;
	m_flSpotDwell = 0;
	m_flLastSpotTest = 0;
	m_flNextDebug = 0;

	MonsterInit();
}

void CPanthereye::Precache()
{
	PRECACHE_MODEL("models/panthereye.mdl");

	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND(PANTHER_GROWL_SOUND);
}

void CPanthereye::SetYawSpeed()
{
	switch (m_Activity)
	{
	case ACT_RUN:
		pev->yaw_speed = m_Gait == EPantherGait::Upright ? 120 : 60;
		break;
	case ACT_MELEE_ATTACK1:
		pev->yaw_speed = 30; // committed to the leap
		break;
	default:
		pev->yaw_speed = 90;
		break;
	}
}

//=========================================================
// Spotted -- the one-way switch to combat.
//=========================================================
void CPanthereye::BecomeSpotted(const char* pszWhy)
{
	if (m_bSpotted)
		return;

	m_bSpotted = true;
	m_flSpotDwell = 0;

	// Ends the stalk at the next schedule check, whatever it is doing.
	SetConditions(bits_COND_SPECIAL1);

	// Stand up out of the crawl now, not at the end of a transition.
	if (m_Gait != EPantherGait::Upright)
	{
		m_Gait = EPantherGait::Upright;

		if (m_Activity == ACT_RUN)
		{
			const int iRun = LookupActivity(ACT_RUN);
			if (iRun != ACTIVITY_NOT_AVAILABLE)
			{
				pev->sequence = iRun;
				ResetSequenceInfo();
			}
		}
	}

	AlertSound();

	if (panther_debug.value != 0)
		ALERT(at_console, "panthereye %d spotted: %s\n", entindex(), pszWhy);
}

//=========================================================
// PlayerSees -- is this Panthereye within flHalfAngle degrees of the
// player's crosshair, with a clear line from their eyes to its centre or its
// head?  The line wants either point: a Panthereye behind a crate with its
// head showing has been seen.
//=========================================================
bool CPanthereye::PlayerSees(CBasePlayer* pPlayer, float flHalfAngle, float* pflAngle, bool* pbLine)
{
	const Vector vecEyes = pPlayer->pev->origin + pPlayer->pev->view_ofs;

	UTIL_MakeVectors(pPlayer->pev->v_angle);
	const Vector vecForward = gpGlobals->v_forward;

	Vector vecHead, vecHeadAngles;
	GetBonePosition(PANTHER_HEAD_BONE, vecHead, vecHeadAngles);

	const Vector vecPoints[2] = {Center(), vecHead};

	float flBestAngle = 180.0f;
	bool bLine = false;

	for (const Vector& vecPoint : vecPoints)
	{
		const Vector vecTo = (vecPoint - vecEyes).Normalize();
		const float flAngle = acos(V_max(-1.0f, V_min(1.0f, DotProduct(vecTo, vecForward)))) * (180.0f / M_PI);

		if (flAngle < flBestAngle)
			flBestAngle = flAngle;

		if (flAngle > flHalfAngle)
			continue;

		TraceResult tr;
		UTIL_TraceLine(vecEyes, vecPoint, dont_ignore_monsters, pPlayer->edict(), &tr);

		if (tr.flFraction >= 1.0f || tr.pHit == edict())
			bLine = true;
	}

	if (pflAngle)
		*pflAngle = flBestAngle;
	if (pbLine)
		*pbLine = bLine;

	return flBestAngle <= flHalfAngle && bLine;
}

//=========================================================
// PrescheduleThink -- the spotted test, the gait, the growl.  Every think,
// so the dwell is measured against real time.
//=========================================================
void CPanthereye::PrescheduleThink()
{
	CBaseEntity* pEnemy = m_hEnemy;
	CBasePlayer* pPlayer = NULL;

	if (pEnemy != NULL && pEnemy->IsPlayer())
		pPlayer = static_cast<CBasePlayer*>(pEnemy);
	else if (panther_debug.value != 0)
	{
		CBaseEntity* pFirst = UTIL_PlayerByIndex(1);
		if (pFirst && pFirst->IsPlayer())
			pPlayer = static_cast<CBasePlayer*>(pFirst);
	}

	const float flNow = gpGlobals->time;
	float flDelta = flNow - m_flLastSpotTest;
	if (flDelta < 0 || flDelta > 0.5f)
		flDelta = 0.1f;
	m_flLastSpotTest = flNow;

	if (pPlayer == NULL || !IsAlive())
		return;

	float flAngle = 180.0f;
	bool bLine = false;
	const bool bInCone = PlayerSees(pPlayer, panther_spot_cone.value, &flAngle, &bLine);
	const float flDist = (pPlayer->pev->origin - pev->origin).Length();

	// On screen: in the wider cone with a line.  Recomputed only when the
	// narrow test did not already answer it, since passing it passes this.
	bool bOnScreen = bInCone;
	if (!bOnScreen && flAngle <= PANTHER_SCREEN_HALF_ANGLE)
		bOnScreen = PlayerSees(pPlayer, PANTHER_SCREEN_HALF_ANGLE, NULL, NULL);

	if (IsStalking() && pPlayer == pEnemy)
	{
		// --- The spotted test ------------------------------------------------
		if (bInCone)
		{
			m_flSpotDwell += flDelta;

			if (m_flSpotDwell >= panther_spot_dwell.value)
				BecomeSpotted("seen");
		}
		else
		{
			m_flSpotDwell = 0;
		}
	}

	if (IsStalking())
	{
		// --- The gait ------------------------------------------------------
		// Low and slow when it is close or could be seen; running where it
		// cannot be, so the dead time happens off screen.
		UpdateGait(bOnScreen || flDist <= panther_stalk_crawl_dist.value);

		// --- The growl -----------------------------------------------------
		// The player's warning, quiet and short-range.  A sound the player
		// hears and no monster does: EMIT_SOUND never touches CSoundEnt.
		if (flNow >= m_flNextGrowl)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, PANTHER_GROWL_SOUND,
				V_max(0.0f, V_min(1.0f, panther_growl_volume.value)), ATTN_IDLE, 0,
				(int)V_max(1.0f, V_min(255.0f, panther_growl_pitch.value)));

			m_flNextGrowl = flNow + V_max(0.5f, panther_growl_interval.value) + RANDOM_FLOAT(0.0f, 2.0f);
		}
	}
	else if (m_Gait != EPantherGait::Upright)
	{
		UpdateGait(false);
	}

	if (panther_debug.value != 0)
		DebugReport(pPlayer, flAngle, bLine, bOnScreen, flDist);
}

//=========================================================
// The gait while moving.  The crawl and its transitions carry no activity
// tag, so the base AI can never choose them: they are played by name over the
// top of ACT_RUN, and the movement code follows whatever ground speed the
// sequence has -- a transition has none, so it pauses to drop or rise.
//=========================================================
void CPanthereye::PlaySequence(const char* pszName)
{
	const int iSequence = LookupSequence(pszName);

	if (iSequence == ACTIVITY_NOT_AVAILABLE)
		return;

	pev->sequence = iSequence;
	pev->frame = 0;
	ResetSequenceInfo();
}

void CPanthereye::UpdateGait(bool bWantCrawl)
{
	// Only while moving: an attack or a flinch plays its own sequence, and
	// the gait picks up again at the next step.
	if (m_Activity != ACT_RUN)
	{
		if (!bWantCrawl)
			m_Gait = EPantherGait::Upright;
		return;
	}

	switch (m_Gait)
	{
	case EPantherGait::Upright:
		if (bWantCrawl)
		{
			m_Gait = EPantherGait::ToCrawl;
			PlaySequence("crouch_to_crawl");
		}
		break;

	case EPantherGait::ToCrawl:
		if (m_fSequenceFinished)
		{
			m_Gait = EPantherGait::Crawl;
			PlaySequence("crawl_on_belly");
		}
		break;

	case EPantherGait::Crawl:
		if (!bWantCrawl)
		{
			m_Gait = EPantherGait::ToStand;
			PlaySequence("walk_to_stand_1");
		}
		break;

	case EPantherGait::ToStand:
		if (m_fSequenceFinished)
		{
			m_Gait = EPantherGait::Upright;

			const int iRun = LookupActivity(ACT_RUN);
			if (iRun != ACTIVITY_NOT_AVAILABLE)
			{
				pev->sequence = iRun;
				pev->frame = 0;
				ResetSequenceInfo();
			}
		}
		break;
	}

	SetYawSpeed();
}

//=========================================================
// SetActivity -- the base picks the sequence, then two exceptions: a run
// that starts while the gait is low stays low, and the leap's crouch is
// rescaled to panther_leap_windup.
//=========================================================
void CPanthereye::SetActivity(Activity NewActivity)
{
	CBaseMonster::SetActivity(NewActivity);

	if (NewActivity == ACT_RUN && IsStalking())
	{
		if (m_Gait == EPantherGait::Crawl || m_Gait == EPantherGait::ToCrawl)
		{
			m_Gait = EPantherGait::Crawl;
			PlaySequence("crawl_on_belly");
			SetYawSpeed();
		}
		else
		{
			m_Gait = EPantherGait::Upright;
		}
	}
	else if (NewActivity == ACT_MELEE_ATTACK1)
	{
		// ResetSequenceInfo has just set the rate to 1.  Scale it so the
		// model's take-off event lands panther_leap_windup seconds in.
		const float flWindup = V_max(0.1f, panther_leap_windup.value);
		pev->framerate = PANTHER_NATURAL_WINDUP / flWindup;
	}
}

//=========================================================
// Attacks.
//=========================================================
bool CPanthereye::CheckMeleeAttack1(float flDot, float flDist)
{
	// The pounce.  Only ever the answer to being seen.
	if (!m_bSpotted)
		return false;

	if (gpGlobals->time < m_flNextLeap)
		return false;

	if (!FBitSet(pev->flags, FL_ONGROUND))
		return false;

	return flDist >= panther_leap_min.value && flDist <= panther_leap_max.value && flDot >= 0.65f;
}

bool CPanthereye::CheckMeleeAttack2(float flDot, float flDist)
{
	// The claws, in either mode.  Reach plus the two hulls' half-widths.
	return flDist <= PANTHER_CLAW_REACH + 32 && flDot >= 0.7f;
}

void CPanthereye::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MELEE_ATTACK1:
		SetTouch(&CPanthereye::LeapTouch);
		CBaseMonster::StartTask(pTask);
		break;

	case TASK_MELEE_ATTACK2:
		// Committing to an attack ends the stalk.
		BecomeSpotted("slashed");
		CBaseMonster::StartTask(pTask);
		break;

	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void CPanthereye::RunTask(Task_t* pTask)
{
	if (pTask->iTask == TASK_MELEE_ATTACK1 && m_fSequenceFinished)
		SetTouch(NULL);

	CBaseMonster::RunTask(pTask);
}

void CPanthereye::Claw()
{
	CBaseEntity* pHurt = CheckTraceHullAttack(PANTHER_CLAW_REACH, gSkillData.panthereyeDmgClaw, DMG_SLASH);

	if (pHurt)
	{
		if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
		{
			pHurt->pev->punchangle.z = RANDOM_LONG(0, 1) ? -12 : 12;
			pHurt->pev->punchangle.x = 5;
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 90 + RANDOM_LONG(-5, 5));
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 90 + RANDOM_LONG(-5, 5));
	}
}

void CPanthereye::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case PANTHER_AE_CLAW1:
	case PANTHER_AE_CLAW2:
	case PANTHER_AE_CLAW3:
		Claw();
		break;

	case PANTHER_AE_LEAP:
	{
		// The headcrab's leap (dlls/headcrab.cpp), at the Panthereye's size.
		ClearBits(pev->flags, FL_ONGROUND);
		UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, 1));
		UTIL_MakeVectors(pev->angles);

		Vector vecJump;
		CBaseEntity* pEnemy = m_hEnemy;

		if (pEnemy != NULL)
		{
			float flGravity = g_psv_gravity->value;
			if (flGravity <= 1)
				flGravity = 1;

			// Up fast enough to reach the enemy's eyes, across in the time that takes.
			float flHeight = pEnemy->pev->origin.z + pEnemy->pev->view_ofs.z - pev->origin.z;
			if (flHeight < 16)
				flHeight = 16;

			const float flSpeed = sqrt(2 * flGravity * flHeight);
			const float flTime = flSpeed / flGravity;

			vecJump = (pEnemy->pev->origin + pEnemy->pev->view_ofs - pev->origin) * (1.0f / flTime);
			vecJump.z = flSpeed;

			const float flCap = V_max(100.0f, panther_leap_speed.value);
			const float flLength = vecJump.Length();
			if (flLength > flCap)
				vecJump = vecJump * (flCap / flLength);
		}
		else
		{
			vecJump = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

		pev->velocity = vecJump;
		m_flNextLeap = gpGlobals->time + V_max(0.0f, panther_leap_cooldown.value);
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// LeapTouch -- once per leap, while in the air.  A miss lands it, and the
// rest of crouch_to_jump is the recovery the player punishes.
//=========================================================
void CPanthereye::LeapTouch(CBaseEntity* pOther)
{
	if (0 == pOther->pev->takedamage)
		return;

	if (pOther->Classify() == Classify())
		return;

	if (!FBitSet(pev->flags, FL_ONGROUND))
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 85 + RANDOM_LONG(-5, 5));
		pOther->TakeDamage(pev, pev, gSkillData.panthereyeDmgLeap, DMG_SLASH);
	}

	SetTouch(NULL);
}

//=========================================================
// Schedules.
//=========================================================
Schedule_t* CPanthereye::GetSchedule()
{
	// The spotted interrupt has done its job once a schedule is being chosen.
	ClearConditions(bits_COND_SPECIAL1);

	if (IsStalking() && !HasConditions(bits_COND_ENEMY_DEAD))
	{
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK2))
			return GetScheduleOfType(SCHED_MELEE_ATTACK2);

		return slPantherStalk;
	}

	return CBaseMonster::GetSchedule();
}

Schedule_t* CPanthereye::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_WAKE_ANGRY:
	case SCHED_CHASE_ENEMY:
		// Unspotted, acquiring the player is the start of the stalk, silently
		// -- the alert is for being seen -- and a chase is a stalk.
		if (!m_bSpotted)
			return slPantherStalk;
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

//=========================================================
// Damage.  The model's head is hitgroup 2, which counts as chest; make it a
// head so a headshot is one.  And being hurt by the player is being spotted.
//=========================================================
void CPanthereye::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == PANTHER_MODEL_HEAD_HITGROUP)
		ptr->iHitgroup = HITGROUP_HEAD;

	CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

bool CPanthereye::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (flDamage > 0 && pevAttacker != NULL)
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);

		if (pAttacker != NULL && pAttacker->IsPlayer())
			BecomeSpotted("hurt");
	}

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// Sounds.
//=========================================================
void CPanthereye::IdleSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 0.8, ATTN_IDLE, 0, 95 + RANDOM_LONG(0, 10));
}

void CPanthereye::AlertSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));
}

void CPanthereye::PainSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));
}

void CPanthereye::DeathSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));
}

//=========================================================
// panther_debug -- the spotted test as it reads right now.  Built with the
// test because spotted is one-way and a wrong answer costs the whole stalk:
// when it flips when it should not have, this says why.  Centre print, the
// same screen space as debug_schedule; run one at a time.
//=========================================================
void CPanthereye::DebugReport(CBasePlayer* pPlayer, float flAngle, bool bLine, bool bOnScreen, float flDist)
{
	if (gpGlobals->time < m_flNextDebug && m_flNextDebug < gpGlobals->time + 1.0f)
		return;

	m_flNextDebug = gpGlobals->time + 0.25f;

	static const char* pGaitNames[] = {"upright", "to crawl", "crawl", "to stand"};

	const char* pszMode = m_bSpotted ? "COMBAT" : (IsStalking() ? "STALK" : "unaware");

	char szReport[160];
	snprintf(szReport, sizeof(szReport),
		"panthereye %d  %s  %s\nangle %.0f / %.0f  line %s  screen %s\ndwell %.2f / %.2f  dist %.0f  susp %.2f\n",
		entindex(), pszMode, pGaitNames[(int)m_Gait],
		flAngle, panther_spot_cone.value, bLine ? "yes" : "no", bOnScreen ? "yes" : "no",
		m_flSpotDwell, panther_spot_dwell.value, flDist, m_flSuspicion);

	ClientPrint(pPlayer->pev, HUD_PRINTCENTER, szReport);
}
