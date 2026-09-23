/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Alien slave monster
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "squadmonster.h"
#include "schedule.h"
#include "effects.h"
#include "weapons.h"
#include "soundent.h"
#include "player.h" // the summon weapon's ghost is made for a player, at the end of this file
#include "game.h"	// the slave boss's tuning cvars
#include "shake.h"	// the freeing's flash
#include <algorithm>

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ISLAVE_AE_CLAW (1)
#define ISLAVE_AE_CLAWRAKE (2)
#define ISLAVE_AE_ZAP_POWERUP (3)
#define ISLAVE_AE_ZAP_SHOOT (4)
#define ISLAVE_AE_ZAP_DONE (5)

#define ISLAVE_MAX_BEAMS 8

class CISlave : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	int Classify() override;
	// One of the four primaries. Squad-capable and deliberate, unlike the
	// scavengers it shares a map with.
	const PerceptionProfile& GetPerceptionProfile() override { return g_ProfileTrained; }
	int IRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();
	void ArmBeam(int side);
	void WackBeam(int side, CBaseEntity* pEntity);
	// flDamage below zero is the stock zap's; the boss's Overcharge bolts pass
	// their own.
	void ZapBeam(int side, float flDamage = -1.0f);
	void BeamGlow();

	// What makes a slave the player's, in one place.  The summon weapon's
	// ghost is the first customer; the freed alien slave on the roster
	// (docs/ROADMAP.md, "friendly alien slave") is the second, and the design
	// says the ally half is shared between them and built once, so it lives
	// here rather than in whatever creates the slave.
	//
	// m_bAlly moves the slave into CLASS_PLAYER_ALLY, which is the whole of
	// being on the player's side: the relationship table then has it fight
	// what the player fights and be fought by what fights the player.
	// m_bVanishOnDeath leaves no corpse -- a ghost is not a body -- and is
	// separate because a friendly slave that stays is expected to leave one.
	bool m_bAlly = false;
	bool m_bVanishOnDeath = false;

	// The teleport-out, and gone.  Also how a ghost's lifetime ends.
	void Vanish();

	// A Stagger (CONTEXT.md): what a Discharge, or a Barrier's returned beam,
	// does to a slave -- the attack in hand broken off and a flinch forced,
	// whatever the flinch memory says.  Queued, not applied: it is usually
	// asked for from inside this slave's own ZAP_SHOOT, through the player's
	// TakeDamage, and changing schedule halfway through its own anim event
	// would pull the bolt it is firing out from under it.  MonsterThink applies
	// it on the next think.  Virtual for the boss, whose Stagger is longer and
	// whose Ward can refuse it.
	virtual void Stagger() { m_bStaggerPending = true; }
	void MonsterThink() override;
	bool m_bStaggerPending = false; // transient; a save between the two frames loses nothing worth keeping

	int m_iBravery;

	CBeam* m_pBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int m_voicePitch;

	EHANDLE m_hDead;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};
LINK_ENTITY_TO_CLASS(monster_alien_slave, CISlave);
LINK_ENTITY_TO_CLASS(monster_vortigaunt, CISlave);


TYPEDESCRIPTION CISlave::m_SaveData[] =
	{
		DEFINE_FIELD(CISlave, m_iBravery, FIELD_INTEGER),

		DEFINE_ARRAY(CISlave, m_pBeam, FIELD_CLASSPTR, ISLAVE_MAX_BEAMS),
		DEFINE_FIELD(CISlave, m_iBeams, FIELD_INTEGER),
		DEFINE_FIELD(CISlave, m_flNextAttack, FIELD_TIME),

		DEFINE_FIELD(CISlave, m_voicePitch, FIELD_INTEGER),

		DEFINE_FIELD(CISlave, m_hDead, FIELD_EHANDLE),

		DEFINE_FIELD(CISlave, m_bAlly, FIELD_BOOLEAN),
		DEFINE_FIELD(CISlave, m_bVanishOnDeath, FIELD_BOOLEAN),

};

IMPLEMENT_SAVERESTORE(CISlave, CSquadMonster);




const char* CISlave::pAttackHitSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav",
};

const char* CISlave::pAttackMissSounds[] =
	{
		"zombie/claw_miss1.wav",
		"zombie/claw_miss2.wav",
};

const char* CISlave::pPainSounds[] =
	{
		"aslave/slv_pain1.wav",
		"aslave/slv_pain2.wav",
};

const char* CISlave::pDeathSounds[] =
	{
		"aslave/slv_die1.wav",
		"aslave/slv_die2.wav",
};

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CISlave::Classify()
{
	// An ally slave is a CLASS_PLAYER_ALLY and nothing else: the relationship
	// table does the rest, so being on the player's side is one line here
	// rather than a set of overrides at every place a slave decides anything.
	return m_bAlly ? CLASS_PLAYER_ALLY : CLASS_ALIEN_MILITARY;
}


int CISlave::IRelationship(CBaseEntity* pTarget)
{
	if ((pTarget->IsPlayer()))
		if ((pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED) != 0 && (m_afMemory & bits_MEMORY_PROVOKED) == 0)
			return R_NO;
	return CBaseMonster::IRelationship(pTarget);
}


void CISlave::CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation)
{
	// ALERT( at_aiconsole, "help " );

	// skip ones not on my netname
	if (FStringNull(pev->netname))
		return;

	CBaseEntity* pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname))) != NULL)
	{
		float d = (pev->origin - pEntity->pev->origin).Length();
		if (d < flDist)
		{
			CBaseMonster* pMonster = pEntity->MyMonsterPointer();
			if (pMonster)
			{
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
				pMonster->PushEnemy(hEnemy, vecLocation);
			}
		}
	}
}


//=========================================================
// ALertSound - scream
//=========================================================
void CISlave::AlertSound()
{
	if (m_hEnemy != NULL)
	{
		SENTENCEG_PlayRndSz(ENT(pev), "SLV_ALERT", 0.85, ATTN_NORM, 0, m_voicePitch);

		CallForHelp("monster_alien_slave", 512, m_hEnemy, m_vecEnemyLKP);
	}
}

