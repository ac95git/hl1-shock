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
4. If it is picked up in the world, give it a `CItem` subclass whose `MyTouch` calls `InventoryGiveItem`
   and then `AnnouncePickup(pPlayer, true)`, so the pickup history flashes it with the carried arrow.
   Walking over it takes it — every pickup is walk-over
   ([ADR-0011](../docs/adr/0011-pickups-are-walk-over.md)); do not override `AutoPickupOnTouch`. If
   using it on the spot can be known to waste nothing, override `ConsumeOnContact` the way the medkit
   and battery do, and announce with `false` there. `MyTouch` must return `false` when
   `InventoryGiveItem` accepts nothing, or a pickup vanishes into a full Grid.
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
| `inv_addrows <n>` | **Cheat-gated.** Grant Rows without an `item_rowgrant` pickup — the tuning aid for judging the Row ceiling by eye |

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

## Progression pickups — Skill Points, Reset Tokens, Row Grants

All three are banked counters rather than Item Types — they occupy no Cells and a full Grid cannot refuse
them. All three are found in the world and taken on contact: `item_skillpoint`, `item_resettoken` and
`item_rowgrant` are plain `CItem`s in `dlls/items.cpp`. The three models are the mod's own since
2026-09-13 (`docs/MODEL_WORKFLOW.md`), each with a part that emits light through an additive texture,
which is what makes them read as one family; the shared pickup sound is still borrowed (see
[ART_DEBT.md](../docs/ART_DEBT.md)).

Skill Points and Reset Tokens bank on `CPlayerSkills`; a Row Grant calls `CPlayerInventory::GrantRows(1)`
and is spent immediately. **The Row Grant is the only one with a real ceiling** (`inv_rows_max`), so it is
also the only one that can be refused — when the Grid is already at maximum it stays standing in the world
rather than being consumed for nothing.

Adding another progression pickup is one class in `dlls/items.cpp`, one `LINK_ENTITY_TO_CLASS`, and one
FGD line. No new file, so no project change.

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
| `skill_battery_bonus` | 50 | Extra max armor from Battery Capacity |
| `skill_melee_reach_scale` | 1.25 | Melee Reach multiplies the 32-unit swing trace |
| `skill_melee_force_scale` | 1.5 | Melee Force multiplies melee damage |
| `skill_melee_speed_scale` | 0.7 | Melee Speed multiplies the swing delay, miss and hit |
| `skill_stat_melee_damage` | 0.05 | Each Melee Damage Stat node adds this to one multiplier on melee damage |
| `skill_backstab_bonus_scale` | 1.5 | The Backstab node multiplies the weapon's own Backstab base |
| `cleave_cooldown` | 4 | Seconds before Cleave is ready again |
| `cleave_arc_dot` | 0.77 | Cosine of Cleave's half-angle: 0.77 is 40° either side of the aim |
| `cleave_radius` | 160 | How far from the eyes the arc reaches |
| `cleave_damage_scale` | 1.5 | Every hit in a Cleave |
| `katana_wave_swing_damage_scale` | 0.5 | The blade's share on the katana's right click, which swings it beside the wave |
| `skill_weapon_damage_scale` | 1.1 | Weapon Mastery multiplies all player-dealt damage |
| `skill_reload_time_scale` | 0.8 | Fast Reload multiplies `DefaultReload`'s delay, and the shotgun's shell-by-shell reload |
| `skill_marksman_scale` | 1.15 | Marksman multiplies player `DMG_BULLET` damage |
| `skill_stat_bullet_damage` | 0.05 | Each Bullet Damage Stat node adds this to one multiplier on player `DMG_BULLET` damage |
| `skill_draw_time_scale` | 0.6 | Quick Draw multiplies `DefaultDeploy`'s draw delay, both copies |
| `skill_demolitions_scale` | 1.25 | Demolitions multiplies player `DMG_BLAST` dealt |
| `skill_demolitions_resist_scale` | 0.5 | Demolitions multiplies `DMG_BLAST` the player takes, own grenades included |
| `skill_headhunter_scale` | 1.5 | Headhunter multiplies the head hitgroup multiplier for player hits |

Two rules that are easy to break:

- **Never cap or fill armor against `MAX_NORMAL_BATTERY`.** Call `PlayerMaxArmor(pPlayer)` — Battery
  Capacity raises the ceiling, and any route that assumes 100 makes the Skill silently do nothing through
  that route. The battery item and the wall charger both ask.
