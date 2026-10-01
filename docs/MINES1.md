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

**Superseded in part on 2026-09-30** by "Ore out" below: the ore bunker, the picking belt, road B, the
waste hopper, the tip, the stope, the cage siding and the skip as counterweight are gone.

### Ore out — settled 2026-09-30, track built, the rest not

Andrei's export of 2026-09-30 added the west annex (the yard out to x −1632, a switchback stairwell
floor → tier 1 → tier 2 at x −1632..−1376, y 288..864, the sump under the fan room) and the north side
(the yard and the hall out to y 1280; on tier 1 a room at x −1120..352, y 864..1280, 192 high, beside
the lab and open to the north ledge along its length). A processing line on tier 1 was worked out the
same day and dropped: **mines1 shows extraction only.** The ore leaves the map on a belt and is
processed in the next map, *Residue Processing*, not built and closed for now. Andrei: "introducing too
many elements in the first map will keep me stuck into it for a month." **`maps/mines1.plan_process.png`**
draws it.

- **Track, as Andrei laid it on 2026-09-30 (the hour 21:10–22:10 and after, export of 23:30; rails not
  yet refined).** A **loop**: the main line at y −400 and a second line at y 400, joined by curved ends
  west of the cage (straight at about x −1290) and at about x 540, the curves built in 2:1 segments.
  Wagons run round to the bin without reversing. The main line runs east from the loop's east curve to
  the portal; it stops at x 1122 today and is to reach the portal door and on into the haulage drift.
- **The turntable** sits in the main line, centre about (−178, −400), deck about 260 across, and feeds
  **three garage roads** south: one curving south-west to x about −610, one straight at x −182, one 45°
  south-east to x about 255, each straight south into the garage. This supersedes "no turntables" in
  Traffic: small wagon turntables are what narrow-gauge mines used to reach roads at right angles. Each
  road crosses the garage fence at y −656 and wants a gate: the fence's one gate is at x 704..832.
- **The loco**, set up 2026-10-01 at Andrei's request, not verified in game: `crashtrain`, `dmg` 5 so it
  shoves rather than kills, the `globalname c2a1_train` copied from Valve's map removed, speed 80.
  **It moves only when the player drives it or a script starts it** (Andrei). Parked at spawn
  (`startspeed 0`); driven from the cab well through a `func_traincontrols` box (x −662..−602,
  y −456..−344, z 24..104), throttle in Valve's five notches; the relays `crash_go` / `crash_park` stay
  for scripts. Its path is the loop clockwise, `mt01`..`mt16` (facing west, as built), or anticlockwise,
  `ma01`..`ma16`, once the turntable has turned it round. Its ORIGIN brush sits at its rear (x −544 on
  a body −768..−512), so it pivots there on curves and on the table; moving it to the middle is offered.
  Its two loaded wagons stand on the second line and are left for Andrei: the loco passes through them.
