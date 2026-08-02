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

## Adding a Skill

Skill definitions live in [dlls/player_skills.h](../dlls/player_skills.h) and [dlls/player_skills.cpp](../dlls/player_skills.cpp).

1. Add a new entry to `ESkillId` in [dlls/player_skills.h](../dlls/player_skills.h).
2. Increase `_Count` automatically by keeping the enum ordered and the new id before `_Count`.
3. Add a matching `SkillDef` entry to `k_SkillDefs[]` in [dlls/player_skills.cpp](../dlls/player_skills.cpp).
4. Set these fields for the new skill:
   - `id` must match the new `ESkillId`
   - `name` is the display label
   - `description` is the tooltip text
   - `gridCol` and `gridRow` place the node in the tree
   - `cost` controls the unlock cost
   - `prereq` links to another `ESkillId`, or `ESkillId::None` for a root skill
   - `tier` controls the visual size of the node in the client UI
5. Update any game logic that should react to the skill being unlocked.

The server sends skill state to the client through `SendSkillTreeToClient()` in [dlls/player_skills.cpp](../dlls/player_skills.cpp). If the new skill changes the tree layout or availability rules, update that function as needed.

## Linking a Skill to a Sprite

Sprite selection is currently client-side in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp).

1. Open [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp).
2. Find the `k_SkillUiInfo[]` table near the top of the file. It holds the display name, description
   **and** sprite name — the wire message carries only ids and state bits, so all three live here.
3. Add or edit the entry for the new skill id. **The table is indexed positionally by id**, so a new
   entry must be appended in id order or every tooltip after it shifts onto the wrong node.
4. Use the sprite name from the HUD sprite set defined in `sprites/hud.txt`.
5. If the sprite name is not present in `hud.txt`, the node will still work but will render without an icon.

The lookup is by numeric skill id, so the client does not need a networking change just to show a different icon.

## Notes

- Keep skill ids stable once they are saved or sent over the network.
- If you change the tier system later, update both the server `SkillDef` data and the client `SkillNode` struct.
- The skill tree UI also uses the local metadata table in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp) for labels and descriptions.
