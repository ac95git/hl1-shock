// The summon weapon, v1 -- the alien Module's first Core weapon.
//
// The Module is a platform and this is the first thing on it
// (docs/ROADMAP.md, "The alien Module -- settled"): gaining the Module grants
// access to alien weapons that run on Cores, and Cores are a real ammo type,
// so the HUD readout, the carry ceiling and the pickup all come from the
// engine's ammo path rather than from anything of ours.
//
// Left click summons one ghost for one Core, on a cooldown, up to a maximum
// out at once.  The cap is Pack's, the lifetime Tether's, the cooldown
// Recall's.  Right click is the ultimate and is NOT built -- it is the next
// slice -- so it spends nothing and does nothing but idle.
//
// What is predicted and what is not.  A summon is the server's alone: the
// spot search, the live-ghost count and the ghost itself all need the world,
// so they sit behind #ifndef CLIENT_DLL.  What the client runs is the Core
// check, the animation and the cooldown, because m_flNextPrimaryAttack is
// predicted and a cooldown only one side knows about hitches at its end.  When
// the server refuses a summon the client cannot see coming -- the cap, or
// nowhere to stand -- the client has already set the long cooldown and the
// server's short one arrives a frame later and replaces it.  No Core is spent
// on either side for a refused summon: the client never spends at all, and
// the server spends only after the ghost stands.
//
// Placeholders, all for docs/ART_DEBT.md: the hivehand's viewmodel, world
// model and player model; the hivehand's HUD sprites (sprites/weapon_summon.txt,
// copied from the base game's weapon_hornetgun.txt) with the gauss's ammo icon
// standing in for a Core; the slave's zap sounds; and the Quake teleport splash
// for a ghost arriving and leaving.  item_core has its own model since
// 2026-09-19, w_core.mdl, the battery in green.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

#include "skill_tuning.h"
#include <algorithm>

// The ghost's own numbers are the server's, like the wave's damage; only the
// cooldown has a reading on both sides, through skill_tuning.h.
#ifndef CLIENT_DLL
#include "game.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_summon, CSummon);

#ifndef CLIENT_DLL
// Both defined in islave.cpp, where CISlave is visible.  The ghost is a slave
// with the ally bit set, so it lives beside the slave rather than here; what
// the weapon needs of it is one call to make one and one call to count them.
extern CBaseEntity* SummonCreateGhost(CBasePlayer* pOwner, const Vector& vecOrigin, float flLifetime);
extern int SummonCountGhosts(CBasePlayer* pOwner);
#endif

void CSummon::Spawn()
{
	Precache();
	m_iId = WEAPON_SUMMON;
	SET_MODEL(ENT(pev), "models/w_hgun.mdl");

	m_iDefaultAmmo = SUMMON_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}

void CSummon::Precache()
{
	PRECACHE_MODEL("models/v_hgun.mdl");
	PRECACHE_MODEL("models/w_hgun.mdl");
	PRECACHE_MODEL("models/p_hgun.mdl");

	// The slave's own, for now: the weapon and the thing it summons should at
	// least sound like each other.
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");

#ifndef CLIENT_DLL
	// A ghost is created in the middle of play, so its model and sounds have
	// to be registered before any of that happens.
	UTIL_PrecacheOther("monster_ghost_slave");
#endif
}

bool CSummon::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Cores";
	p->iMaxAmmo1 = CORE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 4; // beside the hivehand, the other alien weapon
	p->iId = WEAPON_SUMMON;
	p->iWeight = HORNETGUN_WEIGHT;
	// Out of Cores the weapon still deploys and is not switched away from:
	// the ultimate will want the same hand, and a Core is found, not bought.
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTOSWITCHEMPTY;
	return true;
}

bool CSummon::Deploy()
{
	return DefaultDeploy("models/v_hgun.mdl", "models/p_hgun.mdl", HGUN_UP, "hive");
}

void CSummon::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(HGUN_DOWN);
}

float CSummon::SummonCooldown()
{
	float flCooldown = std::max(0.0f, g_tuneSummonCooldown.Value());
	if (m_pPlayer->m_skills.HasSkill(ESkillId::Recall))
		flCooldown *= std::max(0.0f, g_tuneRecallScale.Value());
	return flCooldown;
}

void CSummon::PrimaryAttack()
{
	// The Core check runs on both sides: the client can see the count, and it
	// gates a delay the client predicts.
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 1)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		return;
	}

