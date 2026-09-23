/***
*
*	The Panthereye -- monster_panthereye.
*
*	Half-Life's cut Panthereye, on the model and sounds HL: Extended ships.
*	Its AI there exists only compiled, so this is written new, from the
*	behaviour settled in docs/ROADMAP.md (Pillar 2: Monsters and bosses,
*	Panthereye): the v1 scope grilled on 2026-09-18 and the menace rework
*	grilled on 2026-09-23.
*
*	Two modes, and it moves one way between them.  The STALK is what it does
*	with an enemy it has not been Revealed to: it closes in, running where the
*	player cannot see it, growling low the whole time, and claws if it gets
*	into reach.  Glimpsed -- on the player's screen but not yet Revealed -- it
*	stops coming straight and Circles: a slow crawl on a spiral toward the
*	player's back.  REVEALED is the player seeing IT (the reverse of Spotted),
*	or hurting it, or its first slash: from then on it is a combat monster for
*	good, Circling in at a run and pouncing, sometimes by way of a wall.  It
*	never pounces unRevealed, so being struck out of nowhere is limited to the
*	claws, and the growl warns before those.
*
*	Circling is a spiral, not a ring, because controller 0 turns the upper body
*	toward the player only so far: the heading is held panther_circle_angle off
*	the line to the player, so the shoulders can stay on them the whole way.
*
*	Alone: a CBaseMonster, not a CSquadMonster.  Always the blue body; the red
*	one is the alpha, which is a later slice.  So are the cover-to-cover
*	search, feeding, the glowing-eye light, the wall pose, the knock-off and
*	the head fixed on the player.
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

// "Bip01 Head" in panthereye.mdl, for the Revealed test's second point.
#define PANTHER_HEAD_BONE 20

// The model's head hitbox is group 2, which Half-Life counts as chest.
#define PANTHER_MODEL_HEAD_HITGROUP 2

// Half the width of the player's screen, in degrees from the crosshair: fov
// 90 at 16:9 is about 53 each way.  Off screen is where the stalk runs.
#define PANTHER_SCREEN_HALF_ANGLE 55.0f

// Claw reach, the zombie's.
#define PANTHER_CLAW_REACH 70

// How far ahead on the spiral each step aims.  Re-aimed every think, so the
// path curves; far enough that the base mover never reaches it and stops.
#define PANTHER_CIRCLE_STEP 64.0f

// Combat Circling pounces when it has spiralled down to within this much of
// panther_leap_min, the band's floor, whatever the timer says.
#define PANTHER_BAND_FLOOR_MARGIN 48.0f

// A Wall Pounce that has not reached its wall, or not landed after the
// rebound, in this long gives up.
#define PANTHER_WALL_TIMEOUT 1.5f

// The hull's half-width, and the height of the top of it.
#define PANTHER_HULL_HALF 32.0f
#define PANTHER_HULL_TOP 64.0f

enum
{
	TASK_PANTHER_CIRCLE = LAST_COMMON_TASK + 1,
};

enum class EPantherGait
{
	Upright,
	ToCrawl, // crouch_to_crawl playing
	Crawl,
	ToStand, // walk_to_stand playing
};

enum class EPantherWall
{
	None,
	Chosen, // the crouch, turned toward the wall
	ToWall, // in the air, on the way to it
	Cling,	// on the wall, turned to the player
	Rebound // in the air, on the way to the player
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
	void Killed(entvars_t* pevAttacker, int iGib) override;

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
	bool IsStalking() { return !m_bRevealed && m_hEnemy != NULL && m_MonsterState == MONSTERSTATE_COMBAT; }
	void BecomeRevealed(const char* pszWhy);
	void Claw();
	void PlaySequence(const char* pszName);
	void PlayCrawl();
	void UpdateGait(bool bWantCrawl);
	void UpdateTurn(float flDelta);
	bool CircleStep();
	bool FindWall();
	Vector LeapVelocity(const Vector& vecTarget);
	void EndWallPounce();
	bool PlayerSees(CBasePlayer* pPlayer, float flHalfAngle, float* pflAngle, bool* pbLine);
	void DebugReport(CBasePlayer* pPlayer, float flAngle, bool bLine, bool bOnScreen, float flDist);

	// Saved.
	bool m_bRevealed;
	float m_flNextLeap;
	float m_flNextGrowl;

	// Not saved: a restore comes back upright, not Circling, on the ground,
	// and the dwell starts over.
	EPantherGait m_Gait;
	float m_flSpotDwell;
	float m_flLastSpotTest;
	float m_flNextDebug;
	bool m_bGlimpsed;
	int m_iCircleDir;	   // +1 round the player anticlockwise, -1 clockwise
	float m_flCircleUntil; // combat: when the timer lets it pounce; 0 until it starts
	float m_flTurn;		   // controller 0 as last set, before the sign
	EPantherWall m_Wall;
	Vector m_vecWallTarget; // where the hull's centre aims to meet the wall
	Vector m_vecWallDir;	// horizontal, toward the wall
	float m_flWallTime;
};

LINK_ENTITY_TO_CLASS(monster_panthereye, CPanthereye);

TYPEDESCRIPTION CPanthereye::m_SaveData[] =
	{
		DEFINE_FIELD(CPanthereye, m_bRevealed, FIELD_BOOLEAN),
		DEFINE_FIELD(CPanthereye, m_flNextLeap, FIELD_TIME),
		DEFINE_FIELD(CPanthereye, m_flNextGrowl, FIELD_TIME),
};

bool CPanthereye::Save(CSave& save)
{
	if (!CBaseMonster::Save(save))
		return false;
	return save.WriteFields("CPanthereye", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

bool CPanthereye::Restore(CRestore& restore)
{
	if (!CBaseMonster::Restore(restore))
		return false;
	if (!restore.ReadFields("CPanthereye", this, m_SaveData, ARRAYSIZE(m_SaveData)))
		return false;

	// Saved mid-cling: the wall state is not saved, so come back on the ground.
	if (pev->movetype == MOVETYPE_FLY)
		pev->movetype = MOVETYPE_STEP;

	return true;
}

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
// its own interrupts -- being Revealed (SPECIAL1), being Glimpsed (SPECIAL2)
// and being hurt end it, and being in claw reach does, but the pounce never
// can, because CheckMeleeAttack1 refuses while unRevealed.  How it moves is
// not here: the gait is chosen in SetActivity and PrescheduleThink.
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
				bits_COND_SPECIAL1 |
				bits_COND_SPECIAL2,
			0,
			"PantherStalk"},
};

//=========================================================
// Circling, in both modes.  One task that re-aims a step along the spiral
// every think (CircleStep).  Glimpsed-while-stalking ends when it is no
// longer Glimpsed (SPECIAL2) or is Revealed (SPECIAL1); combat Circling ends
// when the pounce is ready or it has drifted out of the band.
//=========================================================
Task_t tlPantherCircle[] =
	{
		{TASK_SET_FAIL_SCHEDULE, (float)SCHED_COMBAT_FACE},
		{TASK_PANTHER_CIRCLE, (float)0},
};

Schedule_t slPantherCircle[] =
	{
		{tlPantherCircle,
			ARRAYSIZE(tlPantherCircle),
			bits_COND_NEW_ENEMY |
				bits_COND_ENEMY_DEAD |
				bits_COND_ENEMY_OCCLUDED |
				bits_COND_CAN_MELEE_ATTACK1 |
				bits_COND_CAN_MELEE_ATTACK2 |
				bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_TASK_FAILED |
				bits_COND_SPECIAL1 |
				bits_COND_SPECIAL2,
			0,
			"PantherCircle"},
};

//=========================================================
// The Wall Pounce.  No face-the-enemy first: the crouch turns to the wall.
// Nothing interrupts it -- once it is in the air it has to come down through
// RunTask, or it would be left clinging with nobody to let it go.
//=========================================================
Task_t tlPantherWallPounce[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_MELEE_ATTACK1, (float)0},
};

Schedule_t slPantherWallPounce[] =
	{
		{tlPantherWallPounce,
			ARRAYSIZE(tlPantherWallPounce),
			0,
			0,
			"PantherWallPounce"},
};

DEFINE_CUSTOM_SCHEDULES(CPanthereye){
	slPantherStalk,
	slPantherCircle,
	slPantherWallPounce,
};

IMPLEMENT_CUSTOM_SCHEDULES(CPanthereye, CBaseMonster);

//=========================================================
// Spawn
//=========================================================
void CPanthereye::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/panthereye.mdl");
	UTIL_SetSize(pev, Vector(-PANTHER_HULL_HALF, -PANTHER_HULL_HALF, 0), Vector(PANTHER_HULL_HALF, PANTHER_HULL_HALF, PANTHER_HULL_TOP));

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

	m_bRevealed = false;
	m_flNextLeap = 0;
	m_flNextGrowl = 0;
	m_Gait = EPantherGait::Upright;
	m_flSpotDwell = 0;
	m_flLastSpotTest = 0;
	m_flNextDebug = 0;
	m_bGlimpsed = false;
	m_iCircleDir = 1;
	m_flCircleUntil = 0;
	m_flTurn = 0;
	m_Wall = EPantherWall::None;
	m_flWallTime = 0;

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
		// Revealed, the turn to face before a pounce comes out of a spiral 60
		// degrees off; a slow one would be a pause the circling doesn't earn.
		pev->yaw_speed = m_bRevealed ? 180 : 90;
		break;
	}
}

//=========================================================
// Revealed -- the one-way switch to combat.
//=========================================================
void CPanthereye::BecomeRevealed(const char* pszWhy)
{
	if (m_bRevealed)
		return;

	m_bRevealed = true;
	m_bGlimpsed = false;
	m_flSpotDwell = 0;

	// Which way round it Circles in combat, for the whole encounter unless a
	// wall turns it back.
	m_iCircleDir = RANDOM_LONG(0, 1) ? 1 : -1;
	m_flCircleUntil = 0;

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
		ALERT(at_console, "panthereye %d revealed: %s\n", entindex(), pszWhy);
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
// PrescheduleThink -- the Revealed test, Glimpsed, the gait, the growl, the
// upper body's turn.  Every think, so the dwell is measured against real time.
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

	UpdateTurn(flDelta);

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
		// --- The Revealed test -----------------------------------------------
		if (bInCone)
		{
			m_flSpotDwell += flDelta;

			if (m_flSpotDwell >= panther_spot_dwell.value)
				BecomeRevealed("seen");
		}
		else
		{
			m_flSpotDwell = 0;
		}
	}

	// --- Glimpsed --------------------------------------------------------------
	// A change either way ends the schedule it is in: the straight stalk gives
	// way to Circling, and Circling to the run once it is off screen again.
	const bool bGlimpsed = IsStalking() && pPlayer == pEnemy && bOnScreen;
	if (bGlimpsed != m_bGlimpsed)
	{
		m_bGlimpsed = bGlimpsed;
		SetConditions(bits_COND_SPECIAL2);
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
// The upper body's turn: controller 0, "Bip01 Spine" YR 90 -90.  Toward the
// enemy while it moves (the Circling spiral, the chase), clamped at
// panther_turn_clamp and turned no faster than panther_turn_rate; back to
// straight for everything else, so the claws, the leap and the flinches play
// untwisted.  The head only follows the shoulders: fixing the head itself
// needs a controller the model does not have.  panther_turn_sign flips it if
// the controller turns the wrong way -- the model's reversed range makes its
// direction a thing to see, not to derive.
//=========================================================
void CPanthereye::UpdateTurn(float flDelta)
{
	float flTarget = 0;
	CBaseEntity* pEnemy = m_hEnemy;

	if (pEnemy != NULL && IsAlive() && m_Activity == ACT_RUN && m_Wall == EPantherWall::None)
	{
		const float flClamp = V_max(0.0f, panther_turn_clamp.value);
		flTarget = UTIL_AngleDiff(UTIL_VecToYaw(pEnemy->pev->origin - pev->origin), pev->angles.y);
		flTarget = V_max(-flClamp, V_min(flClamp, flTarget));
	}

	const float flStep = V_max(1.0f, panther_turn_rate.value) * flDelta;
	if (m_flTurn < flTarget)
		m_flTurn = V_min(flTarget, m_flTurn + flStep);
	else if (m_flTurn > flTarget)
		m_flTurn = V_max(flTarget, m_flTurn - flStep);

	SetBoneController(0, m_flTurn * (panther_turn_sign.value < 0 ? -1.0f : 1.0f));
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

// The crawl, slowed.  MoveExecute multiplies the sequence's ground speed by
// the framerate, so the feet and the body slow together and nothing slides.
void CPanthereye::PlayCrawl()
{
	PlaySequence("crawl_on_belly");
	pev->framerate = V_max(0.1f, panther_crawl_rate.value);
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
			PlayCrawl();
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
			PlayCrawl();
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
// CircleStep -- aim one step along the spiral.  The heading is held
// panther_circle_angle off the straight line to the enemy, which bends the
// path round them and always a little in: a logarithmic spiral, and the
// controller's clamp is what sets the angle.  Stalking, it goes round the
// way that leads to the player's back; in combat, the way chosen when it was
// Revealed.  Blocked, it tries the other way round, then straight in.
//=========================================================
bool CPanthereye::CircleStep()
{
	CBaseEntity* pEnemy = m_hEnemy;
	if (pEnemy == NULL)
		return false;

	Vector vecFrom = pev->origin - pEnemy->pev->origin;
	vecFrom.z = 0;
	const float flDist = vecFrom.Length();
	if (flDist < 1.0f)
		return false;

	const Vector vecOut = vecFrom * (1.0f / flDist);		  // enemy to Panthereye
	const Vector vecRound = Vector(-vecOut.y, vecOut.x, 0); // anticlockwise round the enemy

	int iDir = m_iCircleDir;
	if (!m_bRevealed)
	{
		// Toward the back: whichever way round moves it further from where the
		// player is looking.
		const float flFromView = UTIL_AngleDiff(UTIL_VecToYaw(vecFrom), pEnemy->pev->v_angle.y);
		iDir = flFromView >= 0 ? 1 : -1;
	}

	const float flAngle = V_max(0.0f, V_min(89.0f, panther_circle_angle.value)) * (M_PI / 180.0f);
	const float flStep = V_min(PANTHER_CIRCLE_STEP, flDist);

	const int iTries[3] = {iDir, -iDir, 0};

	for (int iTry : iTries)
	{
		Vector vecHeading;
		if (iTry == 0)
			vecHeading = vecOut * -1.0f;
		else
			vecHeading = vecOut * -cos(flAngle) + vecRound * (iTry * sin(flAngle));

		const Vector vecGoal = pev->origin + vecHeading * flStep;

		if (CheckLocalMove(pev->origin, vecGoal, NULL, NULL) != LOCALMOVE_VALID)
			continue;

		if (!MoveToLocation(ACT_RUN, 2, vecGoal))
			continue;

		// A wall turns combat Circling back the other way for good.
		if (m_bRevealed && iTry == -iDir)
			m_iCircleDir = -iDir;

		return true;
	}

	return false;
}

//=========================================================
// FindWall -- is there a wall to pounce by?  Sideways from the line to the
// enemy (both perpendiculars, and 30 degrees either side of each) within
// panther_wall_reach.  It counts if it is near vertical, still there at head
// height (tall enough, not a crate), the enemy's eyes can be seen from it,
// and coming off it is at least 45 degrees off the straight approach.
//=========================================================
bool CPanthereye::FindWall()
{
	CBaseEntity* pEnemy = m_hEnemy;
	if (pEnemy == NULL)
		return false;

	const Vector vecEyes = pEnemy->pev->origin + pEnemy->pev->view_ofs;
	Vector vecToEnemy = pEnemy->pev->origin - pev->origin;
	vecToEnemy.z = 0;
	if (vecToEnemy.Length() < 1.0f)
		return false;
	vecToEnemy = vecToEnemy.Normalize();

	const float flBaseYaw = UTIL_VecToYaw(vecToEnemy);
	const float flOffsets[6] = {90, 60, 120, -90, -60, -120};
	const float flReach = V_max(PANTHER_HULL_HALF + 16.0f, panther_wall_reach.value) + PANTHER_HULL_HALF;

	// Start from a random one so it is not always the same side.
	const int iStart = RANDOM_LONG(0, 5);

	for (int i = 0; i < 6; i++)
	{
		const float flYaw = (flBaseYaw + flOffsets[(iStart + i) % 6]) * (M_PI / 180.0f);
		const Vector vecDir = Vector(cos(flYaw), sin(flYaw), 0);

		const Vector vecLow = pev->origin + Vector(0, 0, PANTHER_HULL_TOP * 0.5f);
		TraceResult tr;
		UTIL_TraceLine(vecLow, vecLow + vecDir * flReach, ignore_monsters, edict(), &tr);

		if (tr.flFraction >= 1.0f || fabs(tr.vecPlaneNormal.z) > 0.3f)
			continue;

		const Vector vecHigh = pev->origin + Vector(0, 0, PANTHER_HULL_TOP - 4);
		TraceResult trHigh;
		UTIL_TraceLine(vecHigh, vecHigh + vecDir * flReach, ignore_monsters, edict(), &trHigh);

		if (trHigh.flFraction >= 1.0f)
			continue;

		// Where the hull's centre meets the wall, a little up the leap's arc.
		Vector vecTarget = tr.vecEndPos - vecDir * (PANTHER_HULL_HALF + 8.0f);
		vecTarget.z = pev->origin.z + 48.0f;

		TraceResult trLine;
		UTIL_TraceLine(tr.vecEndPos + tr.vecPlaneNormal * 16.0f, vecEyes, ignore_monsters, edict(), &trLine);
		if (trLine.flFraction < 1.0f)
			continue;

		Vector vecOff = pEnemy->pev->origin - vecTarget;
		vecOff.z = 0;
		if (vecOff.Length() < 1.0f)
			continue;
		if (DotProduct(vecOff.Normalize(), vecToEnemy) > cos(45.0f * (M_PI / 180.0f)))
			continue;

		m_vecWallTarget = vecTarget;
		m_vecWallDir = vecDir;
		return true;
	}

	return false;
}

//=========================================================
// Attacks.
//=========================================================
bool CPanthereye::CheckMeleeAttack1(float flDot, float flDist)
{
	// The pounce.  Only ever the answer to being Revealed.
	if (!m_bRevealed)
		return false;

	if (gpGlobals->time < m_flNextLeap)
		return false;

	if (!FBitSet(pev->flags, FL_ONGROUND))
		return false;

	if (flDist < panther_leap_min.value || flDist > panther_leap_max.value)
		return false;

	// Circling first: it pounces when the timer runs out, or when the spiral
	// has brought it down to the band's floor.  No facing test -- the attack
	// schedule turns it to face before the crouch.
	const bool bTimer = m_flCircleUntil != 0 && gpGlobals->time >= m_flCircleUntil;
	const bool bFloor = flDist <= panther_leap_min.value + PANTHER_BAND_FLOOR_MARGIN;

	return bTimer || bFloor;
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
	case TASK_PANTHER_CIRCLE:
		// Combat Circling starts its clock the first time; it runs until the
		// pounce, however many times the schedule is picked up again.
		if (m_bRevealed && m_flCircleUntil == 0)
		{
			const float flMin = V_max(0.0f, panther_circle_min.value);
			const float flMax = V_max(flMin, panther_circle_max.value);
			m_flCircleUntil = gpGlobals->time + RANDOM_FLOAT(flMin, flMax);
		}

		if (!CircleStep())
			TaskFail();
		break;

	case TASK_MELEE_ATTACK1:
		SetTouch(&CPanthereye::LeapTouch);
		CBaseMonster::StartTask(pTask);
		break;

	case TASK_MELEE_ATTACK2:
		// Committing to an attack ends the stalk.
		BecomeRevealed("slashed");
		CBaseMonster::StartTask(pTask);
		break;

	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void CPanthereye::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_PANTHER_CIRCLE:
	{
		CBaseEntity* pEnemy = m_hEnemy;
		if (pEnemy == NULL)
		{
			TaskFail();
			break;
		}

		// Combat Circling is the band's business; out of it, chase again.
		if (m_bRevealed && (pEnemy->pev->origin - pev->origin).Length() > panther_leap_max.value + 32.0f)
		{
			TaskComplete();
			break;
		}

		if (!CircleStep())
			TaskFail();
		break;
	}

	case TASK_MELEE_ATTACK1:
		if (m_Wall != EPantherWall::None)
		{
			const float flNow = gpGlobals->time;

			switch (m_Wall)
			{
			case EPantherWall::Chosen:
				// The crouch, turned to the wall -- fast, since the wall is off to
				// the side and the crouch is short.  If it never took off, it's over.
				MakeIdealYaw(m_vecWallTarget);
				ChangeYaw(360);
				if (m_fSequenceFinished)
					EndWallPounce();
				break;

			case EPantherWall::ToWall:
				// Landed short of the wall, or never got there.
				if ((FBitSet(pev->flags, FL_ONGROUND) && flNow > m_flWallTime + 0.1f) || flNow > m_flWallTime + PANTHER_WALL_TIMEOUT)
					EndWallPounce();
				break;

			case EPantherWall::Cling:
				if (flNow >= m_flWallTime)
				{
					// Off the wall, at the enemy's eyes.
					CBaseEntity* pEnemy = m_hEnemy;

					pev->movetype = MOVETYPE_STEP;
					pev->framerate = PANTHER_NATURAL_WINDUP / V_max(0.1f, panther_leap_windup.value);
					pev->velocity = pEnemy != NULL ? LeapVelocity(pEnemy->pev->origin + pEnemy->pev->view_ofs) : m_vecWallDir * -350.0f;

					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

					m_Wall = EPantherWall::Rebound;
					m_flWallTime = flNow;
					SetTouch(&CPanthereye::LeapTouch);
				}
				break;

			case EPantherWall::Rebound:
				if ((FBitSet(pev->flags, FL_ONGROUND) && flNow > m_flWallTime + 0.1f) || flNow > m_flWallTime + PANTHER_WALL_TIMEOUT)
					EndWallPounce();
				break;

			default:
				break;
			}

			// The sequence ends long before the wall is done with; the task
			// waits for the wall.
			if (m_Wall != EPantherWall::None)
				break;
		}

		if (m_fSequenceFinished)
			SetTouch(NULL);

		CBaseMonster::RunTask(pTask);
		break;

	default:
		CBaseMonster::RunTask(pTask);
		break;
	}
}

void CPanthereye::EndWallPounce()
{
	if (pev->movetype == MOVETYPE_FLY)
		pev->movetype = MOVETYPE_STEP;

	m_Wall = EPantherWall::None;
}

void CPanthereye::Claw()
{
	// The hit sound only for a blow that hurt; a deflect keeps the (scaled)
	// kick and is heard as the Pulse's clang (docs/TECH_DEBT.md).
	bool bLanded = false;
	CBaseEntity* pHurt = CheckTraceHullAttack(PANTHER_CLAW_REACH, gSkillData.panthereyeDmgClaw, DMG_SLASH, &bLanded);

	if (pHurt)
	{
		if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
		{
			pHurt->pev->punchangle.z = RANDOM_LONG(0, 1) ? -12 : 12;
			pHurt->pev->punchangle.x = 5;
		}

		if (bLanded)
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 90 + RANDOM_LONG(-5, 5));
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 90 + RANDOM_LONG(-5, 5));
	}
}

//=========================================================
// LeapVelocity -- the headcrab's leap (dlls/headcrab.cpp), at the
// Panthereye's size: up fast enough to reach the target's height, across in
// the time that takes, capped at panther_leap_speed.
//=========================================================
Vector CPanthereye::LeapVelocity(const Vector& vecTarget)
{
	float flGravity = g_psv_gravity->value;
	if (flGravity <= 1)
		flGravity = 1;

	float flHeight = vecTarget.z - pev->origin.z;
	if (flHeight < 16)
		flHeight = 16;

	const float flSpeed = sqrt(2 * flGravity * flHeight);
	const float flTime = flSpeed / flGravity;

	Vector vecJump = (vecTarget - pev->origin) * (1.0f / flTime);
	vecJump.z = flSpeed;

	const float flCap = V_max(100.0f, panther_leap_speed.value);
	const float flLength = vecJump.Length();
	if (flLength > flCap)
		vecJump = vecJump * (flCap / flLength);

	return vecJump;
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
		ClearBits(pev->flags, FL_ONGROUND);
		UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, 1));
		UTIL_MakeVectors(pev->angles);

		Vector vecJump;
		CBaseEntity* pEnemy = m_hEnemy;

		if (m_Wall == EPantherWall::Chosen)
		{
			vecJump = LeapVelocity(m_vecWallTarget);
			m_Wall = EPantherWall::ToWall;
			m_flWallTime = gpGlobals->time;
		}
		else if (pEnemy != NULL)
		{
			vecJump = LeapVelocity(pEnemy->pev->origin + pEnemy->pev->view_ofs);
		}
		else
		{
			vecJump = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

		pev->velocity = vecJump;
		m_flNextLeap = gpGlobals->time + V_max(0.0f, panther_leap_cooldown.value);

		// The next Circling starts a fresh clock.
		m_flCircleUntil = 0;
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// LeapTouch -- once per leap, while in the air.  A miss lands it, and the
// rest of crouch_to_jump is the recovery the player punishes.  On the way to
// a wall, touching the wall is the cling: held there, turned to the enemy,
// for panther_wall_cling, then RunTask lets it go.
//=========================================================
void CPanthereye::LeapTouch(CBaseEntity* pOther)
{
	if (m_Wall == EPantherWall::ToWall && pOther->pev->solid == SOLID_BSP)
	{
		// The floor is BSP too: only a surface ahead of it, near vertical, is
		// the wall.
		const Vector vecCentre = Center();
		TraceResult tr;
		UTIL_TraceLine(vecCentre, vecCentre + m_vecWallDir * (PANTHER_HULL_HALF + 16.0f), ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0f && fabs(tr.vecPlaneNormal.z) <= 0.3f)
		{
			m_Wall = EPantherWall::Cling;
			m_flWallTime = gpGlobals->time + V_max(0.0f, panther_wall_cling.value);

			pev->movetype = MOVETYPE_FLY;
			pev->velocity = g_vecZero;
			pev->framerate = 0;

			CBaseEntity* pEnemy = m_hEnemy;
			if (pEnemy != NULL)
			{
				pev->angles.y = UTIL_VecToYaw(pEnemy->pev->origin - pev->origin);
				pev->ideal_yaw = pev->angles.y;
			}
			return;
		}
	}

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
	// The Revealed and Glimpsed interrupts have done their job once a
	// schedule is being chosen.
	ClearConditions(bits_COND_SPECIAL1 | bits_COND_SPECIAL2);

	if (IsStalking() && !HasConditions(bits_COND_ENEMY_DEAD))
	{
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK2))
			return GetScheduleOfType(SCHED_MELEE_ATTACK2);

		if (m_bGlimpsed)
			return slPantherCircle;

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
		// UnRevealed, acquiring the player is the start of the stalk, silently
		// -- the alert is for being Revealed -- and a chase is a stalk.
		if (!m_bRevealed)
			return m_bGlimpsed ? slPantherCircle : slPantherStalk;

		// Revealed and in the pounce band, a chase is combat Circling.
		if (Type == SCHED_CHASE_ENEMY && m_hEnemy != NULL && HasConditions(bits_COND_SEE_ENEMY) &&
			(m_hEnemy->pev->origin - pev->origin).Length() <= panther_leap_max.value)
			return slPantherCircle;
		break;

	case SCHED_MELEE_ATTACK1:
		// Sometimes by way of a wall.
		if (m_bRevealed && RANDOM_FLOAT(0.0f, 1.0f) < panther_wall_chance.value && FindWall())
		{
			m_Wall = EPantherWall::Chosen;
			return slPantherWallPounce;
		}
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

//=========================================================
// Damage.  The model's head is hitgroup 2, which counts as chest; make it a
// head so a headshot is one.  And being hurt by the player Reveals it.
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
			BecomeRevealed("hurt");
	}

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPanthereye::Killed(entvars_t* pevAttacker, int iGib)
{
	// Killed on the wall: fall.
	EndWallPounce();
	SetBoneController(0, 0);

	CBaseMonster::Killed(pevAttacker, iGib);
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
// panther_debug -- the Revealed test as it reads right now, and what the
// menace layer is doing.  Built with the test because Revealed is one-way and
// a wrong answer costs the whole stalk: when it flips when it should not
// have, this says why.  Centre print, the same screen space as
// debug_schedule; run one at a time.
//=========================================================
void CPanthereye::DebugReport(CBasePlayer* pPlayer, float flAngle, bool bLine, bool bOnScreen, float flDist)
{
	if (gpGlobals->time < m_flNextDebug && m_flNextDebug < gpGlobals->time + 1.0f)
		return;

	m_flNextDebug = gpGlobals->time + 0.25f;

	static const char* pGaitNames[] = {"upright", "to crawl", "crawl", "to stand"};
	static const char* pWallNames[] = {"-", "chosen", "to wall", "cling", "rebound"};

	const char* pszMode = m_bRevealed ? "COMBAT" : (IsStalking() ? (m_bGlimpsed ? "GLIMPSED" : "STALK") : "unaware");
	const char* pszSchedule = m_pSchedule != NULL ? m_pSchedule->pName : "none";

	const float flTimer = m_flCircleUntil != 0 ? V_max(0.0f, m_flCircleUntil - gpGlobals->time) : -1.0f;

	char szReport[320];
	snprintf(szReport, sizeof(szReport),
		"panthereye %d  %s  %s  %s\nangle %.0f / %.0f  line %s  screen %s\ndwell %.2f / %.2f  dist %.0f  susp %.2f\n"
		"dir %+d  timer %.1f  turn %.0f  wall %s\n",
		entindex(), pszMode, pGaitNames[(int)m_Gait], pszSchedule,
		flAngle, panther_spot_cone.value, bLine ? "yes" : "no", bOnScreen ? "yes" : "no",
		m_flSpotDwell, panther_spot_dwell.value, flDist, m_flSuspicion,
		m_iCircleDir, flTimer, m_flTurn, pWallNames[(int)m_Wall]);

	ClientPrint(pPlayer->pev, HUD_PRINTCENTER, szReport);
}
