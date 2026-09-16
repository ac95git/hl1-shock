#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "weapons.h" // MAX_NORMAL_BATTERY
#include "player_skills.h"
#include "game.h"
#include "skill.h" // gSkillData, for the medkit's heal
#include "skill_tuning.h" // Hive Replenish and the Hornet Replenish Stat: read the same way the hivehand will
#include "monsters.h" // Ambush: CBaseMonster, R_HT/R_NM/R_DL, SF_MONSTER_IGNORE_CONCEALMENT
#include "perception.h" // PlayerConcealmentScale, for the Status page's Concealment line
#include "UserMessages.h"
#include <algorithm>
#include <cstring>

// Skill definitions live in game_shared/skill_defs.h -- they are identical
// for every player and are compiled into the client as well, so they are
// never sent over the wire.  See
// docs/adr/0008-skill-definitions-are-shared-not-networked.md.

// =====================================================================
// Local save/restore descriptor table
// =====================================================================
static TYPEDESCRIPTION g_SkillsSaveData[] =
{
    DEFINE_FIELD(CPlayerSkills, m_iPointsBase,    FIELD_INTEGER),
    DEFINE_FIELD(CPlayerSkills, m_iPointsGranted, FIELD_INTEGER),
    DEFINE_FIELD(CPlayerSkills, m_iResetTokens,   FIELD_INTEGER),
    DEFINE_FIELD(CPlayerSkills, m_bInitialised,   FIELD_BOOLEAN),
    DEFINE_FIELD(CPlayerSkills, m_iOpenGates,     FIELD_INTEGER),

    // The unlocked array is saved under its own name, not the member's.
    // CRestore::ReadField copies as many entries as the code declares, so a
    // save written when this array was sized by _Count (22 entries, under
    // the name "m_bUnlocked") would be over-read into the bytes that follow
    // it.  A field the save does not contain is simply left cleared, which
    // for this array means an old save loads with its tree reset and every
    // point refunded -- the same self-correction a cut Skill relies on.
    //
    // The name carries the ceiling for the same reason: "m_bUnlockedCeiling"
    // was the 96-entry array, and a save holding it would be over-read by
    // the 256-entry one.  Rename again if the ceiling ever moves again.
    { FIELD_BOOLEAN, "m_bUnlocked256", static_cast<int>(offsetof(CPlayerSkills, m_bUnlocked)), k_SkillIdCeiling, 0 },
};

// =====================================================================
// Cvar readers.  Clamped so a typo cannot produce a negative balance.
// =====================================================================
static int CvarPointsStart()
{
    return std::max(0, static_cast<int>(skill_points_start.value));
}

static int CvarResetTokensStart()
{
    return std::max(0, static_cast<int>(skill_reset_tokens_start.value));
}

// =====================================================================
// SkillsSave / SkillsRestore  (declared in player_skills.h)
// =====================================================================
bool SkillsSave(CPlayerSkills& skills, CSave& save)
{
    return save.WriteFields("SKILLS", &skills, g_SkillsSaveData, ARRAYSIZE(g_SkillsSaveData));
}

bool SkillsRestore(CPlayerSkills& skills, CRestore& restore)
{
    return restore.ReadFields("SKILLS", &skills, g_SkillsSaveData, ARRAYSIZE(g_SkillsSaveData));
}

// =====================================================================
// Setup
// =====================================================================
void CPlayerSkills::EnsureInitialised()
{
    // The Suit is held from the first moment and costs nothing, so it is
    // asserted here rather than bought: outside the one-time guard, because
    // a save written before the Suit existed has the flag set and the bit
    // clear (ADR-0012).
    HoldSuit();

    // The Pulse is suit hardware today, not a found Module (docs/SKILL_TREE.md,
    // Juggernaut), so its gate is open unconditionally -- every call, not just
    // the first -- so a save written before m_iOpenGates existed (the bit
    // clear, same as an old Suit flag above) still shows the Pulse's nodes.
    // Take this out when the Pulse becomes a Module with its own reveal.
    OpenGate(EGate::PulseModule);

    if (m_bInitialised)
        return;

    m_bInitialised   = true;
    m_iPointsBase    = CvarPointsStart();
    m_iResetTokens  += CvarResetTokensStart();
}

