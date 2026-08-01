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
3. If it does something when used, add a case to `InventoryUseEntry` in
   [dlls/player_inventory.cpp](../dlls/player_inventory.cpp). Use behaviour is server-side only; the
   client never decides what using something does.
4. If it is picked up in the world, give it a `CItem` subclass whose `MyTouch` calls `InventoryGiveItem`,
   and override `AutoPickupOnTouch()` to return `false` so it is taken with a use press rather than by
   walking over it.

Weapons are **not** Item Types — they keep Half-Life's own `WeaponId`. See
[ADR-0002](../docs/adr/0002-two-identity-spaces-for-weapons-and-items.md).

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
2. Find the `k_SkillSpriteNames[]` table near the top of the file.
3. Add or edit the entry for the new skill id.
4. Use the sprite name from the HUD sprite set defined in `sprites/hud.txt`.
5. If the sprite name is not present in `hud.txt`, the node will still work but will render without an icon.

The lookup is by numeric skill id, so the client does not need a networking change just to show a different icon.

## Notes

- Keep skill ids stable once they are saved or sent over the network.
- If you change the tier system later, update both the server `SkillDef` data and the client `SkillNode` struct.
- The skill tree UI also uses the local metadata table in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp) for labels and descriptions.
