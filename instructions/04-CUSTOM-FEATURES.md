# Custom Features

This project includes custom gameplay systems such as the player skill tree and the Inventory. This guide
explains how to extend them safely.

Terms used here — Entry, Stack, Item Type, Cell, Row — are defined in [CONTEXT.md](../CONTEXT.md).
Decisions that look arbitrary are explained in [docs/adr/](../docs/adr/).

## Adding an Item Type

Item Types are defined **once**, in [game_shared/inventory_defs.h](../game_shared/inventory_defs.h),
compiled into both DLLs so the client and server cannot disagree. Adding one is a single table entry plus
its behaviour.

1. Add an id to `EItemTypeId`, before `_Count`. **Ids are frozen once written to a save** — adding is
   free, reordering or reusing corrupts existing saves.
2. Add the matching row to `k_ItemTypes[]`:
   - `classname` is the world entity spawned when the item is dropped; `nullptr` means it cannot be dropped
   - `displayName` is shown in the Inventory Panel and the Pickup Prompt
   - `spriteName` comes from the HUD sprite set in `sprites/hud.txt`; `nullptr` renders without an icon
   - `cellWidth` is how many Cells it occupies
   - `maxStack` is the Stack ceiling — `1` means unique
   - `usable` is whether the context menu offers **Use**. Set it and add the `case` below together, or the
     item ships with a verb that does nothing, or an effect the player cannot reach
3. If it does something when used, add a case to `InventoryUseEntry` in
   [dlls/player_inventory.cpp](../dlls/player_inventory.cpp). Use behaviour is server-side only; the
   client never decides what using something does.
4. If it is picked up in the world, give it a `CItem` subclass whose `MyTouch` calls `InventoryGiveItem`,
   and override `AutoPickupOnTouch()` to return `false` so it is taken with a use press rather than by
   walking over it. `MyTouch` must return `false` when `InventoryGiveItem` accepts nothing, or a pickup
   vanishes into a full Grid.
5. Add a `@PointClass` line to [fgd/halflife.fgd](../fgd/halflife.fgd) beside the other `item_*` entries,
   or the entity cannot be placed in a level editor. Easy to forget — nothing fails without it until
   someone tries to build a map.

**The `m_rgItems[]` counters stop at `MAX_ITEMS` (5).** Any Item Type with an id at or past that has no
legacy counter, and `SyncLegacyItemCount` skips it — correctly, since the Inventory is the real record.
Do not widen `MAX_ITEMS` to "fix" this; it is a save-format array from the pre-Inventory design.

The **Health Syringe** is the worked example of all of the above: `EItemTypeId::Syringe` and its row in
`inventory_defs.h`, `CItemSyringe` in [dlls/items.cpp](../dlls/items.cpp), `UseSyringe` and its `case` in
[dlls/player_inventory.cpp](../dlls/player_inventory.cpp), the effect itself in
[dlls/player_infusion.cpp](../dlls/player_infusion.cpp), and one FGD line.

Weapons are **not** Item Types — they keep Half-Life's own `WeaponId`. See
[ADR-0002](../docs/adr/0002-two-identity-spaces-for-weapons-and-items.md).

## Adding a status icon

`CHudStatusIcons` ([cl_dll/status_icons.cpp](../cl_dll/status_icons.cpp)) draws on/off icons up the left
edge of the screen from mid-height. Send `gmsgStatusIcon` — `BYTE enable`, `STRING` sprite name, then
`BYTE r/g/b` when enabling. Nothing client-side needs changing; the sprite only has to exist in
`sprites/hud.txt`.

Two things to get right, both learned from the Infusion:

- **Re-send after a HUD reset.** `ResetHUD` wipes the client's icon list, so anything still active must
  re-assert its icon. The hook is the `m_fInitHUD` block in `CBasePlayer::UpdateClientData`.
- **Turn it off on every exit path**, or it stays on screen forever.

This is *not* the system behind the shock/fire/poison icons beside the health readout. Those are
`CHudHealth`'s damage tiles, keyed to `DMG_*` bits and expiring after 2 seconds on their own — they cannot
represent a state that lasts a known duration.

## Inventory console commands and cvars

| Command | What it does |
| --- | --- |
| `inv_move <entry> <kind> <id> <col> <row>` | Move an Entry. The kind and id guard against a stale index |
| `inv_use <entry> <kind> <id>` | Use an Entry |
| `inv_drop <entry> <kind> <id>` | Drop one item off a Stack |
| `inv_dropall <entry> <kind> <id>` | Drop a whole Stack, one world entity per item |
| `inv_sync` | Ask the server to resend the Inventory |
| `inv_addrows <n>` | **Cheat-gated.** Grant Rows without an upgrade pickup — the tuning aid for judging the Row ceiling by eye |