void CPlayerSkills::Clear()
{
    m_iPointsBase    = 0;
    m_iPointsGranted = 0;
    m_iResetTokens   = 0;
    m_bInitialised   = false;
    m_iOpenGates     = 0;

    memset(m_bUnlocked, 0, sizeof(m_bUnlocked));
}

// =====================================================================
// Queries
// =====================================================================
bool CPlayerSkills::AnyUnlocked() const
{
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        // The Suit is always held and is not something a Reset undoes, so
        // it must not make an empty tree look like one worth a Token.
        if (m_bUnlocked[i] && k_SkillDefs[i].tier != ENodeTier::Suit)
            return true;
    }
    return false;
}

int CPlayerSkills::CountStat(EStat stat) const
{
    if (stat == EStat::None)
        return 0;

    int count = 0;
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        if (m_bUnlocked[i] && k_SkillDefs[i].stat == stat)
            ++count;
    }
    return count;
}

int CPlayerSkills::SpentPoints() const
{
    int spent = 0;
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        if (!m_bUnlocked[i])
            continue;

        // A Skill cut from the tree keeps its reserved id but has no row to
        // charge for, so an old save that bought it gets those points back.
        const SkillDef& def = k_SkillDefs[i];
        if (!def.name || !def.name[0])
            continue;

        spent += def.cost;
    }
    return spent;
}

int CPlayerSkills::AvailablePoints() const
{
    return std::max(0, TotalPoints() - SpentPoints());
}

// =====================================================================
// CPlayerSkills::Reachable
//   Defers to the shared rule -- any owned orthogonal neighbour -- so the
//   client cannot disagree about what opens a node.
// =====================================================================
bool CPlayerSkills::Reachable(ESkillId id) const
{
    return SkillReachable(static_cast<int>(id),
        [this](int other) { return HasSkill(static_cast<ESkillId>(other)); });
}

// =====================================================================
// Mutations
// =====================================================================
bool CPlayerSkills::TryUnlock(ESkillId id)
{
    EnsureInitialised();

    int i = static_cast<int>(id);
    if (i <= 0 || i >= k_MaxSkills) return false;
    if (m_bUnlocked[i]) return false;

    // A Skill with no row is a reserved id, not something buyable; the Suit
    // is held, never bought.
    const SkillDef& def = k_SkillDefs[i];
    if (!def.name || !def.name[0]) return false;
    if (def.tier == ENodeTier::Suit) return false;

    // A hidden node -- its gate closed -- is a blank pad and cannot be
    // bought, whatever it is otherwise reachable through (ADR-0012,
    // "Hidden means impassable").
    if (IsNodeHidden(id)) return false;

    if (!Reachable(id)) return false;
    if (AvailablePoints() < def.cost) return false;

    // Nothing is decremented: spending is derived from what is unlocked.
    m_bUnlocked[i] = true;
    return true;
}

bool CPlayerSkills::TryReset()
{
    EnsureInitialised();

    if (m_iResetTokens <= 0)
        return false;

    // Refuse rather than burn a Token on a tree with nothing in it.
    if (!AnyUnlocked())
        return false;

    --m_iResetTokens;
    memset(m_bUnlocked, 0, sizeof(m_bUnlocked));
    HoldSuit();
    return true;
}

void CPlayerSkills::AddSkillPoints(int pts)
{
    EnsureInitialised();

    if (pts <= 0)
        return;

    m_iPointsGranted += pts;
}

void CPlayerSkills::AddResetTokens(int tokens)
{
    EnsureInitialised();

    if (tokens <= 0)
        return;

    m_iResetTokens += tokens;
}

