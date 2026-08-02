#pragma once

#include "skill_defs.h"

// =====================================================================
// CPlayerSkills
//   Stored directly inside CBasePlayer.  All per-player skill state
//   lives here; everything that is the same for every player lives in
//   game_shared/skill_defs.h.
//
//   Points follow the inv_rows precedent: store what was EARNED and
//   derive what was spent, rather than storing a running balance.  A
//   reset is then just clearing the unlocked array -- the refund falls
//   out of the arithmetic, so there is no refund code to get wrong, and
//   retuning a cost or cutting a Skill corrects existing saves instead
//   of leaking or fabricating points.
// =====================================================================
struct CPlayerSkills
{
    // ---- Persistent data (save/restore) ----

    // Points a new game starts with, captured once from skill_points_start.
    // Stored rather than read live so that changing the cvar mid-playthrough
    // can neither hand out points retroactively nor take spent ones away.
    int m_iPointsBase    = 0;

    // Points found in the world since.  Never taken away, and never capped:
    // the only ceiling is how many pickups a map places.
    int m_iPointsGranted = 0;

    // Reset Tokens banked.  Spent one at a time, each wiping the whole tree.
    int m_iResetTokens   = 0;

    // Whether the two "start" cvars have been applied.  A separate flag
    // because zero is a legitimate starting value for both -- the Inventory
    // gets to test m_iRowsBase <= 0 only because a Grid always has a Row.
    bool m_bInitialised  = false;

    bool m_bUnlocked[k_MaxSkills] = {};

    // ---- Setup ----

    // Applies skill_points_start and skill_reset_tokens_start if they have
    // not been applied yet.  Safe to call repeatedly.
    void EnsureInitialised();

    void Clear();

    // ---- Queries ----

    bool HasSkill(ESkillId id) const
    {
        int i = static_cast<int>(id);
        if (i <= 0 || i >= k_MaxSkills) return false;
        return m_bUnlocked[i];
    }

    bool AnyUnlocked() const;

    // Every point the player has ever had.
    int TotalPoints() const { return m_iPointsBase + m_iPointsGranted; }

    // Summed cost of everything unlocked.  Skills no longer in the tree
    // contribute nothing, so cutting one refunds it on the next load.
    int SpentPoints() const;

    // What is left to spend.  Floored at zero so a cost change that pushes
    // spending past what was earned reads as "none left" rather than a
    // negative balance.
    int AvailablePoints() const;

    int ResetTokens() const { return m_iResetTokens; }

    // Returns true if every prerequisite for 'id' is satisfied.
    bool PrereqMet(ESkillId id) const;

    // ---- Mutations ----

    // Try to unlock a skill.  Returns true on success.
    bool TryUnlock(ESkillId id);

    // Spends one Reset Token and clears every unlocked Skill.  Returns false
    // -- changing nothing -- with no Token, or with nothing to undo, so a
    // Token can never be burned for no effect.
    bool TryReset();

    void AddSkillPoints(int pts);
    void AddResetTokens(int tokens);

    // Fills 'mask' (k_SkillMaskBytes long) with one bit per unlocked Skill.
    void BuildUnlockedMask(unsigned char* mask) const;
};

// =====================================================================
// Save/restore helpers � implemented in player_skills.cpp.
// CSave/CRestore are forward-declared so saverestore.h is NOT needed here.
// =====================================================================
class CSave;
class CRestore;

bool SkillsSave(CPlayerSkills& skills, CSave& save);
bool SkillsRestore(CPlayerSkills& skills, CRestore& restore);

// =====================================================================
// Sends the player's skill state -- an unlocked mask, unspent Skill
// Points and banked Reset Tokens.  Everything else the client needs it
// already has, from the shared definition table.
// =====================================================================
class CBasePlayer;
void SendSkillTreeToClient(CBasePlayer* pPlayer);

// =====================================================================
// Brings max health into line with whether Fortitude is held.
//
// Called at spawn AND whenever the tree changes, because a Skill can be
// unlocked or reset mid-life -- unlike the Pulse and Infusion modifiers,
// which are read fresh each time the effect is computed, this one edits
// durable state and so has to be re-applied rather than consulted.
// =====================================================================
void ApplySkillHealthBonus(CBasePlayer* pPlayer);

// =====================================================================
// The player's armour ceiling: MAX_NORMAL_BATTERY, plus the Battery
// Capacity bonus if that Skill is held.
//
// Every place that caps or fills armour must ask this rather than
// MAX_NORMAL_BATTERY, or the Skill silently does nothing through that
// route.  The client is told the answer so its armour bar scales.
// =====================================================================
int PlayerMaxArmor(CBasePlayer* pPlayer);
