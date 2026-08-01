//=========================================================
// player_pulse.cpp
//
// The Pulse.  See player_pulse.h and docs/PILLARS.md pillar 2.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "player_pulse.h"
#include "UserMessages.h"
#include "game.h"
#include <algorithm>

//=========================================================
// Damage the Shield turns away.
//
// This list is deliberate, not a convenience -- see
// docs/adr/0005-the-shield-negates-a-curated-damage-list.md.
// A damage type that is not here is NOT negated, and adding one
// to the game means deciding which side of this line it falls on.
//
// DMG_FALL is excluded because the FallResistance Skill already
// sells that, and everything under DMG_TIMEBASED is excluded
// because it ticks rather than lands.
//=========================================================
static constexpr int k_PulseNegatedDamage =
	DMG_BULLET | DMG_SLASH | DMG_CLUB | DMG_BLAST |
	DMG_SHOCK | DMG_SONIC | DMG_ENERGYBEAM | DMG_CRUSH;

//=========================================================
// What a Discharge deals.
//
// DMG_ENERGYBEAM rather than DMG_SHOCK, and the difference is not cosmetic:
// the alien slave is immune to DMG_SHOCK (dlls/islave.cpp:585) and it is the
// ONLY thing in the game that is, so venting shock at one did nothing at all.
// Slaves are a headline target for the Pulse -- they have both a telegraphed
// melee and a hitscan attack -- so a counter that they alone shrug off is the
// wrong counter.
//
// DMG_ENERGYBEAM has no immunity anywhere, is what the Egon fires
// (dlls/egon.cpp:271), and describes vented energy better than "electric
// shock" did. The Apache's rotor still ignores it (dlls/apache.cpp:969),
// which is correct.
//=========================================================
static constexpr int k_PulseDischargeDamage = DMG_ENERGYBEAM;

// What counts as melee for pulse_discharge_melee.  Claws, bites and blunt
// blows -- deliberately not DMG_CRUSH, which is falling debris rather than
// something swung at the player.
static constexpr int k_PulseMeleeDamage = DMG_SLASH | DMG_CLUB;

// Placeholder assets -- all stock Half-Life, pending the mod's own.
//
// The Pulse and its deflects borrow the crowbar's vocabulary deliberately: the
// Shield going up is a swing, and something being turned away is an impact.
// They started out as electro4 and electro6, which landed a fraction of a
// second apart in the same timbre and made the deflect impossible to hear.
static const char* const k_PulseSoundFire    = "weapons/cbar_miss1.wav";
static const char* const k_PulseSoundReady   = "items/suitchargeok1.wav";
static const char* const k_PulseSoundDenied  = "items/suitchargeno1.wav";
static const char* const k_PulseSpriteShield = "sprites/shockwave.spr";

// Hit sounds, not hitbod -- a deflect is metal turning something away, not
// something being struck. Randomised across the two, and pitched on top of
// that, so a run of deflects does not sound like a stuck record.
static const char* const k_PulseSoundsDeflect[] =
{
	"weapons/cbar_hit1.wav",
	"weapons/cbar_hit2.wav",
};

static short g_sModelIndexPulseShield = 0;

// How far a Discharge reaches.  Generous -- it is an energy bolt, not a swing.
static constexpr float k_DischargeRange = 4096.0f;

//=========================================================
// Local save/restore descriptor table
//=========================================================
static TYPEDESCRIPTION g_PulseSaveData[] =
{
	DEFINE_FIELD(CPlayerPulse, m_bShieldUp,       FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerPulse, m_flShieldEndTime, FIELD_TIME),
	DEFINE_FIELD(CPlayerPulse, m_bAbsorbed,       FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerPulse, m_bRecharging,     FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerPulse, m_flReadyTime,     FIELD_TIME),
	DEFINE_FIELD(CPlayerPulse, m_iRebounds,       FIELD_INTEGER),
	DEFINE_FIELD(CPlayerPulse, m_flFollowUpUntil, FIELD_TIME),
};

bool PulseSave(CPlayerPulse& pulse, CSave& save)
{
	return save.WriteFields("PULSE", &pulse, g_PulseSaveData, ARRAYSIZE(g_PulseSaveData));
}

bool PulseRestore(CPlayerPulse& pulse, CRestore& restore)
{
	return restore.ReadFields("PULSE", &pulse, g_PulseSaveData, ARRAYSIZE(g_PulseSaveData));
}

