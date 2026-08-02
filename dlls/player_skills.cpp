#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "weapons.h" // MAX_NORMAL_BATTERY
#include "player_skills.h"
#include "game.h"
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
    DEFINE_ARRAY(CPlayerSkills, m_bUnlocked,      FIELD_BOOLEAN, k_MaxSkills),
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

    memset(m_bUnlocked, 0, sizeof(m_bUnlocked));
}

// =====================================================================
// Queries
// =====================================================================
bool CPlayerSkills::AnyUnlocked() const
{
    for (int i = 1; i < k_MaxSkills; ++i)
    {
        if (m_bUnlocked[i])
            return true;
    }
    return false;
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
// CPlayerSkills::PrereqMet
//   Defers to the shared rule so the client cannot disagree about what
//   gates a Skill.
// =====================================================================
bool CPlayerSkills::PrereqMet(ESkillId id) const
{
    return SkillPrereqMet(static_cast<int>(id),
        [this](ESkillId prereq) { return HasSkill(prereq); });
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

    // A Skill with no row is a reserved id, not something buyable.
    const SkillDef& def = k_SkillDefs[i];
    if (!def.name || !def.name[0]) return false;

    if (!PrereqMet(id)) return false;
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

    // 100 is what CBasePlayer::Spawn sets, and the only baseline there is.
    const float desired = 100.0f + bonus;
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

    if (!pPlayer->m_skills.HasSkill(ESkillId::BatteryCapacity))
        return MAX_NORMAL_BATTERY;

    return MAX_NORMAL_BATTERY + std::max(0, (int)skill_battery_bonus.value);
}

// =====================================================================
// SkillScaleWeaponDamage
// =====================================================================
float SkillScaleWeaponDamage(entvars_t* pevAttacker, float flDamage)
{
    if (!pevAttacker || flDamage <= 0.0f)
        return flDamage;

    CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
    if (!pAttacker || !pAttacker->IsPlayer())
        return flDamage;

    CBasePlayer* pPlayer = (CBasePlayer*)pAttacker;
    if (!pPlayer->m_skills.HasSkill(ESkillId::ExtraDamage))
        return flDamage;

    return flDamage * std::max(0.0f, skill_weapon_damage_scale.value);
}

// =====================================================================
// SendSkillTreeToClient
//
// State only: one bit per unlocked Skill, then unspent Skill Points,
// then banked Reset Tokens.  Position, cost, prerequisites and tier all
// come from the shared table, and "available" is a display state the
// client derives from what is sent here.  The server stays authoritative
// where it matters -- TryUnlock and TryReset validate independently of
// anything the client believes.
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
    MESSAGE_END();
}
