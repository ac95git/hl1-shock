#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "player_skills.h"
#include "UserMessages.h"
#include <algorithm>

// =====================================================================
// Skill definitions
// =====================================================================
const SkillDef k_SkillDefs[k_MaxSkills] =
{
    //  id                          name                 description                           col  row  cost  prereq                   tier
    { ESkillId::None,               "None",              "",                                   0,   0,   0,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::CrowbarRange,       "Crowbar Reach",     "+25% melee range.",                  1,   0,   1,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::CrowbarDamage,      "Crowbar Force",     "+50% melee damage.",                 1,   1,   2,    ESkillId::CrowbarRange,  ENodeTier::Medium },
    { ESkillId::FastReload,         "Fast Reload",       "-20% reload time.",                  3,   0,   1,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::ExtraDamage,        "Weapon Mastery",    "+10% weapon damage.",                3,   1,   3,    ESkillId::FastReload,    ENodeTier::Major  },
    { ESkillId::HighJump,           "High Jump",         "+30% jump height.",                  5,   0,   1,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::SprintSpeed,        "Sprint",            "+15% movement speed.",               5,   1,   2,    ESkillId::SprintSpeed,   ENodeTier::Major  },
    { ESkillId::FallResistance,     "Fall Resist",       "-50% fall damage.",                  5,   2,   1,    ESkillId::HighJump,      ENodeTier::Medium },
    { ESkillId::MoreHealth,         "Fortitude",         "+25 max health.",                    7,   0,   2,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::ArmorEfficiency,    "Armor Expert",      "Armor absorbs 10% more damage.",     7,   1,   2,    ESkillId::MoreHealth,    ENodeTier::Medium },
    { ESkillId::HealthRegen,        "Regen",             "Slowly regenerate health.",          7,   2,   3,    ESkillId::MoreHealth,    ENodeTier::Major  },
    { ESkillId::CrowbarSpeed,       "Crowbar Speed",     "+30% crowbar attack speed.",         0,   2,   2,    ESkillId::CrowbarDamage, ENodeTier::Medium },
    { ESkillId::CrowbarParry,       "Crowbar Parry",     "Wider crowbar parry timing window.", 2,   2,   2,    ESkillId::CrowbarDamage, ENodeTier::Medium },
    { ESkillId::BatteryCapacity,    "Battery Capacity",  "+50 max battery.",                   9,   0,   2,    ESkillId::None,          ENodeTier::Minor  },
    { ESkillId::BatteryRegen,       "Battery Regen",     "Regenerate armor over time.",        9,   1,   3,    ESkillId::BatteryCapacity, ENodeTier::Major },
};

// =====================================================================
// Local save/restore descriptor table
// =====================================================================
static TYPEDESCRIPTION g_SkillsSaveData[] =
{
    DEFINE_FIELD(CPlayerSkills, m_iSkillPoints, FIELD_INTEGER),
    DEFINE_ARRAY(CPlayerSkills, m_bUnlocked,    FIELD_BOOLEAN, k_MaxSkills),
};

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
// CPlayerSkills::PrereqMet
// =====================================================================
bool CPlayerSkills::PrereqMet(ESkillId id) const
{
    int i = static_cast<int>(id);
    if (i <= 0 || i >= k_MaxSkills) return false;

    ESkillId prereq = k_SkillDefs[i].prereq;
    if (prereq == ESkillId::None) return true;
    return HasSkill(prereq);
}

// =====================================================================
// CPlayerSkills::TryUnlock
// =====================================================================
bool CPlayerSkills::TryUnlock(ESkillId id)
{
    int i = static_cast<int>(id);
    if (i <= 0 || i >= k_MaxSkills) return false;
    if (m_bUnlocked[i]) return false;
    if (!PrereqMet(id)) return false;

    int cost = k_SkillDefs[i].cost;
    if (m_iSkillPoints < cost) return false;

    m_bUnlocked[i]  = true;
    m_iSkillPoints -= cost;
    return true;
}

// =====================================================================
// SendSkillTreeToClient
// =====================================================================
void SendSkillTreeToClient(CBasePlayer* pPlayer)
{
    if (!pPlayer || gmsgSkillTree == 0) return;

    const CPlayerSkills& sk = pPlayer->m_skills;
    int count = k_MaxSkills - 1; // skip index 0 (None)

    MESSAGE_BEGIN(MSG_ONE, gmsgSkillTree, NULL, pPlayer->pev);
    WRITE_BYTE(count);

    for (int i = 1; i < k_MaxSkills; ++i)
    {
        const SkillDef& def = k_SkillDefs[i];
        bool unlocked  = sk.m_bUnlocked[i];
        bool available = !unlocked && sk.PrereqMet(static_cast<ESkillId>(i))
                         && sk.m_iSkillPoints >= def.cost;

        WRITE_BYTE(i);
        WRITE_BYTE((unsigned char)def.gridCol);
        WRITE_BYTE((unsigned char)def.gridRow);
        WRITE_BYTE((unsigned char)def.cost);
        WRITE_BYTE((unsigned char)static_cast<int>(def.prereq));
        WRITE_BYTE((unlocked ? 1 : 0) | (available ? 2 : 0) | (static_cast<int>(def.tier) << 2));
    }

    WRITE_BYTE((unsigned char)std::min(sk.m_iSkillPoints, 255));
    MESSAGE_END();
}
