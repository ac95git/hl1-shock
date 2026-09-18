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

**Prompt**:
The text under the crosshair naming what the player is looking at and what a use press will do to it —
a title, then either the bound key and an action, or a **state line** saying what is in the way. Every
entity a use press can act on has one. It defaults by class, a mapper overrides it with `prompt_title`
and `prompt_action`, and `prompt_suppress` hides it without disabling the thing, which is how an
unmarked secret panel stays unmarked. The word is bare: there is no other kind of prompt in this game.
_Avoid_: use prompt, hint, tooltip, HUD hint

**Pickup Prompt**:
The Prompt's oldest and narrowest case: the one naming a thing that can be taken. Walking over a thing
takes it too, so this case is for reaching what cannot be stepped on, for explaining a full Grid, and
for the one use-only pickup, a suit switch. Say **Prompt** unless the distinction is the point.
_Avoid_: item prompt, pickup hint

**State line**:
The line a Prompt shows instead of a key and an action, when the thing cannot be acted on yet — *Code
required*, *No power*. It is how a hard gate looks impassable. A gate that offered an action and then
refused it would teach the player that use presses are unreliable.
_Avoid_: locked message, error text, denial

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

### Records

**Record**:
A document the player reads in the world with a use press — a notice, a log, a file, a transmission.
Its text lives in `records.txt` and is the client's; the server owns only which ones the suit has
**registered**, saved as a mask of numeric ids. A Record is *read*, never taken: it stays where it is,
re-opens on every press, and occupies no Cells. It **glows** while unread. Ids are saved, so they are
stable once added and never reused.
_Avoid_: entry (that is an occupant of an Inventory), note, lore pickup, document, log, transmission as
the general word — *Transmission* is one category of Record

**Register**:
What the suit does to a Record the first time it is read: the id goes into its memory and stays there.
_Avoid_: collect, pick up, unlock, acquire

**Guidance**:
Objectives, as Records in a pinned category, handed over and taken back by `record_grant`. "Done" is a
revoke, optionally granting the next. Written in the advisor's voice. Guidance is the only kind of
Record that is ever revoked: a Record the player *read* never leaves the suit's memory.
_Avoid_: objective, quest, mission, task list

**Reader**:
The page a Record is shown on. One reader serves both the world and the Records tab. In the world it
takes no input at all, so the player keeps moving and looking while reading; damage, a second press or
walking away shut it.
_Avoid_: viewer, popup, document window

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
node beside it, so roads have no gates of their own (2026-09-15). A Stat node is a node, not a Skill.
_Avoid_: passive, filler, minor (a tier name), travel node

**Skill Tree**:
The full set of Skills and Stat nodes on one grid. Any node opens from any owned node beside it, and
empty cells are the only walls; there are no prerequisites (settled and built 2026-09-15). The suit at
the centre is the one start, held from the first moment and kept through a Reset. Every other node costs one Skill Point, and the tree is deliberately not completable.
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
The suit's brief defensive discharge, and the act of triggering it. A found Module since 2026-09-16
(`item_pulsemodule`, [ADR-0013](docs/adr/0013-the-pulse-is-a-found-module.md)) — before that it was suit
hardware, and it is still no Skill's to grant.
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

**Defense Matrix** (the **Matrix** for short):
The Juggernaut Route's stance. The Pulse key *held* for a second raises it, and from then on it keeps its
own time; while it stands nothing reaches health — armour pays for every hit — and the player is slowed. It
drops when its time is up or at zero armour, then waits out a cooldown. Armour is its pool — there is no second bar, and nothing refills by
waiting. The tap's Shield still fires at the front of every hold; the two are separate verbs on one key.
**Not a Shield**: that word is the field a Pulse raises, and the Matrix must not be called one in code,
docs or commits.
_Avoid_: shield, stance mode, bubble, fortify, bulwark