bool CPlayerSkills::UnlockAll()
{
    EnsureInitialised();

    bool changed = false;
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        // A reserved id has no row and is not a Skill to hold.
        const SkillDef& def = k_SkillDefs[i];
        if (!def.name || !def.name[0])
            continue;
        if (m_bUnlocked[i])
            continue;

        // A hidden node stays hidden: skill_unlock_all is a debugging aid
        // for the tree, not a way to see past a Module that has not been
        // found.
        if (IsNodeHidden(static_cast<ESkillId>(i)))
            continue;

        m_bUnlocked[i] = true;
        changed = true;
    }
    return changed;
}

// =====================================================================
// CPlayerSkills::BuildUnlockedMask
// =====================================================================
void CPlayerSkills::BuildUnlockedMask(unsigned char* mask) const
{
    if (!mask) return;

    memset(mask, 0, k_SkillMaskBytes);
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        if (m_bUnlocked[i])
            SkillMaskSet(mask, i, true);
    }
}

// =====================================================================
// ApplySkillHealthBonus
// =====================================================================
void ApplySkillHealthBonus(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    const float bonus = pPlayer->m_skills.HasSkill(ESkillId::MoreHealth)
        ? std::max(0.0f, skill_health_bonus.value)
        : 0.0f;

    // The hub's Max Health Stat nodes: a fraction each, on top of Fortitude's
    // flat bonus, so the rim Minor and its road multiply as every other Route's
    // entry and roads do.  100 is what CBasePlayer::Spawn sets, and the only
    // baseline there is.  Rounded so max health stays a whole number on the HUD.
    const int   iStat   = pPlayer->m_skills.CountStat(EStat::MaxHealth);
    const float scale   = 1.0f + iStat * std::max(0.0f, skill_stat_max_health.value);
    float desired = (float)(int)((100.0f + bonus) * scale + 0.5f);

    // Glass Cannon (the keystone): the ceiling AFTER every other health bonus
    // -- Fortitude and the hub's Max Health Stat nodes included -- never the
    // floor, so a build with fewer of those still lands on 50.
    if (pPlayer->m_skills.HasSkill(ESkillId::GlassCannon))
        desired = std::min(desired, std::max(1.0f, skill_glass_cannon_max_health.value));

    const float delta   = desired - pPlayer->pev->max_health;
    if (delta == 0.0f)
        return;

    pPlayer->pev->max_health = desired;

    if (delta > 0.0f)
    {
        // Raising the cap alone would mean unlocking Fortitude does nothing
        // until the next medkit, which reads as the Skill being broken.
        pPlayer->pev->health += delta;
    }
    else if (pPlayer->pev->health > desired)
    {
        // A reset must not leave the player above their new maximum.
        pPlayer->pev->health = desired;
    }
}

// =====================================================================
// PlayerMaxArmor
// =====================================================================
int PlayerMaxArmor(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return MAX_NORMAL_BATTERY;

    const int base = pPlayer->m_skills.HasSkill(ESkillId::BatteryCapacity)
        ? MAX_NORMAL_BATTERY + std::max(0, (int)skill_battery_bonus.value)
        : MAX_NORMAL_BATTERY;

    // The Max Armour Stat nodes -- the hub's south side and the Juggernaut's
    // roads -- a fraction each on top of Battery Capacity's flat bonus.
    const int   iStat = pPlayer->m_skills.CountStat(EStat::MaxArmour);
    const float scale = 1.0f + iStat * std::max(0.0f, skill_stat_max_armor.value);
    return (int)(base * scale + 0.5f);
}

// =====================================================================
// PlayerHornetMaxCarry
// =====================================================================
int PlayerHornetMaxCarry(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return HORNET_MAX_CARRY;

    if (!pPlayer->m_skills.HasSkill(ESkillId::HiveCapacity))
        return HORNET_MAX_CARRY;

    return HORNET_MAX_CARRY + std::max(0, (int)skill_hive_capacity_bonus.value);
}