//=========================================================
// IdleSound
//=========================================================
void CISlave::IdleSound()
{
	if (RANDOM_LONG(0, 2) == 0)
	{
		SENTENCEG_PlayRndSz(ENT(pev), "SLV_IDLE", 0.85, ATTN_NORM, 0, m_voicePitch);
	}

#if 0
	int side = RANDOM_LONG( 0, 1 ) * 2 - 1;

	ClearBeams( );
	ArmBeam( side );

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = pev->origin + gpGlobals->v_right * 2 * side;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 8 );		// radius * 0.1
		WRITE_BYTE( 255 );		// r
		WRITE_BYTE( 180 );		// g
		WRITE_BYTE( 96 );		// b
		WRITE_BYTE( 10 );		// time * 10
		WRITE_BYTE( 0 );		// decay * 0.1
	MESSAGE_END( );

	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "debris/zap1.wav", 1, ATTN_NORM, 0, 100 );
#endif
}

//=========================================================
// PainSound
//=========================================================
void CISlave::PainSound()
{
	if (RANDOM_LONG(0, 2) == 0)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
	}
}

//=========================================================
// DieSound
//=========================================================

void CISlave::DeathSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
}


//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards.
//=========================================================
int CISlave::ISoundMask()
{
	return bits_SOUND_WORLD |
		   bits_SOUND_COMBAT |
		   bits_SOUND_DANGER |
		   bits_SOUND_PLAYER;
}


void CISlave::Killed(entvars_t* pevAttacker, int iGib)
{
	ClearBeams();
	CSquadMonster::Killed(pevAttacker, iGib);

	// Death ends a ghost early, with no refund and no body: it goes the way
	// it arrived rather than falling over.
	if (m_bVanishOnDeath)
		Vanish();
}

//=========================================================
// Vanish - the teleport-out, and gone.  No corpse, so nothing is left to
// gib, to be a Disturbance, or to be walked around.
//=========================================================
void CISlave::Vanish()
{
	ClearBeams();

	// Quake's teleport splash, which needs no sprite of its own; a look of
	// the mod's own is an ART_DEBT entry (docs/ART_DEBT.md).
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_TELEPORT);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	MESSAGE_END();

	UTIL_EmitAmbientSound(ENT(pev), pev->origin, "debris/zap1.wav", 1.0, ATTN_NORM, 0, 100);

	UTIL_Remove(this);
}

//=========================================================
// MonsterThink - applies a queued Stagger, then thinks as normal.
//=========================================================
void CISlave::MonsterThink()
{
	if (m_bStaggerPending)
	{
		m_bStaggerPending = false;

		if (IsAlive() && m_MonsterState != MONSTERSTATE_SCRIPT)
		{
			ClearBeams();
			// Broken off, not merely delayed: the zap it was winding up does not
			// come straight back the moment the flinch ends.
			if (m_flNextAttack < gpGlobals->time + 1.0f)
				m_flNextAttack = gpGlobals->time + 1.0f;
			ChangeSchedule(GetScheduleOfType(SCHED_SMALL_FLINCH));
		}
	}

	CSquadMonster::MonsterThink();
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CISlave::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_WALK:
		ys = 50;
		break;
	case ACT_RUN:
		ys = 70;
		break;
	case ACT_IDLE:
		ys = 50;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CISlave::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	// ALERT( at_console, "event %d : %f\n", pEvent->event, pev->frame );
	switch (pEvent->event)
	{
	case ISLAVE_AE_CLAW:
	{
		// SOUND HERE!
		// The hit sound only for a blow that hurt; a deflect keeps the kick
		// (scaled) and is heard as the Pulse's clang (docs/TECH_DEBT.md).
		bool bLanded = false;
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.slaveDmgClaw, DMG_SLASH, &bLanded);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
			// Play a random attack hit sound
			if (bLanded)
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
		}
		else
		{
			// Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
		}
	}
	break;

	case ISLAVE_AE_CLAWRAKE:
	{
		bool bLanded = false;
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.slaveDmgClawrake, DMG_SLASH, &bLanded);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
			if (bLanded)
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
		}
	}
	break;

	case ISLAVE_AE_ZAP_POWERUP:
	{
		// speed up attack when on hard
		if (g_iSkillLevel == SKILL_HARD)
			pev->framerate = 1.5;

		UTIL_MakeAimVectors(pev->angles);

		if (m_iBeams == 0)
		{
			Vector vecSrc = pev->origin + gpGlobals->v_forward * 2;
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecSrc.x);			 // X
			WRITE_COORD(vecSrc.y);			 // Y
			WRITE_COORD(vecSrc.z);			 // Z
			WRITE_BYTE(12);					 // radius * 0.1
			WRITE_BYTE(255);				 // r
			WRITE_BYTE(180);				 // g
			WRITE_BYTE(96);					 // b
			WRITE_BYTE(20 / pev->framerate); // time * 10
			WRITE_BYTE(0);					 // decay * 0.1
			MESSAGE_END();
		}
		if (m_hDead != NULL)
		{
			WackBeam(-1, m_hDead);
			WackBeam(1, m_hDead);
		}
		else
		{
			ArmBeam(-1);
			ArmBeam(1);
			BeamGlow();
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM, 0, 100 + m_iBeams * 10);
		pev->skin = m_iBeams / 2;
	}
	break;

	case ISLAVE_AE_ZAP_SHOOT:
	{
		ClearBeams();

		if (m_hDead != NULL)
		{
			Vector vecDest = m_hDead->pev->origin + Vector(0, 0, 38);
			TraceResult trace;
			UTIL_TraceHull(vecDest, vecDest, dont_ignore_monsters, human_hull, m_hDead->edict(), &trace);

			if (0 == trace.fStartSolid)
			{
				CBaseEntity* pNew = Create("monster_alien_slave", m_hDead->pev->origin, m_hDead->pev->angles);
				CBaseMonster* pNewMonster = pNew->MyMonsterPointer();
				pNew->pev->spawnflags |= 1;
				WackBeam(-1, pNew);
				WackBeam(1, pNew);
				UTIL_Remove(m_hDead);
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(130, 160));

				/*
					CBaseEntity *pEffect = Create( "test_effect", pNew->Center(), pev->angles );
					pEffect->Use( this, this, USE_ON, 1 );
					*/
				break;
			}
		}
		ClearMultiDamage();

		UTIL_MakeAimVectors(pev->angles);

		ZapBeam(-1);
		ZapBeam(1);

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(130, 160));
		// STOP_SOUND( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
		ApplyMultiDamage(pev, pev);

		m_flNextAttack = gpGlobals->time + RANDOM_FLOAT(0.5, 4.0);
	}
	break;

	case ISLAVE_AE_ZAP_DONE:
	{
		ClearBeams();
	}
	break;

	default:
		CSquadMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// CheckRangeAttack1 - normal beam attack
//=========================================================
bool CISlave::CheckRangeAttack1(float flDot, float flDist)
{
	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	return CSquadMonster::CheckRangeAttack1(flDot, flDist);
}