**Decaying Armor**:
The Juggernaut's Major, and the armour it grants when the Matrix comes up: a fixed amount above the
armour cap that fades on its own and is spent like any armour before then. Fuel, not a refill.
_Avoid_: overshield, temporary armour, bonus armour

### The Dash

**Dash**:
The Dash Module's burst along the direction the player is moving, on a tap of shift, from the ground only.
Found with the long jump module, which it does not replace. Built 2026-09-15.
_Avoid_: dodge, roll, sprint, blink, long jump (a different verb that still exists)

**Charge**:
One Dash's worth of readiness. A Dash spends one; they come back one at a time.
_Avoid_: stack, stamina; cooldown for the charges themselves (Recharge is the Pulse's word)

### The Status page

**Status**:
The Inventory Panel's third tab: the Modules found, as Slots on the suit, and the build's final numbers.
Every number is what the game actually uses, with every always-on bonus folded in; situational bonuses
are not on it. Designed 2026-09-16 ([docs/STATUS_PANEL.md](docs/STATUS_PANEL.md)).
_Avoid_: stats page, character sheet, loadout

**Slot**:
A Module's fixed place on the suit — head, body, left arm, right arm, legs — shown on the Status page. One
Module per Slot, always the same one; nothing is put into a Slot or taken out. Never a unit of Grid space
(that is a Cell) or a weapon's HUD position.
_Avoid_: socket, mount, equipment slot (it implies a choice there is none of)

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
What a monster does about a place it has reason to look at — where the player was last seen, or where a
body fell: walk there, look, walk back. One monster at a time from a squad; every loner that heard.
It ends when it finds the player or comes home. Since 2026-09-17 it is the SDK's own investigate schedule.
_Avoid_: sweep, hunt, patrol — the first two already name other behaviour

**Post**:
The place a monster stood when it left to Search, and returns to. Since 2026-09-17 a Search does not move
Posts; what a failed hunt leaves behind is the floor, not a new position.
_Avoid_: station (a Station takes items in and gives items out), spot, waypoint, position

**Witness**:
A monster that saw a kill happen — it had a line to the victim as it died, whether or not it can see the
player. A witness jumps to Noticed, speaks, and knows where the body is. Player-dealt kills only.
_Avoid_: observer, spectator, alerted

**The floor**:
The lowest a monster's Suspicion can drain to for the rest of the level, once it has witnessed a kill or
given up a chase. Just under the Noticed line, so the room is primed while the readout stays dim. Cleared
by a level change, never by a save.
_Avoid_: alert level, memory, grudge

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

**Carbon Pickaxe**:
The miner's tool and the heavy end of the melee roster: on the crowbar's swing, slower and harder-hitting
than the crowbar, lighter than the katana. It is the **mining tool** — the only thing that breaks a
Deposit. Every melee Skill applies to it. `weapon_pickaxe` in code and maps; *the pickaxe* in prose.
_Avoid_: pick, mattock; *mining laser* or *drill* for this weapon

**Disturbance**:
What a player-dealt death leaves behind at the place it happened: a sound, not a sight. Monsters that care
about such things — soldiers — hear it and one comes to look, for as long as it lasts. A Silent Kill leaves
none.
_Avoid_: corpse, body, evidence, alert marker

### The Alien Route

**Core**:
The Alien Route's ammunition — an ordinary ammo type, found in the world only and never made, that the
summon weapon and whatever else the alien Module serves spend. Built 2026-09-16.
_Avoid_: green battery (the name it replaced), cell (the Grid's unit), energy (the Energy Route's word)

**Ghost**:
A monster_ghost_slave summoned by the alien Module's weapon: an alien slave fighting on the player's side,
translucent, with no corpse, gone when its lifetime runs out, on death, or — once built — with the
ultimate's volley. Built 2026-09-16; it does not yet follow the player.
_Avoid_: ghost slave (the classname says slave once already), pet, minion, summon (the weapon and the verb)