//=========================================================
// PulsePrecache
//=========================================================
void PulsePrecache()
{
	g_sModelIndexPulseShield = PRECACHE_MODEL(k_PulseSpriteShield);

	PRECACHE_SOUND(k_PulseSoundFire);
	PRECACHE_SOUND(k_PulseSoundReady);
	PRECACHE_SOUND(k_PulseSoundDenied);

	for (const char* sound : k_PulseSoundsDeflect)
		PRECACHE_SOUND(sound);
}

//=========================================================
// Tuning readers.  Every one of these is a starting guess to be
// judged in play, which is why they are cvars and not constants.
//=========================================================
static float PulseWindowFor(const CBasePlayer* pPlayer)
{
	float window = pulse_window.value;

	if (pPlayer->m_skills.HasSkill(ESkillId::PulseWindow))
		window += pulse_window_bonus.value;

	return std::max(0.05f, window);
}

static float PulseRechargeFor(const CBasePlayer* pPlayer, bool bAbsorbed)
{
	float recharge = bAbsorbed ? pulse_recharge_hit.value : pulse_recharge_miss.value;

	if (pPlayer->m_skills.HasSkill(ESkillId::PulseRecharge))
		recharge *= pulse_recharge_scale.value;

	return std::max(0.0f, recharge);
}

//=========================================================
// How many Rebounds the player can hold at once.
//
// A count rather than a flag because the ceiling is meant to rise: a second
// Skill raising this to 2 turns the chain into three Shields back to back
// without touching anything else here.
//=========================================================
static int PulseMaxRebounds(const CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return 0;

	if (pPlayer->m_skills.HasSkill(ESkillId::PulseRebound))
		return 1;

	return 0;
}

//=========================================================
// DrawShieldRing
//
// One expanding ring centred on the player.
//
// TE_BEAMTORUS is screen-aligned and centred on a point, so it reads as a
// bubble around the player; TE_BEAMCYLINDER expands along the ground and
// reads as a ring at their feet (it is the houndeye's blast, borrowed from
// dlls/houndeye.cpp:576-616).  Nothing else in this SDK uses TE_BEAMTORUS,
// so which one looks right is a judgement call rather than a known answer --
// hence pulse_ring_style, which switches between them live.
//
// Both take the same fields; only the byte at the front differs.
//=========================================================
static void DrawShieldRing(CBasePlayer* pPlayer, float flScale, int width, int r, int g, int b)
{
	const Vector& vecOrigin = pPlayer->pev->origin;

	const bool bTorus = pulse_ring_style.value != 0;

	// The torus is centred on the player; the cylinder starts at their feet
	// and grows upward, so it wants a lower anchor.
	const float flAnchorZ = bTorus ? vecOrigin.z : vecOrigin.z - 16;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(bTorus ? TE_BEAMTORUS : TE_BEAMCYLINDER);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(flAnchorZ);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(flAnchorZ + flScale); // axis and radius
	WRITE_SHORT(g_sModelIndexPulseShield);
	WRITE_BYTE(0);     // startframe
	WRITE_BYTE(0);     // framerate
	WRITE_BYTE(2);     // life, in tenths
	WRITE_BYTE(width); // width
	WRITE_BYTE(0);     // noise
	WRITE_BYTE(r);
	WRITE_BYTE(g);
	WRITE_BYTE(b);
	WRITE_BYTE(255);   // brightness
	WRITE_BYTE(0);     // speed
	MESSAGE_END();
}

//=========================================================
// DrawShieldEffect
//
// Everything the player sees when a Shield goes up: nested rings, plus a
// dynamic light so the Pulse actually lights the room around them.
//=========================================================
static void DrawShieldEffect(CBasePlayer* pPlayer, int r, int g, int b)
{
	const Vector& vecOrigin = pPlayer->pev->origin;
	const float flScale = pulse_ring_scale.value;

	// Nested, at different widths, so the edge reads as a surface rather than
	// a single line.
	DrawShieldRing(pPlayer, flScale, 12, r, g, b);
	DrawShieldRing(pPlayer, flScale * 0.5f, 6, r, g, b);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_BYTE(12);  // radius, in tens
	WRITE_BYTE(r);
	WRITE_BYTE(g);
	WRITE_BYTE(b);
	WRITE_BYTE(3);   // life, in tenths
	WRITE_BYTE(20);  // decay, in tens
	MESSAGE_END();
}