// =====================================================================
// OverdrawSpendArmor
//   Overdraw (the Energy major): every point of uranium an energy attack
//   spends also drains armour, at skill_overdraw_armor_per_uranium per
//   unit, never below skill_overdraw_floor.  Called from the egon (per
//   UseAmmo) and the katana wave (per WaveCost charge), after Energy
//   Efficiency has already reduced uraniumSpent, so the drain always tracks
//   what actually left the player.  Declared in game.h -- see there for why.
// =====================================================================
void OverdrawSpendArmor(CBasePlayer* pPlayer, int uraniumSpent)
{
    if (!pPlayer || uraniumSpent <= 0)
        return;
    if (!pPlayer->m_skills.HasSkill(ESkillId::EnergyMajor))
        return;

    const float floor = std::max(0.0f, skill_overdraw_floor.value);
    if (pPlayer->pev->armorvalue <= floor)
        return;

    const float perUranium = std::max(0.0f, skill_overdraw_armor_per_uranium.value);
    const float drain = std::min(uraniumSpent * perUranium, pPlayer->pev->armorvalue - floor);
    if (drain <= 0.0f)
        return;

    pPlayer->pev->armorvalue -= drain;

    // The Ricochet lesson: a Skill whose effect is a number changing
    // quietly needs a readout, not just a code-derived guess.
    if (debug_damage.value != 0)
    {
        ALERT(at_console, "overdraw: spent %d uranium, drained %.1f armour, armour now %.1f\n",
            uraniumSpent, drain, pPlayer->pev->armorvalue);
    }
}

// =====================================================================
// SkillScaleWeaponDamage
// =====================================================================
float SkillScaleWeaponDamage(entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    if (!pevAttacker || flDamage <= 0.0f)
        return flDamage;

    CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
    if (!pAttacker || !pAttacker->IsPlayer())
        return flDamage;

    CBasePlayer* pPlayer = (CBasePlayer*)pAttacker;
    const CPlayerSkills& sk = pPlayer->m_skills;

    // Everything that always applies: Weapon Mastery and the typed damage.
    flDamage *= PlayerStandingDamageScale(pPlayer, bitsDamageType);

    // Swap Surge: everything, while the window after a swap is open.  A
    // window rather than one empowered shot, so the egon and MP5 get their
    // burst as much as the shotgun and python get a big first shot.
    if (pPlayer->SwapSurgeActive())
        flDamage *= std::max(0.0f, skill_swap_surge_scale.value);

    // Overdraw: armour as fuel, both ways.  While there is armour above
    // the floor left to drain, energy attacks hit harder too.  Situational,
    // so not in the standing scale or on the Status page.
    if ((bitsDamageType & DMG_ENERGYBEAM) != 0 && sk.HasSkill(ESkillId::EnergyMajor)
        && pPlayer->pev->armorvalue > std::max(0.0f, skill_overdraw_floor.value))
        flDamage *= std::max(0.0f, skill_overdraw_damage_scale.value);

    return flDamage;
}

