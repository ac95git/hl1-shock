# Half-Life Shock

The custom gameplay layer built on top of the Half-Life SDK: exploration, enhanced combat, custom items,
skill trees, and inventory management. This file defines what the words mean. It is a glossary — how any
of it is built belongs in [docs/PILLARS.md](docs/PILLARS.md) and [docs/adr/](docs/adr/).

## Language

### Inventory

**Inventory**:
The container the player carries things in. It owns its contents rather than reflecting them from
elsewhere.
_Avoid_: bag, backpack, pack

**Inventory Panel**:
The VGUI screen that displays the Inventory. Distinct from the Inventory, which exists whether or not the
panel is open.
_Avoid_: inventory screen, inventory UI, inventory window

**Grid**:
The two-dimensional space an Inventory occupies. Fixed in width, variable in number of Rows.
_Avoid_: board, canvas, layout

**Cell**:
One unit of Grid space. An Entry occupies a whole number of Cells.
_Avoid_: slot, square, tile

**Row**:
One horizontal line of Cells. The unit in which an Inventory grows.
_Avoid_: line, tier

**Row Grant**:
A permanent increase to an Inventory's Row count. Granted by things found in the world, never taken away.
_Avoid_: expansion, upgrade, capacity level

**Entry**:
One occupant of an Inventory — the thing that sits in Cells, is moved, used, and dropped. Two Entries of
the same Item Type are distinct and may sit in different places.
_Avoid_: slot, item instance, object

**Stack**:
An Entry holding more than one of the same Item Type. Bounded by that Item Type's maximum; picking up past
it creates a second Entry.
_Avoid_: pile, bundle, group

**Item Type**:
The kind of a thing — medkit, battery, keycard. Defines display name, icon, Cell width, maximum Stack
size, and what using it does. Half-Life's weapons are not Item Types; they keep their own identity.
_Avoid_: item definition, template, item class, item kind

### Acquisition

**Pickup Prompt**:
The text shown under the crosshair naming a nearby thing that can be taken. The general way things enter
an Inventory.
_Avoid_: use prompt, hint, tooltip

**Auto-Consume**:
Using a medkit on contact instead of taking it, when doing so would waste none of its healing.
_Avoid_: instant use, auto-heal, quick use

**Box**:
A world entity holding contents that are not in anyone's Inventory — what is left behind, and what is
found. Its contents are taken as they fit; whatever does not fit stays in it.
_Avoid_: container, loot bag, stash, weaponbox

**Ammo**:
Ammunition. A pool per type with a fixed cap, exactly as in Half-Life. Not an Item Type, never an Entry,
and occupies no Cells — but is displayed on the Inventory Panel.
_Avoid_: rounds, ammunition items, bullets

### Skills

**Skill**:
An unlockable player ability. Once unlocked it is never lost — there is no respec.
_Avoid_: perk, talent, upgrade, ability; and "installed", "bought", or "learned" for unlocking

**Skill Point**:
The currency spent to unlock a Skill.
_Avoid_: point, XP, credit

**Skill Tree**:
The full set of Skills and the prerequisites between them.
_Avoid_: upgrade tree, talent tree, perk tree
