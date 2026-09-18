# The morning checklist — the overnight session of 2026-09-18

Everything built overnight, as rows to run in game. Written **as each slice landed**, so it always matches
the tree: if a slice has no rows here, it was not built. The decisions behind the work are in
[OVERNIGHT_BRIEF.md](OVERNIGHT_BRIEF.md). Every commit it covers ends "Not verified in game". Report row by
row — "pass", "fail" and what was seen instead is enough.

## What landed

Ten commits on `hl-shock` after `050f864`, both DLLs building clean after each, none pushed:

| Commit | What | Rows |
| --- | --- | --- |
| `e2b57db` | The Carbon Pickaxe, and the overnight brief | P |
| `3ee6c86` | The Crystal Shard | S |
| `42f2f32` | Deposits | D |
| `c577b9e` | Unstable deposits, in arcs | U |
| `57ea7e7` | Stations: the Fuel processor and the Ammunition station | T |
| `3d8b619` | `minemap`, the test bed | M |
| `4c21515` | The Pulse's tail | Q |
| `eeac7e5` | The base Dash in the air | A |
| `db24fff` | The pickaxe in black metal (the stretch goal) | P1b, P1c, P2 |
| `e838f5e` | The melee alien grunt (reserve ②), and the arena in `minemap` | G |

**Not done, on purpose:** reserve ③, Station recycling — a use press cannot say *which* item to break
down, so it needs the Inventory as its UI or an arbitrary rule; a design fork, not an implementation
detail. The Air Dash's double-jump gate — see the Dash decisions below. Reserve ① turned out to be done
already (last section).

## Setup

Debug build, installed. Everything below can be run on `map minemap` (section M), which was generated and
compiled overnight and has everything placed. Console:

```
sv_cheats 1
map minemap
debug_damage 1          // numbers for the P rows
```

`impulse 101` includes the pickaxe, and `give item_shard` makes Shards, if you would rather use `topmap`.

## P. The Carbon Pickaxe

