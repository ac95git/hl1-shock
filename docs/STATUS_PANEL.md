# The Status page — the suit's Modules and the build's numbers

The Inventory Panel's third tab, settled in a grill on 2026-09-16. It answers two questions the other tabs
do not: **what can I do** (the Modules found, as Slots on the suit) and **how strong am I now** (the final
numbers of the build). The Skill Tree already records what was bought; this page is the result of it. **Built
the same day** as `CStatusView` (`cl_dll/vgui_status.cpp`), a third view beside `CInventoryGridView` and
`CSkillTreeView`; what exists is recorded in [PILLARS pillar 5](PILLARS.md#5-inventory-management). The
look follows the panel's chrome in [SKILL_PANEL.md](SKILL_PANEL.md).

**Where the build differs from the text below:** multipliers print as ASCII `x1.73`, not `×`, because the
scheme fonts print a byte at a time; the left arm is on the screen's left; Night Vision's tooltip names the
flashlight's key (`impulse 100`), which it replaces.

## The panel

| Element | Settled |
| --- | --- |
| **Tab** | `Status`, a third nav button under `Inventory` and `Upgrades` in the left column. The column's top panel already fits it at 76 px a button; the ammo readout below is unchanged |
| **Header** | Unchanged: `HEV MK IV  //  <CODENAME>`, the same on every tab |
| **Theme** | **Chrome only.** The field is the dark panel in the suit colour; Slot frames and block headers are drawn like the Skill page's gauge strip — small-font caps labels, boxed, suit-coloured; numbers plain text. No circuit substrate, no traces |
| **Layout** | Two columns in the view area: **the doll on the left, the stats column on the right**. Everything fits one screen; no scroll view |

## The doll — Modules as Slots

| Element | Settled |
| --- | --- |
| **Slots** | Five, **fixed**: a Module always sits in its own Slot and nothing is dragged. A Slot is where a Module lives on the suit, not a loadout |
| **Assignment** | **Head** Night Vision · **Body** Dash · **Left arm** Pulse · **Right arm** the Alien Module · **Legs** ~~undecided~~ the double jump Module (settled 2026-09-17, not built) |
| **Frames** | Five boxed frames arranged as a body — head above, arms either side of the body, legs below — drawn with rects. A suit silhouette sprite behind them is the target ([ART_DEBT.md](ART_DEBT.md#the-status-page--suit-silhouette-and-module-glyphs)) |
| **Filled Slot** | The Module's **monochrome glyph, tinted the suit colour**, one motif per Module matching its Route's glyph on the tree: eye (Night Vision), wind (Dash), shield (Pulse), hive (Alien). Existing HUD sprites stand in until the glyphs exist |
| **Empty Slot** | A **dim, unlabelled frame**. No name, no icon, no hint — the player sees that a Slot is empty, never what fills it |
| **Tooltip, filled** | On hover: **name, bound key, one-sentence description**. The key is looked up live with `gEngfuncs.Key_LookupBinding` (as `cl_dll/input.cpp` already does), so it never goes stale when the player rebinds. The name and description come from a new shared Module table beside `EGate` in `game_shared/`, the way a Skill's come from `k_SkillDefs`. Styled as the Skill Tree's tooltip |
| **Tooltip, empty** | **None.** Hovering an empty Slot does nothing |
| **When a Slot fills** | When its Module's gate opens (`openGates` on `gmsgSkillTree`, which the client already has). ~~The Pulse's gate is open while the Pulse is suit hardware, so the left arm is filled whenever the panel can open~~ Since 2026-09-16 the Pulse is a found Module (`item_pulsemodule`, [ADR-0013](adr/0013-the-pulse-is-a-found-module.md)) and the left arm fills when it is found, with no change here |

**No live state.** Dash Charges and the Pulse's Recharge are on the HUD already (`CHudDash`, the readiness
bar); the tooltip says what a Module does and how to use it, not how ready it is.

## The stats column

**What earns a line:** a stat the tree increments **often** — every road stat (`EStat`) — plus the
protection stats kept by decision. **Situational bonuses never get a line** (Backstab, Headhunter,
Follow-Up, Ambush, Swap Surge, Cleave), nor one-off stats with no road (reload, draw and swing time,
reach). **Nor are they folded into a number:** Overdraw's energy bonus holds only while armor is above
its floor, so the Energy damage line leaves it out, as it leaves out Swap Surge (decided 2026-09-16).

**What a number is:** the **real final value**, everything always-on folded in. Fortitude's +25 is inside
Health's maximum and Weapon Mastery inside every damage line, even though neither has a line of its own.

| Block | Line | Form | Example |
| --- | --- | --- | --- |
| **HEALTH** | Health | current / max | `Health 87 / 131` |
| | Healing | × multiplier | `Healing ×1.20` |
| **ARMOR** | Armor | current / max | `Armor 40 / 157` |
| | Armor efficiency | percent | `Armor efficiency 72%` |
| | Explosive resistance | percent resisted, 0% untouched | `Explosive resistance 50%` |
| | Energy resistance | percent resisted, energy and shock (Insulation), 0% untouched | `Energy resistance 30%` |
| | Fall resistance | percent resisted, 0% untouched | `Fall resistance 50%` |
| **DAMAGE** | Melee damage | × multiplier | `Melee damage ×1.73` |
| | Bullet damage | × multiplier | `Bullet damage ×1.27` |
| | Energy damage | × multiplier | `Energy damage ×1.10` |
| | Explosive damage | × multiplier | `Explosive damage ×1.25` |
| **MODULES** | Concealment | percent slower to be learned about, 0% untouched | `Concealment 15%` |
| | Dash recharge | seconds per Charge | `Dash recharge 5.6 s` |
| | Hornet replenish | × multiplier | `Hornet replenish ×1.50` |

- **One line per stat**, blocks stacked top to bottom in the order above. The examples are illustrative.
- **A multiplier nothing has touched reads ×1.00**, so an uninvested stat is visible as such.
- **Bigger is better wherever the label names a virtue.** That is why the resistances and Concealment are
  percents rather than the ×0.50 / ×0.85 the code multiplies by.
- **The MODULES block** holds the stats whose road sits behind a Module, in the doll's order — head, body,
  right arm. **Each line is hidden until its Module is found, and the block with it until the first one
  is.** The tree hides those regions; this page does not name them early. The Pulse has no road stat, so
  it adds no line. **Concealment and Hornet replenish went live 2026-09-16**, when the Stealth region and
  the Hive nodes gave those two stats an effect (`k_ConcealmentBuilt`, `k_HornetReplenishBuilt`); until
  then each stayed hidden behind its own check even with its gate open, so `skill_open_gates` could never
  show a number that did nothing.

## Where the numbers come from

**The server computes, a new message carries, the client draws.** The per-node amounts are server cvars
(`skill_stat_*` and the Skill cvars in `dlls/game.cpp`) the client cannot read, and the maxima are
server-side (`PlayerMaxArmor`). The message holds every maximum and multiplier in the table, computed by
the same functions the damage and pickup code use, so the page cannot disagree with the game. It is sent
on spawn and restore, and whenever a value can change: a Skill unlocked, a Reset, a Module found. Current
Health and Armor still come from the stock HUD messages. A copy of the formulas on the client was
rejected: two copies drift, and this page's whole promise is the real value (the reasoning of
[ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md), applied the other way round — shared
static data, server-owned derived data).

## Open

- ~~**The leg Slot** has no Module.~~ **Decided 2026-09-17: the double jump**, the sixth Module, found
  later in Xen ([ROADMAP.md](ROADMAP.md#pillar-3-modules)). Until it is built the Slot draws as the empty
  frame it is today; when it is, it wants a row in `module_defs.h`, a glyph, and a tooltip naming the jump
  key.
- **The Hook** is outside this design, and gets its Slot, or none, when it is designed. Low priority since
  2026-09-17, and with the legs taken all five Slots are spoken for — a sixth Slot, or none, is the
  question it will have to answer.
- ~~**The Alien Module's key.**~~ **Resolved 2026-09-16**: it is `weapon_summon`, an ordinary weapon, so its
  tooltip names whatever is bound to primary attack, the same as any other weapon's Slot would.

## Rejected in the grill

A hover breakdown of where each number comes from; lines for situational buffs; timing stats outside the
roads; Module-gated lines shown dimmed or at ×1.00 before the Module; a minimal icon-and-name Module entry,
and one with live readiness; one column, or Modules as a card row on top; **swappable Slots — rejected for
now**, since a swappable Module is one level design can never assume (ROADMAP pillar 3); the Hook in the
leg Slot or an arm; always-visible callouts and a click-to-select detail box; a figure-less doll as the
final art; the circuit theme on this page; full-colour Module renders; text-only Slots; a `No signal`
tooltip on an empty Slot; ×-multipliers for resistances and Concealment; one block per Module stat;
computing the numbers on the client.

## What it costs to build

- `EInventoryTab::Status`, a third nav button, and a `CStatusView` helper beside the other two.
- The shared Module table: per `EGate`, a name, a description, a glyph sprite name, a Slot, and the
  command whose binding the tooltip looks up.
- The stats message: one server function that fills every value from the functions already applying them,
  sent at the points above; a client handler that stores them for the view.
- The doll's frames and tooltip hit-testing, and the stats column's blocks with the Module-gated lines.
- Art, in ART_DEBT: the suit silhouette and four Module glyphs.