//=========================================================
// CheckRangeAttack2 - check bravery and try to resurect dead comrades
//=========================================================
bool CISlave::CheckRangeAttack2(float flDot, float flDist)
{
	return false;

	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	m_hDead = NULL;
	m_iBravery = 0;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_alien_slave")) != NULL)
	{
		TraceResult tr;

		UTIL_TraceLine(EyePosition(), pEntity->EyePosition(), ignore_monsters, ENT(pev), &tr);
		if (tr.flFraction == 1.0 || tr.pHit == pEntity->edict())
		{
			if (pEntity->pev->deadflag == DEAD_DEAD)
			{
				float d = (pev->origin - pEntity->pev->origin).Length();
				if (d < flDist)
				{
					m_hDead = pEntity;
					flDist = d;
				}
				m_iBravery--;
			}
			else
			{
				m_iBravery++;
			}
		}
	}
	if (m_hDead != NULL)
		return true;
	else
		return false;
}


//=========================================================
// StartTask
//=========================================================
void CISlave::StartTask(Task_t* pTask)
{
	ClearBeams();

	CSquadMonster::StartTask(pTask);
}


//=========================================================
// Spawn
//=========================================================
void CISlave::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/islave.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = gSkillData.slaveHealth;
	pev->view_ofs = Vector(0, 0, 64);  // position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP;

	m_voicePitch = RANDOM_LONG(85, 110);

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CISlave::Precache()
{
	PRECACHE_MODEL("models/islave.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("zombie/zo_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	UTIL_PrecacheOther("test_effect");
}


//=========================================================
// TakeDamage - get provoked when injured
//=========================================================

bool CISlave::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// don't slash one of your own
	if ((bitsDamageType & DMG_SLASH) != 0 && pevAttacker && IRelationship(Instance(pevAttacker)) < R_DL)
		return false;

	m_afMemory |= bits_MEMORY_PROVOKED;
	return CSquadMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}


void CISlave::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if ((bitsDamageType & DMG_SHOCK) != 0)
		return;

	CSquadMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


//=========================================================
// AI Schedules Specific to this monster
//=========================================================



// primary range attack
Task_t tlSlaveAttack1[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slSlaveAttack1[] =
	{
		{tlSlaveAttack1,
			ARRAYSIZE(tlSlaveAttack1),
			bits_COND_CAN_MELEE_ATTACK1 |
				bits_COND_HEAR_SOUND |
				bits_COND_HEAVY_DAMAGE,

			bits_SOUND_DANGER,
			"Slave Range Attack1"},
};


DEFINE_CUSTOM_SCHEDULES(CISlave){
	slSlaveAttack1,
};

IMPLEMENT_CUSTOM_SCHEDULES(CISlave, CSquadMonster);


//=========================================================
//=========================================================
Schedule_t* CISlave::GetSchedule()
{
	ClearBeams();

	/*
	if (pev->spawnflags)
	{
		pev->spawnflags = 0;
		return GetScheduleOfType( SCHED_RELOAD );
	}
*/

	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound;
		pSound = PBestSound();

		ASSERT(pSound != NULL);

		if (pSound && (pSound->m_iType & bits_SOUND_DANGER) != 0)
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
		if ((pSound->m_iType & bits_SOUND_COMBAT) != 0)
			m_afMemory |= bits_MEMORY_PROVOKED;
	}

	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (pev->health < 20 || m_iBravery < 0)
		{
			if (!HasConditions(bits_COND_CAN_MELEE_ATTACK1))
			{
				m_failSchedule = SCHED_CHASE_ENEMY;
				if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
				{
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
				if (HasConditions(bits_COND_SEE_ENEMY) && HasConditions(bits_COND_ENEMY_FACING_ME))
				{
					// ALERT( at_console, "exposed\n");
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
			}
		}
		break;
	}
	return CSquadMonster::GetSchedule();
}


Schedule_t* CISlave::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_FAIL:
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return CSquadMonster::GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		break;
	case SCHED_RANGE_ATTACK1:
		return slSlaveAttack1;
	case SCHED_RANGE_ATTACK2:
		return slSlaveAttack1;
	}
	return CSquadMonster::GetScheduleOfType(Type);
}


//=========================================================
// ArmBeam - small beam from arm to nearby geometry
//=========================================================

void CISlave::ArmBeam(int side)
{
	TraceResult tr;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	UTIL_MakeAimVectors(pev->angles);
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1);
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT(pev), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if (flDist == 1.0)
		return;

	DecalGunshot(&tr, BULLET_PLAYER_CROWBAR);

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.vecEndPos, entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	// m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetColor(96, 128, 16);
	m_pBeam[m_iBeams]->SetBrightness(64);
	m_pBeam[m_iBeams]->SetNoise(80);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;
}


//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CISlave::BeamGlow()
{
	int b = m_iBeams * 32;
	if (b > 255)
		b = 255;

	for (int i = 0; i < m_iBeams; i++)
	{
		if (m_pBeam[i]->GetBrightness() != 255)
		{
			m_pBeam[i]->SetBrightness(b);
		}
	}
}


//=========================================================
// WackBeam - regenerate dead colleagues
//=========================================================
void CISlave::WackBeam(int side, CBaseEntity* pEntity)
{
	Vector vecDest;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	if (pEntity == NULL)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(pEntity->Center(), entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(180, 255, 96);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(80);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;
}

//=========================================================
// ZapBeam - heavy damage directly forward
//=========================================================
void CISlave::ZapBeam(int side, float flDamage)
{
	if (flDamage < 0)
		flDamage = gSkillData.slaveDmgZap;

	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity* pEntity;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	vecSrc = pev->origin + gpGlobals->v_up * 36;
	vecAim = ShootAtEnemy(vecSrc);
	float deflection = 0.01;
	vecAim = vecAim + side * gpGlobals->v_right * RANDOM_FLOAT(0, deflection) + gpGlobals->v_up * RANDOM_FLOAT(-deflection, deflection);
	UTIL_TraceLine(vecSrc, vecSrc + vecAim * 1024, dont_ignore_monsters, ENT(pev), &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 50);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.vecEndPos, entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(180, 255, 96);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(20);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;

	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && 0 != pEntity->pev->takedamage)
	{
		pEntity->TraceAttack(pev, flDamage, vecAim, &tr, DMG_SHOCK);
	}
	UTIL_EmitAmbientSound(ENT(pev), tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));
}


