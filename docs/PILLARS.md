# Gameplay Pillars — Progress

The five things this mod is actually about. Everything inherited from the base SDK is scaffolding; this
document tracks the custom gameplay on top of it.

This is a living document. When a pillar's state changes, update its section and the summary table in the
same commit as the code change.

**Last updated:** 2026-08-01 (branch `hl-shock`, at `ecd76a5`)

## Status legend

| Label | Meaning |
| --- | --- |
| **Not started** | No code exists. |
| **Scaffolded** | Data structures, networking, or UI exist, but the player cannot feel it in-game. |
| **Playable** | The player can use it in a normal play session, rough edges accepted. |
| **Done** | Behaviour is final and covered by the acceptance criteria in this document. |

## Summary

| # | Pillar | Status | One-line state |
| --- | --- | --- | --- |
| 1 | [Exploration](#1-exploration) | **Not started** | No code. |
| 2 | [Enhanced combat](#2-enhanced-combat) | **Not started** | Exists only as skill descriptions; no weapon or damage code changed. |
| 3 | [Custom items](#3-custom-items) | **Scaffolded** | Two stock consumables usable from the inventory; no custom item framework. |
| 4 | [Skill trees](#4-skill-trees) | **Scaffolded** | Full tree unlocks, saves, and renders — but no unlocked skill changes gameplay. |
| 5 | [Inventory management](#5-inventory-management) | **Playable** | Grid, drag-drop, and context actions work. Client-side model only — the server-owned rebuild is designed and scheduled. |

---

## 1. Exploration

**Status: Not started**

### What exists

Nothing. No custom code touches level traversal, discovery, secrets, map flow, or navigation aids.

### What's missing

The pillar has no definition yet, let alone an implementation. Open questions that need answering before any
code is worth writing:

- Is exploration rewarded through **items** (pillar 3), **skill points** (pillar 4), or both?
- Does it need new entities (discoverable caches, lore pickups, optional-area triggers), new map work, or
  only tuning of existing HL maps?
- Is there any navigation/HUD affordance (compass, objective marker, map notes), or is it purely level design?

### Next step

The reward loop has its first concrete answer: **Row Grants**. Inventory capacity grows from things found
in the world rather than from Skills (see pillar 5), so a hidden cache off the critical path permanently
increases what the player can carry. That is exploration's first real mechanic, and it arrives as part of
inventory iteration 3.

Skill points remain unearnable — `m_iSkillPoints` is still hardcoded to 20 for UI testing. Whether
exploration should *also* award them is open.

### Acceptance criteria (draft)

- A player who explores off the critical path is measurably better off than one who does not.
- The reward is visible in the UI at the moment it is earned.

---

## 2. Enhanced combat

**Status: Not started**

### What exists

Only *descriptions*. `dlls/player_skills.cpp` defines combat skills — Crowbar Reach, Crowbar Force, Fast
Reload, Weapon Mastery, Crowbar Speed, Crowbar Parry — but no weapon, damage, or melee code has been
modified anywhere in `dlls/`.

### What's missing

Everything behind those strings. In particular:

- Melee reach/damage/speed multipliers in `dlls/weapons.cpp` and the crowbar implementation.
- A parry mechanic — `CrowbarParry` names a "wider parry timing window" for a system that does not exist.
- Reload-time and global damage modifiers.

### Next step

Pick one skill and wire it end to end as the pattern the rest follow. `CrowbarDamage` is the cheapest —
it's a single damage multiplier, server-side only, and it makes the skill tree stop being decorative.
Establish where the modifier is read (a `CBasePlayer` helper that consults `m_skills`) so later skills
don't each invent their own hook.

### Acceptance criteria (draft)

- Every combat skill in the tree has a measurable in-game effect.
- Effects are computed server-side; the client never decides damage.
- No skill effect is applied twice (prediction and server both).

---

## 3. Custom items

**Status: Scaffolded**

### What exists

- A client-side item template table in `cl_dll/vgui_inventory.cpp:378` with four entries: **Medkit**,
  **Antidote**, **Keycard**, **Battery**. All four are stock Half-Life entities.
- `gmsgInventoryItem` (`dlls/UserMessages.cpp:83`) pushes a `(itemId, count)` pair to the client on pickup,
  sent from `dlls/items.cpp` (battery) and `dlls/healthkit.cpp`.
- Server-side `inv_use` handling in `dlls/client.cpp:566` for exactly two classnames: `item_healthkit`
  (heals, decrements, plays a sound) and `item_battery` (charges armour, HEV voice line). Both re-sync the
  count to the client afterwards.
- Item sprites resolve through the HUD sprite set; only healthkit and battery have one, so Antidote and
  Keycard render without icons.

### What's missing

- Any item that isn't already in stock Half-Life. There is no new entity, no new pickup, no new effect.
- A framework: item ids are implicit array indices in a *client-side* table, and the server's notion of an
  item is `m_rgItems[]` plus a hardcoded `if/else` chain in `client.cpp`. Adding a fifth item today means
  editing both sides in a way nothing enforces.
- Drop is wired in the UI (`cl_dll/vgui_inventory.cpp:248`) and sends `drop <classname>`, but no server
  handler exists for dropping non-weapon items.

### Next step

Decided as part of the inventory design: **one Item Type table under `game_shared/`, compiled into both
DLLs**, so the client and server cannot disagree. It lands in inventory iteration 1, and the first
genuinely custom item becomes one table entry plus its effect. Weapons stay on Half-Life's own identity —
see [ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md).

### Acceptance criteria (draft)

- Adding an item means adding one table entry plus its effect, and nothing else.
- Item ids are stable across saves and the network, like skill ids.
- Every item in the inventory can be used or dropped, or is explicitly marked as neither.

---

## 4. Skill trees

**Status: Scaffolded**

The most complete system by line count, and the one furthest from affecting play.

### What exists

**Server** — `dlls/player_skills.cpp` / `dlls/player_skills.h`

- 15 skill definitions across Combat, Mobility, Survivability, plus four test-branch nodes added to validate
  connector rendering.
- Each `SkillDef` carries id, display name, description, grid column/row, cost, prerequisite, and a visual
  tier (`Minor` / `Medium` / `Major`).
- `TryUnlock()` validates prerequisite, cost, and duplicate unlock — server-authoritative.
- State lives in `CBasePlayer::m_skills` (`dlls/player.h:362`) and saves/restores through a
  `TYPEDESCRIPTION` table (`dlls/player.cpp:3057`, `:3079`).

**Networking**

- `gmsgSkillTree`, variable length: one byte count, then 6 bytes per node, then the player's skill points.
  Unlocked / available / tier are packed into a single flags byte (tier in bits 2–3).
- Client → server is the `skill_unlock <id>` console command (`dlls/client.cpp:647`); on success the server
  re-sends the whole tree.
- Client handler: `CHudAmmo::MsgFunc_SkillTree` in `cl_dll/ammo.cpp:559`.

**Client** — `cl_dll/vgui_skilltree.cpp` / `.h`

- `CSkillTreeView`, a plain C++ helper owned by `CInventoryPanel` rather than a VGUI panel of its own.
- Tier-sized nodes, lazily loaded HUD sprite icons, edge-anchored connector lines that prefer vertical
  routing, hover tooltips, and a skill-point counter.
- Labels and descriptions come from a **client-local** metadata table, not from the wire — so adding a skill
  requires editing the server `SkillDef` array *and* the client table.

### What's missing

- **The effects.** `CPlayerSkills::HasSkill()` has no callers outside `player_skills.cpp`. Every skill in the
  tree unlocks, persists, and renders, and none of them do anything.
- **A way to earn points.** `m_iSkillPoints` defaults to 20 for UI testing (`dlls/player_skills.h:58`).
- **A bug:** `SprintSpeed` lists itself as its own prerequisite (`dlls/player_skills.cpp:22`), so
  `PrereqMet()` requires the skill to already be unlocked and the node can never become available.
- Tooltip layout debt — heuristic text measurement rather than font metrics. See
  [TECH_DEBT.md](TECH_DEBT.md).

### Next step

Fix the `SprintSpeed` prerequisite, then wire the first effect (see pillar 2). Movement skills — `HighJump`,
`SprintSpeed` — need `pm_shared/` and therefore touch both DLLs; start with a server-only effect instead.

Note that inventory capacity was considered as the first Skill effect and deliberately moved to exploration
instead (pillar 5). Nothing in the inventory work will give this pillar an effect, so it stays Scaffolded
until a combat skill is wired. The Row Grant counter is source-agnostic, so a capacity Skill can still
grant Rows later without rework.

### Acceptance criteria (draft)

- Every unlocked skill has an observable effect.
- Skill points are earned through play, not seeded.
- Skill ids stay stable; save games from before a skill was added still load.

---

## 5. Inventory management

**Status: Playable**

### What exists

**`CInventoryPanel`** (`cl_dll/vgui_inventory.cpp`) — a VGUI panel with two tabs, Inventory and Upgrades,
each delegating to a plain helper view. Opened via the `+inventory` command bound in `cl_dll/input.cpp:990`.

**`CInventoryGridView`** (`cl_dll/vgui_inventory_grid.cpp`)

- Renders weapons, inventory items, and ammo in one grid with mixed cell widths (weapons occupy three cells).
- Drag and drop with live clamping during the drag and normalization after the drop, so slots cannot overlap
  or overflow the grid.
- Per-slot pixel offsets persist across opens, owned by `CInventoryPanel` so context-menu actions can read
  them.
- Hit rectangles rebuilt every paint.

**Context menu** — right-click gives Use and Drop. Weapons issue `use <classname>` and close the panel;
items issue `inv_use <classname>` and stay open.

**HUD integration** — while the panel is visible, `CHudAmmo::Draw` returns early (`cl_dll/ammo.cpp:923`) so
the default weapon sprites don't bleed through the panel.

### What's missing

Iterations 1 and 2 are **done**, and dropping is complete — the model is server-owned, saved and
capacity-limited, things enter the Inventory deliberately, and anything in it can be dropped back out.
Remaining:

- **No Row Grant pickup**, so Rows can only be earned via `inv_addrows`. Needs custom maps before it can
  be placed at all.
- **No Boxes**, so nothing in the world holds items — lootable caches are still future work.
- No custom Item Types yet — the table holds the four stock ones.
- The ammo readout overflows its panel when the player carries many ammo types; it needs a taller panel,
  a scroll, or two columns. Deferred to a UI pass.
- The Pickup Prompt is unstyled — it uses the engine console font pending a visual style for the mod.
- **Untested:** dropping an exhaustible weapon (satchel, tripmine, snark, hand grenade). Their ammo is the
  item itself and lives in the pool rather than a clip, so the clip-transfer on drop does nothing for them.
  Nothing should be lost, but it is the one weapon class whose drop path has not been exercised.
- `inv_rows_max` has been eyeballed at 9 against vanilla content only. Re-judge it once there are more
  weapons and items than vanilla has.

### Deliberately deferred

Designed, agreed, and **not** in the first server-side version. Recorded so they aren't rediscovered as
bugs:

- **Boxes, and everything that needs them.** Dropping a Stack was going to need a `CWeaponBox` extended
  with `(itemTypeId, count)` arrays. It does not: "Drop all" spawns one world entity per item with a
  small scatter, and a Stack is at most a handful. Boxes are now purely a future feature — lootable
  caches and the two-panel loot window that goes with them.
- **Container UI.** Taking things out of a box would be "take whatever fits, leave the rest", driven by
  the same look-and-use prompt as any other pickup, with the loot window as a later presentation change
  over the same `TryAdd` logic. Nothing depends on it today.
- **Splitting and merging Stacks by hand.** Pickups merge automatically; there is no manual split. A
  player wanting to drop one medkit out of five must drop all five. Purely additive to fix.
- **Auto-sort / re-pack button.** Deliberately absent. Nothing may re-arrange the Grid behind the
  player's back — that was the whole point of making placement explicit.

Two hazards in `CWeaponBox` that must be fixed *before* anything puts items in a box, or the player's
belongings get deleted:

- `IsEmpty()` (`dlls/weapons.cpp:1357`) only checks weapons and ammo. A box holding only items would
  report empty and be removed.
- `Touch` clears each ammo slot unconditionally after `GiveAmmo` (`dlls/weapons.cpp:1184`), destroying
  ammo that didn't fit in the player's pool.
- Boxes created by the multiplayer death-drop path get `SetThink(&CWeaponBox::Kill)` (`dlls/player.cpp:698`)
  and self-destruct on a timer. A box holding deliberately dropped belongings must never do that.

### Agreed design

Settled 2026-08-01. Vocabulary is in [CONTEXT.md](../CONTEXT.md); the decisions with lasting consequences
have their own records in [adr/](adr/).

**Shape.** The Inventory is a container that owns its contents, not a view over `m_rgItems[]`. An Entry is
`(what, count, col, row)` and carries no state beyond that — there is no half-used medkit. Item Types
declare a maximum Stack size; unique things are simply those with a maximum of one, and picking up past the
maximum creates a second Entry.

**What competes for space.** Weapons (3 Cells) and items (1 Cell). Ammo does not — see
[ADR-0001](adr/0001-ammo-is-not-in-the-inventory.md). Ammo moves to the Inventory Panel's left column.

**Grid.** 12 Cells wide, fixed; **5 Rows to start and 4 more to earn, 9 maximum** (60 to 108 Cells), all
drawn at all times with un-granted Rows greyed out — see
[ADR-0003](adr/0003-fixed-grid-width-rows-only-growth.md). Rows are granted by things found in the world,
not by Skills, which makes Inventory growth a reward for exploration. Because Rows are only ever granted,
the Grid never shrinks and no eviction rules are needed.

The base Grid is deliberately generous: this mod adds more weapons and items than vanilla Half-Life, whose
full 15-weapon arsenal would occupy 45 of the 60 starting Cells on its own.

`inv_rows_start` and `inv_rows_max` are tuning cvars; granted Rows are saved player state, clamped only at
grant time and never re-clamped on load, so lowering a cvar can never shrink a Grid under a player.
`inv_addrows <n>` (cheat-gated) exists to tune the maximum by eye.

**Ownership.** The server owns contents *and* placement; the client renders and requests — see
[ADR-0004](adr/0004-the-server-owns-the-inventory.md). Placement on pickup is first-fit, scanning
left-to-right, top-to-bottom.

**Item Types** are defined once, in a table under `game_shared/` compiled into both DLLs. Weapons keep
Half-Life's `WeaponId` — see [ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md).

**Verbs.** Move, Use, Drop. Nothing else. Dropping a single item spawns its world entity; dropping a
Stack offers "Drop 1" or "Drop all", the latter spawning one entity per item with a small scatter.
Weapons drop through Half-Life's own `DropPlayerItem`, which spawns the real weapon model.

A dropped weapon keeps its **loaded clip** but grants no reserve ammo, so dropping and retaking is exactly
lossless and never profitable. **Reserve ammo stays with the player** — it is a pool, not a property of the
weapon (ADR-0001), so you keep shells for a shotgun you no longer own. Vanilla differs here: `CWeaponBox`
packs half your reserve into the box. Ours is the deliberate choice, on the grounds that reserve ammo costs
no Cells and so is harmless to carry.

**Acquisition.** A weapon walked over is taken automatically if there is room, and otherwise left where it
is. A medkit walked over is consumed on the spot when doing so wastes none of its healing — the test is
`health + heal <= maxHealth`, so a perfect fit is consumed rather than left. Everything else is taken by
looking at it and pressing use.

**Persistence.** Save games and level transitions, via `CBasePlayer`'s save table. Death is a full reload
from the last save, so there is no respawn-inventory case to design. Things left behind stay in the map
they were left in; maps are designed with backtracking in mind, so this is a consequence to build levels
around rather than a problem to solve.

### Planned iterations

1. ~~**The model, server-side.**~~ **Done 2026-08-01.** `game_shared/inventory_defs.h` holds the Item
   Type table; `CPlayerInventory` (`dlls/player_inventory.cpp`) holds Entries in the player's save table;
   placement is first-fit with Stack top-up; `gmsgInventory` syncs in chunks; the client renders and
   requests. `NormalizeGridLayout`, the offset vectors, and `gWR.riGridCell` are gone.
2. ~~**The pickup interaction.**~~ **Done 2026-08-01.** `FindLookedAtPickup` answers "what would a use
   press take?" once, and both the Pickup Prompt and the take use that same answer, so they cannot
   disagree. It deliberately also weighs ordinary usable entities, so pressing use at a button never
   grabs a medkit behind it. Items are use-only now; medkits Auto-Consume on contact when nothing is
   wasted; weapons keep walk-over pickup. Ammo readout moved to the left column in iteration 1.
   *Done when a map plays start-to-finish without opening the panel and it feels normal.*
3. ~~**Containers and the world.**~~ **Postponed 2026-08-01.** Dropping was completed without needing
   containers: weapons became droppable, and a Stack offers "Drop 1" and "Drop all", spawning one world
   entity per item. What remains under this heading is genuinely future work — lootable Boxes, the loot
   window, and the `item_inventory_upgrade` pickup that grants Rows. Until that entity exists Rows come
   only from `inv_addrows`, and it cannot be exercised in-game before there are custom maps to place one
   in.

Iteration 1 is the only one that is hard to reverse.

### Acceptance criteria

- No overlap or overflow across all Entry widths.
- Repeated drag/drop sequences produce stable, predictable placements, and nothing moves that the player
  did not move.
- Inventory contents *and* layout survive save/load and level transitions.
- Lowering an `inv_rows_*` cvar never destroys or displaces anything a player is carrying.

---

## Cross-cutting cleanup

Not pillars, but they affect all of them:

- Debug `ALERT(at_console, ...)` calls left in `dlls/items.cpp:229`, `dlls/healthkit.cpp:78`, and
  `dlls/UserMessages.cpp:27` log to console on every pickup and on message registration.
- Custom systems have no tests and no debug visualization; both TECH_DEBT entries ask for a debug overlay.