| Cvar | Default | What it does |
| --- | --- | --- |
| `inv_rows_start` | 5 | Rows a new game begins with |
| `inv_rows_max` | 9 | Ceiling on granted Rows, and how many Rows are drawn |

Both cvars are **tuning knobs only**. The Rows a player actually has are saved state, captured at spawn
and granted since, so lowering either can never shrink a Grid that already holds things.

## The Infusion

Started by using a Health Syringe; see [ADR-0007](../docs/adr/0007-the-infusion-is-one-at-a-time.md) for
why it may be used at full health and why a second one is refused.

| Cvar | Default | What it does |
| --- | --- | --- |
| `infusion_rate` | 4 | HP healed per second |
| `infusion_duration` | 10 | Seconds an Infusion runs |
| `infusion_duration_bonus` | 5 | Extra seconds from the Med Expert Skill |

Healing lands as whole HP every 0.25s through an accumulator, so `infusion_rate` may be any value — it
does not have to divide evenly into the tick.

## Skill Points and Reset Tokens

Both are banked counters on `CPlayerSkills`, not Item Types — they occupy no Cells and a full Grid cannot
refuse them. Both are found in the world: `item_skillpoint` and `item_resettoken` are plain `CItem`s taken
on contact.

| Cvar | Default | What it does |
| --- | --- | --- |
| `skill_points_start` | 0 | Skill Points a new game begins with |
| `skill_reset_tokens_start` | 0 | Reset Tokens a new game begins with |

| Command | What it does |
| --- | --- |
| `skill_unlock <id>` | Spend points on a Skill |
| `skill_reset` | Spend one Reset Token, clearing every unlocked Skill |
| `skill_addpoints <n>` | **Cheat-gated.** Grant Skill Points without a pickup |
| `skill_addtokens <n>` | **Cheat-gated.** Grant Reset Tokens without a pickup |

Neither cvar is capped, deliberately: the ceiling on each is how many pickups a map places, and a cap
would let a found pickup silently do nothing.

**Points are never decremented.** `m_iPointsBase` + `m_iPointsGranted` is what the player has ever earned;
what is *left* is that minus the summed cost of everything unlocked, computed on demand by `SpentPoints()`.
Follow this if you add anything that spends points — store what was earned, derive what was spent. It is
why `TryReset()` is a `memset` with no refund logic, why retuning a `cost` corrects existing saves rather
than leaking points, and why a Skill cut from the tree refunds itself on the next load.

Raising `skill_points_start` is the way to work on the tree UI without hunting for pickups.

### Skill effect tuning

| Cvar | Default | What it does |
| --- | --- | --- |
| `skill_health_bonus` | 25 | Extra max health from Fortitude |
| `skill_armor_ratio_scale` | 0.9 | Armor Expert multiplies `ARMOR_RATIO` — the share of a blow that gets **past** armor — so lower is better armor |
| `skill_fall_damage_scale` | 0.5 | Sure Footing multiplies fall damage |
| `skill_health_regen_rate` | 0.5 | Regeneration, HP per second |
| `skill_battery_regen_rate` | 0.5 | Battery Regen, armor per second |
| `skill_battery_bonus` | 50 | Extra max armor from Battery Capacity |
| `skill_crowbar_range_scale` | 1.25 | Crowbar Reach multiplies the 32-unit swing trace |
| `skill_crowbar_damage_scale` | 1.5 | Crowbar Force multiplies crowbar damage |
| `skill_weapon_damage_scale` | 1.1 | Weapon Mastery multiplies all player-dealt damage |
| `skill_reload_time_scale` | 0.8 | Fast Reload multiplies `DefaultReload`'s delay |

Two rules that are easy to break:

- **Never cap or fill armor against `MAX_NORMAL_BATTERY`.** Call `PlayerMaxArmor(pPlayer)` — Battery
  Capacity raises the ceiling, and any route that assumes 100 makes the Skill silently do nothing through
  that route. The battery item, the wall charger and the regenerator all ask.
- **Regeneration is not an Infusion.** `CPlayerRegen` has no duration, icon or start; see
  [CONTEXT.md](../CONTEXT.md). It shares only the fractional accumulator, which is load-bearing at these
  rates — 0.5 HP/s rounded to whole points per tick would round the whole effect away.
- **Scale player damage at the chokepoints, not per weapon.** `SkillScaleWeaponDamage` is called from
  `ApplyMultiDamage` and from the direct-`TakeDamage` branch of `RadiusDamage` — every player weapon
  reaches one or the other. Do not scale before the branch in `RadiusDamage`: the other side already goes
  through `ApplyMultiDamage` and would be scaled twice.

### Effects in shared weapon code