//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CISlave::ClearBeams()
{
	for (int i = 0; i < ISLAVE_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove(m_pBeam[i]);
			m_pBeam[i] = NULL;
		}
	}
	m_iBeams = 0;
	pev->skin = 0;

	STOP_SOUND(ENT(pev), CHAN_WEAPON, "debris/zap4.wav");
}

//=========================================================
// monster_ghost_slave -- what the summon weapon summons.
//
// The stock alien slave on the player's side, with a lifetime and no corpse
// (docs/ROADMAP.md, "The summon weapon -- settled").  It fights what it sees;
// the timer ends it, and death ends it early with no refund.  Following the
// player at a distance when idle is the design and is NOT built: that is
// CTalkMonster's behaviour and pulling CTalkMonster in for it would make a
// ghost a talker.
//
// Everything that makes it the player's -- the classification and the bodiless
// death -- is CISlave's, two flags above, so the friendly alien slave on the
// roster inherits it rather than having it written twice.  What is this
// class's own is the leash: the summoner it counts against and the clock.
//
// It lives at the end of islave.cpp rather than in summon.cpp because CISlave
// is declared in this file and nowhere else.
//=========================================================
class CGhostSlave : public CISlave
{
public:
	void Spawn() override;
	void MonsterThink() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	// A ghost is a thing of the fight it was summoned into and does not
	// follow the player through a level change.
	int ObjectCaps() override { return CISlave::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHANDLE m_hSummoner;        // whose cap this ghost counts against
	float m_flVanishTime = 0.0f; // when the lifetime is up; 0 for no clock
};

LINK_ENTITY_TO_CLASS(monster_ghost_slave, CGhostSlave);

TYPEDESCRIPTION CGhostSlave::m_SaveData[] =
	{
		DEFINE_FIELD(CGhostSlave, m_hSummoner, FIELD_EHANDLE),
		DEFINE_FIELD(CGhostSlave, m_flVanishTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CGhostSlave, CISlave);

void CGhostSlave::Spawn()
{
	// Before the slave's own Spawn: MonsterInit asks Classify, and a ghost
	// that starts as CLASS_ALIEN_MILITARY is briefly an enemy of the thing
	// that summoned it.
	m_bAlly = true;
	m_bVanishOnDeath = true;

	CISlave::Spawn();

	// The model is the stock slave's, so being drawn through is the whole of
	// what says this one is not.  A ghost of its own is an ART_DEBT entry.
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 160;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_TELEPORT);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	MESSAGE_END();

	UTIL_EmitAmbientSound(ENT(pev), pev->origin, "debris/zap1.wav", 1.0, ATTN_NORM, 0, 120);
}

void CGhostSlave::MonsterThink()
{
	// The clock is read here rather than on a think of its own because a
	// monster's think IS its AI: taking it over would stop the ghost fighting.
	if (m_flVanishTime != 0 && gpGlobals->time >= m_flVanishTime)
	{
		Vanish();
		return;
	}

	CISlave::MonsterThink();
}

//=========================================================
// The two calls dlls/summon.cpp makes.  Declared extern there: CISlave and
// CGhostSlave are visible in this file alone, and the weapon needs no more of
// them than "make one" and "how many are out".
//=========================================================
CBaseEntity* SummonCreateGhost(CBasePlayer* pOwner, const Vector& vecOrigin, float flLifetime)
{
	// Facing the way the player faces, so a ghost summoned behind the player
	// is already looking at what the player is looking at.  No owner: an
	// owned entity is skipped by traces the player makes, and a ghost is
	// something the player's own shots should be able to hit.
	CBaseEntity* pEntity = CBaseEntity::Create("monster_ghost_slave", vecOrigin,
		Vector(0, pOwner->pev->v_angle.y, 0), NULL);
	if (pEntity == nullptr)
		return nullptr;

	CGhostSlave* pGhost = static_cast<CGhostSlave*>(pEntity);
	pGhost->m_hSummoner = pOwner;
	pGhost->m_flVanishTime = flLifetime > 0.0f ? gpGlobals->time + flLifetime : 0.0f;
	return pGhost;
}

int SummonCountGhosts(CBasePlayer* pOwner)
{
	int iCount = 0;
	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_ghost_slave")) != nullptr)
	{
		// A ghost on its way out is already spent: it does not hold a place
		// in the pack the player is about to fill.
		if (pEntity->pev->deadflag != DEAD_NO || (pEntity->pev->flags & FL_KILLME) != 0)
			continue;

		CGhostSlave* pGhost = static_cast<CGhostSlave*>(pEntity);
		if (static_cast<CBaseEntity*>(pGhost->m_hSummoner) == static_cast<CBaseEntity*>(pOwner))
			iCount++;
	}
	return iCount;
}

//=========================================================
// monster_alien_slave_boss -- the alien slave boss.
//
// The design is docs/ROADMAP.md, "The alien slave boss"; the words are
// CONTEXT.md's.  In short:
//
//   - The Ward, a green glow shell, takes every blow -- melee included --
//     before his health does, and absorbs every Discharge while it stands.
//   - Down, it stays down until his next Overcharge resolves, then comes back.
//   - The Overcharge is his attack like any other: a long channel, eight bolts.
//     A Discharge off one with the Ward down breaks a Binding (left bracelet,
//     right bracelet, collar) and deals his health nothing.  It is the only
//     thing that moves the fight on, so the rotation never goes more than two
//     ranged attacks without one.
//   - The Bindings drive the escalation: the volley replaces the stock zap
//     after the first, and the Overcharge winds up faster after the second.
//   - The third frees him: collar1, the beam-out, target_freed.  Killed
//     instead -- only by out-shooting him through the Ward's windows -- he
//     fires target_killed and drops the alien Module.
//   - He never flees, and damage never makes him flinch; only a Stagger does.
//
// A Discharge off his own zap reaches him inside the ApplyMultiDamage that zap
// is in: the player's TakeDamage negates it and fires the Discharge back
// before the zap's call returns.  m_iFiring is what tells such a Discharge from
// any other energy the player deals -- it is only ever set around that call.
//=========================================================
enum
{
	TASK_BOSS_OVERCHARGE = LAST_COMMON_TASK + 1,
	TASK_BOSS_VOLLEY,
	TASK_BOSS_COLLAR,	// plays a collar sequence by name: flData 1 is collar1 (the freeing), 0 collar2 (a Binding)
	TASK_BOSS_LINGER,	// freed, he stands until slaveboss_freed_linger has passed since the collar broke
	TASK_BOSS_BEAM_OUT, // target_freed, then the teleport-out
};

enum EBossAttack
{
	BOSS_ATTACK_NONE = 0,
	BOSS_ATTACK_VOLLEY,
	BOSS_ATTACK_OVERCHARGE,
};

enum EBossFiring
{
	BOSS_FIRING_NONE = 0,
	BOSS_FIRING_ZAP,
	BOSS_FIRING_VOLLEY,
	BOSS_FIRING_OVERCHARGE,
};

// zapattack1 is 35 frames at 15 fps: the last ZAP_POWERUP is on frame 15 and
// ZAP_SHOOT on frame 24, 1.6 s in.  The Overcharge plays those 24 frames slowed
// to fill its channel.  The volley holds the pose on frame 17 instead -- arms
// up, charged -- which is 17 * 256 / 34 in pev->frame's units, since it fires
// on its own clock and has no single shot to land.
static constexpr float k_BossHoldFrame = 128.0f;
static constexpr float k_BossShotTime = 24.0f / 15.0f;
static constexpr int k_BossOverchargeBolts = 8; // exactly ISLAVE_MAX_BEAMS: the shot clears the channel's beams first

// Stand-ins, all stock, each in its own timbre (docs/ART_DEBT.md, "The alien
// slave boss -- sounds"): the Nihilanth's own charge for the Overcharge, since
// the power is his; a short rise for the volley; a crack for a Binding.
static const char* const k_BossSoundOvercharge = "x/x_teleattack1.wav";
static const char* const k_BossSoundVolley = "debris/beamstart7.wav";
static const char* const k_BossSoundBinding = "buttons/spark6.wav";

static const Vector k_BossGreen(96, 255, 96);

class CISlaveBoss : public CISlave
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void MonsterThink() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void Stagger() override;

	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	void SetWard(bool bUp);
	void BreakWard();
	void BreakBinding();
	void FireOvercharge();
	void FireVolleyZap();
	void ChargeBeam();
	void HoldPose();
	bool IsDischargeNow(entvars_t* pevAttacker, int bitsDamageType) const;
	Vector BindingPosition(int iBinding);

