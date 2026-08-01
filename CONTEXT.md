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

### The Pulse

**Pulse**:
The suit's brief defensive discharge, and the act of triggering it. Suit hardware — anyone wearing the HEV
suit has it, and no Skill grants it.
_Avoid_: parry, block, dodge, guard

**Shield**:
The field a Pulse raises around the player. It exists only for the Pulse Window and is gone once that
closes.
_Avoid_: bubble, barrier, aura, forcefield

**Pulse Window**:
The interval a Shield stands for. Damage that arrives inside it is negated; damage a moment either side of
it is not.
_Avoid_: active frames, i-frames, parry window

**Recharge**:
The wait between a Shield falling and the next Pulse being available. Shorter when the Shield negated
something than when it negated nothing.
_Avoid_: cooldown, refresh, reload

**Discharge**:
The energy a Shield vents toward the player's crosshair when it negates a hit. It goes where the player is
aiming, not back where the damage came from.
_Avoid_: reflect, riposte, counter, retaliation

**Rebound**:
A Recharge skipped because the Shield deflected something. Held as a count, spent when such a window
closes, and restored only by sitting through a normal Recharge.
_Avoid_: reset, refund, proc, free recharge

**Follow-Up**:
A crowbar swing empowered by having just deflected. Primed by a deflect for a short time and spent on the
next swing that connects, so missing costs nothing.
_Avoid_: riposte, counter, punish, parry attack