//=========================================================
// FireDischarge
//
// Vents what the Shield absorbed toward the player's crosshair.
// NOT a reflection -- Half-Life's bullets are hitscan and there is
// nothing in flight to turn around.  See
// docs/adr/0006-the-discharge-vents-at-the-crosshair.md.
//=========================================================
static void FireDischarge(CBasePlayer* pPlayer, float flAbsorbed)
{
	float flDamage = flAbsorbed * pulse_discharge_scale.value;
	flDamage = std::max(pulse_discharge_min.value,
			   std::min(flDamage, pulse_discharge_max.value));

	if (flDamage <= 0)
		return;

	UTIL_MakeVectors(pPlayer->pev->v_angle);

	const Vector vecSrc = pPlayer->GetGunPosition();
	const Vector vecEnd = vecSrc + gpGlobals->v_forward * k_DischargeRange;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pPlayer->pev), &tr);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(tr.vecEndPos.x);
	WRITE_COORD(tr.vecEndPos.y);
	WRITE_COORD(tr.vecEndPos.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);   // startframe
	WRITE_BYTE(0);   // framerate
	WRITE_BYTE(2);   // life, in tenths
	WRITE_BYTE(12);  // width
	WRITE_BYTE(24);  // noise
	WRITE_BYTE(64);  // r
	WRITE_BYTE(200); // g
	WRITE_BYTE(255); // b
	WRITE_BYTE(255); // brightness
	WRITE_BYTE(0);   // speed
	MESSAGE_END();

	CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

	if (pHit && pHit->pev->takedamage != DAMAGE_NO)
	{
		// Damage goes through TraceAttack, not straight to TakeDamage, because
		// Half-Life keeps a great deal of per-entity damage logic there and
		// nowhere else -- hitgroup multipliers, the HGrunt's helmet, the
		// AGrunt's armour plate, the Apache's rotor.  Calling TakeDamage
		// directly walked past all of it.
		//
		// The catch is that gMultiDamage is a single global accumulator and we
		// are running inside a live one: this is reached from
		// CBasePlayer::TakeDamage, which ApplyMultiDamage (dlls/weapons.cpp:97)
		// calls without clearing afterwards.  So the sequence in progress is
		// copied out, ours runs on a clean slate, and the original is put back.
		const MULTIDAMAGE savedMultiDamage = gMultiDamage;

		ClearMultiDamage();
		pHit->TraceAttack(pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, k_PulseDischargeDamage);
		ApplyMultiDamage(pPlayer->pev, pPlayer->pev);

		gMultiDamage = savedMultiDamage;
	}
}

//=========================================================
// CPlayerPulse::Clear
//=========================================================
void CPlayerPulse::Clear(CBasePlayer* pPlayer)
{
	m_bShieldUp = false;
	m_flShieldEndTime = 0;
	m_bAbsorbed = false;
	m_bRecharging = false;
	m_flReadyTime = 0;
	m_bDischarging = false;
	m_iSentState = -1;
	m_iRebounds = PulseMaxRebounds(pPlayer);
	m_bDampenPunch = false;
	m_vecPunchBefore = g_vecZero;
	m_flFollowUpUntil = 0;
}

//=========================================================
// PulseTakeCrowbarFollowUp
//=========================================================
bool PulseTakeCrowbarFollowUp(CBasePlayer* pPlayer, float& flDamage)
{
	if (!pPlayer)
		return false;

	CPlayerPulse& pulse = pPlayer->m_pulse;

	if (pulse.m_flFollowUpUntil <= 0 || gpGlobals->time > pulse.m_flFollowUpUntil)
		return false;

	// Spent on a HIT, not on a swing -- this is only reached once the crowbar
	// has connected with something, so whiffing after a deflect costs nothing.
	pulse.m_flFollowUpUntil = 0;

	flDamage *= std::max(1.0f, pulse_followup_damage.value);

	return true;
}