	// Saved.
	float m_flWard = 0;
	bool m_bWardUp = false;
	int m_iBindings = 0; // broken, 0..3
	bool m_bFreeing = false;
	float m_flFreedAt = 0; // when the collar broke
	int m_iSinceOvercharge = 0;
	string_t m_iszTargetFreed = 0;
	string_t m_iszTargetKilled = 0;

	// Transient: an attack in hand is restarted from scratch by a load.
	int m_iAttack = BOSS_ATTACK_NONE;
	int m_iFiring = BOSS_FIRING_NONE;
	bool m_bOverchargeCaught = false;
	bool m_bLongStaggerPending = false;
	bool m_bFreeingPending = false;
	float m_flChargeRate = 1.0f;    // the Overcharge's wind-up framerate
	bool m_bOverchargeFired = false; // its shot has gone; the rest plays at stock speed
	float m_flNextChargeBeam = 0;
	int m_iVolleyShots = 0;
	float m_flNextVolleyShot = 0;
	float m_flClearBeamsAt = 0;
	float m_flNextSpark = 0;
};

LINK_ENTITY_TO_CLASS(monster_alien_slave_boss, CISlaveBoss);

TYPEDESCRIPTION CISlaveBoss::m_SaveData[] =
	{
		DEFINE_FIELD(CISlaveBoss, m_flWard, FIELD_FLOAT),
		DEFINE_FIELD(CISlaveBoss, m_bWardUp, FIELD_BOOLEAN),
		DEFINE_FIELD(CISlaveBoss, m_iBindings, FIELD_INTEGER),
		DEFINE_FIELD(CISlaveBoss, m_bFreeing, FIELD_BOOLEAN),
		DEFINE_FIELD(CISlaveBoss, m_flFreedAt, FIELD_TIME),
		DEFINE_FIELD(CISlaveBoss, m_iSinceOvercharge, FIELD_INTEGER),
		DEFINE_FIELD(CISlaveBoss, m_iszTargetFreed, FIELD_STRING),
		DEFINE_FIELD(CISlaveBoss, m_iszTargetKilled, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CISlaveBoss, CISlave);

//=========================================================
// Schedules.  None of them is interrupted by damage: a boss that flinched
// from every bullet could be held in place by anyone with a magazine.
//=========================================================
Task_t tlBossZap[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slBossZap[] =
	{
		{tlBossZap, ARRAYSIZE(tlBossZap), 0, 0, "Slave Boss Zap"},
};

Task_t tlBossOvercharge[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_ENEMY, (float)0},
		{TASK_BOSS_OVERCHARGE, (float)0},
};

Schedule_t slBossOvercharge[] =
	{
		{tlBossOvercharge, ARRAYSIZE(tlBossOvercharge), 0, 0, "Slave Boss Overcharge"},
};

Task_t tlBossVolley[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_ENEMY, (float)0},
		{TASK_BOSS_VOLLEY, (float)0},
};

Schedule_t slBossVolley[] =
	{
		{tlBossVolley, ARRAYSIZE(tlBossVolley), 0, 0, "Slave Boss Volley"},
};

Task_t tlBossBindingBreak[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_BOSS_COLLAR, (float)0},
};

Schedule_t slBossBindingBreak[] =
	{
		{tlBossBindingBreak, ARRAYSIZE(tlBossBindingBreak), 0, 0, "Slave Boss Binding Break"},
};

Task_t tlBossFreed[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_BOSS_COLLAR, (float)1},
		{TASK_BOSS_LINGER, (float)0},
		{TASK_BOSS_BEAM_OUT, (float)0},
};

Schedule_t slBossFreed[] =
	{
		{tlBossFreed, ARRAYSIZE(tlBossFreed), 0, 0, "Slave Boss Freed"},
};

DEFINE_CUSTOM_SCHEDULES(CISlaveBoss){
	slBossZap,
	slBossOvercharge,
	slBossVolley,
	slBossBindingBreak,
	slBossFreed,
};

IMPLEMENT_CUSTOM_SCHEDULES(CISlaveBoss, CISlave);

//=========================================================
// Spawn, Precache, KeyValue
//=========================================================
void CISlaveBoss::Spawn()
{
	CISlave::Spawn();

	pev->health = std::max(1.0f, slaveboss_health.value);
	pev->max_health = pev->health;

	SetWard(true);
}

void CISlaveBoss::Precache()
{
	CISlave::Precache();

	PRECACHE_SOUND(k_BossSoundOvercharge);
	PRECACHE_SOUND(k_BossSoundVolley);
	PRECACHE_SOUND(k_BossSoundBinding);
	PRECACHE_SOUND("weapons/electro6.wav");

	UTIL_PrecacheOther("item_alienmodule");
}

