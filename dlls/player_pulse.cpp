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
#include "suit_defs.h"
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

// Placeholder assets -- stock Half-Life, pending the mod's own, except the
// Recharge cue.
//
// The Pulse and its deflects borrow the crowbar's vocabulary deliberately: the
// Shield going up is a swing, and something being turned away is an impact.
// They started out as electro4 and electro6, which landed a fraction of a
// second apart in the same timbre and made the deflect impossible to hear.
//
// The Recharge cue is imported from Team Fortress 2 (its sound/player/
// recharged.wav), downmixed to mono; it ships in the repo's sound/.
static const char* const k_PulseSoundFire    = "weapons/cbar_miss1.wav";
static const char* const k_PulseSoundReady   = "player/recharged.wav";
static const char* const k_PulseSoundDenied  = "items/suitchargeno1.wav";

// Hit sounds, not hitbod -- a deflect is metal turning something away, not
// something being struck. Randomised across the two, and pitched on top of
// that, so a run of deflects does not sound like a stuck record.
static const char* const k_PulseSoundsDeflect[] =
{
	"weapons/cbar_hit1.wav",
	"weapons/cbar_hit2.wav",
};

// The Defense Matrix's sounds, placeholders too (docs/ART_DEBT.md, "The Pulse
// -- sounds").  The slave's zap for the field coming up and going down: an
// electrical timbre, which the Pulse set gave up because a deflect landed a
// tenth of a second after it.  The Matrix has no such neighbour -- it comes up
// a second after the press's Shield and drops six seconds later -- so the
// timbre the tap could not use says "powered field" here without colliding.
static const char* const k_MatrixSoundRaise = "weapons/electro4.wav";
static const char* const k_MatrixSoundDrop  = "weapons/electro5.wav";
// Matrix on Kill: the charger's "ok" -- armour coming back, which is exactly
// what it says.
static const char* const k_MatrixSoundKill  = "items/suitchargeok1.wav";

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
	DEFINE_FIELD(CPlayerPulse, m_bTailUp,         FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerPulse, m_flTailEndTime,   FIELD_TIME),
	DEFINE_FIELD(CPlayerPulse, m_iRebounds,       FIELD_INTEGER),
	DEFINE_FIELD(CPlayerPulse, m_flFollowUpUntil, FIELD_TIME),
	DEFINE_FIELD(CPlayerPulse, m_flMatrixReadyTime, FIELD_TIME),
	DEFINE_FIELD(CPlayerPulse, m_flMatrixGrant,   FIELD_FLOAT),
	DEFINE_FIELD(CPlayerPulse, m_bMatrixUp,       FIELD_BOOLEAN),
	DEFINE_FIELD(CPlayerPulse, m_flMatrixEndTime, FIELD_TIME),
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
	// No sprite: the Shield's rings went on 2026-09-20 and nothing the Pulse
	// draws in the world needs a model index any more.  The dlight and the
	// Discharge's beam are the only world effects left, and the Discharge uses
	// g_sModelIndexLaser, which the SDK precaches for everyone.
	PRECACHE_SOUND(k_PulseSoundFire);
	PRECACHE_SOUND(k_PulseSoundReady);
	PRECACHE_SOUND(k_PulseSoundDenied);
	PRECACHE_SOUND(k_MatrixSoundRaise);
	PRECACHE_SOUND(k_MatrixSoundDrop);
	PRECACHE_SOUND(k_MatrixSoundKill);

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
// DrawShieldEffect
//
// What the WORLD sees when a Shield goes up: a dynamic light, and nothing else.
//
// There used to be rings here -- two nested TE_BEAMTORUS, and before that a
// TE_BEAMCYLINDER alternative that was the houndeye's floor blast
// (dlls/houndeye.cpp:576-616).  All of it deleted 2026-09-20, the cylinder
// first and the torus a few minutes later once the first-person Shield could be
// seen beside them.  Andrei: "disable or remove the torus/rings all together,
// they are ugly compared to what we have on our hand."
//
// The dlight stays, and is the whole point of this function still existing: it
// throws suit-coloured light onto real walls, which is the one thing a
// first-person overlay fundamentally cannot do.  Losing it would leave the
// Pulse with no physical presence in the room at all.
//
// docs/ROADMAP.md, "The Shield in first person", has the reasoning.
//=========================================================
static void DrawShieldEffect(CBasePlayer* pPlayer, int r, int g, int b)
{
	const Vector& vecOrigin = pPlayer->pev->origin;

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
	m_bTailUp = false;
	m_flTailEndTime = 0;
	m_bDischarging = false;
	m_iSentState = -1;
	m_iRebounds = PulseMaxRebounds(pPlayer);
	m_bDampenPunch = false;
	m_vecPunchBefore = g_vecZero;
	m_flFollowUpUntil = 0;

	m_flMatrixReadyTime = 0;
	m_flMatrixGrant = 0;
	m_bMatrixUp = false;
	m_flMatrixEndTime = 0;
	m_bHeld = false;
	m_flPressTime = 0;
	m_bHoldSpent = false;
	m_flMatrixLastThink = 0;
	m_bMatrixReadyCue = false;
	m_iSentMatrix = -1;
	m_flAppliedMaxspeed = -1;
}