//=========================================================
// PulseCrowbarFollowUpKnockback
//
// Headcrabs only, and deliberately so.
//
// Half-Life does not knock monsters back from damage at all: the one place it
// happens (dlls/combat.cpp:891) is gated on MOVETYPE_WALK, which is the player
// -- monsters are MOVETYPE_STEP. The corpse equivalent was written and then
// disabled (dlls/combat.cpp:986, "turn this back on when the bounding box
// issues are resolved"), so this is a road Valve started down and abandoned.
//
// Restricting it to headcrabs keeps it on the one enemy small and light enough
// for the result to look right, and away from anything mid-script, boss-sized,
// or standing somewhere it must not be shoved out of.
//=========================================================
void PulseCrowbarFollowUpKnockback(CBaseEntity* pTarget, const Vector& vecDir)
{
	if (!pTarget)
		return;

	// CBabyCrab derives from CHeadCrab (dlls/headcrab.cpp:479) but neither is
	// declared in a header, so this goes by classname rather than by cast.
	if (!FClassnameIs(pTarget->pev, "monster_headcrab") &&
		!FClassnameIs(pTarget->pev, "monster_babycrab"))
	{
		return;
	}

	const float flForce = pulse_followup_knockback.value;
	if (flForce <= 0)
		return;

	// Velocity alone does nothing to something resting on the floor: ground
	// friction eats it. Lifting it clear is what makes the throw read.
	pTarget->pev->flags &= ~FL_ONGROUND;
	pTarget->pev->velocity = pTarget->pev->velocity + vecDir * flForce + Vector(0, 0, flForce * 0.4f);
}

//=========================================================
// CPlayerPulse::DampenDeflectPunch
//
// A deflect is supposed to feel like the blow glanced off, not like it landed
// and not like nothing happened at all -- so the attacker's view kick is
// scaled rather than suppressed.
//
// It cannot be suppressed at the source without editing every melee monster:
// CheckTraceHullAttack (dlls/combat.cpp:1153) hands back the entity it hit
// whether or not the damage was taken, and monsters set punchangle off that
// return value. By the time TryNegate runs, the kick has not been applied yet.
//
// Only the part the attacker ADDED is scaled, so a kick the player was already
// carrying from something else is left alone.
//=========================================================
void CPlayerPulse::DampenDeflectPunch(CBasePlayer* pPlayer)
{
	if (!m_bDampenPunch || !pPlayer)
		return;

	m_bDampenPunch = false;

	const float flScale = std::max(0.0f, std::min(pulse_deflect_punch.value, 1.0f));

	const Vector vecAdded = pPlayer->pev->punchangle - m_vecPunchBefore;
	pPlayer->pev->punchangle = m_vecPunchBefore + vecAdded * flScale;
}

//=========================================================
// CPlayerPulse::SyncClient
//
// Only state CHANGES go over the wire.  The duration travels with
// the change and the client runs the bar off its own clock, so a
// whole Pulse costs three 2-byte messages rather than one a frame.
//=========================================================
void CPlayerPulse::SyncClient(CBasePlayer* pPlayer)
{
	if (gmsgPulse == 0)
		return;

	int state = PULSE_READY;
	float flRemaining = 0;

	if (m_bShieldUp)
	{
		state = PULSE_SHIELD;
		flRemaining = m_flShieldEndTime - gpGlobals->time;
	}
	else if (m_bRecharging)
	{
		state = PULSE_RECHARGING;
		flRemaining = m_flReadyTime - gpGlobals->time;
	}

	if (state == m_iSentState)
		return;

	m_iSentState = state;

	// Tenths of a second, clamped to the byte the message is sized for.
	int tenths = static_cast<int>(flRemaining * 10.0f + 0.5f);
	tenths = std::max(0, std::min(tenths, 255));

	MESSAGE_BEGIN(MSG_ONE, gmsgPulse, NULL, pPlayer->pev);
	WRITE_BYTE(state);
	WRITE_BYTE(tenths);
	MESSAGE_END();
}

//=========================================================
// CPlayerPulse::TryPulse
//=========================================================
bool CPlayerPulse::TryPulse(CBasePlayer* pPlayer)
{
	if (!pPlayer || !pPlayer->IsAlive())
		return false;

	// The Pulse is suit hardware.  No suit, no Pulse -- and no sound
	// either, because a player without the suit should have no idea the
	// ability exists.
	if (!pPlayer->HasSuit())
		return false;

	if (!Ready())
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundDenied, 0.7, ATTN_NORM);
		return false;
	}

	m_bShieldUp = true;
	m_flShieldEndTime = gpGlobals->time + PulseWindowFor(pPlayer);
	m_bAbsorbed = false;

	DrawShieldEffect(pPlayer, 64, 200, 255);

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundFire, 1.0, ATTN_NORM);

	// Synced here as well as in Think because ImpulseCommands runs in PostThink,
	// after this frame's Think -- otherwise the bar would lag the Shield by a frame.
	SyncClient(pPlayer);

	return true;
}