// =====================================================================
// PlayerStandingDamageScale
// =====================================================================
float PlayerStandingDamageScale(CBasePlayer* pPlayer, int bitsDamageType)
{
    if (!pPlayer)
        return 1.0f;

    const CPlayerSkills& sk = pPlayer->m_skills;
    float scale = 1.0f;

    // Weapon Mastery: everything.
    if (sk.HasSkill(ESkillId::ExtraDamage))
        scale *= std::max(0.0f, skill_weapon_damage_scale.value);

    // The Weapon Specialist's typed damage.  A damage-type test here rather
    // than a weapon list: whatever a weapon fires as bullets is bullets, so
    // the glock, MP5, shotgun and python are covered and the crossbow's bolt
    // (DMG_NEVERGIB alone) is not.  Melee and energy have their own Routes.
    if ((bitsDamageType & DMG_BULLET) != 0)
    {
        if (sk.HasSkill(ESkillId::Marksman))
            scale *= std::max(0.0f, skill_marksman_scale.value);

        // The Bullet Damage Stat nodes: additive within the stat, multiplied
        // with everything else, the Melee Damage nodes' shape.
        const int iStat = sk.CountStat(EStat::BulletDamage);
        if (iStat > 0)
            scale *= 1.0f + iStat * std::max(0.0f, skill_stat_bullet_damage.value);
    }

    // The Energy Route: energy is DMG_ENERGYBEAM and nothing else, so the
    // katana (slash and wave), the egon and the Discharge all read it here.
    // The katana therefore scales off Melee (in its Swing) and Energy (here)
    // both, which is the Gargantua build in one sentence.
    if ((bitsDamageType & DMG_ENERGYBEAM) != 0)
    {
        if (sk.HasSkill(ESkillId::EnergyDamage))
            scale *= std::max(0.0f, skill_energy_damage_scale.value);

        const int iStat = sk.CountStat(EStat::EnergyDamage);
        if (iStat > 0)
            scale *= 1.0f + iStat * std::max(0.0f, skill_stat_energy_damage.value);
    }

    // Demolitions, dealt.  Grenades, the satchel, the tripmine, the RPG and
    // the MP5's launcher all arrive here as DMG_BLAST through RadiusDamage,
    // by either branch.  So does the egon's splash, which carries the bit
    // beside its energy; accepted rather than special-cased.
    if ((bitsDamageType & DMG_BLAST) != 0 && sk.HasSkill(ESkillId::Demolitions))
        scale *= std::max(0.0f, skill_demolitions_scale.value);

    return scale;
}

// =====================================================================
// PlayerMeleeScale
// =====================================================================
float PlayerMeleeScale(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 1.0f;

    const CPlayerSkills& sk = pPlayer->m_skills;
    float scale = 1.0f;

    // Melee Force.
    if (sk.HasSkill(ESkillId::MeleeForce))
        scale *= std::max(0.0f, skill_melee_force_scale.value);

    // The Melee Damage Stat nodes: additive within the stat, multiplied
    // with everything else. Five at 0.05 are x1.25 on top of Force.
    const int iStat = sk.CountStat(EStat::MeleeDamage);
    if (iStat > 0)
        scale *= 1.0f + iStat * std::max(0.0f, skill_stat_melee_damage.value);

    return scale;
}

// =====================================================================
// The protection scales
// =====================================================================
float PlayerArmorRatioScale(CBasePlayer* pPlayer)
{
    // Armor Expert.  The ratio is the fraction of a blow that gets PAST
    // armour, so scaling it DOWN is what makes armour better.
    if (!pPlayer || !pPlayer->m_skills.HasSkill(ESkillId::ArmorEfficiency))
        return 1.0f;
    return std::max(0.0f, skill_armor_ratio_scale.value);
}

float PlayerBlastTakenScale(CBasePlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->m_skills.HasSkill(ESkillId::Demolitions))
        return 1.0f;
    return std::max(0.0f, skill_demolitions_resist_scale.value);
}

float PlayerEnergyTakenScale(CBasePlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->m_skills.HasSkill(ESkillId::Insulation))
        return 1.0f;
    return std::max(0.0f, skill_insulation_scale.value);
}

float PlayerFallTakenScale(CBasePlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->m_skills.HasSkill(ESkillId::FallResistance))
        return 1.0f;
    return std::max(0.0f, skill_fall_damage_scale.value);
}

