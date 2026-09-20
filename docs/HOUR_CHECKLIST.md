# The hour checklist — the independent session of 2026-09-20

Everything built in the hour, as rows to run in game. Six commits on `hl-shock` after `fee92b4`, none
verified; the decisions behind each slice, with their rejected alternatives, are in
[HOUR_BRIEF.md](HOUR_BRIEF.md). Report row by row — "pass", "fail" and what was seen instead is enough.

| Commit | What | Rows |
| --- | --- | --- |
| `1395a1b` | The pickaxe's first hand-made cut | — |
| `fb310a9` | 5g — the level-change reset | L |
| `873734d` | Phase's effect | H |
| `079f42e` | The deflected melee root cause (does not build the client alone; fixed in `6c281a2`) | F |
| `ee79dd8` | 5e — the captain's channel | K |
| `6c281a2` | One deferred-text collector for the inventory panels, and the client fix | V |

## Setup

Debug build, installed. Console:

```
sv_cheats 1
debug_schedule 1     // state and meter over every monster you aim at
debug_damage 1       // every refused or landed hit, by name
```

## L. 5g — the level-change reset

Needs a vanilla map with a transition a monster can cross: any `trigger_changelevel` whose transition
volume holds the monster when you cross (one following you through the door is enough). `topmap` has none.

| # | Do | Expect |
| --- | --- | --- |
| L1 | Aggro a grunt or zombie, run through a transition with it close behind, aim at it on the far side | State **Idle**, meter 0; it does not hunt you until it sees you again |
| L2 | Same setup, but quicksave and quickload instead of crossing | Still Combat or Alert, meter as it was |
| L3 | Be Noticed (icon dim) but not acquired, cross | Meter 0 on the far side, the floor gone |
| L4 | A scientist mid-script crosses with you | The script plays as it always did |
| L5 | A body killed before the crossing crosses | Still a corpse |

## H. Phase

`give item_longjump` for the Dash; buy Phase (Shinobi, behind the Dash gate) or `skill_unlock_all`. A zombie
or headcrab as the attacker.

| # | Do | Expect |
| --- | --- | --- |
| H1 | Dash through a zombie's swing so the claw lands mid-burst | No damage; console prints `phase: dodged` with the burst left; the shove and the small view kick still happen |
| H2 | Stand still and take the same swing | Full damage, no `phase` line |
| H3 | Take a hit just after the burst ends | Full damage: the dodge is only the burst |
| H4 | Dash across a tripmine's beam or into a blast timed to the burst | No damage if it lands inside the burst |
| H5 | Air Dash straight down into the floor from height | Fall damage as before: falls are not dodged |
| H6 | Without Phase, H1 again | Damage lands; nothing prints |
| H7 | Judge it | Does a dodge with no damage window make melee enemies a free hit? Phase is the first to cut if so |

## F. The deflected melee root cause

With the Pulse Module (`give item_pulsemodule`). `minemap` has zombies, headcrabs and alien grunts;
`give monster_<name>` for the rest. Sound up.

| # | Do | Expect |
| --- | --- | --- |
| F1 | Pulse into a zombie's slash | The Pulse's clang, the small (scaled) view kick, the sideways shove; **no** claw-strike sound from the zombie and no miss whoosh either |
| F2 | Take the same slash without a Pulse | The claw-strike sound as always, full kick, damage |
| F3 | Zombie swings and misses you | The miss whoosh as always |
| F4 | F1 against an alien slave's claw, the maddened miner's swing, a Panthereye's claw | Same as F1 |
| F5 | F1 against an alien grunt's punch | Unchanged from before: shove, no hit sound, no blood |
| F6 | F1 against a Gargantua's slash (`god` on) | Clang, the big shove and kick, no hit sound |
| F7 | A zombie slashes a scientist or a grunt | The hit sound plays: a blow that lands still sounds |
| F8 | Take a zombie's slash inside Last Stand's window | Silent from the zombie, like a deflect |
| F9 | Pulse into a bullsquid's bite or tail whip | As before (no hit sound of their own): shove, no damage |

## K. 5e — the captain's channel

A grunt squad of three or more (`topmap`'s, or `give monster_human_grunt` three times close together so
they recruit; the leader wears the beret). Approach so that **one** grunt can see you and the others face
away.

| # | Do | Expect |
| --- | --- | --- |
| K1 | Let one member see you until the icon reaches Noticed, then duck back | Console prints `captain: monster_human_grunt crossed notice, leader lifts N`; the leader says "stay alert people" (or another `HG_LOST` line) |
| K2 | Aim at the members who did not see you | Their meters sit at the notice line (0.35), icon dim, not at 0 |
| K3 | Stay hidden | Every meter drains as normal; nobody searches, nobody comes |
| K4 | Let the **leader** be the one who sees you | The same: he lifts the others and speaks |
| K5 | Show yourself again to a member already at the line | No second line and no `captain:` print until someone has drained below the line and crossed it again |
| K6 | Kill the leader first (the beret), then K1 on a survivor | Nothing printed, nobody lifted, no line |
| K7 | A lone grunt, far from the others, crosses Noticed | Nothing printed, no line |
| K8 | Kill a grunt in view of one member, the others facing away (the 5f witness case) | Unchanged: the witness jumps to 0.75 and speaks `HG_WITNESS`; the others are **not** lifted by that jump |

## V. The deferred-text collector

A refactor with no intended change on screen. Two looks.

| # | Do | Expect |
| --- | --- | --- |
| V1 | Open the Inventory with a few weapons and some ammo | The `n / max` readout beside each ammo icon, exactly as before, on every row |
| V2 | With a Stack in the Grid (`give item_shard` a few times) | The `x3` count in the footprint's bottom-right corner, on top of the icon |

## Results

*Andrei's, row by row, when played.*
