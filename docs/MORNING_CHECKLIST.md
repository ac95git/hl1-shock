# The morning checklist — the overnight session of 2026-09-18

Everything built overnight, as rows to run in game. Written **as each slice landed**, so it always matches
the tree: if a slice has no rows here, it was not built. The decisions behind the work are in
[OVERNIGHT_BRIEF.md](OVERNIGHT_BRIEF.md). Every commit it covers ends "Not verified in game". Report row by
row — "pass", "fail" and what was seen instead is enough.

## Setup

Debug build, installed. Console:

```
sv_cheats 1
impulse 101             // now includes weapon_pickaxe
debug_damage 1          // numbers for the P rows
```

## P. The Carbon Pickaxe

| # | Do | Expect |
| --- | --- | --- |
| P1 | `impulse 101`, open the melee bucket | Three entries: crowbar, katana, pickaxe — the pickaxe **looks exactly like the crowbar** (placeholder; ART_DEBT) |
| P2 | Open the Inventory | The pickaxe has an Entry, with the crowbar's Icon |
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

Needs a `func_deposit` in a map: `minemap` (built later tonight, if you see no M rows it was not), or
carve one in `topmap` from the FGD.

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

## Decisions I made

Things the grill did not settle, decided during the night. Each can be overturned in a line.

- **The mining check is `IsMiningTool()` on the item, not a damage bit.** A deposit asks the striking
  player's active item. Rejected: a new `DMG_` bit, which costs one of the few free bits in the damage word
  for a rule a virtual answers. Consequence: something the player's pickaxe *throws* would not mine — there
  is no such thing.
- **"~0.75 s" means after a miss.** The crowbar's stock delays are 0.5 s after a miss and 0.25 s after a
  hit; the settled "1.5× the crowbar's time" is `pickaxe_swing_time_scale` 1.5, which is 0.75 s and
  0.375 s. DPS 67 against the crowbar's 40 and the katana slash's 100.
- **The pickaxe's Grid Icon is a copy of the crowbar's**, so the Grid shows a picture rather than falling
  back to a tinted HUD sprite. Rejected: no Icon file, which draws the crowbar's HUD sprite anyway.
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

## Found stale in the docs

- The roadmap still listed the first-swing/follow-up timer split as work (reserve ① of the brief).
  `CCrowbar::Swing` shows the half-damage follow-up was dropped on 2026-09-13; the two roadmap passages
  are corrected, and the reserve starts at ② instead.