//=========================================================
// CPlayerPulse::Think
//=========================================================
void CPlayerPulse::Think(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return;

	if (m_bShieldUp)
	{
		if (gpGlobals->time >= m_flShieldEndTime)
		{
			// The window always runs its full duration -- absorbing does not
			// close it early, so several attackers landing inside one window
			// are all negated.  What happens next is only decided here, which
			// is also what makes a Rebound impossible to overlap: the player
			// cannot become Ready while a Shield is still standing, so two
			// Shields can never coexist.
			m_bShieldUp = false;

			if (m_bAbsorbed && m_iRebounds > 0)
			{
				// Rebound: the window deflected something, so skip its
				// Recharge outright. Spending the last one is what bounds the
				// chain -- two Shields back to back, then a real wait.
				--m_iRebounds;
				m_bRecharging = false;

				// The ready chime, pitched up, so a Rebound is audibly the
				// fast path rather than a Recharge that finished suspiciously
				// early.
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundReady,
					0.7, ATTN_NORM, 0, 130);
			}
			else
			{
				m_bRecharging = true;
				m_flReadyTime = gpGlobals->time + PulseRechargeFor(pPlayer, m_bAbsorbed);
			}

			m_bAbsorbed = false;
		}
	}
	else if (m_bRecharging && gpGlobals->time >= m_flReadyTime)
	{
		m_bRecharging = false;

		// Sitting through a normal Recharge is what buys the Rebounds back --
		// all of them, so the fantasy stays "spend the burst, wait, burst
		// again" rather than a slow drip.
		m_iRebounds = PulseMaxRebounds(pPlayer);

		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundReady, 0.6, ATTN_NORM);
	}

	SyncClient(pPlayer);
}

//=========================================================
// CPlayerPulse::WouldNegate
//=========================================================
bool CPlayerPulse::WouldNegate(int bitsDamageType) const
{
	return m_bShieldUp && (bitsDamageType & k_PulseNegatedDamage) != 0;
}

//=========================================================
// CPlayerPulse::TryNegate
//=========================================================
bool CPlayerPulse::TryNegate(CBasePlayer* pPlayer, float flDamage, int bitsDamageType)
{
	if (!pPlayer || !WouldNegate(bitsDamageType))
		return false;

	m_bAbsorbed = true;

	// Remember the view kick as it stands *now*. Melee attackers set punchangle
	// after this returns, so what they add can only be scaled back later --
	// see DampenDeflectPunch.
	m_bDampenPunch = true;
	m_vecPunchBefore = pPlayer->pev->punchangle;

	// Prime the crowbar. Only while the Skill is held, so nothing carries
	// state it can never spend.
	if (pPlayer->m_skills.HasSkill(ESkillId::CrowbarFollowUp))
		m_flFollowUpUntil = gpGlobals->time + std::max(0.1f, pulse_followup_time.value);

	// CHAN_AUTO so several hits landing inside one window each get their own
	// impact rather than cutting each other off.
	EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_AUTO,
		k_PulseSoundsDeflect[RANDOM_LONG(0, ARRAYSIZE(k_PulseSoundsDeflect) - 1)],
		1.0, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 8));

	// A Discharge on a MELEE deflect was never designed -- it falls out of
	// "one Discharge per negated hit", which does not care what dealt the
	// damage. It plays well, so it stays on by default, but it is the part of
	// this behaviour most likely to be judged wrong later: pulse_discharge_melee
	// 0 turns it off without touching anything else.
	const bool bMelee = (bitsDamageType & k_PulseMeleeDamage) != 0;
	const bool bWantDischarge = !bMelee || pulse_discharge_melee.value != 0;

	// One Discharge per negated hit, fired in the same frame the hit lands.
	// The reentrancy guard stops a Discharge that causes damage back to the
	// player -- venting into an explosive barrel at point blank -- from
	// recursing.
	if (bWantDischarge && !m_bDischarging && pPlayer->m_skills.HasSkill(ESkillId::PulseDischarge))
	{
		m_bDischarging = true;
		FireDischarge(pPlayer, flDamage);
		m_bDischarging = false;
	}

	return true;
}
