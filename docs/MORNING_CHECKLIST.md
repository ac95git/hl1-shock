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

## Found stale in the docs

- The roadmap still listed the first-swing/follow-up timer split as work (reserve ① of the brief).
  `CCrowbar::Swing` shows the half-damage follow-up was dropped on 2026-09-13; the two roadmap passages
  are corrected, and the reserve starts at ② instead.