//=========================================================
// PulseTakeCrowbarFollowUp
//=========================================================
bool PulseFollowUpPrimed(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return false;

	const CPlayerPulse& pulse = pPlayer->m_pulse;
	return pulse.m_flFollowUpUntil > 0 && gpGlobals->time <= pulse.m_flFollowUpUntil;
}

float PulseFollowUpScale()
{
	return std::max(1.0f, pulse_followup_damage.value);
}

void PulseSpendFollowUp(CBasePlayer* pPlayer)
{
	if (pPlayer)
		pPlayer->m_pulse.m_flFollowUpUntil = 0;
}

bool PulseTakeCrowbarFollowUp(CBasePlayer* pPlayer, float& flDamage)
{
	if (!PulseFollowUpPrimed(pPlayer))
		return false;

	// Spent on a HIT, not on a swing -- this is only reached once the weapon
	// has connected with something, so whiffing after a deflect costs nothing.
	PulseSpendFollowUp(pPlayer);
	flDamage *= PulseFollowUpScale();
	return true;
}

//=========================================================
// CPlayerPulse::SyncFollowUpIcon
//
// A primed Follow-Up shows at the screen edge the way a ready Cleave does,
// so the player knows the next swing is the big one without counting.
// Placeholder sprite: the Follow-Up's own tree icon.  docs/ART_DEBT.md.
//=========================================================
void CPlayerPulse::SyncFollowUpIcon(CBasePlayer* pPlayer)
{
	const bool bWant = pPlayer->m_skills.HasSkill(ESkillId::FollowUp) && PulseFollowUpPrimed(pPlayer);
	if (bWant == m_bFollowUpIconSent)
		return;

	m_bFollowUpIconSent = bWant;

	if (gmsgStatusIcon == 0)
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
	WRITE_BYTE(bWant ? 1 : 0);
	WRITE_STRING("d_gauss");
	if (bWant)
	{
		WRITE_BYTE(255);
		WRITE_BYTE(200);
		WRITE_BYTE(60);
	}
	MESSAGE_END();
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
// It is scaled here rather than at the source by choice: CheckTraceHullAttack
// (dlls/combat.cpp) hands back the entity it hit whether or not the damage was
// taken, and monsters set punchangle off that return value. Since 2026-09-20
// it also reports whether the blow landed (pbLanded), and the melee monsters
// gate their hit sound on that -- but the kick deliberately stays, scaled, so
// a deflect reads as a blow glancing off (docs/TECH_DEBT.md, the
// deflected-melee entry). By the time TryNegate runs, the kick has not been
// applied yet.
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

	// No Module, no readout: the bar would otherwise stand READY for a verb
	// the player does not have.  Sent as its own state rather than left to
	// the client's HasSuit test, since the suit no longer implies the Pulse.
	if (!pPlayer->m_skills.IsGateOpen(EGate::PulseModule))
	{
		state = PULSE_NONE;
	}
	else if (m_bShieldUp)
	{
		state = PULSE_SHIELD;
		flRemaining = m_flShieldEndTime - gpGlobals->time;
	}
	else if (m_bTailUp)
	{
		// The bar shows the tail and the Recharge after it as one countdown to
		// Ready -- which is what the player needs from the bar -- and the ring
		// and the brace's sound say the tail is there.  The client needs no
		// new state, and the count does not resend when the tail hands over to
		// the Recharge, because the state it sent does not change.
		state = PULSE_RECHARGING;
		flRemaining = (m_flTailEndTime - gpGlobals->time) + PulseRechargeFor(pPlayer, false);
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

	// The Pulse is a found Module (docs/adr/0013): no suit or no Module, no
	// Pulse -- and no sound either, because a player without it should have
	// no idea the ability exists.
	if (!pPlayer->HasSuit() || !pPlayer->m_skills.IsGateOpen(EGate::PulseModule))
		return false;

	if (!Ready())
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundDenied, 0.7, ATTN_NORM);
		return false;
	}

	m_bShieldUp = true;
	m_flShieldEndTime = gpGlobals->time + PulseWindowFor(pPlayer);
	m_bAbsorbed = false;

	// Where the tail would end: the moment a hold of this press would raise
	// the Defense Matrix, so window, tail and Matrix are one motion with no
	// unprotected gap (docs/ROADMAP.md, "The Pulse's tail", rule 5).  Read
	// from the Matrix's own number rather than a cvar of the tail's, which
	// could only ever disagree with it.  Pulse Window widens the window
	// inside that second and does not extend it (rule 3): a window as long
	// as the second has no tail at all.
	m_flTailEndTime = gpGlobals->time + std::max(PulseWindowFor(pPlayer), skill_matrix_hold.value);

	// The Shield is the suit's own field, so it is the suit's own colour.
	const SuitVariantDef& suit = GetSuitVariant(pPlayer->pev->skin);
	DrawShieldEffect(pPlayer, suit.r, suit.g, suit.b);

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
			else if (!m_bAbsorbed && gpGlobals->time < m_flTailEndTime)
			{
				// The tail: nothing was deflected, so the Pulse stands on,
				// braced, and its Recharge -- the long one, a miss's -- waits
				// for it.  A window that DID deflect never gets here: it ends
				// exactly as it always has, so the parry, the short Recharge
				// and the Rebound are untouched by any of this.
				//
				// The tail has NO visual of its own as of 2026-09-20.  It used
				// to get a dimmer, smaller ring so "I braced" could be told from
				// "I parried" without a number; that ring went with all the
				// others when the first-person Shield landed.  Deliberate, and
				// Andrei's call: the tail may not survive to the final game, so
				// it was left out of the Shield's v1 rather than given a
				// treatment that might be thrown away.  Its duller braced sound
				// (TailScale) is the only cue it has left.  See docs/ROADMAP.md,
				// "The Pulse's tail".
				m_bTailUp = true;
			}
			else
			{
				m_bRecharging = true;
				m_flReadyTime = gpGlobals->time + PulseRechargeFor(pPlayer, m_bAbsorbed);
			}

			m_bAbsorbed = false;
		}
	}
	else if (m_bTailUp)
	{
		if (gpGlobals->time >= m_flTailEndTime)
		{
			m_bTailUp = false;
			m_bRecharging = true;
			m_flReadyTime = gpGlobals->time + PulseRechargeFor(pPlayer, false);
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
	SyncFollowUpIcon(pPlayer);

	MatrixThink(pPlayer);
}

//=========================================================
// The Defense Matrix
//
// The Juggernaut Route's stance (docs/SKILL_TREE.md, "Juggernaut"; the
// reasoning in docs/ROADMAP.md).  Hold the Pulse key for a second and it
// comes up: armour takes a far larger share of every hit (the split itself
// is in CBasePlayer::TakeDamage, which reads MatrixUp()), the player is
// slowed, and the Major grants decaying armour above the cap.  It drops on
// release, when its time is up, or at zero armour, and then waits out a
// cooldown.
//
// The tap's Shield still fires at the front of every press, so a hold
// begins with the deflect window and the Matrix follows it; deflect and
// Matrix are separate verbs on one key and neither invalidates the other.
//=========================================================
void CPlayerPulse::OnPress(CBasePlayer* pPlayer)
{
	// The tap, exactly as before: a bare "impulse 150" bind is still a Pulse.
	TryPulse(pPlayer);

	m_bHeld = true;
	m_flPressTime = gpGlobals->time;
	m_bHoldSpent = false;
}

void CPlayerPulse::OnRelease(CBasePlayer* pPlayer)
{
	// The release only ends the hold.  A standing Matrix keeps its own time
	// (Andrei, 2026-09-16, after the first play): holding the key through the
	// whole six seconds was the first shape, and it tied a hand up for the
	// stance's whole life.
	m_bHeld = false;
}

void CPlayerPulse::MatrixThink(CBasePlayer* pPlayer)
{
	if (m_bMatrixUp)
	{
		// The drop rules, in the order they are checked.
		if (!pPlayer->IsAlive())
			DropMatrix(pPlayer, "dead");
		else if (gpGlobals->time >= m_flMatrixEndTime)
			DropMatrix(pPlayer, "time");
		else if (pPlayer->pev->armorvalue <= 0)
			DropMatrix(pPlayer, "no armour");
	}
	else if (m_bHeld && !m_bHoldSpent
		&& gpGlobals->time >= m_flPressTime + std::max(0.1f, skill_matrix_hold.value))
	{
		// One attempt per press: a hold that finds the Matrix on cooldown is
		// refused once, not every frame until the key comes up.
		m_bHoldSpent = true;

		// Gated by its Skill, and needing the Pulse like everything else on
		// the press -- and without the Skill a held key is just a held key,
		// so no sound either.
		if (pPlayer->m_skills.HasSkill(ESkillId::DefenseMatrix) && pPlayer->HasSuit() && pPlayer->IsAlive()
			&& pPlayer->m_skills.IsGateOpen(EGate::PulseModule))
		{
			// Armour is the pool, so with none there is nothing to raise --
			// unless the Major is about to grant some.
			const bool bHasPool = pPlayer->pev->armorvalue > 0 || pPlayer->m_skills.HasSkill(ESkillId::JuggernautMajor);

			if (gpGlobals->time >= m_flMatrixReadyTime && bHasPool)
				RaiseMatrix(pPlayer);
			else
				EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundDenied, 0.7, ATTN_NORM);
		}
	}

	// The ready cue: the Pulse's chime pitched down, once, when the cooldown
	// runs out -- the player's readiness cue when they are not looking at the
	// bar, as the Pulse's is.  Only for a player who holds the Skill.
	if (m_bMatrixReadyCue && !m_bMatrixUp && gpGlobals->time >= m_flMatrixReadyTime)
	{
		m_bMatrixReadyCue = false;
		if (pPlayer->m_skills.HasSkill(ESkillId::DefenseMatrix))
			EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_ITEM, k_PulseSoundReady, 0.6, ATTN_NORM, 0, 80);
	}

	DecayMatrixGrant(pPlayer);
	ApplyMatrixSpeed(pPlayer);
	SyncMatrix(pPlayer);
}

