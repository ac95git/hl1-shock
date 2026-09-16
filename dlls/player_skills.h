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

    // Sized to the id ceiling rather than to _Count, so the saved array keeps
    // one shape as Skills are added.  Only entries below k_MaxSkills mean
    // anything; the rest are room.
    bool m_bUnlocked[k_SkillIdCeiling] = {};

    // Which gates (EGate) are open, one bit per value: bit N is
    // static_cast<int>(EGate) == N, so bit 0 is spare since EGate::None
    // never gates anything.  EGate::PulseModule starts open (see
    // EnsureInitialised); every other gate starts closed.  Per-player state,
    // so it is saved and sent to the client alongside the unlocked mask.
    int m_iOpenGates = 0;

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

    // How many unlocked Stat nodes grant 'stat'.  A Stat node's effect is
    // the same for every node of its kind, so this count times the stat's
    // cvar is the whole bonus; read it where the effect is computed, the
    // way HasSkill is.
    int CountStat(EStat stat) const;

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

    // The raw bitmask, for SendSkillTreeToClient -- everything else should
    // ask IsGateOpen/IsNodeHidden instead of reading bits itself.
    int OpenGatesMask() const { return m_iOpenGates; }

    // Returns true if a node in a cell orthogonally adjacent to 'id' is held
    // (ADR-0012: the tree has no prerequisites, only neighbours).
    bool Reachable(ESkillId id) const;

    // The Suit is the one start: held from the first moment, kept through a
    // Reset, never bought.  Idempotent; called wherever the array is set up
    // or cleared so no code path can leave a player without it.
    void HoldSuit() { m_bUnlocked[static_cast<int>(ESkillId::Suit)] = true; }

    // Whether a gate value has a bit in m_iOpenGates: EGate::None does not
    // (it never gates anything), and neither does a value past _Count --
    // 1 << N is undefined for N >= 32, and skill_open_gates/close_gates
    // pass a raw console int straight through, so this is the one place
    // that range check has to happen.
    static bool GateHasBit(EGate gate)
    {
        return gate > EGate::None && static_cast<int>(gate) < static_cast<int>(EGate::_Count);
    }

    // Whether the Module a gate stands for has been found.  EGate::None is
    // always "open" -- it never gates anything -- so a caller can pass a
    // node's gate straight through with no special case.
    bool IsGateOpen(EGate gate) const
    {
        if (!GateHasBit(gate))
            return true;
        return (m_iOpenGates & (1 << static_cast<int>(gate))) != 0;
    }

    // A node behind a closed gate is a blank pad and cannot be bought
    // (ADR-0012, "Hidden means impassable").
    bool IsNodeHidden(ESkillId id) const
    {
        const SkillDef* def = GetSkillDef(id);
        if (!def || def->gate == EGate::None)
            return false;
        return !IsGateOpen(def->gate);
    }

    // ---- Mutations ----

    // Opens or closes one gate.  A gate with no bit (None, or past _Count --
    // see GateHasBit) is a no-op, which is what a garbage console argument
    // to skill_open_gates/close_gates should do: nothing.
    void OpenGate(EGate gate)
    {
        if (GateHasBit(gate))
            m_iOpenGates |= (1 << static_cast<int>(gate));
    }

    void CloseGate(EGate gate)
    {
        if (GateHasBit(gate))
            m_iOpenGates &= ~(1 << static_cast<int>(gate));
    }

    // Try to unlock a skill.  Returns true on success.
    bool TryUnlock(ESkillId id);

    // Spends one Reset Token and clears every unlocked Skill.  Returns false
    // -- changing nothing -- with no Token, or with nothing to undo, so a
    // Token can never be burned for no effect.
    bool TryReset();

    void AddSkillPoints(int pts);
    void AddResetTokens(int tokens);

    // The skill_unlock_all debugging aid: marks every buyable Skill held,
    // cost and prerequisites ignored.  Returns true if anything changed, so
    // the caller knows whether to re-apply the health bonus and resync.
    // Spending is derived, so afterwards SpentPoints exceeds TotalPoints and
    // AvailablePoints reads zero, which is the honest figure.
    bool UnlockAll();

    // Fills 'mask' (k_SkillMaskBytes long) with one bit per unlocked Skill.
    void BuildUnlockedMask(unsigned char* mask) const;

    // The inverse.  Kept beside BuildUnlockedMask, and both go through the
    // SkillMaskGet/SkillMaskSet pair in skill_defs.h, so the packing can
    // only ever be defined once.
    //
    // Inline because the client needs it and player_skills.cpp is not in
    // the client project: this is how the predicted player learns which
    // Skills are held (see HUD_SetPredictedSkills).  Points and Reset
    // Tokens are deliberately not carried -- nothing the client predicts
    // reads them.
    void ApplyUnlockedMask(const unsigned char* mask)
    {
        for (int i = 1; i < k_MaxSkills; ++i)
            m_bUnlocked[i] = SkillMaskGet(mask, i);
        HoldSuit();
    }
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

