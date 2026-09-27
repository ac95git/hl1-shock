# mines1 — the Pit Bottom

The campaign's first map, one file per map from here on. Designed with Andrei on 2026-09-22 in five
rounds of the plan, generated the same day and walked fullbright: "it's insane, I like it a lot", with
minor issues, listed below as work. Andrei has edited it by hand in J.A.C.K. since, so the `.map` is
the source now and `maps/mines1.rooms.txt` is the original intent only; nothing is regenerated. The
plans of that intent are `maps/mines1.plan_floor.png`, `_tier1`, `_tier2`, `_shaft` and the combined
`mines1.plan.png`; the traffic plans are below.

**What it is.** The foot of the shaft, built into a worked-out salt chamber 3072 by 2048 and 1536 high,
on three tiers: the yard on the floor, two terraces above it, each 192 wider than the one below. The
cage descends through the void in a mesh tower. The cold open passes through it twice: in through the
decontamination airlock into `shaft1`, and out at the end through the gate the old workings climb up to.
Not the game's hub (ROADMAP, "The structure"); CONTEXT.md, *The mine*, has the term.

**Sequence.** Cage descends (three seconds of rock, eighteen of chamber) → floor → security at the drift
door turns the player back → tag board says where the dry is → yard stair to the first terrace, top by
the no-entry gate → east along the ledge to the dry, suit and pick from the open locker → down → airlock,
steam, outer door → `shaft1`. Later: out through the gate → the sample hoist takes the Heart up its pipe →
dry, kit handed in → cage button → up → end text.

**Gates.** All hard: the airlock's inner door on the suit's master; the outer door after the cycle; the
old-workings gate only from behind; the cage button after the hand-in.

## Work, from the walk of 2026-09-22

Andrei's findings go under each heading in his words; the fix goes to the spec; the map is regenerated
and walked again. A heading is closed when he says so.

### Fix stairs

Three runs, all `stairs` lines at tread 32 rise 16, 1024 long for 512 of climb: the yard stair up the
south wall (bottom east, top west by the gate), the terrace stair along the north ledge (bottom west,
top east by the lab door), and the conveyor, which is not walked. What is wrong with them:

- *(Andrei)*

**The lifts, second walk 2026-09-22:** "a lift cannot be called again if the button is pressed while
the lift moves; move the buttons closer to the platforms." Then, plainly: "the lifts don't go down once
they go up." The buttons moved to the platform's edge: on the wall beside it at the yard and on the
first ledge's east wall, on a post on the ledge where there is no wall (the first lift's top, the
second's top). The not-coming-down was read out of `doors.cpp`: a `func_door`'s Use at the top is
honoured only with the Toggle spawnflag (32); `wait -1` alone leaves the door open for good. Both lifts
have the flag now, and so does the cage, which would have failed the same way at the end of the map.
The rule is in the generator's grammar and the workflow's table.

**The skip, same walk:** "reads as a box and rises into the ceiling." Now three doors with one name
moving as one, bucket, rim and bail, and a tube of its own through the roof (`SKIPSHAFT`) beside the
cage's, so it rises into the shaft and not the rock.

### Fix level transition

Both `trigger_changelevel`s target `shaft1`, which has no matching landmarks yet, so the transition
fails today. The retune of `shaft1` is the fix: its station rooms go (`CAGE` to `GALLERY`), a west
vestibule mirrors `TRANSIT` about `lm_in`, the old workings' end mirrors `OLDWAY` about `lm_old`, and
its return triggers sit where the arriving player never stands. Recommended as a regeneration from a
retuned spec with the eight deposit units and the pillars re-pasted from `prefabs.map`. What else:

- *(Andrei)*

### Fix rooms

Every room in the spec, by tier. A doorway room is the 32-deep opening that joins two rooms; it is
listed so a fix can name it. Findings go in the last column.