void CPlayerPulse::RaiseMatrix(CBasePlayer* pPlayer)
{
	m_bMatrixUp = true;
	m_flMatrixEndTime = gpGlobals->time + std::max(0.1f, skill_matrix_duration.value);

	// Decaying Armor, the Major: a grant above the cap, allowed there on
	// purpose -- a grant that respected PlayerMaxArmor would do nothing for a
	// player at full armour.  It fades on its own (DecayMatrixGrant) and is
	// spent like any armour before then, so it is fuel: Overdraw's drain and
	// the Matrix's own share both eat it first.
	if (pPlayer->m_skills.HasSkill(ESkillId::JuggernautMajor))
	{
		const float flGrant = std::max(0.0f, skill_matrix_grant.value);
		if (flGrant > 0)
		{
			pPlayer->pev->armorvalue += flGrant;
			m_flMatrixGrant += flGrant;
		}
	}

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_MatrixSoundRaise, 0.9, ATTN_NORM);

	// A light rather than the Shield's rings, so the two verbs on the key do
	// not look alike.  In the suit's colour, held for a second.
	const SuitVariantDef& suit = GetSuitVariant(pPlayer->pev->skin);
	const Vector& vecOrigin = pPlayer->pev->origin;
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_BYTE(16); // radius, in tens
	WRITE_BYTE(suit.r);
	WRITE_BYTE(suit.g);
	WRITE_BYTE(suit.b);
	WRITE_BYTE(10); // life, in tenths
	WRITE_BYTE(8);  // decay, in tens
	MESSAGE_END();

	if (debug_damage.value != 0)
	{
		ALERT(at_console, "matrix: UP for %.1fs, armour %.1f (grant %.1f standing)\n",
			m_flMatrixEndTime - gpGlobals->time, pPlayer->pev->armorvalue, m_flMatrixGrant);
	}
}