Weapon files like `crowbar.cpp` and `weapons_shared.cpp` compile into **both** DLLs for client prediction.
`m_skills` is populated on both sides, so `m_pPlayer->m_skills.HasSkill(...)` gives the same answer in
either — `HUD_SetPredictedSkills` (`cl_dll/hl/hl_weapons.cpp`) fills the client's copy from the same
`gmsgSkillTree` message the Skill Tree panel draws from.

**Decide which side of the wire the effect belongs on, and be consistent:**

| The effect changes | Where it goes | Reads its cvar from |
| --- | --- | --- |
| A value the client predicts — `m_flNextAttack`, `m_flNextPrimaryAttack`, a trace that picks an animation | both DLLs, no guard | `dlls/skill_tuning.h` |
| Damage, health, armour, anything the server alone resolves | inside `#ifndef CLIENT_DLL` | `game.h`, include also guarded |

The rule behind the table is unchanged: **the server decides, the client agrees.** Predicting a Skill does
not mean the client gets a vote on whether a hit landed — Crowbar Reach extends the client's trace only so
that it plays the right swing animation for a hit the *server* resolved.

Two traps:

- **A tuning cvar is not visible from the client.** They are defined and registered in `dlls/game.cpp`,
  which is not in the client project, so naming `skill_crowbar_range_scale` in unguarded code is an
  undefined symbol. Add a `CSkillTuning` to `dlls/skill_tuning.h` instead; it resolves the cvar by name
  through `CVAR_GET_POINTER`, which works in both DLLs. Put only predicted knobs there.
- **A `CSkillTuning` falls back to the *neutral* value**, not to the cvar's default, so a failed lookup
  reads as the Skill not being held rather than as a number the two sides disagree about. That is
  deliberate — it keeps a second copy of every default out of the header.

`pm_shared/` is still out of reach. It runs from `playermove_t`, not `CBasePlayer`, so a Skill that changes
movement speed or jump height cannot use any of this; `SprintSpeed` and `HighJump` stay `SKILL_RESERVED`
for that reason.

## Adding a Skill

Everything static about a Skill lives in one table, [game_shared/skill_defs.h](../game_shared/skill_defs.h),
compiled into both DLLs. Adding a Skill is one enum entry and one table row — there is no second table to
keep in step, and no networking change. See
[ADR-0008](../docs/adr/0008-skill-definitions-are-shared-not-networked.md).

1. Add a new entry to `ESkillId`, before `_Count`, which sizes everything else.
2. Add a matching row to `k_SkillDefs[]`. **The table is indexed positionally by id**, so the row must sit
   at its id's index or every Skill after it shifts onto the wrong node.
3. Set the fields:
   - `id` must match the new `ESkillId`
   - `name` is the display label
   - `description` is the hover text
   - `spriteName` is a HUD sprite from `sprites/hud.txt`; `nullptr` renders the node without an icon
   - `gridCol` and `gridRow` place the node in the tree
   - `cost` is the Skill Point cost
   - `prereq` and `prereq2` each link to another `ESkillId`, or `ESkillId::None`. **Both are required** —
     a Skill with `None` in both slots is a root.
   - `tier` controls the visual size of the node
4. Update the game logic that should react to the Skill being unlocked. Read the modifier server-side from
   `m_skills` at the point the effect is computed, following `PulseWindowFor` / `PulseRechargeFor` in
   [dlls/player_pulse.cpp](../dlls/player_pulse.cpp). Never apply an effect in prediction as well.

`SendSkillTreeToClient()` needs no change: it sends only which Skills are unlocked and how many points are
unspent. Position, cost, prerequisites and tier are already on the client.

## Removing or reserving a Skill

Replace the row with `SKILL_RESERVED(TheId)` — but **never reuse the id**. Ids are frozen twice over: they
index the saved unlocked array *and* they are bit positions in the sync message, so handing one to a
different Skill silently reassigns what old saves unlocked. A reserved Skill keeps its `ESkillId` entry and
returns later with the same number.

A reserved row has a null `name`, which is what both sides key off: the client's `RebuildNodeList` gives it
no node, and `SpentPoints()` charges nothing for it. A player who had bought it gets those points back on
the next load, with no migration step.

The same macro covers a Skill held for a branch that has not opened yet, not just one that was cut.

**The table is ordered by id and a `static_assert` enforces it.** Grouping rows by branch reads better and
is exactly the mistake — entry `[n]` must be the Skill with id `n`, or a save's unlocked bits start meaning
different abilities with nothing to indicate it. Put the row at its id's index and use a comment for the
grouping.

## Notes

- Keep skill ids stable once they are saved or sent over the network.
- Client and server derive `k_MaxSkills` and `k_SkillMaskBytes` from the same constant, so they cannot
  disagree about the Skill set. `MsgFunc_SkillTree` drops a message whose size does not match.
- "Available" is computed client-side for display. The server re-validates every unlock in `TryUnlock()`,
  so a client that lies about availability still cannot unlock anything.
