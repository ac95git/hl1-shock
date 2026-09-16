#pragma once

#include <VGUI_Font.h>

#include "module_defs.h"

class CInventoryPanel;

// =====================================================================
// SkillStatsView
//   The client's copy of the build's final numbers, exactly as the
//   server computed them (SendSkillStatsToClient).  The client never
//   derives one: the per-node amounts are server cvars it cannot read,
//   and a second copy of the formulas would drift from the game's.
// =====================================================================
struct SkillStatsView
{
    bool  valid        = false; // false until the first message arrives
    int   maxHealth    = 100;
    int   maxArmor     = 100;
    float healing      = 1.0f;  // multiplier
    float armorEff     = 0.8f;  // share of a hit armor takes
    float blastResist  = 0.0f;  // share resisted
    float energyResist = 0.0f;
    float fallResist   = 0.0f;
    float melee        = 1.0f;  // multipliers
    float bullet       = 1.0f;
    float energy       = 1.0f;
    float explosive    = 1.0f;
    float dashRecharge = 0.0f;  // seconds per Charge
};

// =====================================================================
// CStatusView
//   Plain C++ helper, not a VGUI panel -- the Status tab's view, owned
//   by CInventoryPanel like the Grid and the Skill Tree.  The design is
//   docs/STATUS_PANEL.md: the Modules as fixed Slots on a doll of the
//   suit on the left, the build's final numbers on the right.
// =====================================================================
class CStatusView
{
public:
    void UpdateStats(const SkillStatsView& stats) { m_stats = stats; }

    // One bit per EGate value, from gmsgSkillTree.  A Slot is filled
    // when its Module's gate is open.
    void SetOpenGates(unsigned char openGates) { m_openGates = openGates; }

    void Paint(CInventoryPanel* ctx,
               int x0, int y0, int areaW, int areaH,
               vgui::Font* smallFont, vgui::Font* titleFont);

    // Hover tracking for the Slot tooltip; (-1, -1) clears it.
    void HandleMouseMove(int localX, int localY) { m_iHoverX = localX; m_iHoverY = localY; }

private:
    struct IRect { int x, y, w, h; };
    struct ModuleSprite { HSPRITE hSprite = 0; Rect rc = {}; };

    bool IsGateOpen(EGate gate) const { return (m_openGates & (1 << (int)gate)) != 0; }

    // Lazily load the stand-in glyphs (no-op once loaded).
    void EnsureSprites();

    SkillStatsView m_stats;
    unsigned char  m_openGates = 0;
    int            m_iHoverX = -1;
    int            m_iHoverY = -1;

    IRect        m_slotRects[(int)ESlot::_Count] = {};
    ModuleSprite m_sprites[k_NumModuleDefs] = {};

    // The frames are drawn boxes until the suit silhouette exists
    // (docs/ART_DEBT.md, "The Status page").
    static constexpr int k_SlotSize = 72;
    static constexpr int k_SlotGap  = 14;
    static constexpr int k_IconPad  = 10;

    // Concealment and Hornet replenish have no effect in code yet, so their
    // lines stay hidden even with the gate open (docs/STATUS_PANEL.md).
    // Flip each when its stat is built, and send its number.
    static constexpr bool k_ConcealmentBuilt    = false;
    static constexpr bool k_HornetReplenishBuilt = false;
};