void CPlayerPulse::DropMatrix(CBasePlayer* pPlayer, const char* pszReason)
{
	m_bMatrixUp = false;
	m_flMatrixReadyTime = gpGlobals->time + std::max(0.0f, skill_matrix_cooldown.value);
	m_bMatrixReadyCue = true;

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_MatrixSoundDrop, 0.7, ATTN_NORM);

	if (debug_damage.value != 0)
	{
		ALERT(at_console, "matrix: DOWN (%s), armour %.1f, ready in %.1fs\n",
			pszReason, pPlayer->pev->armorvalue, m_flMatrixReadyTime - gpGlobals->time);
	}
}

void CPlayerPulse::DecayMatrixGrant(CBasePlayer* pPlayer)
{
	// Frame time from our own clock rather than gpGlobals->frametime, and
	// clamped, so a long hitch or a load does not drain a second's worth at
	// once.
	const float flNow = gpGlobals->time;
	const float flDelta = (m_flMatrixLastThink > 0) ? std::max(0.0f, std::min(flNow - m_flMatrixLastThink, 0.25f)) : 0.0f;
	m_flMatrixLastThink = flNow;

	if (m_flMatrixGrant <= 0)
	{
		m_flMatrixGrant = 0;
		return;
	}

	// Whatever a hit or Overdraw already took off the top came out of the
	// grant first: the grant can never exceed the armour actually standing.
	m_flMatrixGrant = std::min(m_flMatrixGrant, std::max(0.0f, (float)pPlayer->pev->armorvalue));

	// The grant fades over the Matrix's own duration -- grant / duration per
	// second -- so it is gone in the same moment the Matrix drops (Andrei,
	// 2026-09-16), and the two are read as one thing.
	const float flRate = std::max(0.0f, skill_matrix_grant.value) / std::max(0.1f, skill_matrix_duration.value);
	const float flDrain = std::min(m_flMatrixGrant, flRate * flDelta);
	if (flDrain <= 0)
		return;

	pPlayer->pev->armorvalue -= flDrain;
	m_flMatrixGrant -= flDrain;

	if (m_flMatrixGrant < 0.01f)
	{
		m_flMatrixGrant = 0;
		if (debug_damage.value != 0)
			ALERT(at_console, "matrix: grant gone, armour %.1f\n", pPlayer->pev->armorvalue);
	}
}