#ifndef CLIENT_DLL
	if (!TrySummon())
	{
		// The cap, or nowhere near the player a ghost fits.  The empty click,
		// and the Core stays: a summon that did not happen costs nothing.
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;

	// Server-side, unlike the animation: the client predicting a sound the
	// server also plays is the sound twice.
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM, 0, 100);
#endif

	SendWeaponAnim(HGUN_SHOOT);
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + SummonCooldown();
	// One cadence for both clicks, so the ultimate cannot be alternated with
	// the summon faster than either happens.
	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CSummon::SecondaryAttack()
{
	// The ultimate is the next slice (docs/ROADMAP.md, "The summon weapon --
	// settled"): fill the pack, hold the charge, fire the volley.  Until it
	// exists the right click spends nothing and summons nothing; it falls to
	// the idle, so a leaned-on button reads as a weapon doing nothing rather
	// than as a weapon that is broken.
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CSummon::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	const float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
	if (flRand <= 0.75)
	{
		SendWeaponAnim(HGUN_IDLE1);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
	else if (flRand <= 0.875)
	{
		SendWeaponAnim(HGUN_FIDGETSWAY);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
	}
	else
	{
		SendWeaponAnim(HGUN_FIDGETSHAKE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 35.0 / 16.0;
	}
}

#ifndef CLIENT_DLL
bool CSummon::TrySummon()
{
	// Pack raises the cap by a whole ghost rather than a scale: the design's
	// 1 -> 2 -> 3 ranks are one node in the tree today (docs/SKILL_TREE.md).
	int iMax = static_cast<int>(summon_max_ghosts.value);
	if (m_pPlayer->m_skills.HasSkill(ESkillId::Pack))
		iMax += static_cast<int>(skill_pack_bonus.value);
	iMax = std::max(1, iMax);

	const int iOut = SummonCountGhosts(m_pPlayer);
	if (iOut >= iMax)
	{
		if (debug_damage.value != 0)
			ALERT(at_console, "summon: refused, %d/%d ghosts already out\n", iOut, iMax);
		return false;
	}

	Vector vecSpot;
	if (!FindGhostSpot(vecSpot))
	{
		// The vent case the design left open: nowhere near the player a slave
		// fits standing on ground.  Refused outright rather than dropped
		// somewhere wrong, and the Core is kept.
		if (debug_damage.value != 0)
			ALERT(at_console, "summon: refused, no eligible surface near the player\n");
		return false;
	}

	float flLifetime = std::max(0.0f, summon_ghost_lifetime.value);
	if (m_pPlayer->m_skills.HasSkill(ESkillId::Tether))
		flLifetime *= std::max(0.0f, skill_tether_scale.value);

	if (SummonCreateGhost(m_pPlayer, vecSpot, flLifetime) == nullptr)
		return false;

	if (debug_damage.value != 0)
	{
		ALERT(at_console, "summon: spot (%.0f %.0f %.0f), %d Cores left, %d/%d ghosts out, %.1fs lifetime, %.1fs cooldown\n",
			vecSpot.x, vecSpot.y, vecSpot.z,
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] - 1,
			iOut + 1, iMax, flLifetime, SummonCooldown());
	}
	return true;
}

bool CSummon::FindGhostSpot(Vector& vecSpot)
{
	// Flat: only the yaw matters, or looking at the floor puts the ghost in
	// it.  Behind first, then the two sides -- a ghost that appears in front
	// of the player stands in the shot they were taking.  Spawning at the
	// crosshair was rejected in the design for the same reason.
	UTIL_MakeVectors(Vector(0, m_pPlayer->pev->v_angle.y, 0));
	const Vector rgvecDir[3] = {gpGlobals->v_forward * -1, gpGlobals->v_right, gpGlobals->v_right * -1};
	const float rgflDist[2] = {64.0f, 96.0f};

	// The player's origin sits at the middle of his hull, and so does the
	// point a hull trace is measured from, so the two are directly
	// comparable; a slave's own origin is at its feet, 36 below.
	const Vector vecEye = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
	const float flCentreZ = m_pPlayer->pev->origin.z + m_pPlayer->pev->mins.z + 36.0f;

	for (float flDist : rgflDist)
	{
		for (const Vector& vecDir : rgvecDir)
		{
			Vector vecTry = m_pPlayer->pev->origin + vecDir * flDist;
			vecTry.z = flCentreZ;

			// Down onto whatever floor is under it, from a little above the
			// player's own feet so a low step is still found.  A start inside
			// the world, or no floor within reach, is not a spot.
			TraceResult tr;
			UTIL_TraceHull(vecTry + Vector(0, 0, 16), vecTry - Vector(0, 0, 64),
				dont_ignore_monsters, human_hull, m_pPlayer->edict(), &tr);
			if (0 != tr.fStartSolid || 0 != tr.fAllSolid || tr.flFraction >= 1.0f)
				continue;

			const Vector vecCentre = tr.vecEndPos;

			// Nothing between the player and the ghost: a spot on the far
			// side of a wall is not "near the player" in any sense that
			// helps the fight the player is in.
			UTIL_TraceLine(vecEye, vecCentre, ignore_monsters, m_pPlayer->edict(), &tr);
			if (tr.flFraction < 1.0f)
				continue;

			vecSpot = vecCentre - Vector(0, 0, 36);
			return true;
		}
	}
	return false;
}
#endif

//=========================================================
// item_core -- the world pickup: a green battery.
//
// The HEV battery reskinned to the vortigaunt beam's green (Andrei,
// 2026-09-18; E:\CustomAssets\scripts\core_world.py), and it gives off light in
// that green on the client (cl_dll/entity.cpp, ProgressionLight), as the
// battery does in its own teal.
//
// Cores are found in the world only and are finite (docs/ROADMAP.md): one
// pickup is one Core, and the carry ceiling is CORE_MAX_CARRY.  Nothing makes
// them, so there is no place the player treks back to for more.
//=========================================================
#ifndef CLIENT_DLL
class CCoreAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_core.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_core.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_COREBOX_GIVE, "Cores", CORE_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_core, CCoreAmmo);
#endif