// =====================================================================
// PlayerAmbushScale -- see player_skills.h for the shape of the problem.
//
// The tighter tier replaces the looser one rather than stacking: a monster
// that has not even Noticed the player is also below Spotted, and the
// design is one bonus per hit, not two added together.
// =====================================================================
float PlayerAmbushScale(CBasePlayer* pPlayer, CBaseEntity* pVictim)
{
    if (!pPlayer || !pVictim)
        return 1.0f;
    if (!pPlayer->m_skills.HasSkill(ESkillId::Ambush))
        return 1.0f;

    // suspicion_enable 0 pins every meter to 1.0 (UpdateSuspicion), which
    // already reads as "aware" below -- this early-out is just cheaper.
    if (suspicion_enable.value == 0)
        return 1.0f;

    CBaseMonster* pMonster = pVictim->MyMonsterPointer();
    if (!pMonster || !pMonster->IsAlive())
        return 1.0f;

    // The same hostility test SyncConcealState uses (dlls/perception.cpp):
    // a monster fighting something else is still ambushable, a friendly one
    // never is.
    const int iRelationship = pMonster->IRelationship(pPlayer);
    if (iRelationship != R_HT && iRelationship != R_NM && iRelationship != R_DL)
        return 1.0f;

    // An always-aware profile or a mapper's SF_MONSTER_IGNORE_CONCEALMENT
    // both mean the meter is not a real answer -- pinned full, acquiring on
    // sight -- so a per-hit bonus keyed to it would be free damage rather
    // than a reward for stealth.
    if (!pMonster->GetPerceptionProfile().bUsesSuspicion)
        return 1.0f;
    if (FBitSet(pMonster->pev->spawnflags, SF_MONSTER_IGNORE_CONCEALMENT))
        return 1.0f;

    if (pMonster->m_flSuspicion < suspicion_notice.value)
        return std::max(1.0f, skill_ambush_noticed_scale.value);
    if (pMonster->m_flSuspicion < suspicion_acquire.value)
        return std::max(1.0f, skill_ambush_spotted_scale.value);

    return 1.0f;
}

// =====================================================================
// SendSkillStatsToClient
//
// Two maxima, then nine multipliers and shares in thousandths, then the
// Dash recharge in milliseconds, then the Concealment share and the hornet
// replenish multiplier -- CHudAmmo::MsgFunc_SkillStats reads them in this
// order.  Every value comes from the function the effect itself
// reads, so the Status page cannot disagree with the game.  A resistance
// is sent as the share resisted, because the page writes it that way.
// =====================================================================
void SendSkillStatsToClient(CBasePlayer* pPlayer)
{
    if (!pPlayer || gmsgSkillStats == 0)
        return;

    auto milli = [](float v) { return (int)std::min(32767.0f, std::max(0.0f, v * 1000.0f + 0.5f)); };

    MESSAGE_BEGIN(MSG_ONE, gmsgSkillStats, NULL, pPlayer->pev);
    WRITE_SHORT(std::min(32767, std::max(0, (int)pPlayer->pev->max_health)));
    WRITE_SHORT(std::min(32767, std::max(0, PlayerMaxArmor(pPlayer))));
    WRITE_SHORT(milli(PlayerHealingScale(pPlayer)));
    WRITE_SHORT(milli(1.0f - (float)ARMOR_RATIO * PlayerArmorRatioScale(pPlayer)));
    WRITE_SHORT(milli(1.0f - PlayerBlastTakenScale(pPlayer)));
    WRITE_SHORT(milli(1.0f - PlayerEnergyTakenScale(pPlayer)));
    WRITE_SHORT(milli(1.0f - PlayerFallTakenScale(pPlayer)));
    WRITE_SHORT(milli(PlayerMeleeScale(pPlayer) * PlayerStandingDamageScale(pPlayer, DMG_CLUB)));
    WRITE_SHORT(milli(PlayerStandingDamageScale(pPlayer, DMG_BULLET)));
    WRITE_SHORT(milli(PlayerStandingDamageScale(pPlayer, DMG_ENERGYBEAM)));
    WRITE_SHORT(milli(PlayerStandingDamageScale(pPlayer, DMG_BLAST)));
    WRITE_SHORT(milli(pPlayer->DashRechargeTime()));
    // The other two Module stats, since 2026-09-16.  Concealment is sent as
    // the share by which monsters learn slower (a virtue grows with its
    // label: "+25%", never "x0.75"); hornet replenish as its multiplier.
    WRITE_SHORT(milli(1.0f - PlayerConcealmentScale(pPlayer)));
    WRITE_SHORT(milli(PlayerHornetReplenishScale(pPlayer)));
    MESSAGE_END();
}

