# Half-Life Shock

The custom gameplay layer built on top of the Half-Life SDK: exploration, enhanced combat, custom items,
skill trees, inventory management, and stealth. This file defines what the words mean. It is a glossary —
how any of it is built belongs in [docs/PILLARS.md](docs/PILLARS.md) and [docs/adr/](docs/adr/).

Every term here is **settled**: it names something that exists, and code, comments and commits are expected
to use it. Names for things that are only intended are held separately, as proposals, under
[Proposed vocabulary](docs/ROADMAP.md#proposed-vocabulary) in the roadmap. A term graduates from there to
here when the feature it names is designed.

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

**Footprint**:
The Cells an Entry occupies, and the rectangle drawn for them — where a hover outline goes, what a drag
carries, what a click hits. Distinct from the Icon drawn inside it.
_Avoid_: tile, box, item rect

**Icon**:
The picture of an Entry in the Grid: a full-colour render of the thing as it is seen in the world,
untinted, one file per classname at `sprites/inv/<classname>.spr`. An Entry without one falls back to its
HUD sprite, tinted and additive.
_Avoid_: sprite (that is the file format), thumbnail, tile art

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
The text shown under the crosshair naming a nearby thing that can be taken. A use press takes what it
names; walking over the thing takes it too, so the prompt is for reaching what cannot be stepped on,
for explaining a full Grid, and for the one use-only pickup, a suit switch.
_Avoid_: use prompt, hint, tooltip

**Auto-Consume**:
Using a medkit or battery on contact instead of carrying it, when doing so would waste none of it. A
pickup that would waste some is carried instead; nothing walked over is left behind unless the Grid is
full.
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
An unlockable player ability. Unlocking is one-way; only a Reset Token undoes it, and it undoes all of
them at once.
_Avoid_: perk, talent, upgrade, ability; and "installed", "bought", or "learned" for unlocking

**Skill Point**:
The currency spent to unlock a Skill. Found in the world, never awarded for progress, and never lost —
spending one is recorded by the Skill it bought rather than by a balance, so a Reset returns it.
_Avoid_: point, XP, credit

**Reset Token**:
A consumable that returns every unlocked Skill and refunds every Skill Point spent. All or nothing —
there is no unlocking a single Skill. Banked as a count rather than carried, and spent from the Skill
Tree, so it occupies no Cells and cannot be dropped.
_Avoid_: respec token, refund, reroll

**Stat node**:
The smallest node in the Skill Tree: one flat bonus (+5% melee damage), one Skill Point, and a shared icon
with every other node of the same stat. Stat nodes are the roads between Skills and the whole of their
price; a player takes them to get somewhere, never for their own sake. A Stat node opens from any owned
node beside it, so roads have no gates of their own (settled 2026-09-15, not yet built). A Stat node is
a node, not a Skill.
_Avoid_: passive, filler, minor (a tier name), travel node

**Skill Tree**:
The full set of Skills and Stat nodes and the prerequisites between them. A Skill may be gated on two
nodes, and both are required; a Stat node is gated on adjacency alone. The suit at the centre is the one
start, held from the first moment and kept through a Reset. Every other node costs one Skill Point, and
the tree is deliberately not completable.
_Avoid_: upgrade tree, talent tree, perk tree; and board (a Grid word to avoid, and the tree is not one)

### The Infusion

**Health Syringe**:
The Item Type that starts an Infusion. Stacks three deep, used from the Inventory, and usable at any
health — it is spent going *into* damage where a medkit answers damage already taken.
_Avoid_: stim, stimpack, adrenaline, needle, shot

**Infusion**:
The healing a Health Syringe starts. It runs for its own duration, heals in small regular ticks while it
does, and is gone when it ends. Nothing interrupts one, and only one runs at a time. Passive
regeneration is not an Infusion, and the mod has none: its two regeneration Skills were cut on 2026-09-13.
_Avoid_: regen, heal-over-time, HoT, buff, dose

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
A melee swing empowered by having just deflected. Primed by a deflect for a short time and spent on the
next swing that connects, so missing costs nothing; a Cleave swing spends it on everything in its arc at
once. Any weapon on the melee roster carries it, and it shows at the screen edge while primed.
_Avoid_: riposte, counter, punish, parry attack

### The Suit

**Suit Variant**:
Which of the mod's three HEV suits the player wears. Set by the suit pickup the player last used, saved
with the player, and reflected in the gloves on every viewmodel and in the HUD's colour. Three values,
each a codename for a specialization the suit does not yet have: **Agility** (cyan, id 0, the default),
**Strength** (red, id 1), **Intelligence** (purple, id 2). Cosmetic today; the codenames are there so the
names survive the day it is not.
_Avoid_: suit colour, suit type, suit class, skin (for the concept — skin is the engine mechanism it rides on)

### Stealth

**Concealment**:
How hard the player is to perceive, moment to moment. It comes from darkness, stance, distance, and how far
off a monster's centre of view they are. It sets how quickly a monster learns about the player; it never
decides whether one can.
_Avoid_: stealth mode, invisibility, hidden state, sneak

**Suspicion**:
How close a single monster is to treating the player as an enemy. Every monster holds its own, and a squad
never holds one between them — a leader raises its members' Suspicion rather than having any of its own.
_Avoid_: awareness, alertness, aggro, detection meter; and **Alert**, which already names a monster state

**Unseen / Noticed / Spotted**:
The three states the player is told about — nobody is learning about them, somebody is, somebody has
finished. They report Suspicion, which is what monsters know, and not Concealment, which is what the player
is.
_Avoid_: hidden, detected, exposed; and *concealed* especially, which invites confusion with Concealment

**Search**:
What a squad does after losing the player: go to where they were last seen, and look. It ends when they
find the player or give up.
_Avoid_: sweep, hunt, patrol — the first two already name other behaviour

**Post**:
The place a monster guards. A Search that finds nothing moves the squad's Posts rather than returning them
to where they began, so a failed hunt permanently reshapes the space.
_Avoid_: station (a Station takes items in and gives items out), spot, waypoint, position

**Perception Profile**:
How good one kind of monster is at perceiving. Every monster has one. A poor Profile makes a monster slower
to notice things, never blind to something it would otherwise notice.
_Avoid_: senses, awareness level, difficulty, tier

**Backstab**:
A melee hit landed in a monster's rear arc. Purely a matter of where the attacker stands — whether the
monster has noticed them does not enter into it. Some monsters cannot be backstabbed at all.
_Avoid_: sneak attack, assassination, critical hit, ambush

### Weapons

**Gauss Katana**:
The mod's heavy melee weapon: a blade on the crowbar's swing, slower and far harder-hitting. Two clicks:
the left is the slash, blade only, and carries Cleave; the right swings the blade at a reduced share and
throws **the wave**, a piercing energy projectile that spends uranium and whose look is **the crescent**.
Every swing heats the blade; a thrown wave spends some of that heat. Everything that applies to a melee
hit — the Backstab, Melee Reach, Force and Speed, the Melee Damage Stat nodes, the Follow-Up — applies to
the blade. `weapon_katana` in code and maps; `katana_wave` is the projectile.
_Avoid_: sword, blade, energy sword, gauss blade; and *katana* alone where the gauss half matters

**Disturbance**:
What a death leaves behind at the place it happened. Monsters that care about such things can be drawn to
one, for as long as it lasts.
_Avoid_: corpse, body, evidence, alert marker
