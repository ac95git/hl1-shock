#pragma once

// =====================================================================
// ENodeTier
//   Controls the visual weight of a skill node in the client UI.
//   Encoded in bits 2-3 of the flags byte sent via gmsgSkillTree.
// =====================================================================
enum class ENodeTier : uint8_t
{
    Minor  = 0,   // small node  – cheap/root skills
    Medium = 1,   // medium node – mid-tree skills
    Major  = 2,   // large node  – powerful end-tree skills
};

// =====================================================================
// ESkillId
//   Canonical list of all unlockable skills.
//   IDs must be stable � they are saved to disk and sent over the wire.
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
    ArmorEfficiency     = 9,  // armor absorbs 10 % more damage
    HealthRegen         = 10,  // slowly regenerate health out of combat

    // ---- Test branches (layout/connector validation) ----
    CrowbarSpeed        = 11,  // crowbar swing speed +30 %
    CrowbarFollowUp     = 18,  // the swing after a deflect hits harder

    // ---- The Pulse ----
    // Id 12 was CrowbarParry, a placeholder for a mechanic that turned out to
    // be suit hardware rather than a crowbar technique.  The id is reused
    // rather than retired: ids are saved and networked, so keeping 12 keeps
    // old saves loading, and changing its meaning is only safe because no
    // Skill had an effect yet.  Do not treat this as a precedent.
    PulseWindow         = 12,  // wider Pulse Window
    PulseRecharge       = 15,  // shorter Recharge
    PulseDischarge      = 16,  // negated hits vent at the crosshair
    PulseRebound        = 17,  // a deflect skips the Recharge, once per charge

    BatteryCapacity     = 13,  // +50 max battery
    BatteryRegen        = 14,  // passive armor regeneration

    _Count              = 19,  // keep last
};

static constexpr int k_MaxSkills = static_cast<int>(ESkillId::_Count);

// =====================================================================
// CPlayerSkills
//   Stored directly inside CBasePlayer.  All skill state lives here.
// =====================================================================
struct CPlayerSkills
{
    // ---- Persistent data (save/restore) ----
    int m_iSkillPoints                  = 20;    // unspent points
    bool m_bUnlocked[k_MaxSkills]       = {};   // bUnlocked[ESkillId]

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
    ENodeTier   tier;        // visual weight in the client UI
};

// Array of all skill definitions; defined in player_skills.cpp.
extern const SkillDef k_SkillDefs[k_MaxSkills];

// =====================================================================
// Save/restore helpers � implemented in player_skills.cpp.
// CSave/CRestore are forward-declared so saverestore.h is NOT needed here.
// =====================================================================
class CSave;
class CRestore;

bool SkillsSave(CPlayerSkills& skills, CSave& save);
bool SkillsRestore(CPlayerSkills& skills, CRestore& restore);

// =====================================================================
// Sends the full skill-tree state to a single player.
// =====================================================================
class CBasePlayer;
void SendSkillTreeToClient(CBasePlayer* pPlayer);