// =====================================================================
// PlayerHealingScale / PlayerMedkitHeal
// =====================================================================
float PlayerHealingScale(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 1.0f;

    const int iStat = pPlayer->m_skills.CountStat(EStat::Healing);
    if (iStat <= 0)
        return 1.0f;

    return 1.0f + iStat * std::max(0.0f, skill_stat_healing.value);
}

float PlayerMedkitHeal(CBasePlayer* pPlayer)
{
    return gSkillData.healthkitCapacity * PlayerHealingScale(pPlayer);
}

// =====================================================================
// PlayerHornetReplenishScale
//
// Hive Replenish and the Hornet Replenish Stat nodes, multiplied together
// like Melee Force and the Melee Damage Stat nodes.  Both cvars are read
// through skill_tuning.h's CSkillTuning rather than game.h -- see the
// comment beside skill_hive_replenish_scale there -- even though this
// function is only ever called from CHgun::Reload's #ifndef CLIENT_DLL
// block today, so the reads already agree if that ever changes.
// =====================================================================
float PlayerHornetReplenishScale(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 1.0f;

    const CPlayerSkills& sk = pPlayer->m_skills;
    float scale = 1.0f;

    if (sk.HasSkill(ESkillId::HiveRegrowth))
        scale *= std::max(0.0f, g_tuneHiveReplenish.Value());

    const int iStat = sk.CountStat(EStat::HornetReplenish);
    if (iStat > 0)
        scale *= 1.0f + iStat * std::max(0.0f, g_tuneHornetReplenishStat.Value());

    return scale;
}

// =====================================================================
// SkillHeadshotScale
// =====================================================================
float SkillHeadshotScale(entvars_t* pevAttacker)
{
    if (!pevAttacker)
        return 1.0f;

    CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
    if (!pAttacker || !pAttacker->IsPlayer())
        return 1.0f;

    if (!((CBasePlayer*)pAttacker)->m_skills.HasSkill(ESkillId::Headhunter))
        return 1.0f;

    return std::max(0.0f, skill_headhunter_scale.value);
}

// =====================================================================
// SendSkillTreeToClient
//
// State only: one bit per unlocked Skill, then unspent Skill Points,
// then banked Reset Tokens, then the open-gates bitmask.  Position, cost,
// prerequisites and tier all come from the shared table, and "available"
// is a display state the client derives from what is sent here.  The
// server stays authoritative where it matters -- TryUnlock and TryReset
// validate independently of anything the client believes.
// =====================================================================
void SendSkillTreeToClient(CBasePlayer* pPlayer)
{
    if (!pPlayer || gmsgSkillTree == 0) return;

    CPlayerSkills& sk = pPlayer->m_skills;
    sk.EnsureInitialised();

    unsigned char mask[k_SkillMaskBytes];
    sk.BuildUnlockedMask(mask);

    MESSAGE_BEGIN(MSG_ONE, gmsgSkillTree, NULL, pPlayer->pev);

    for (int i = 0; i < k_SkillMaskBytes; ++i)
        WRITE_BYTE(mask[i]);

    WRITE_BYTE((unsigned char)std::min(sk.AvailablePoints(), 255));
    WRITE_BYTE((unsigned char)std::min(sk.ResetTokens(), 255));

    // One byte: which gates are open, bit N == EGate value N.  Small enough
    // that a byte covers every EGate today; widen this (and the message
    // length below and in UserMessages.cpp) if a fifth Module needs bit 8.
    WRITE_BYTE((unsigned char)(sk.OpenGatesMask() & 0xFF));
    MESSAGE_END();

    // Every change that can move a Status page number -- a Skill unlocked, a
    // Reset, a Module found, spawn and restore -- already ends up here.
    SendSkillStatsToClient(pPlayer);
}