bool CISlaveBoss::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "target_freed"))
	{
		m_iszTargetFreed = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	if (FStrEq(pkvd->szKeyName, "target_killed"))
	{
		m_iszTargetKilled = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CISlave::KeyValue(pkvd);
}

//=========================================================
// The Ward
//=========================================================
void CISlaveBoss::SetWard(bool bUp)
{
	m_bWardUp = bUp;

	if (bUp)
	{
		m_flWard = std::max(1.0f, slaveboss_ward.value);
		pev->renderfx = kRenderFxGlowShell;
		pev->rendercolor = k_BossGreen;
		pev->renderamt = 10; // the shell's thickness
	}
	else
	{
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 0;
	}
}

void CISlaveBoss::BreakWard()
{
	SetWard(false);

	UTIL_Sparks(Center());
	UTIL_Sparks(Center() + Vector(0, 0, 20));
	UTIL_EmitAmbientSound(ENT(pev), Center(), "debris/zap1.wav", 1.0, ATTN_NORM, 0, 60);
}

//=========================================================
// The Bindings
//=========================================================
Vector CISlaveBoss::BindingPosition(int iBinding)
{
	Vector vecPos, vecAngles;

	// The beams leave from beam attachments 2 (left hand) and 1 (right);
	// beam attachments count from 1, the model's from 0.  The collar has no
	// attachment of its own, so it is a point at the neck.
	if (iBinding == 0)
		GetAttachment(1, vecPos, vecAngles);
	else if (iBinding == 1)
		GetAttachment(0, vecPos, vecAngles);
	else
	{
		UTIL_MakeVectors(pev->angles);
		vecPos = pev->origin + Vector(0, 0, 58) + gpGlobals->v_forward * 6;
	}

	return vecPos;
}

void CISlaveBoss::BreakBinding()
{
	if (m_iBindings >= 3)
		return;

	const Vector vecPos = BindingPosition(m_iBindings);
	++m_iBindings;

	for (int i = 0; i < 3; i++)
		UTIL_Sparks(vecPos);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecPos);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecPos.x);
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_BYTE(24); // radius, in tens
	WRITE_BYTE(96);
	WRITE_BYTE(255);
	WRITE_BYTE(96);
	WRITE_BYTE(6);	// life, in tenths
	WRITE_BYTE(20); // decay, in tens
	MESSAGE_END();

	UTIL_ScreenShake(pev->origin, 8.0f, 5.0f, 1.0f, 700.0f);
	UTIL_EmitAmbientSound(ENT(pev), vecPos, k_BossSoundBinding, 1.0, ATTN_NORM, 0, 70);

	if (m_iBindings >= 3)
	{
		// Freed.  The fight is over: nothing hurts him from here, and the
		// Ward is gone for good.
		m_bFreeing = true;
		m_bFreeingPending = true;
		m_flFreedAt = gpGlobals->time;
		SetWard(false);
		pev->takedamage = DAMAGE_NO;
		UTIL_ScreenFadeAll(k_BossGreen, 1.0f, 0.0f, 90, FFADE_IN);
	}
	else
	{
		m_bLongStaggerPending = true;
	}
}

//=========================================================
// Damage
//=========================================================
bool CISlaveBoss::IsDischargeNow(entvars_t* pevAttacker, int bitsDamageType) const
{
	if (m_iFiring == BOSS_FIRING_NONE || (bitsDamageType & DMG_ENERGYBEAM) == 0 || !pevAttacker)
		return false;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
	return pAttacker && pAttacker->IsPlayer();
}

void CISlaveBoss::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (m_bFreeing)
		return;

	if (IsDischargeNow(pevAttacker, bitsDamageType))
	{
		// A Discharge off an Overcharge, with the Ward down: the Binding
		// breaks when the shot resolves, and his health is not touched.
		if (m_iFiring == BOSS_FIRING_OVERCHARGE && !m_bWardUp)
			m_bOverchargeCaught = true;

		// Absorbed whole while the Ward stands: no damage, not even to the
		// Ward, and no Stagger (Stagger checks the Ward itself).
		if (m_bWardUp || m_iFiring == BOSS_FIRING_OVERCHARGE)
		{
			UTIL_Ricochet(ptr->vecEndPos, 2.0f);
			return;
		}
	}

	if (m_bWardUp)
	{
		// Straight to the Ward: no blood, no hitgroup multiplier.  Shock is
		// still his kind's and does nothing at all.
		if ((bitsDamageType & DMG_SHOCK) != 0)
			return;

		UTIL_Ricochet(ptr->vecEndPos, 1.0f);
		AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
		return;
	}

	CISlave::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

bool CISlaveBoss::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_bFreeing)
		return false;

	if (m_bWardUp)
	{
		if ((bitsDamageType & DMG_SHOCK) != 0)
			return false;

		m_afMemory |= bits_MEMORY_PROVOKED;
		m_flWard -= flDamage;
		if (m_flWard <= 0)
			BreakWard();
		return false;
	}

	return CISlave::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CISlaveBoss::Stagger()
{
	// The Ward refuses it; the Overcharge's own resolution staggers him, long,
	// if a Binding breaks; and a freed slave is past being hit.
	if (m_bWardUp || m_bFreeing || m_iFiring == BOSS_FIRING_OVERCHARGE || m_iAttack == BOSS_ATTACK_OVERCHARGE)
		return;

	CISlave::Stagger();
}

void CISlaveBoss::Killed(entvars_t* pevAttacker, int iGib)
{
	// A channel cut short by death does not go on charging.
	STOP_SOUND(ENT(pev), CHAN_BODY, k_BossSoundOvercharge);
	STOP_SOUND(ENT(pev), CHAN_BODY, k_BossSoundVolley);
	m_iAttack = BOSS_ATTACK_NONE;
	pev->framerate = 1.0f;

	if (!m_bFreeing)
	{
		// Killed, not freed: he dies with his Bindings on.  The alien Module
		// drops from him so the Alien Route stays open on both paths.
		SetWard(false);
		CBaseEntity::Create("item_alienmodule", pev->origin + Vector(0, 0, 24), pev->angles, nullptr);

		if (!FStringNull(m_iszTargetKilled))
			FireTargets(STRING(m_iszTargetKilled), this, this, USE_TOGGLE, 0);
	}

	CISlave::Killed(pevAttacker, iGib);
}