| Room | Tier | Size (x × y × h) | What it is | Fix |
| --- | --- | --- | --- | --- |
| `SHAFT` | above the roof | 160 × 256 × 1024 | The tube the cage starts in, 1536 to 2560 | |
| `TIER2` | 1024 | 3072 × 2048 × 512 | The top of the chamber; the second terrace is its rim: the water tanks and the fire main west, the dispatcher's cabin with its window south, the caged inspection ladder and the catwalk to the tower's top west (unreachable), pallets and drums east, the upper bridge | |
| `OLDMOUTH` / `BEHIND` | 1024 | 352 × 256 × 128 | The old salt working's mouth in the west wall, bricked but for a 24 gap; a lamp still burning behind | |
| `SURVDOOR` / `SURVWIN` / `SURVEY` | 1024 | 448 × 256 × 128 | The company's locked survey store; steel door never opens; window | |
| `OFFICEDOOR` / `OFFWIN` / `OFFICE` | 1024 | 384 × 256 × 128 | The foreman's office; window over the drop; the notebook Record; security at the desk | |
| `CANTDOOR` / `CANTEEN` | 1024 | 512 × 256 × 128 | Two tables, two of the crew | |
| `MAINTDOOR` / `MAINT` | 1024 | 480 × 256 × 128 | Over the lab: the elevator's drive head, pipe run, bench, one fitter | |
| `TIER1` | 512 | 2688 × 1664 × 512 | The middle of the chamber; the first terrace is its rim; bridge across; the smokers at the fan door; the two goods lifts land on the east ledge | |
| `TOILDOOR` / `TOILET` | 512 | 192 × 216 × 112 | Off the dry, east: three stalls, a sink | |
| `OLDGATEWAY` / `OLDWAY` | 512 | 256 × 480 × 120 | Behind the no-entry gate; the button that opens it; `lm_old` and the change to `shaft1` | |
| `DRYDOOR` / `DRY` | 512 | 512 × 512 × 192 | Lockers on the south wall, the player's open; bench; the guard; hand-in button; the bench-note Record | |
| `WORKDOOR` / `WORKSHOP` | 512 | 512 × 640 × 192 | Bench, a car up on blocks, one of the crew | |
| `ELECDOOR` / `ELECTRICAL` | 512 | 256 × 256 × 128 | The plant room: panels on the back wall, the compressor, the air main leaving along the ledge and down the yard's east wall to the drift door | |
| `FANDOOR` / `FANROOM` | 512 | 512 × 512 × 256 | The fan, a box until built by hand; the smokers on the ledge outside | |
| `LABDOOR` / `LABWIN1` / `LABWIN2` / `LAB` | 512 | 640 × 416 × 192 | Over the hall's north end, sealed: steel door, two windows onto the ledge; the pipe comes up into the lab's machine and a second pipe leaves it; benches, glass, the technician and one more | |
| `YARD` | 0 | 2304 × 1280 × 512 | The floor: the tower and the skip beside it, the onsetter's hut and tag board, the meal corner and the first-aid cabinet, the main line east to the portal with the working loco and two cars, the spur to the hall, the branch to the garage, the crusher and the stockpile bay, the conveyor to the bin, the sump and pump, the stair, the first goods lift | |
| `LAMPDOOR` / `LAMPROOM` | 0 | 256 × 256 × 112 | Counter | |
| `CRIBDOOR` / `CRIB` | 0 | 256 × 256 × 112 | The tool crib; counter; one of the crew | |
| `PROCDOOR` / `PROCWIN` / `PROCESS` | 0 | 640 × 416 × 224 | The spur in through the wide door, loaded car, hopper, belt to the sample hoist, whose pipe rises to the lab; sorting benches, two of the crew, the window; the graded-crystal store behind chainlink in the south-east corner | |
| `DUCT` | 224 to 512 | 96 × 96 × 288 | The pipe's shaft from the hall to the lab; 16 clear each side, impassable | |
| `SUMP` | −128 | 256 × 128 × 128 | Water 104 deep; the pump beside it | |
| `MAGWAY` / `MAGAZINE` | 0 | 256 × 252 × 128 | The explosives store; mesh gate never opens; crates seen through it; the sign at the passage | |
| `NICHE` | 32 | 96 × 80 × 96 | The miners' saint: figure, two candles, low rail | |
| `REFDOOR` / `REFUGE` | 0 | 256 × 256 × 112 | Heavy door; benches, water tank, phone, stretcher, first-aid cabinet | |
| `DOOR_IN` / `AIRLOCK` / `DOOR_OUT` | 0 | 384 × 320 × 192 | The decontamination airlock: inner door on `suit_on`, the cycle, outer door | |
| `TRANSIT` | 0 | 384 × 256 × 176 | The drift's first 384: autosave, `lm_in`, the change to `shaft1` | |
| `PORTALWAY` / `HAULAGE` | 0 | 800 × 320 × 208 | The rail portal south of the airlock, its roller door never open; the haulage drift behind it with a loco idling; the rails go through | |
| `GARAGEWAY` / `GARAGE` | 0 | 640 × 416 × 256 | The rail plant's bay: the branch in through the shut rail gate in the fence, the digging machine, the battery loco on charge and the rack; placeholder boxes | |

