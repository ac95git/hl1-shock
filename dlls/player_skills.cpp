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
    //  id                          name                 description                           col  row  cost  prereq
    { ESkillId::None,               "None",              "",                                   0,   0,   0,    ESkillId::None          },
    { ESkillId::CrowbarRange,       "Crowbar Reach",     "+25% melee range.",                  0,   0,   1,    ESkillId::None          },
    { ESkillId::CrowbarDamage,      "Crowbar Force",     "+50% melee damage.",                 0,   1,   2,    ESkillId::CrowbarRange  },
    { ESkillId::FastReload,         "Fast Reload",       "-20% reload time.",                  1,   0,   1,    ESkillId::None          },
    { ESkillId::ExtraDamage,        "Weapon Mastery",    "+10% weapon damage.",                1,   1,   3,    ESkillId::FastReload     },
    { ESkillId::HighJump,           "High Jump",         "+30% jump height.",                  2,   0,   1,    ESkillId::None          },
    { ESkillId::SprintSpeed,        "Sprint",            "+15% movement speed.",               2,   1,   2,    ESkillId::HighJump       },
    { ESkillId::FallResistance,     "Fall Resist",       "-50% fall damage.",                  2,   2,   1,    ESkillId::HighJump       },
    { ESkillId::MoreHealth,         "Fortitude",         "+25 max health.",                    3,   0,   2,    ESkillId::None          },
    { ESkillId::ArmorEfficiency,    "Armor Expert",      "Armor absorbs 10% more damage.",     3,   1,   2,    ESkillId::MoreHealth     },
    { ESkillId::HealthRegen,        "Regen",             "Slowly regenerate health.",          3,   2,   3,    ESkillId::MoreHealth     },
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
        WRITE_BYTE((unlocked ? 1 : 0) | (available ? 2 : 0));
    }

    WRITE_BYTE((unsigned char)std::min(sk.m_iSkillPoints, 255));
    MESSAGE_END();
}