// =====================================================================
// Weapon Mastery and the Weapon Specialist's typed damage: scales damage
// a PLAYER is dealing.
//
// Applied at the two chokepoints every player weapon funnels through --
// ApplyMultiDamage, and the direct-TakeDamage branch of RadiusDamage --
// rather than in each weapon, so "every weapon you carry" is true by
// construction instead of by a list somebody has to maintain.  The typed
// Skills (Marksman and the Bullet Damage Stat nodes on DMG_BULLET) test
// bitsDamageType here for the same reason: no weapon list.
//
// Returns flDamage unchanged when the attacker is not a player, or holds
// none of the Skills, so monsters are unaffected.
// =====================================================================
struct entvars_s;
float SkillScaleWeaponDamage(struct entvars_s* pevAttacker, float flDamage, int bitsDamageType);

// =====================================================================
// Headhunter: the multiplier on a PLAYER's hit to the head hitgroup, on
// top of gSkillData.monHead, read where that multiplier is applied
// (CBaseMonster::TraceAttack).  1.0 for a non-player or without the
// Skill.  Decapitation, when built, keys on the same hitgroup.
// =====================================================================
float SkillHeadshotScale(struct entvars_s* pevAttacker);

// =====================================================================
// The Healing Stat nodes: one multiplier on what an Infusion tick and a
// medkit heal, 1 + count * skill_stat_healing.  Read where each heal
// is computed (player_infusion.cpp, healthkit.cpp, UseMedkit), never
// anywhere passive.
// =====================================================================
float PlayerHealingScale(CBasePlayer* pPlayer);

// =====================================================================
// The standing multipliers: what always applies, never what depends on a
// moment (Swap Surge, Overdraw, Backstab, Follow-Up, Cleave).  Each is read
// both where its effect is applied and by SendSkillStatsToClient, so the
// Status page shows the number the game uses.
// =====================================================================

// Weapon Mastery and the typed damage Skills and Stat nodes, for a hit of
// 'bitsDamageType'.  SkillScaleWeaponDamage is this times the situational.
float PlayerStandingDamageScale(CBasePlayer* pPlayer, int bitsDamageType);
// Melee Force and the Melee Damage Stat nodes, applied in CCrowbar::SwingDamage.
float PlayerMeleeScale(CBasePlayer* pPlayer);
// Armor Expert: the scale on ARMOR_RATIO, the fraction that gets past armour.
float PlayerArmorRatioScale(CBasePlayer* pPlayer);
// Damage taken: Demolitions on blasts, Insulation on energy and shock,
// Sure Footing on falls.  1.0 without the Skill.
float PlayerBlastTakenScale(CBasePlayer* pPlayer);
float PlayerEnergyTakenScale(CBasePlayer* pPlayer);
float PlayerFallTakenScale(CBasePlayer* pPlayer);

// Sends the Status page's numbers (docs/STATUS_PANEL.md).  Called at the
// end of SendSkillTreeToClient, since every change that moves a number is
// already a skill-tree sync.
void SendSkillStatsToClient(CBasePlayer* pPlayer);
// The medkit's heal for this player: gSkillData.healthkitCapacity scaled.
// Both places a medkit heals ask this, so the "wastes nothing" test and
// the heal agree.
float PlayerMedkitHeal(CBasePlayer* pPlayer);