//=========================================================
// The attacks
//=========================================================
void CISlaveBoss::HoldPose()
{
	if (pev->frame >= k_BossHoldFrame)
	{
		pev->frame = k_BossHoldFrame;
		pev->framerate = 0;
	}
}

void CISlaveBoss::ChargeBeam()
{
	if (m_iBeams >= ISLAVE_MAX_BEAMS)
	{
		BeamGlow();
		return;
	}

	// Beams to points round him rather than to the nearest wall: the stock
	// ArmBeam draws nothing with no geometry inside 512 units, and a boss
	// arena is open.
	UTIL_MakeAimVectors(pev->angles);
	const int side = (m_iBeams % 2) != 0 ? 1 : -1;
	const Vector vecEnd = pev->origin + gpGlobals->v_right * side * RANDOM_FLOAT(40, 110) + gpGlobals->v_forward * RANDOM_FLOAT(-60, 60) + gpGlobals->v_up * RANDOM_FLOAT(0, 96);

	CBeam* pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!pBeam)
		return;

	pBeam->PointEntInit(vecEnd, entindex());
	pBeam->SetEndAttachment(side < 0 ? 2 : 1);
	pBeam->SetColor(96, 255, 96);
	pBeam->SetNoise(80);
	pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;
	m_pBeam[m_iBeams++] = pBeam;

	BeamGlow();
	pev->skin = m_iBeams / 2;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 40);
	WRITE_BYTE(8 + m_iBeams * 2); // radius, in tens, growing with the charge
	WRITE_BYTE(96);
	WRITE_BYTE(255);
	WRITE_BYTE(96);
	WRITE_BYTE(4); // life, in tenths
	WRITE_BYTE(0); // decay
	MESSAGE_END();
}

void CISlaveBoss::FireOvercharge()
{
	m_bOverchargeFired = true;
	ClearBeams();
	UTIL_MakeAimVectors(pev->angles);

	ClearMultiDamage();
	m_iFiring = BOSS_FIRING_OVERCHARGE;
	for (int i = 0; i < k_BossOverchargeBolts; i++)
		ZapBeam((i % 2) != 0 ? 1 : -1, std::max(0.0f, slaveboss_bolt_damage.value));
	ApplyMultiDamage(pev, pev);
	m_iFiring = BOSS_FIRING_NONE;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(90, 100));
	m_flClearBeamsAt = gpGlobals->time + 0.3f;

	// The Overcharge resolves: a Binding if it was caught with the Ward
	// down, and the Ward back either way -- the one rule that ends a window.
	if (!m_bWardUp)
	{
		if (m_bOverchargeCaught)
			BreakBinding();
		if (!m_bFreeing)
			SetWard(true);
	}
	m_bOverchargeCaught = false;
}

void CISlaveBoss::FireVolleyZap()
{
	ClearBeams();
	UTIL_MakeAimVectors(pev->angles);

	ClearMultiDamage();
	m_iFiring = BOSS_FIRING_VOLLEY;
	ZapBeam(-1);
	ZapBeam(1);
	ApplyMultiDamage(pev, pev);
	m_iFiring = BOSS_FIRING_NONE;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(130, 160));
	m_flClearBeamsAt = gpGlobals->time + 0.2f;
}

void CISlaveBoss::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ISLAVE_AE_ZAP_POWERUP:
		// The channel makes its own beams; the stock wind-up's need walls.
		if (m_iAttack == BOSS_ATTACK_OVERCHARGE)
			return;
		if (m_iAttack == BOSS_ATTACK_VOLLEY)
		{
			ChargeBeam();
			return;
		}
		CISlave::HandleAnimEvent(pEvent);
		return;

	case ISLAVE_AE_ZAP_SHOOT:
		if (m_iAttack == BOSS_ATTACK_OVERCHARGE)
		{
			FireOvercharge();
			return;
		}
		if (m_iAttack == BOSS_ATTACK_VOLLEY)
			return; // the volley fired on its own clock while the pose was held

		m_iFiring = BOSS_FIRING_ZAP;
		CISlave::HandleAnimEvent(pEvent);
		m_iFiring = BOSS_FIRING_NONE;
		return;

	default:
		CISlave::HandleAnimEvent(pEvent);
		return;
	}
}

//=========================================================
// Tasks
//=========================================================
void CISlaveBoss::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_BOSS_OVERCHARGE:
	{
		ClearBeams();
		m_iAttack = BOSS_ATTACK_OVERCHARGE;
		m_IdealActivity = ACT_RANGE_ATTACK1;
		pev->framerate = 1.0f;

		// The stock wind-up, slowed so its shot lands when the channel ends
		// (Andrei, 2026-09-23: slowed rather than frozen on a held pose).
		// Applied in RunTask every frame, since SetActivity resets the
		// framerate to 1 after this returns.
		const float flCharge = std::max(0.2f,
			m_iBindings >= 2 ? slaveboss_charge_fast.value : slaveboss_charge.value);
		m_flChargeRate = k_BossShotTime / flCharge;
		m_bOverchargeFired = false;
		m_flNextChargeBeam = gpGlobals->time;

		EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, k_BossSoundOvercharge, 1.0, ATTN_NORM, 0, 100);
		break;
	}

	case TASK_BOSS_VOLLEY:
		ClearBeams();
		m_iAttack = BOSS_ATTACK_VOLLEY;
		m_IdealActivity = ACT_RANGE_ATTACK1;
		pev->framerate = 1.0f;
		m_iVolleyShots = 0;
		m_flNextVolleyShot = gpGlobals->time + 1.2f; // the arms are up by then
		EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, k_BossSoundVolley, 1.0, ATTN_NORM, 0, 100);
		break;

	case TASK_BOSS_COLLAR:
	{
		ClearBeams();
		m_iAttack = BOSS_ATTACK_NONE;

		// By name: both collar sequences are ACT_SPECIAL_ATTACK1, and an
		// activity would pick between them at random.
		const int iSequence = LookupSequence(pTask->flData > 0 ? "collar1" : "collar2");
		if (iSequence < 0)
		{
			TaskComplete();
			break;
		}

		pev->sequence = iSequence;
		pev->frame = 0;
		pev->framerate = 1.0f;
		ResetSequenceInfo();
		m_Activity = m_IdealActivity = ACT_SPECIAL_ATTACK1;
		break;
	}

	case TASK_BOSS_LINGER:
		// Standing, free, before he goes (Andrei, 2026-09-23): five seconds
		// from the collar breaking, the collar animation inside them.
		m_IdealActivity = ACT_IDLE;
		if (gpGlobals->time >= m_flFreedAt + std::max(0.0f, slaveboss_freed_linger.value))
			TaskComplete();
		break;

	case TASK_BOSS_BEAM_OUT:
		if (!FStringNull(m_iszTargetFreed))
			FireTargets(STRING(m_iszTargetFreed), this, this, USE_TOGGLE, 0);
		TaskComplete();
		Vanish();
		break;

	default:
		CISlave::StartTask(pTask);
		break;
	}
}