- **Nothing passive.** Regeneration and Battery Regen were cut on 2026-09-13 because they rewarded
  standing still, and `CPlayerRegen` went with them. Do not add a Skill whose effect accrues while the
  player does nothing; see the principles in [docs/SKILL_TREE.md](../docs/SKILL_TREE.md).
- **Scale player damage at the chokepoints, not per weapon.** `SkillScaleWeaponDamage` is called from
  `ApplyMultiDamage` and from the direct-`TakeDamage` branch of `RadiusDamage` — every player weapon
  reaches one or the other. Do not scale before the branch in `RadiusDamage`: the other side already goes
  through `ApplyMultiDamage` and would be scaled twice. It takes the damage type, so a **typed** Skill
  (Marksman on `DMG_BULLET`) is a test there, never a weapon list.

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
  which is not in the client project, so naming `skill_melee_reach_scale` in unguarded code is an
  undefined symbol. Add a `CSkillTuning` to `dlls/skill_tuning.h` instead; it resolves the cvar by name
  through `CVAR_GET_POINTER`, which works in both DLLs. Put only predicted knobs there.
- **A `CSkillTuning` falls back to the *neutral* value**, not to the cvar's default, so a failed lookup
  reads as the Skill not being held rather than as a number the two sides disagree about. That is
  deliberate — it keeps a second copy of every default out of the header.

`pm_shared/` is still out of reach. It runs from `playermove_t`, not `CBasePlayer`, so a Skill that changes
movement speed or jump height cannot use any of this. No Skill should: `SprintSpeed` and `HighJump` were
cut for good on 2026-09-13 because the normal movement rules stay, and a Module (the long jump's
physics-key route) is how movement changes reach `pm_shared/`.

## Adding a Skill

Everything static about a Skill lives in one table, [game_shared/skill_defs.h](../game_shared/skill_defs.h),
compiled into both DLLs. Adding a Skill is one enum entry and one table row — there is no second table to
keep in step, and no networking change. See
[ADR-0008](../docs/adr/0008-skill-definitions-are-shared-not-networked.md).

1. Add a new entry to `ESkillId`, before `_Count`, which bounds the definition table. The saved unlocked
   array and the sync mask are sized by `k_SkillIdCeiling` (256), not by `_Count`, so adding a Skill changes
   neither the save format nor the message length; a `static_assert` fires if `_Count` ever passes the
   ceiling. Do not lower the ceiling, and treat raising it as a save-format change.
2. Add a matching row to `k_SkillDefs[]`. **The table is indexed positionally by id**, so the row must sit
   at its id's index or every Skill after it shifts onto the wrong node.
3. Set the fields:
   - `id` must match the new `ESkillId`
   - `name` is the display label
   - `description` is the hover text
   - `spriteName` is a HUD sprite from `sprites/hud.txt`; `nullptr` renders the node without an icon
   - `gridCol` and `gridRow` place the node in the tree. No two rows may share a cell; a `static_assert`
     refuses it.
   - `cost` is **always 1**, and a `static_assert` holds every row to it. The price of a Skill is the road
     of Stat nodes to it ([docs/SKILL_TREE.md](../docs/SKILL_TREE.md), *The matrix*); nothing is printed
     on a node.
   - `prereq` and `prereq2` each link to another `ESkillId`, or `ESkillId::None`. **Both are required** —
     a Skill with `None` in both slots is a root. Roads are made by chaining Stat nodes on `prereq`.
   - `tier` controls the visual size of the node: `Stat` for a Stat node, `Minor` / `Medium` / `Major` for
     a Skill.
   - `stat` is `EStat::None` for a Skill. A **Stat node** is a row whose `stat` names what it grants and
     whose effect is the same for every node of that stat; use the `STAT_MELEE`-style macro for its
     kind rather than writing the row out, so every node of a stat has one name, text and icon.
4. Update the game logic that should react to the Skill being unlocked. Read the modifier server-side from
   `m_skills` at the point the effect is computed, following `PulseWindowFor` / `PulseRechargeFor` in
   [dlls/player_pulse.cpp](../dlls/player_pulse.cpp). Never apply an effect in prediction as well. A Stat
   node's effect is `m_skills.CountStat(stat)` times its cvar, read the same way; adding a new stat is one
   `EStat` value, one cvar, and one place that reads the count — see the Melee Damage nodes in
   `CCrowbar::Swing`.

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
- Client and server derive `k_MaxSkills` and `k_SkillMaskBytes` from the same header, so they cannot
  disagree about the Skill set or the message length. `MsgFunc_SkillTree` drops a message whose size does
  not match.
- "Available" is computed client-side for display. The server re-validates every unlock in `TryUnlock()`,
  so a client that lies about availability still cannot unlock anything.