void CPlayerPulse::ApplyMatrixSpeed(CBasePlayer* pPlayer)
{
	// The slow is a cap, not a rule change: the engine feeds pev->maxspeed to
	// the movement code as pmove->clientmaxspeed, which PM_CheckParamters
	// clamps the wish speed against, and 0 means no cap.  Applied on change
	// only, from the state rather than at raise and drop, so a save, a spawn
	// or a cvar edit all come out right without their own bookkeeping.
	float flDesired = 0;
	if (m_bMatrixUp)
	{
		float flMax = CVAR_GET_FLOAT("sv_maxspeed");
		if (flMax <= 0)
			flMax = 320.0f;
		flDesired = flMax * std::max(0.05f, std::min(skill_matrix_speed_scale.value, 1.0f));
	}

	if (flDesired == m_flAppliedMaxspeed)
		return;

	m_flAppliedMaxspeed = flDesired;
	g_engfuncs.pfnSetClientMaxspeed(pPlayer->edict(), flDesired);
}

void CPlayerPulse::SyncMatrix(CBasePlayer* pPlayer)
{
	if (gmsgMatrix == 0)
		return;

	// Only state CHANGES go over the wire, the Pulse's pattern: the duration
	// travels with the change and the client runs its bar off its own clock.
	int state = MATRIX_NONE;
	float flRemaining = 0;

	if (pPlayer->m_skills.HasSkill(ESkillId::DefenseMatrix) && pPlayer->HasSuit())
	{
		if (m_bMatrixUp)
		{
			state = MATRIX_UP;
			flRemaining = m_flMatrixEndTime - gpGlobals->time;
		}
		else if (gpGlobals->time < m_flMatrixReadyTime)
		{
			state = MATRIX_COOLDOWN;
			flRemaining = m_flMatrixReadyTime - gpGlobals->time;
		}
		else
		{
			state = MATRIX_READY;
		}
	}

	if (state == m_iSentMatrix)
		return;

	m_iSentMatrix = state;

	int tenths = static_cast<int>(flRemaining * 10.0f + 0.5f);
	tenths = std::max(0, std::min(tenths, 255));

	MESSAGE_BEGIN(MSG_ONE, gmsgMatrix, NULL, pPlayer->pev);
	WRITE_BYTE(state);
	WRITE_BYTE(tenths);
	MESSAGE_END();
}

