# Gameplay Pillars — Progress

The five things this mod is actually about. Everything inherited from the base SDK is scaffolding; this
document tracks the custom gameplay on top of it.

This is a living document. When a pillar's state changes, update its section and the summary table in the
same commit as the code change.

**Last updated:** 2026-08-01 (branch `hl-shock`, including uncommitted working-tree changes)

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
| 5 | [Inventory management](#5-inventory-management) | **Playable** | Grid, drag-drop, and context actions work. Client-side model only. |

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

Decide the reward loop first. Exploration is the natural source of skill points — right now `m_iSkillPoints`
is hardcoded to 20 for UI testing and there is no way to earn one. Wiring "discover an optional area →
gain a skill point" would give pillars 1 and 4 a shared spine.

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

Before adding items, give them a shared definition — one table both DLLs agree on, keyed by a stable id, in
the spirit of `k_SkillDefs`. Then the first genuinely custom item is a small addition rather than a fifth
special case.

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

- **There is no server-side inventory.** The client mirrors `m_rgItems[]` counts and weapon state; layout,
  ordering, and slot positions exist only on the client and are not saved with the game.
- No capacity, weight, or stack limits — nothing that makes "management" a decision.
- Drop is wired for items in the UI but unhandled on the server (see pillar 3).
- Placement solver debt: repeated manual drags can produce surprising placements. See
  [TECH_DEBT.md](TECH_DEBT.md).

### Next step

Decide whether inventory layout is *presentation* (fine as-is, client-only) or *state* (needs to persist
through save/load and belongs on the server). That answer determines everything else in this pillar. If
management is meant to be a real constraint, capacity limits come next.

### Acceptance criteria (draft)

- No overlap or overflow across all slot types and widths.
- Repeated drag/drop sequences produce stable, predictable placements.
- Inventory contents survive save/load and level transitions.

---

## Cross-cutting cleanup

Not pillars, but they affect all of them:

- Debug `ALERT(at_console, ...)` calls left in `dlls/items.cpp:229`, `dlls/healthkit.cpp:78`, and
  `dlls/UserMessages.cpp:27` log to console on every pickup and on message registration.
- Custom systems have no tests and no debug visualization; both TECH_DEBT entries ask for a debug overlay.
- ~1,080 lines of skill-tree and inventory work are currently uncommitted on `hl-shock`.
