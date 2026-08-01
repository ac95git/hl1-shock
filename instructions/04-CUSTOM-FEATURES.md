# Custom Features

This project includes custom gameplay systems such as the player skill tree and inventory UI. This guide explains how to extend the skill tree safely.

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