void CPlayerPulse::OnKill(CBasePlayer* pPlayer, CBaseMonster* pVictim)
{
	if (!pPlayer || !pVictim || !m_bMatrixUp)
		return;
	if (!pPlayer->m_skills.HasSkill(ESkillId::MatrixOnKill))
		return;

	// A kill, not a murder: the Route's one way to sustain is fighting, and
	// killing a scientist for armour should not be it.
	switch (pVictim->Classify())
	{
	case CLASS_NONE:
	case CLASS_PLAYER:
	case CLASS_PLAYER_ALLY:
		return;
	default:
		break;
	}

	// Restored up to the maximum and no further: the Major's grant is the only
	// thing allowed above it, and a kill must not top that up.
	const float flMax = (float)PlayerMaxArmor(pPlayer);
	const float flBefore = pPlayer->pev->armorvalue;
	if (flBefore >= flMax)
		return;

	pPlayer->pev->armorvalue = std::min(flMax, flBefore + std::max(0.0f, skill_matrix_kill_armor.value));

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, k_MatrixSoundKill, 0.5, ATTN_NORM);

	if (debug_damage.value != 0)
	{
		ALERT(at_console, "matrix: kill (%s), armour %.1f -> %.1f\n",
			STRING(pVictim->pev->classname), flBefore, pPlayer->pev->armorvalue);
	}
}

//=========================================================
// CPlayerPulse::WouldNegate
//=========================================================
bool CPlayerPulse::WouldNegate(int bitsDamageType) const
{
	return m_bShieldUp && (bitsDamageType & k_PulseNegatedDamage) != 0;
}

//=========================================================
// CPlayerPulse::TailScale
//
// A hit in the tail is a miss with a discount: the share, and the long
// Recharge that is already coming.  The Shield's list decides what it
// applies to (ADR-0005, rule 2), so a fall or drowning is not halved.
//=========================================================
float CPlayerPulse::TailScale(CBasePlayer* pPlayer, int bitsDamageType)
{
	if (!pPlayer || !m_bTailUp || gpGlobals->time >= m_flTailEndTime)
		return 1.0f;
	if ((bitsDamageType & k_PulseNegatedDamage) == 0)
		return 1.0f;

	// The deflect's own impact, pitched well down and quieter: the same
	// family, so it reads as the Pulse, but duller, so it never reads as a
	// parry.  Never in the same instant as a deflect -- one hit is one or the
	// other -- which is the collision ART_DEBT.md records the first Pulse
	// sounds failing on.
	EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_AUTO,
		k_PulseSoundsDeflect[RANDOM_LONG(0, ARRAYSIZE(k_PulseSoundsDeflect) - 1)],
		0.7, ATTN_NORM, 0, 62 + RANDOM_LONG(0, 6));

	return std::max(0.0f, std::min(pulse_tail_scale.value, 1.0f));
}

//=========================================================
// CPlayerPulse::ReportDeflect
//
// Tells the client where a turned-away blow came from, so the Shield can flare
// on that side (cl_dll/pulse_shield.cpp).
//
// The origin is chosen exactly as gmsgDamage chooses its vecFrom
// (dlls/player.cpp, UpdateClientData): the inflictor's Center(), falling back to
// the player's own origin when there is no inflictor.  Matching it matters --
// the client runs both through the same CalcDamageDirection, so any difference
// here would make a deflect point somewhere a landed hit would not.
//
// Sent per negated hit rather than per window.  Several hits inside one window
// are all negated (see Think: the window never closes early), and each deserves
// its own flare.
//=========================================================
void CPlayerPulse::ReportDeflect(CBasePlayer* pPlayer, entvars_t* pevInflictor)
{
	if (gmsgPulseHit == 0)
		return;

	Vector vecFrom = pPlayer->pev->origin;

	if (pevInflictor)
	{
		if (CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor))
			vecFrom = pInflictor->Center();
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgPulseHit, NULL, pPlayer->pev);
	WRITE_COORD(vecFrom.x);
	WRITE_COORD(vecFrom.y);
	WRITE_COORD(vecFrom.z);
	MESSAGE_END();
}

//=========================================================
// CPlayerPulse::TryNegate
//=========================================================
bool CPlayerPulse::TryNegate(CBasePlayer* pPlayer, float flDamage, int bitsDamageType, entvars_t* pevInflictor)
{
	if (!pPlayer || !WouldNegate(bitsDamageType))
		return false;

	m_bAbsorbed = true;

	ReportDeflect(pPlayer, pevInflictor);

	// Remember the view kick as it stands *now*. Melee attackers set punchangle
	// after this returns, so what they add can only be scaled back later --
	// see DampenDeflectPunch.
	m_bDampenPunch = true;
	m_vecPunchBefore = pPlayer->pev->punchangle;

	// Prime the crowbar. Only while the Skill is held, so nothing carries
	// state it can never spend.
	if (pPlayer->m_skills.HasSkill(ESkillId::FollowUp))
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