| # | Do | Expect |
| --- | --- | --- |
| P1 | `impulse 101`, open the melee bucket | Three entries: crowbar, katana, pickaxe — the pickaxe's HUD icon is still the crowbar's (ART_DEBT) |
| P1b | Draw the pickaxe, in each Suit Variant | A **black metal crowbar** in hand, blued-steel shaft with a highlight and a near-black grip; your gloves as usual; the Cleave swing plays |
| P1c | Look at one on the floor | The black crowbar lying there |
| P2 | Open the Inventory | The pickaxe has an Entry with a black crowbar Icon. Say whether it reads on the dark lattice |
| P3 | Swing at a zombie, `debug_damage 1` | 25 per hit on any difficulty (the crowbar's is 10) |
| P4 | Swing at the air, then at a wall, several times | Slower than the crowbar, faster than the katana's slash: 0.75 s between misses, 0.375 s between hits (`pickaxe_swing_time_scale` 1.5) |
| P5 | With Melee Force, Melee Speed, Cleave bought | Each applies, as on the crowbar |
| P6 | `give weapon_pickaxe` on a fresh map; also place one from the FGD | It spawns, falls to the floor, is picked up by walking over it |

## S. The Crystal Shard

| # | Do | Expect |
| --- | --- | --- |
| S1 | `give item_shard` | A small glass piece in an amber glow at your feet; walking over it takes it, with a high clink and the pickup history showing the crystal icon |
| S2 | Open the Inventory | A *Crystal Shard* Entry, one Cell, drawn with the crystal icon tinted (no Grid Icon yet) |
| S3 | `give item_shard` eleven more times | One Stack of 10 and a second Entry of 2 |
| S4 | Drop the Stack of 10 | Ten Shards scatter in front of you, and are not picked straight back up |
| S5 | Fill the Grid, then walk over a Shard | Refused; it stays on the floor |
| S6 | Look at a Shard on the floor | The Prompt names it *Crystal Shard* |
| S7 | Judge the look | Does the amber shell read as crystal? It was chosen by elimination — see ART_DEBT |

## D. Deposits

Needs a `func_deposit` in a map: `minemap` (the M section), or carve one in `topmap` from the FGD.

| # | Do | Expect |
| --- | --- | --- |
| D1 | Look at a deposit with the crowbar out | *Crystal deposit*, and the state line *Requires a mining tool*, no key |
| D2 | Switch to the pickaxe while still looking | The line changes to *Strike to mine* |
| D3 | Press use on it | Nothing happens |
| D4 | Hit it with the crowbar, the katana (both clicks), a pistol, a grenade | A ricochet spark on every direct hit; it never breaks; no crack decals |
| D5 | Hit it with the pickaxe | Glass-on-crystal sounds; it breaks on the **third** swing (the second with Melee Force) |
| D6 | On the break | A glass burst, and three Shards land on **your** side of it, even with the vein set into a wall |
| D7 | Throw a grenade, swap to the pickaxe before it goes off | The blast does nothing to the deposit |
| D8 | A deposit with a `targetname`, triggered by a button | It breaks and drops its Shards with nobody swinging |
| D9 | Quicksave with a deposit half-broken, quickload | Still half-broken; break it; its Shards drop once |

## U. Unstable deposits

A `func_deposit` with the *Unstable* flag.

| # | Do | Expect |
| --- | --- | --- |
| U1 | Watch it from a distance | Every 4 s: a crackle, 1.2 s of small arcs crawling over the vein with a swelling amber light, then a big crack of arcs into nearby walls with sparks |
| U2 | Look at the rock side of a vein set into a wall | No arc ever seems to come out of the rock |
| U3 | Stand within ~190 units, in the open | On the discharge an arc hits you for 20 |
| U4 | Same, behind a pillar or crate | No arc, no damage |
| U5 | Pulse just before the discharge | Parried — the deflect's usual cue, no damage |
| U6 | Lure a zombie or headcrab past it | It is struck too; two things in reach take two arcs |
| U7 | Break it with the pickaxe mid-telegraph | It stops at once; its Shards drop as usual |
| U8 | Two unstable veins in one room | They do not pulse in step |
| U9 | Quicksave, quickload | It keeps its rhythm |
| U10 | Judge the rhythm | Is 1.2 s of warning readable, and is 20 damage right for a tutor? `deposit_arc_period`, `_warn`, `_damage`, `_radius` |

## T. Stations

`func_station` of each type. `give item_shard` for Shards if no deposits are to hand.

| # | Do | Expect |
| --- | --- | --- |
| T1 | Look at a Fuel processor | *Fuel processor*, *[E] Insert 10 Shards for a Skill Point* |
| T2 | Press use with 4 Shards | The charger's refusal, and *Needs 10 Crystal Shards - you have 4.* on screen. Still 4 Shards |
| T3 | Press use with 12 Shards | The charger's accept, *Skill Point acquired.*, 2 Shards left, the Skill Tree shows one more point |
| T4 | Look at it again, press again | *Spent*, no key; a press only refuses |
| T5 | Ammunition station (uranium), 9 Shards | Three presses, +20 uranium each; 0 Shards; then *Empty* |
| T6 | Same with uranium at 90 of 100 | Refused, *No room for 20 uranium.*, **no Shards taken** |
| T7 | Ammunition station (Cores) at 5 of 6 Cores | Refused, no Shards taken; at 4 of 6 it pays 2 |
| T8 | Hold use, or double-tap | One trade per deliberate press (0.6 s lockout) |
| T9 | A station with a `target` | The target fires on each trade |
| T10 | Quicksave after one of three trades, quickload | Two trades left; a spent Fuel processor stays spent |

## M. minemap — the test bed

`map minemap`. Generated by `utils/maptool/greybox_minemap.py`, compiled through all four tools with no
leak, error or warning, nineteen lights. Nobody has walked it yet.

| Where | What is there | Rows it serves |
| --- | --- | --- |
| The hall (start) | Suit, pickaxe, crowbar, katana, pistol and clip, a grenade, the gauss (for the uranium count), the summon (for the Cores count); seven loose Shards; on the north wall, left to right, a Fuel processor, a uranium station, a Cores station | P, S, T |
| The closet, south of the hall | Forty keycards, one Cell each: walk through it to fill the Grid | S5, T refusals |
| The tunnel, east | Five stable deposits: north wall at waist height, south wall, north wall **high**, a free-standing boulder (rock sounds), and a fat one at the end (6 Shards, strength 200) | D |
| The chamber, through the tunnel's south doorway | A free-standing unstable vein behind a wall of cover, a second unstable vein in the east wall, a zombie and a headcrab | U |
| The arena, behind the door on the tunnel's north side (button beside it) | Two melee alien grunts and one hornet grunt, one squad | G |

| # | Do | Expect |
| --- | --- | --- |
| M1 | `map minemap` | Loads; a rock-walled hall; crystal veins textured with Xen crystal |
| M2 | Walk it end to end | No holes, nothing stuck, the doorway to the chamber passable |
| M3 | The high vein | Its Shards land on the floor by you, not lost |

Note for T5: picking up the gauss already gives 20 uranium, so the station's trades read 40, 60, 80.

## Q. The Pulse's tail

With the Pulse Module (`give item_pulsemodule`) and `debug_damage 1`. A zombie is the easiest attacker.

| # | Do | Expect |
| --- | --- | --- |
| Q1 | Pulse so a hit lands **inside** the window | Exactly as before: the deflect, no damage, the short Recharge (and the Rebound if bought) |
| Q2 | Pulse with nothing hitting you | After the Shield's ring, a dimmer, smaller ring: the tail. The bar goes straight to recharging, for about 3.75 s |
| Q3 | Pulse early, so the hit lands about 0.5 s after the press | A dull, low clang, and **half** the damage (`debug_damage`); no Discharge, no Follow-Up icon |
| Q4 | Pulse, then take a hit about 1.2 s after the press | Full damage: the tail is over |
| Q5 | Pulse, then fall a long way inside the tail | Full fall damage: falls are not on the Shield's list |
| Q6 | With the Defense Matrix: hold the key | Window, then tail, then the Matrix rising at 1 s — no moment unprotected, and the Matrix behaves as before |
| Q7 | Mash the key against a grunt | Roughly a second of cover every four seconds, most of it at half |
| Q8 | With Pulse Window bought | The deflect part grows; the whole still ends at 1 s |
| Q9 | Judge the two cues | Is "I braced" told from "I parried" by ear and by eye? |

## A. The Dash in the air

With the Dash (`give item_longjump`).

| # | Do | Expect |
| --- | --- | --- |
| A1 | Jump, then Dash with a movement key held | A flat burst along the keys, mid-air; you keep falling as before, the burst adds only horizontal speed |
| A2 | Dash along the ground off a ledge | The burst carries on past the edge and across a short gap (it used to stop dead at the edge) |
| A3 | Dash and jump together | The burst carries into the jump — a longer jump than before. Say whether this reads as a feature or an exploit |
| A4 | With the Air Dash bought: jump, then Dash | The directional one, as before: along the crosshair, up included, gravity off |
| A5 | With the Air Dash: Dash on the ground | The flat one, as before |
| A6 | Read the Air Dash's tooltip | *In the air, the Dash goes where you aim, up included.* |

## G. The melee alien grunt (reserve ②)

On `minemap`: the button beside the door on the tunnel's north wall opens the arena. `debug_schedule 1`,
then `debug_damage 1`.

| # | Do | Expect |
| --- | --- | --- |
| G1 | Open the arena | Two grunts with an empty right arm, one with the hivehand. `debug_schedule` names the first two `monster_alien_grunt_melee` |
| G2 | Let them see you | Both melee grunts close in; the hornet grunt holds back and shoots. Nobody stands still facing you doing nothing |
| G3 | Aim at each | One squad: one `leader`, two `member` |
| G4 | Crowbar a melee grunt in the back and the shoulder | Full damage, blood, no ricochet spark (a hornet grunt still ricochets) |
| G5 | Backstab one | The Backstab lands in full |
| G6 | Let one punch you | Damage, the claw hit sound, a shove sideways |
| G7 | Pulse into a punch | The deflect; you are still shoved; **no** hit sound, **no** blood on you. Same with a hornet grunt's punch |
| G8 | Kill one while the other watches | It reacts as any alien grunt does (5f's witness rules are unchanged) |

## Decisions I made

Things the grill did not settle, decided during the night. Each can be overturned in a line.

- **The mining check is `IsMiningTool()` on the item, not a damage bit.** A deposit asks the striking
  player's active item. Rejected: a new `DMG_` bit, which costs one of the few free bits in the damage word
  for a rule a virtual answers. Consequence: something the player's pickaxe *throws* would not mine — there
  is no such thing.
- **"~0.75 s" means after a miss.** The crowbar's stock delays are 0.5 s after a miss and 0.25 s after a
  hit; the settled "1.5× the crowbar's time" is `pickaxe_swing_time_scale` 1.5, which is 0.75 s and
  0.375 s. DPS 67 against the crowbar's 40 and the katana slash's 100.
- **The pickaxe's Grid Icon** was a copy of the crowbar's until the black metal landed; it is now rendered
  from `w_pickaxe` the way every Icon is (`render_icon.py`, top view, rolled 180° to match the crowbar's).
- **Black metal is a palette remap, not a repaint**: luminance onto a dark curve (10 to 175, gamma 1.7)
  with a faint cold cast. The two textures are chrome environment maps, so the curve *is* the finish.
  Luminance rather than the brightest channel, because by the brightest channel the red paint came out
  lighter than the metal. The numbers are at the top of `pickaxe_black.py`.
- **The Shard got a real HUD icon** (`utils/sprtool/icons/shard.py`) rather than borrowing one: an Item
  Type with no sprite draws as an empty box in the Grid, which is useless to test with, and no stock icon
  looks like a crystal.
- **The Shard's stand-in model is the glass gib, in an amber glow shell.** Amber by elimination: the
  Records' glow is pale cold, the progression pickups' is cyan and gold. The whole game's crystal hue is
  unsettled — ART_DEBT asks for the deposit, the Shard, the arcs and the Heart to agree once it is.
- **A deposit's default strength is 120, three pickaxe swings.** Breakables take double from `DMG_CLUB`
  (vanilla, `CBreakable::TakeDamage`), so the pickaxe lands 50. Two swings felt like breaking a crate;
  three is a small commitment. `health` on the entity overrides it.
- **A deposit is prompted with state lines only**, *Strike to mine* with the pickaxe in hand and *Requires
  a mining tool* without — never *[E] Mine*, because a use press does not mine. It needed a use cap to be
  prompted at all, so a use press on one is swallowed.
- **Shards drop on the striker's side**, from the point of the brush nearest the striker's eyes, capped at
  their waist height: a vein cut into a wall would otherwise spill into the rock, and one high on a wall
  would drop its Shards more than the 256 units an item falls to find a floor.
- **The material choice is glass (default) or rock**, nothing else: glass sounds and gibs are what the
  Shard stand-in already is. The FGD offers only those two.
- **The arcs strike only the living** — players and monsters. "Every damageable thing" would have arced to
  crates and breakables, which reads as noise rather than as a rule.
- **The flash is arcs plus a dynamic light**, not the Records halo the brief mentioned: a halo sprite does
  nothing to say *when*, while a light that swells with the telegraph does, and a hazard's warning is not
  the beacon the Records rule forbids.
- **The numbers**: a 4 s cycle, 1.2 s of warning, 20 damage, 192 units. First guesses, all cvars. The warning
  is longer than the Pulse window on purpose — this is the tutor.
- **The arc colour is the Shard's amber pushed toward white** (255, 200, 120), so crystal and its discharge
  read as one substance; it moves with the Shard's hue when that settles.
- **The discharge's attacker is the vein itself**, so a monster it kills is not a player kill and leaves no
  witnesses or Disturbance — stealth's rule for player-dealt deaths, applied as written.
- **A trade whose ammunition would only partly fit is refused**, not paid out partly. The rule was "check
  room before consuming"; half a payout for a full price is the same bug, smaller. The cost is that a player
  at 81 uranium cannot trade until they have fired some.
- **The ammunition choice is folded into the type**: three types (fuel, uranium, Cores) rather than a type
  plus an ammo key. One dropdown in the editor, and a row per recipe in the table.
- **The Prompt strings live in the Station table**, on the same line as the numbers they state, and the
  Prompt's rows are built from them — so a price change is one line and cannot show one price while
  charging another. `func_station` does not advertise the mapper's Prompt overrides for the same reason.
- **Not enough Shards gets a centre-print** with the count, rather than a Prompt state line per shortfall:
  the Prompt would need a class per price, and the refusal is the moment the player needs the number.
- **Sounds are the HEV charger's accept and refuse**, a Station being `func_recharge`'s shape; a Skill Point
  adds the Skill Point pickup sound.
- **The tail follows only a window that deflected nothing.** A window that deflected ends exactly as it
  always has — short Recharge, Rebound — so the verified parry and Rebound timing do not move at all. The
  cost: after a deflect, the rest of the second is unprotected, as it is today.
- **The tail's length is `skill_matrix_hold`**, not a cvar of its own: rule 5 says the tail runs exactly to
  the Matrix's raise, and two numbers for one moment could only ever disagree. So one new cvar,
  `pulse_tail_scale`, where the roadmap guessed two. The Matrix's code is not touched.
- **The bar shows the tail and the Recharge as one countdown** to Ready, not a new state: the client needs
  no change, and what the bar is for is *when can I Pulse again*. The ring and the sound carry the tail.
- **The flat Dash no longer ends on leaving the ground, at all.** "Along the movement keys, the ground
  Dash's own rule, because a Dash glued to the ground cannot cross a gap" — and the old end-at-the-ledge
  rule was exactly the glue. Consequence: a Dash carries into a jump (A3). Rejected: a separate
  third kind of Dash for the air, which would need a third state in `fuser1` and changes nothing a player
  could feel, since a jump followed by a Dash covers the same ground.
- **The Air Dash's double-jump gate was not built**, though settled: it strands the Dash Recovery Stat node
  at (1,0), which the memory lists as your open placement question. Only the tooltip changed.
- **The melee grunt is a classname, `monster_alien_grunt_melee`**, the entry's open question. A keyvalue was
  the alternative; it would have mattered only if squads recruited by classname, and for alien military
  they do not. A classname shows by name in the editor and in `debug_schedule`.
- **Its chase bypasses the slot rather than taking it**, so a melee grunt can never be the one holding the
  chase slot a hornet grunt would have used — the hornet grunt's behaviour is exactly vanilla.
- **The deflected-punch fix applies to every alien grunt**, as the entry said to do "while there": the
  shove stays, the hit sound and the blood go.
- **The arena is behind a door** in `minemap`, so the grunts do not come to the mining tests on their own.
- **The brace applies after Last Stand and Ricochet**, so a hit either of those refuses entirely never makes
  the brace's sound; and before the Demolitions and Insulation scales, which then multiply as usual.

## Found stale in the docs

- The roadmap still listed the first-swing/follow-up timer split as work (reserve ① of the brief).
  `CCrowbar::Swing` shows the half-damage follow-up was dropped on 2026-09-13; the two roadmap passages
  are corrected, and the reserve starts at ② instead.
- PILLARS pillar 2 still said Crowbar Speed (id 11) was the one combat Skill doing nothing, blocked on that
  same rule. It was built as Melee Speed on 2026-09-14; corrected with the melee grunt's commit.