### Create hoist machine

**Andrei, 2026-09-22:** "essentially a pipe with a window in which a sample is inserted, the window
then closes and the sample is pushed up. The crystal will be inserted in a tube, and the tube will be
inserted in the hoist machine." A pneumatic sample line to the surface, not the cage.

**Moved into the processing hall the same day, and the lab sealed** (Andrei: "the sample reaches the
lab, then the lab pushes it further up, a scripted sequence; the lab becomes inaccessible and the
sequence is viewed through the windows").

In the spec, as placeholder boxes. In the hall: the machine's body at the end of the belt is the
`func_station` (stationtype 3, prompt *Sample hoist*), 96 by 96 by 112, its shutter door on the south
face starting open and closing on use for eight seconds; the push is a steam burst; the pipe rises from
its top through the duct into the lab. In the lab: the pipe comes up into the lab's own machine, whose
shutter faces the windows, and a second pipe leaves its top into the rock. The lab's door is a steel
`func_wall` that never opens; two windows either side of it look in from the north ledge. The sequence,
from the hall's use: shutter, push, then two seconds later the lab's manager: an arrival burst, the
technician walks to the machine and presses (`scripted_sequence`, `push_button` then `console`), the
lab's shutter closes, the second push, the text, and the hand-in unlocks. The cage no longer moves
when the Heart goes: the button sends it up at the end and that is its only ride after the descent.

Still to make: the two machines and the pipes as craft (J.A.C.K. from prefabs, or a model), and the
tube the crystal goes into, which is a model and an Item Type question (does the player hold a tube, or
does the Station supply it?). Open:

- *(Andrei)*

### Review of 2026-09-22, before the second generation

Andrei's four questions and the calls, all built the same day: the second tier was empty, so it got the
water tanks, the dispatcher's cabin, the inspection ladder and catwalk, and the pallets; travel between
tiers gets **one goods lift in two platforms** (yard to first terrace by the plant room, first to second
on the east ledge, a call button at each landing), and the stairs stay; bulk crystal is stored **in the
stockpile bay beside the crusher** before sorting and **in the mesh store in the hall** after; and the
rails get **their own portal** south of the airlock, since a loco cannot pass a decontamination lock.
The loco is in three places: working on the main line, on charge in the garage, idling behind the
roller door. The skip, the sealed working, the compressor, the toilets, the second first-aid cabinet and
the charging rack went in on "add all suggested".

### Traffic — settled 2026-09-27, not built

Andrei's hour of 2026-09-26 removed both stairs (the lifts are now the only way between tiers), the
old rails and trackbed, the crusher and stockpile placeholders; extended the yard north to y 896 and
the garage west to x −832; moved the lamp room north and the niche up to z 256; and hung two cables
on the cage (`lip −550`, the cables in the door entity so they ride with it). The traffic plan was
drawn over that export. **`maps/mines1.plan_traffic.png` is the chosen one**;
`mines1.plan_traffic_salt.png` is the first draft, in which salt was also a product.

- **Only the crystals are worth anything.** Everything else dug is waste.
- **Rails:** two rails 24 apart (narrow gauge), sleepers 48 every 32; straights and 45° only;
  turnouts, no turntables, all facing a train from the portal. One main line at y −368 from the portal
  west to the tip; turnouts to the hall spur (x 848), the garage (x 1088, two roads: the jumbo and the
  loco's charger) and the cage siding (x −560). The drill is a rail jumbo, static, in the garage.
- **Ore:** portal → hall spur, road A → tipped into the ore bunker → picking belt with benches →
  crystals by hand to the mesh store → crated → up in the cage with a guard, from the cage siding.
- **Waste:** the rest of the belt falls into a waste hopper over the hall's road B, and back out;
  waste from new tunnels comes in at the portal and runs straight through. Both go to **the tip**:
  the main line's west end, where cars dump into **the stope**, a worked-out void from the floor to
  tier 2 behind the west wall (x −1888..−1600, y −496..−240). The tier-2 bricked mouth moves south
  onto it, so the lamp behind the bricks looks down on the spoil. Named the stope, never "old
  workings", which is `OLDWAY`.
- **The skip becomes the cage's counterweight:** same compartment, same trigger as the cage, starting
  at the bottom and moving opposite. Its two triggers stood at the removed stair tops and go.
- **Moves:** the crib (the tip drift takes its door); the fan room (out of the stope's column, anywhere).
  The tier-1 conveyor has no job left. A player-driven train is not for mines1.
- **Open:** where the crib and the fan room go; whether `crashtrain` (a `func_tracktrain` with no
  `path_track`, so it never moves) is deleted; parked trains or one scripted straight run.

## The cast — the Crew, grilled 2026-09-24

The 26 stand-ins become the Crew (ROADMAP, "The Crew — grilled 2026-09-24"), placed by Andrei by hand:
the seven Barneys `monster_security`; the lab's two `monster_technician`; the workshop's and the
garage's `monster_construction` once that model exists, miners until then; the other ten
`monster_miner`. The three who gate the sequence — the drift-door guard (`air_guard`), the dry guard
(`dry_guard`) and the lab technician (`lab_tech`) — carry a game-over trigger on death, set in the map.
The technician's push uses Barney's `buttonpush`, not the scientist's `console`. The miners are on the
maddened's rig and cannot sit: the canteen's two and the meal corner stand or crouch until the Barney-rig
crew exist. Everyone answers a use press with one printed line from the lines file; nothing is voiced
until Andrei records.

## Names, for J.A.C.K.

Every brush entity has a `targetname` that says where and what: `tower_w`, `skip_n`, `t1_rail_e_s`,
`t2_bridge_rail_n`, `lab_door`, `lab_window_w`, `mag_gate`, `garage_gate`, `store_fence_n`, `lift1`,
`lift2_call_hi`, `portal_door`, `cl_in`, `save_oldway`. The two doors that open on touch (the airlock's
inner door, the refuge door) are unnamed on purpose: a named `func_door` stops answering touch. Lights
are never named: a named light is a switchable one and there are 64 styles in the engine. Rooms are
world brushes and cannot be named, so the generator drops an `info_target` called `room_<NAME>` just
inside each room's minimum corner; go-to-entity on it lands you in the room.

## Facts that bind this map

- The cage is a `func_door` placed at the floor with `lip -2288`, *Starts Open* and *Toggle*
  (spawnflags 33), so it spawns at the top, its first trigger brings it down, `wait -1` holds it there,
  and the next trigger takes it up again. The button (master `ride_ok`) sends it up with the player;
  nothing else moves it. Without Toggle a wait -1 door never answers a second trigger: the lifts' fault.
- The suit gate needs no code: `item_suit` fires its `target` on pickup, lighting the multisource
  `suit_on` that masters the inner door.
- Two leak lessons from its first compile, now in MAP_WORKFLOW: a logic entity above a low room's
  ceiling, and a landmark at exactly floor height, both leak.
- The two bridge slabs are world brushes and split the chamber for VIS; `func_detail` is the better
  entity for them and is offered, not applied.
- Lit with point lights until the emitting-texture pass; RAD not yet run on it.