void CISlaveBoss::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_BOSS_OVERCHARGE:
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (!m_bOverchargeFired)
		{
			pev->framerate = m_flChargeRate;
			if (gpGlobals->time >= m_flNextChargeBeam)
			{
				ChargeBeam();
				m_flNextChargeBeam = gpGlobals->time + 0.25f;
			}
		}
		else
		{
			// The recovery after the shot at the stock speed.
			pev->framerate = 1.0f;
		}

		if (m_fSequenceFinished)
		{
			m_iAttack = BOSS_ATTACK_NONE;
			m_Activity = ACT_RESET;
			m_flNextAttack = gpGlobals->time + RANDOM_FLOAT(1.0f, 2.0f);
			TaskComplete();
		}
		break;

	case TASK_BOSS_VOLLEY:
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_flClearBeamsAt != 0 && gpGlobals->time >= m_flClearBeamsAt)
		{
			m_flClearBeamsAt = 0;
			ClearBeams();
		}

		if (m_iVolleyShots < std::max(1, (int)slaveboss_volley_count.value))
		{
			HoldPose();
			if (gpGlobals->time >= m_flNextVolleyShot)
			{
				FireVolleyZap();
				++m_iVolleyShots;
				m_flNextVolleyShot = gpGlobals->time + std::max(0.1f, slaveboss_volley_gap.value);
			}
		}
		else
		{
			pev->framerate = 1.0f;
		}

		if (m_fSequenceFinished)
		{
			m_iAttack = BOSS_ATTACK_NONE;
			m_Activity = ACT_RESET;
			m_flNextAttack = gpGlobals->time + RANDOM_FLOAT(1.0f, 2.0f);
			TaskComplete();
		}
		break;

	case TASK_BOSS_COLLAR:
		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;

	case TASK_BOSS_LINGER:
		if (gpGlobals->time >= m_flFreedAt + std::max(0.0f, slaveboss_freed_linger.value))
			TaskComplete();
		break;

	default:
		CISlave::RunTask(pTask);
		break;
	}
}

//=========================================================
// Thinking and choosing
//=========================================================
void CISlaveBoss::MonsterThink()
{
	if (m_bFreeingPending)
	{
		m_bFreeingPending = false;
		m_bLongStaggerPending = false;
		m_bStaggerPending = false;
		m_iAttack = BOSS_ATTACK_NONE;
		ChangeSchedule(slBossFreed);
	}
	else if (m_bLongStaggerPending)
	{
		m_bLongStaggerPending = false;
		m_bStaggerPending = false;
		m_iAttack = BOSS_ATTACK_NONE;
		ChangeSchedule(slBossBindingBreak);
	}
	else if (m_bStaggerPending)
	{
		// The slave's own MonsterThink applies it; the attack in hand is
		// dropped here so the held pose does not outlive it.
		m_iAttack = BOSS_ATTACK_NONE;
		pev->framerate = 1.0f;
	}

	if (m_iAttack == BOSS_ATTACK_OVERCHARGE && m_flClearBeamsAt != 0 && gpGlobals->time >= m_flClearBeamsAt)
	{
		m_flClearBeamsAt = 0;
		ClearBeams();
	}

	// A broken Binding goes on sparking, so the player can count them.
	if (IsAlive() && m_iBindings > 0 && gpGlobals->time >= m_flNextSpark)
	{
		for (int i = 0; i < m_iBindings && i < 3; i++)
			UTIL_Sparks(BindingPosition(i));
		m_flNextSpark = gpGlobals->time + RANDOM_FLOAT(1.5f, 3.0f);
	}

	CISlave::MonsterThink();
}

Schedule_t* CISlaveBoss::GetSchedule()
{
	// Damage never makes him flinch; only a Stagger does.
	ClearConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);

	m_iAttack = BOSS_ATTACK_NONE;
	pev->framerate = 1.0f;
	ClearBeams();

	if (m_bFreeing)
		return slBossFreed;

	// Past the slave's own GetSchedule on purpose: that is where it runs for
	// cover at low health, and he never flees.
	return CSquadMonster::GetSchedule();
}

Schedule_t* CISlaveBoss::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
	case SCHED_RANGE_ATTACK2:
		// The rotation.  An Overcharge by chance, and never more than two
		// ranged attacks without one: it is the only thing that advances the
		// fight, and the Ward's window lasts until the next one.
		if (m_iSinceOvercharge >= 2 || RANDOM_FLOAT(0, 1) < slaveboss_overcharge_chance.value)
		{
			m_iSinceOvercharge = 0;
			return slBossOvercharge;
		}
		++m_iSinceOvercharge;
		return m_iBindings >= 1 ? slBossVolley : slBossZap;
	}

	return CISlave::GetScheduleOfType(Type);
}

//=========================================================
// The two things the Pulse asks about slaves (docs/adr/0016).  Declared
// extern in player_pulse.cpp, the summon weapon's pattern: CISlave is visible
// in this file alone.
//
// By classname, because nothing else marks a slave: every class this file
// links, and whatever derives from CISlave further down it.
//=========================================================
static CISlave* AsSlave(CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->MyMonsterPointer())
		return nullptr;

	if (FClassnameIs(pEntity->pev, "monster_alien_slave") || FClassnameIs(pEntity->pev, "monster_vortigaunt") || FClassnameIs(pEntity->pev, "monster_ghost_slave") || FClassnameIs(pEntity->pev, "monster_alien_slave_boss"))
		return static_cast<CISlave*>(pEntity);

	return nullptr;
}

// Was this blow a slave's beam?  The zap is DMG_SHOCK with the slave as its
// inflictor (ZapBeam's TraceAttack, then ApplyMultiDamage( pev, pev )).
bool SlaveBeamFrom(entvars_t* pevInflictor, int bitsDamageType)
{
	if ((bitsDamageType & DMG_SHOCK) == 0 || !pevInflictor)
		return false;

	return AsSlave(CBaseEntity::Instance(pevInflictor)) != nullptr;
}

// Stagger it, if it is a slave.  Anything else is untouched.
void SlaveStagger(CBaseEntity* pEntity)
{
	if (CISlave* pSlave = AsSlave(pEntity))
		pSlave->Stagger();
}
