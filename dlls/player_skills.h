#pragma once
#include "saverestore.h"

// =====================================================================
// ESkillId
//   Canonical list of all unlockable skills.
//   IDs must be stable – they are saved to disk and sent over the wire.
// =====================================================================
enum class ESkillId : int
{
    None = 0,

    // ---- Combat ----
    CrowbarRange        = 1,   // melee reach +25 %
    CrowbarDamage       = 2,   // melee damage +50 %
    FastReload          = 3,   // reload time -20 %
    ExtraDamage         = 4,   // all weapon damage +10 %

    // ---- Mobility ----
    HighJump            = 5,   // jump height +30 %
    SprintSpeed         = 6,   // movement speed +15 %
    FallResistance      = 7,   // fall damage -50 %

    // ---- Survivability ----
    MoreHealth          = 8,   // max health +25
    ArmorEfficiency     = 9,   // armor absorbs 10 % more damage
    HealthRegen         = 10,  // slowly regenerate health out of combat

    _Count              = 11,  // keep last
};

static constexpr int k_MaxSkills = static_cast<int>(ESkillId::_Count);

// =====================================================================
// CPlayerSkills
//   Stored directly inside CBasePlayer.  All skill state lives here.
// =====================================================================
struct CPlayerSkills
{
    // ---- Persistent data (save/restore) ----
    int m_iSkillPoints                  = 0;                     // unspent points
    bool m_bUnlocked[k_MaxSkills]       = {};                    // bUnlocked[ESkillId]

    // ---- Helpers ----
    bool HasSkill(ESkillId id) const
    {
        int i = static_cast<int>(id);
        if (i <= 0 || i >= k_MaxSkills) return false;
        return m_bUnlocked[i];
    }

    // Returns true if the prerequisite for 'id' is satisfied.
    bool PrereqMet(ESkillId id) const;

    // Try to unlock a skill.  Returns true on success.
    bool TryUnlock(ESkillId id);

    // Award skill points (e.g., on level-up or quest completion).
    void AddSkillPoints(int pts) { m_iSkillPoints += pts; }

    // Save/restore descriptor table (registered once in player_skills.cpp).
    static TYPEDESCRIPTION m_SaveData[];
    static int             m_SaveDataCount;
};

// =====================================================================
// Skill definition record (static, server-side only)
// =====================================================================
struct SkillDef
{
    ESkillId    id;
    const char* name;        // display name sent to client
    const char* description; // tooltip text
    int         gridCol;     // column in the client UI grid
    int         gridRow;     // row    in the client UI grid
    int         cost;        // skill-point cost
    ESkillId    prereq;      // ESkillId::None = root node
};

// Array of all skill definitions; defined in player_skills.cpp.
extern const SkillDef k_SkillDefs[k_MaxSkills];

// Forward: sends the full skill-tree state to a single player.
// Declared here so CHalfLifeRules::InitHUD and ClientCommand can call it.
class CBasePlayer;
void SendSkillTreeToClient(CBasePlayer* pPlayer);