- **The turntable, working**: a `func_turntable` (new, `dlls/plats.cpp`): `func_trackchange` for any number of
  tracks. `turntable`, about its ORIGIN brush at (−176, −400), `startyaw 45` so home is the main line.
  A valve (`func_rot_button`, Andrei's to place) turns it one stop per use, always anticlockwise:
  home → SW road → S road → SE road → turned round (180°) → home. A loco standing still with its
  origin within 100 (its `wheels`) of the pivot is **carried**, and put on that stop's track: `mt15`
  (home), `sw00`, `s00`, `se00` (each road's chain starts at the pivot with nothing before it, so a
  loco reversing out of a garage stops on the table), `ma01` (turned round). While the deck is off the
  main line or turning, the gates `mt14` and `ma16` are Disabled, so a loco coming at the table finds
  the end of the line (at `mt13` from the east, `ma15` from the west). With the loco over its swing,
  or moving closer than those, it will not turn and sounds Valve's alarm. **Seen in game 2026-10-01:**
  Andrei's valve (copied from a stock map) turns it, after a fix to the in-the-way test, which had
  measured the loco by the world box the engine grows round a turned brush entity. Not yet tried:
  driving, carrying, the roads, the gates, the alarm under a moving loco. The valve that turns on
  45° a use instead of winding back is an Idea in ROADMAP.md, "The step valve".
- **Remove:** the old garage road at x 720..816, which meets the main line at 90°, and its gate if no
  road uses it; the loaded car in the hall (x 508..564).
- **Gauge:** the new roads match the main line (rails 32 wide, centres 96 apart); the old garage road's
  16-wide rails go with it.
- **The bin.** On legs between the second line and the belt's tail, about x 180..276, y 480..560: rim
  about 56, so a wagon on the second line side-tips north into it. Its throat feeds the belt.
- **The belt**, as built: a flat `func_conveyor` at x 196..260 from y 560 north under the ledge and the
  tier-1 room to the north wall. Through the wall at x 180..276, a short tunnel with a strip curtain
  across it and a sealed end behind; the belt is the only way the ore leaves.
- **No waste in mines1.** The old stope box west (x −2488..−1664, y −656..112) goes, and with it the tip.
- **The hall** keeps the sample hoist, the Heart's route to the lab above; the loaded car, the bunker,
  the sorting benches, the old belt to the hoist and the store leave it. **The lab** receives the sample.
- **The processing door.** West of the belt, in the north wall at x 96..160, a steel door
  to processing that never opens: a `func_door` with a `targetname` and nothing targeting it, which
  plays its `locked_sound` on touch and never moves (`CBaseDoor::DoorTouch`). Behind it a sealed
  vestibule, 64 deep. The belt and the door read together as "processing is through there".
- **The tier-1 room beside the lab** is a **store room**: pallets, crates, drums, a gas-bottle rack,
  from props and prefabs already in hand; one or two of the crew.
- **The skip goes**, compartment, triggers and shaft tube. The crib and the fan room stay where they are.

Next: vanilla references for the bin and the belt, then the build in J.A.C.K.

### Steel — proposed 2026-09-27, not built

Andrei's lattice pillar (`{truss_*` in `topmod.wad`, 2026-09-27) wanted a place. The rock holds itself
up: a chamber this size stands on its walls, rock left in place and roof bolts, and a steel column
1536 tall holding a roof would read as dressing. So the steel carries machines and people only.
`maps/mines1.plan_steel.png` draws it over the traffic plan:

- **A, the cage headframe:** truss posts at the corners of the cage and skip compartment and two on
  their shared wall, 0 to 1536; the mesh stays between. The chamber's one full-height column.
- **B and C, the lift towers:** four posts round each platform, `lift1` 0 to 640 and `lift2` 512 to
  1152, a head over the top landing; panels only on the faces not against rock or used as entry.
- **D, the tier-1 bridge:** four legs down to the yard at y ±256, clear of the main line.
- **E, the tier-2 bridge:** hung from the roof on twelve rods, since the main line runs under it.
- **F, the ore bunker and waste hopper:** four short legs each, cars beneath; when they are built.

Offered with it and not taken yet: roof bolts and mesh where people work, one or two salt pillars left
standing in the yard, steel arch sets in the drifts. Build tip: a column's faces as thin separate
brushes, since a box shows only its near side through the mask.

### The cage landing — session card, 2026-09-28

The corner to finish first, as the style reference: x −1184 to −560, y −460 to 560. The card
(https://claude.ai/artifact/9rU347qsdsgLcdqwr4mSyw) has the corner's plan, a palette measured from Valve's
three landings of the same lift (`floathumanlift` in `c2a2e` and `t0a0`, the grunt lift in `c2a2b1`), the
`setpos` lines to go and look at them, and the done-when list. The cage is textured like Valve's
`floathumanlift` and targets `floathumanlift1`. The onsetter's hut and the tag board are in the spec but not
in this corner.

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
