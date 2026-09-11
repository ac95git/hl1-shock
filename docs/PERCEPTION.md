# Perception — how monsters find the player

The reference for everything a monster knows and how it comes to know it. Read it before touching
`Look`, `Listen`, the sound list, the monster state machine, or anything that decides whether the player
has been noticed.

Two halves. **[Part 1](#part-1--what-half-life-does-today)** documents the base SDK exactly as it is, with
no changes proposed — it is the thing being built on, and most of it is undocumented anywhere else.
**[Part 2](#part-2--the-model-this-mod-adds)** is the model this mod layers on top, settled 2026-08-31.
Concealment, Suspicion, the Perception Profile and the Backstab are **built**; de-escalation, the Search,
the Post, the squad channel, the Disturbance, the readout and the noise multiplier are not. Each section
below says which it is.

What is intended and unbuilt is tracked in [ROADMAP.md](ROADMAP.md#pillar-6-stealth); what exists today is
in [PILLARS.md](PILLARS.md#6-stealth). Vocabulary is in [CONTEXT.md](../CONTEXT.md) — **Concealment**,
**Suspicion**, **Search**, **Post**, **Perception Profile**, **Backstab**.

**Last updated:** 2026-09-02 (branch `hl-shock` — **everything before acquisition is built**: Concealment,
Suspicion, the Perception Profile, the noise multipliers. Everything after acquisition is deferred to
[the post-aggro step](ROADMAP.md#the-post-aggro-step). The readout is next)

---

## Part 1 — what Half-Life does today

### Where perception runs

`CBaseMonster::RunAI()` (`dlls/monsterstate.cpp:61`) calls `Look(m_flDistLook)` and `Listen()` once per
monster think — but **only when a client is in the monster's PVS, or the monster is already in combat**
(`dlls/monsterstate.cpp:82`). A monster in an unvisited part of the map perceives nothing and costs nothing.
That gate is why adding work inside `Look` is affordable.

### Sight — `CBaseMonster::Look`

`dlls/monsters.cpp:298`. Collects entities and sets the sight bits of `m_afConditions`.

It gathers with `UTIL_EntitiesInBox(pList, 100, ...)` filtered to `FL_CLIENT | FL_MONSTER`
(`dlls/monsters.cpp:317`) — a box of side `2 × iDistance`, **not limited to PVS**, capped at 100 entities.
A monster flagged `SF_MONSTER_PRISONER` sees nothing at all (`:310`), and nothing with `health <= 0` is
ever considered (`:324`).

Each candidate then passes exactly **four** tests and nothing else (`dlls/monsters.cpp:328`):

| Test | Meaning |
| --- | --- |
| `IRelationship(pSightEnt) != R_NO` | there is some relationship, hostile or otherwise |
| `FInViewCone(pSightEnt)` | inside the forward cone, width `m_flFieldOfView` |
| `!FBitSet(pSightEnt->pev->flags, FL_NOTARGET)` | the `notarget` cheat is off |
| `FVisible(pSightEnt)` | an unobstructed trace |

There is no light term, no stance term, no speed term, and no time term. Sight is instantaneous and binary.

What a passing entity produces:

- It is pushed onto the `m_pLink` list, which `BestVisibleEnemy()` (`:2418`) later walks.
- If it is the player, `bits_COND_SEE_CLIENT` is set (`:351`) — used by scripted AI, and by
  `SF_MONSTER_WAIT_TILL_SEEN`, which additionally requires the *player* to be facing the monster before it
  activates (`:332-348`).
- If it is already `m_hEnemy`, `bits_COND_SEE_ENEMY` is set (`:357-361`).
- Its relationship sets one of `bits_COND_SEE_NEMESIS` / `SEE_HATE` / `SEE_DISLIKE` / `SEE_FEAR`
  (`:365-384`). `R_AL` sets nothing.

**`SEE_HATE`, `SEE_DISLIKE` and `SEE_NEMESIS` are the bits that lead to acquisition.** Everything else in
that list is presentation, scripting, or tracking of an enemy already held.

Range defaults are `m_flDistLook = 2048` and `m_flDistTooFar = 1024` (`dlls/monsters.cpp:2032-2033`).

Field of view varies far more than the range does, and it is already the sharpest per-monster difference in
the game. `m_flFieldOfView` is a dot product, so **lower is wider**:

| Value | Angle | Monsters |
| --- | --- | --- |
| `-1` | 360° | nihilanth |
| `VIEW_FIELD_FULL` | 360° | controller, turret family |
| `-0.707` | 270° | apache |
| `-0.5` | 180° | leech |
| `-0.2` | ~203° | gargantua |
| `0` | 180° | osprey, snark |
| `0.2` | ~157° | human grunt, alien grunt, bullsquid, flyer |
| `0.3` | ~145° | big momma |
| `VIEW_FIELD_WIDE` | wide | assassin, alien slave, barney, scientist, ichthyosaur |
| `0.5` | 120° | zombie, headcrab, houndeye, barnacle, bloater, gman, roach, rat, the player |
| `0.9` | ~50° | hornet |

### Acquisition — `CBaseMonster::GetEnemy`

`dlls/monsters.cpp:3334`. Runs only when `bits_COND_SEE_HATE | SEE_DISLIKE | SEE_NEMESIS` is set (`:3338`),
takes `BestVisibleEnemy()`, and swaps `m_hEnemy` **only if the current schedule can be interrupted by
`bits_COND_NEW_ENEMY`** (`:3348-3356`) — a deliberate Valve guard, commented as not a good permanent fix.
The previous enemy is pushed onto a small stack (`PushEnemy` / `PopEnemy`, `:1166`) so a monster returns to
an older target when the current one is gone.

So the whole chain is: `Look` sets a relationship bit → `GetEnemy` sets `m_hEnemy` → `GetIdealState`
promotes to `MONSTERSTATE_COMBAT`. **One frame, start to finish.**

### Tracking an acquired enemy — `CBaseMonster::CheckEnemy`

`dlls/monsters.cpp:1052`. Runs against `m_hEnemy` and maintains:

- `bits_COND_ENEMY_OCCLUDED` from `!FVisible` (`:1060-1066`).
- `bits_COND_ENEMY_TOOFAR` beyond `m_flDistTooFar` (`:1130-1136`).
- `bits_COND_ENEMY_FACING_ME` (`:1103-1108`).
- `m_vecEnemyLKP`, the last known position — updated when the enemy is seen, and *trailed slightly behind*
  by its velocity so the monster aims where it was rather than where it is (`:1111-1115`). Also updated when
  the enemy is unseen but unoccluded within 256 units, on the reasoning that it must be beside or behind the
  monster (`:1121-1128`).

`CSquadMonster::CheckEnemy` (`dlls/squadmonster.cpp:387`) additionally pastes the LKP to the squad leader
when it is fresh, and copies it back from the leader when it is not.

### Aim does not come from facing

Worth stating on its own, because it is invisible until stealth makes it visible and it looks like a bug
when it appears.

A monster's shot direction is `ShootAtEnemy` (`dlls/monsters.cpp:3234`), which aims from the gun position at
`m_vecEnemyLKP`. **`pev->angles` does not enter into it.** `CHGrunt::Shoot` (`dlls/hgrunt.cpp:788`) is
typical: it calls `UTIL_MakeVectors(pev->angles)` only to throw the shell casing, and `SetBlending(0, ...)`
blends **pitch** alone — there is no yaw blend and no check that the target is in front.

So a monster with a live LKP fires at full accuracy through its own back while its model faces elsewhere.
The turn is cosmetic; only the LKP is real.

Two things feed a monster an LKP it did not see:

- the *"behind or beside"* clause in `CheckEnemy` (`:1165`) — enemy within 256 units, unoccluded and outside
  the view cone, and the monster is simply handed the enemy's exact origin. It is an anti-cheese hack
  against standing where a monster cannot see you, and it sets `iUpdatedLKP`, so in a squad that position is
  then **pasted to every squadmate**.
- `SquadCopyEnemyInfo`, which is squad information sharing and legitimate.

This mod withholds the first for the player; see part 2.

### The state machine — `CBaseMonster::GetIdealState`

`dlls/monsterstate.cpp:118`.

| From | To | On |
| --- | --- | --- |
| IDLE | COMBAT | `bits_COND_NEW_ENEMY` (`:141`) |
| IDLE | ALERT | light or heavy damage, or a `bits_SOUND_COMBAT`/`bits_SOUND_DANGER` sound (`:143-165`) |
| IDLE | ALERT | a smell (`:166-169`) |
| ALERT | COMBAT | `NEW_ENEMY` or `SEE_ENEMY` (`:180-184`) |
| ALERT | ALERT | any heard sound — turns to face it and nothing more (`:185-192`) |
| COMBAT | ALERT | **`m_hEnemy == NULL`, and nothing else** (`:201-206`) |

That last row is the important one. **A monster in combat never de-escalates while its enemy exists.**
Losing sight, losing the trail, and the player leaving the area all do nothing. There is no give-up path in
the base game.

Note also what an IDLE monster does on hearing a plain player noise: `MakeIdealYaw` toward it (`:161`), and
the state change is gated on `COMBAT|DANGER`, which `bits_SOUND_PLAYER` is not. **It turns and that is all.**

`CSquadMonster::GetIdealState` (`dlls/squadmonster.cpp:518`) adds one thing: on `NEW_ENEMY` in IDLE or
ALERT, it calls `SquadMakeEnemy`, handing the enemy to every squad member not already engaged.

### The player's noise — `CBasePlayer::UpdatePlayerSound`

`dlls/player.cpp:2586-2666`, run every frame. It is complete, correct, and nothing rewards or punishes it.

- **Body volume** is `pev->velocity.Length()`, clamped at 512 (`:2588`, `:2592`). Moving slower is already
  quieter, so crouching (`PLAYER_DUCKING_MULTIPLIER` 0.333, `pm_shared/pm_shared.cpp:104`) and walking
  (⅓ speed, `pm_shared/pm_shared.cpp:2949`) already reduce it — as a side effect of speed, not because
  anything decided they should.
- **Airborne is silent**; a jump adds 100 (`:2599`, `:2602`).
- **Weapon volume competes** with body volume and the loudest wins; if the weapon wins, the sound is
  additionally flagged `bits_SOUND_COMBAT` (`:2608-2614`). The scale is `dlls/weapons.h:415-417` —
  `LOUD_GUN_VOLUME` 1000, `NORMAL_GUN_VOLUME` 600, `QUIET_GUN_VOLUME` 200.
- **Volume decays toward its target** rather than snapping, so a monster that listens infrequently still
  catches a noise that has already stopped (`:2638-2646`).
- **`m_fNoPlayerSound`** (`dlls/player.h:175`) zeroes it outright (`:2648`). Its own comment calls it a
  debugging feature. It is a working silent-movement switch that nothing turns on.
- `bits_SOUND_PLAYER` is OR'd in every frame (`:2664`). Fall damage inserts one separately (`:2759`).

### Hearing and smelling — the sound list

`CSoundEnt` holds up to **`MAX_WORLD_SOUNDS` = 64** entries for the whole world (`dlls/soundent.h:24`) —
a shared, small pool. `InsertSound(type, origin, volume, duration)` adds one with a lifetime.

Types are single bits (`dlls/soundent.h:26-33`), and **bits above `1 << 6` are free**:

| Bit | Type | Classified as |
| --- | --- | --- |
| `1 << 0` | `bits_SOUND_COMBAT` | sound |
| `1 << 1` | `bits_SOUND_WORLD` | sound |
| `1 << 2` | `bits_SOUND_PLAYER` | sound |
| `1 << 3` | `bits_SOUND_CARCASS` | **scent** |
| `1 << 4` | `bits_SOUND_MEAT` | **scent** |
| `1 << 5` | `bits_SOUND_DANGER` | sound |
| `1 << 6` | `bits_SOUND_GARBAGE` | **scent** |

The split is `CSound::FIsSound()` and `CSound::FIsScent()` (`dlls/soundent.cpp:52-73`). It matters: only a
sound sets `bits_COND_HEAR_SOUND` and only a sound is returned by `PBestSound()` (`dlls/monsters.cpp:409`,
which returns the **nearest**, not the loudest). Scents set `bits_COND_SMELL` / `bits_COND_SMELL_FOOD`
instead.

`CBaseMonster::Listen()` (`dlls/monsters.cpp:192`) walks the active list. A sound registers if its type is
in the monster's mask **and** in the current schedule's `iSoundMask` (`:209` — the two must agree, and the
code says so in capitals), and if the monster is within `m_iVolume * HearingSensitivity()` (`:224`).

The base sound mask is `WORLD | COMBAT | PLAYER` (`dlls/monsters.cpp:398-403`), and most monsters keep
`bits_SOUND_PLAYER` — grunts (`dlls/hgrunt.cpp:322`), alien slaves (`dlls/islave.cpp:265`), bullsquids
(`dlls/bullsquid.cpp:419`), houndeyes, alien grunts, assassins.

**What hearing the player actually causes** is worth stating plainly, because two other documents got it
wrong: a `MakeIdealYaw` toward the noise, and nothing else. Grunts react strongly to `bits_SOUND_DANGER`
(`SCHED_TAKE_COVER_FROM_BEST_SOUND`, `dlls/hgrunt.cpp:1966-1981`) — that is the grenade response — but the
block that would face them toward player noise is **commented out** (`dlls/hgrunt.cpp:1983-1988`).

`SCHED_GRUNT_SWEEP` is not a sweep. It is `TURN_LEFT 179, WAIT 1, TURN_LEFT 179, WAIT 1`
(`dlls/hgrunt.cpp:1594-1619`) — a look-around-in-place, reached from `SCHED_GRUNT_COMBAT_FACE` after 1.5
seconds of facing an enemy.

### A monster that fails schedules throws sparks

Not perception, but it is how a perception bug first shows itself, so it belongs here.

`FScheduleValid` (`dlls/schedule.cpp:179-191`) has a Valve debug aid: when a task fails and
`m_failSchedule == SCHED_NONE`, it emits `UTIL_Sparks` above the monster's head. **It is inside `#ifdef
DEBUG`**, so it appears in this mod's Debug builds and never in a Release one.

Sparks therefore mean *"this monster is failing schedules"* — a symptom, never a cause. The usual cause is a
pathing or cover task that cannot succeed, and the usual consequence is `SCHED_FAIL`
(`TASK_STOP_MOVING, ACT_IDLE, TASK_WAIT 2, TASK_WAIT_PVS`, `dlls/defaultai.cpp:30`), which looks exactly
like the monster has become unresponsive.

The grunt has a reachable version of this loop with no exit: `ENEMY_OCCLUDED` →
`SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE` → `TASK_GET_PATH_TO_ENEMY` fails → `SCHED_GRUNT_ELOF_FAIL` →
`SCHED_TAKE_COVER_FROM_ENEMY`, whose schedule (`dlls/hgrunt.cpp:1493`) sets **no** fail schedule — so a
failed `TASK_FIND_COVER_FROM_ENEMY` sparks, drops to `SCHED_FAIL`, waits, and starts again. In between
successful cover searches it can walk to a node 384 units away, which is how a "stuck" grunt also manages to
wander out of the room.

In the base game this is rare because nothing makes losing the player a normal event. Under this mod it is
the *point*, which is why [de-escalation](#losing-the-player--de-escalation-search-post) is not optional.

### Light — `Illumination()`

`CBaseEntity::Illumination()` is `GETENTITYILLUM(ENT(pev))`, the engine's light level at the entity
(`dlls/cbase.h:360`).

`CBasePlayer::Illumination()` **overrides it** to add `m_iWeaponFlash`, clamped to 255
(`dlls/player.cpp:4567-4575`). `m_iWeaponFlash` is a `CBasePlayer` member (`dlls/player.h:113`), saved
(`:106`), set by every player gun to `BRIGHT`/`NORMAL`/`DIM_GUN_FLASH` = 512/256/128
(`dlls/weapons.h:419-421`), and decayed at 256 per second (`dlls/player.cpp:2669`). So firing lights the
player for roughly a second afterwards, at no cost to whoever reads it.

**Nothing calls `Illumination()`.** `Look` does not consult light in any form. The darkness half of
concealment is a finished, correct query with no consumer.

Note the asymmetry: monsters use the base implementation, so **a monster firing a weapon illuminates
nothing**. See [Deliberately not generalised](#deliberately-not-generalised).

### Squads — `CSquadMonster`

`dlls/squadmonster.h` / `.cpp`. Up to `MAX_SQUAD_MEMBERS` = 5 (`dlls/squadmonster.h:54`), formed at
`StartMonster` via `SquadRecruit(1024, 4)` (`:430`) among monsters of the same `Classify()`, either by
`pev->netname` or by proximity plus a clear trace.

State held **by the leader** on behalf of the squad:

| Field | Saved | What it is |
| --- | --- | --- |
| `m_afSquadSlots` | **no** — commented *"these need to be reset after transitions!"* (`:35`) | which attack slots are taken |
| `m_vecEnemyLKP` | yes (on `CBaseMonster`) | shared last known position, via `SquadPasteEnemyInfo` / `SquadCopyEnemyInfo` (`:215-235`) |
| `m_flLastEnemySightTime` | yes (`:37`) | last time **anyone** in the squad saw the enemy |
| `m_fEnemyEluded` | yes (`:36`) | the squad has lost the enemy |

`m_fEnemyEluded` and `m_flLastEnemySightTime` are declared on the base class and **only the grunt uses
them** — set true after 5 seconds unseen (`dlls/hgrunt.cpp:377`), consumed once to trigger a "found him!"
callout when a member relocates the player and the player is not facing them (`:2104-2108`). A squad-level
lost-track flag, sitting unused on `CSquadMonster`.

`SquadMakeEnemy` (`:243`) hands an enemy to every member not already engaged, pushing their old enemy onto
the stack first. `SquadMemberInRange(vec, 128)` is the spacing rule `FValidateCover` uses to stop members
piling onto the same cover (`:544-558`, `:588`).

**Killing the leader dissolves the squad.** `SquadRemove` on the leader nulls every member's
`m_hSquadLeader` (`:145-181`) and there is no promotion anywhere in the SDK. The survivors keep fighting
individually but can no longer share enemy information, coordinate slots, or be given an enemy by anyone.

### Death and corpses

**Nothing perceives a death.** `CBaseMonster::Killed` (`dlls/combat.cpp:588`) notifies only `pev->owner`,
for monstermaker bookkeeping. `CSquadMonster::Killed` (`dlls/squadmonster.cpp:124`) vacates its slot,
removes itself from the squad, and calls the base — **`SquadRemove` runs before `CBaseMonster::Killed`**,
so anything wanting to notify squadmates must do so before that line or the member list is already gone.

No sound enters `CSoundEnt` on death. The one death-time insert is `bits_SOUND_CARCASS` at volume 384 for
30 seconds (`dlls/schedule.cpp:475`), which is classified as a **scent** and appears only in scavenger sound
masks (bullsquid, houndeye) and in the friendly-NPC masks. Grunts do not listen for it.

**Corpses are invisible.** `Look` skips anything with `health <= 0` (`dlls/monsters.cpp:324`). A body left
in the middle of a lit corridor is never noticed by anyone, ever.

### A monster's guard position

`m_vecLastPosition` (`dlls/basemonster.h:71`) is commented *"monster sometimes wants to return to where it
started after an operation"* and is in the save table (`dlls/monsters.cpp:78`). An unused guard-post slot.

`SCHED_GUARD` is a declared common schedule (`dlls/schedule.h:64`) that **only the houndeye implements**
(`dlls/houndeye.cpp:1206`).

A monster with `pev->target` walks a `path_corner` chain while idle (`dlls/monsters.cpp:2096-2131`, via
`SCHED_IDLE_WALK`), advanced through `GetNextTarget()` (`:1458-1460`). So an authored patrol route is
already expressible in map data, and repointing `m_pGoalEnt` is how it is changed.

### What survives a save, and what survives a level change

`CBaseMonster::m_SaveData` (`dlls/monsters.cpp:47-104`) saves `m_MonsterState`, `m_IdealMonsterState`,
`m_afConditions`, `m_hEnemy`, `m_hOldEnemy[]`, `m_vecEnemyLKP`, `m_vecLastPosition`, `m_afMemory`,
`m_flFieldOfView`, `m_flDistLook` and `m_flDistTooFar`. Routes and schedules are **not** saved.

`CBaseMonster::Restore` (`:114-135`) already scrubs: clears the route, nulls the schedule, resets
`m_iTaskStatus` and the activity, and clears `m_afConditions` entirely if there is no enemy. So there is
precedent for restore-time correction of AI state.

**Save/load and level change run through the same `Restore`.** They are distinguished by
`SAVERESTOREDATA::fUseLandmark` (`engine/eiface.h:344`), which is non-zero for a landmark transition and
zero for a plain save load — and `CBasePlayer::Restore` already reads it (`dlls/player.cpp:3193`).

Consequence worth knowing: an `EHANDLE` to the player does not survive a transition, so a transition-carried
monster comes back with `m_hEnemy` null, its conditions cleared by `Restore`, and `GetIdealState` drops it
from COMBAT to **ALERT** on the next think. It does not reach IDLE on its own.

### Two corrections

Both [PILLARS.md](PILLARS.md) and [ROADMAP.md](ROADMAP.md) previously stated that grunts investigate player
noise when they cannot see their enemy, citing `dlls/hgrunt.cpp:1984`. **That code is inside a `/* */`
block** (`:1983-1988`) and has never run. Corrected in both, 2026-08-31.

Neither document recorded that deaths and corpses are entirely imperceptible, which is load-bearing for
anything built on stealth. Recorded here.

---

## Part 2 — the model this mod adds

Settled 2026-08-31. The two decisions with lasting consequences have their own records —
[`adr/0009`](adr/0009-suspicion-gates-the-relationship-bits.md) for where the meter gates,
[`adr/0010`](adr/0010-the-backstab-is-positional.md) for the Backstab.

### The shape in one paragraph

**Concealment** is what the player *is*. **Suspicion** is what a monster *holds*. Concealment sets the rate
at which Suspicion fills; it does not decide whether it fills. When a monster's Suspicion reaches the
acquisition threshold, everything below that line happens exactly as Half-Life already does it.

### Concealment — built 2026-09-01

`CBaseMonster::ConcealmentOf(CBaseEntity*)` (`dlls/perception.cpp`). Returns 0 (fully exposed) to 1
(invisible). Internally it multiplies four **exposure** fractions and returns what is left over, so the
name the code carries and the name the design carries are complements of each other.

Four terms, computed per monster/player pair:

| Term | Source | Effect |
| --- | --- | --- |
| Term | Source | Worst-case exposure | Effect |
| --- | --- | --- | --- |
| Angle | dot product against the monster's **own** `m_flFieldOfView` | `conceal_angle_edge` 0.25 | the rim of the cone is far slower than the centre |
| Distance | fraction of the monster's **own** `m_flDistLook` | `conceal_dist_far` 0.25 | far is slower than near |
| Stance | `FL_DUCKING`, else speed against `pev->maxspeed` | `conceal_stance_duck` 0.4, `conceal_stance_walk` 0.7 | crouched is slower; slow is slower |
| Light | `Illumination()` | `conceal_light_dark` 0.5 | dark is slower |

Each cvar is that term's exposure at its *worst* end. **None of them is ever zero**, and that is a rule
rather than a default: a term that could reach zero would zero the product and make stealth absolute, which
is a bug and not a build.

Both the angle and the distance term are scaled to the individual monster's own cone and range, so the
per-monster FOV table in part 1 — already the sharpest difference between monsters in the game — carries
straight through into how hard each one is to sneak past, without a second table having to say so.

The stance term treats **standing still as walking**: a player who has stopped moving is not the one giving
themselves away. Speed is compared against half of `pev->maxspeed`, which puts vanilla walk (⅓ speed) and
crouch-walk comfortably under the line and a run comfortably over it.

The muzzle-flash term arrives free, because `CBasePlayer::Illumination()` already includes it — firing in
the dark lights the player for about a second and nobody has to write that rule.

Angle, distance and stance carry the first version, so the model is tunable in vanilla Half-Life maps.
Light is wired from the first commit and contributes, but is deliberately the mildest of the four, because
**vanilla maps are lit for readability rather than for hiding** — it becomes the dominant lever only when
there are custom maps with dark places in them. That dependency is tracked under [Maps](ROADMAP.md#maps).

### Suspicion — built 2026-09-01

`CBaseMonster::m_flSuspicion`, one float per monster, 0 to 1, **saved** — a quickload is not a "calm
everyone down" button. `m_flSuspicionTime` is saved alongside it as a `FIELD_TIME` so the engine rebases it
across a transition.

It is advanced by `UpdateSuspicion`, called **exactly once per `Look`** whether or not the player is in
sight — a meter that only moved while the player was visible would never drain. It fills at
`(1 - Concealment) × suspicion_fill × profile.flFillScale` per second while a hostile monster can see the
player, and drains at `suspicion_drain × profile.flDrainScale` when it cannot.

Two thresholds: `suspicion_notice` (0.35), which drives the player's readout and squad chatter, and
`suspicion_acquire` (1.0), at which the monster becomes hostile exactly as it does today.

Three cases short-circuit the meter, in this order:

| Case | Behaviour |
| --- | --- |
| `suspicion_enable 0`, or a profile with `bUsesSuspicion` false | pinned full, acquisition allowed — vanilla, restored exactly |
| `SF_MONSTER_IGNORE_CONCEALMENT` (1024) | pinned full — the mapper said this set piece has to fire |
| `m_pCine != NULL` | **frozen**, not filled: performing, not perceiving. The gate answers from whatever the meter already holds, so when the script releases the monster the model resumes from where it left off |

And one more, which is not a special case so much as a boundary: while `m_hEnemy` **is** the player the
meter is pinned full. Dropping an enemy is de-escalation, which this does not own; the meter must not
quietly become a give-up timer under a monster that is actively shooting.

### Where it gates — [adr/0009](adr/0009-suspicion-gates-the-relationship-bits.md)

Inside `Look`, and it gates **only the relationship bits** — `bits_COND_SEE_HATE`, `SEE_DISLIKE`,
`SEE_NEMESIS` — which are the bits `GetEnemy` reads. Scoped further to `pSightEnt->IsPlayer()`, so every
other hostile is still acquired the instant it is seen and a friendly monster's meter is never touched at
all.

Untouched, deliberately: `bits_COND_SEE_CLIENT`, the `m_pLink` list, `SF_MONSTER_WAIT_TILL_SEEN`,
`bits_COND_SEE_FEAR`, and `bits_COND_SEE_ENEMY` for an enemy already acquired. So Barney still says hello,
scripted AI still receives the condition it expects, fleeing still works, and an alerted monster still
tracks the player at full speed. The change is surgical because everything downstream of acquisition is left
alone. The ADR has the four rejected placements and what each would have broken.

### The debug view — built 2026-09-01

`debug_suspicion 1` centre-prints the four highest live meters, four times a second, with a bar, the raw
value, the Concealment that produced it, and a `NOTICED` marker past `suspicion_notice`:

```
hgrunt         [======....] 0.62  cnc 0.38  NOTICED
zombie         [=.........] 0.08  cnc 0.71
```

Built alongside the meter rather than after it, because every number above is a first guess and a meter
nobody can see is a meter nobody can tune. It shares the screen centre with `debug_damage`, so the two
should not be run together.

### Noise steers the cone; it does not fill the meter

Sound keeps its own path. Hearing the player turns a monster toward the noise — which is already what
happens — and the commented-out grunt investigate block is restored so a loud noise also draws them to
walk to it. Turning to face the player collapses the angle term, so the fill rate jumps.

**The quiet half — built 2026-09-02.** That turn is the mechanic working, but it needs something to work
*against*, and until now there was nothing. `UpdatePlayerSound` made a crouching player quieter only as a
side effect of being slower, which left a crouched approach at roughly 107 volume — and since `Listen`
hears a sound out to its volume in units (`dlls/monsters.cpp:231`) while the crowbar reaches about 32, a
crouched player could never get close enough to Backstab anything without turning it around first. The
approach the Backstab exists for was impossible.

`noise_stance_duck` (0.3) and `noise_stance_walk` (0.6) scale the **body** volume in `UpdatePlayerSound`,
giving a crouched approach an audible radius of about 32 units instead of 107. They deliberately do not
touch `m_iWeaponVolume`: firing is exactly as loud crouched as standing, because a quiet *weapon* is the
silencer, which is [deferred to Evolutions](ROADMAP.md#deliberately-deferred).

Two properties of the base model that this inherits and that make it behave better than the multiplier
alone suggests: a **stationary** player makes no body noise at all (volume is velocity), and volume decays
at 250/sec rather than snapping, so stopping goes quiet within a fraction of a second while a monster that
listens infrequently still catches a noise that has already ended.

**Sight remains the only thing that fills Suspicion.** That keeps "break line of sight and they stop
learning about you" true without exception, which is the rule the whole mechanic has to be readable
through. There are two exceptions — the Disturbance, below, and damage.

### Damage fills the meter outright — built 2026-09-01

Not in the settled design, because the design did not anticipate what the base game does here.
`CBaseMonster::TakeDamage` sets `m_vecEnemyLKP` and turns the monster toward the attack, but **it never sets
`m_hEnemy`** (`dlls/combat.cpp:998-1017`). Acquisition comes only through `Look` and `GetEnemy`. So with the
gate in place and nothing else, a monster the player shot would stand and take it for however long its meter
needed — measured, in a dark room at range, in whole seconds.

`SuspicionFromDamage` fills the meter to full for player-dealt damage, called from that same block. Being
shot is proof, on the same reasoning that admits the Disturbance: a body is proof, and so is a bullet.

Player-dealt only, for the same reason the sight gate is scoped that way — every other hostile is acquired
instantly regardless.

### Everything after acquisition is deferred, deliberately

**Nothing below this line is built, and the boundary is enforced in one place:** `UpdateSuspicion` pins the
meter to 1.0 the moment `m_hEnemy` is the player and returns. Past that point the meter has no effect on
anything, so combat is byte-for-byte the base game.

That is a property worth keeping rather than an accident. It means every combat complaint has exactly two
possible causes — vanilla, or the fact that this mod lets you reach vanilla states that Half-Life never
expected you to reach — and never a third.

The post-aggro work was attempted on 2026-09-02 and reverted the same day. What it found is recorded in
[ROADMAP.md](ROADMAP.md#the-post-aggro-step) rather than here, because it is a plan and not a description
of the code. The short version: **aim is not facing**, the LKP has four writers, and the right seam is
`ShootAtEnemy` rather than the four writers.

### Losing the player — de-escalation, Search, Post

**None of this is built.** Once acquired, a monster keeps the player forever — `GetIdealState`'s only exit
from `MONSTERSTATE_COMBAT` is a null enemy, and nothing sets one. This is the base game's behaviour,
unchanged, and it is the first item of [the post-aggro step](ROADMAP.md#the-post-aggro-step).

Intended: if the enemy stays occluded and deals no damage for a give-up interval, Suspicion drains. At the
floor the monster drops `m_hEnemy` and runs a **Search** toward the last known position. Taking damage
resets the interval, so shooting a monster and strolling away does not work. What holds an enemy must be
**contact** — last sight or last damage — and not the meter, or a monster under fire from an unseen attacker
would quietly time out mid-firefight.

A Search that finds nothing resolves to a **Post**:

- If the mapper gave the monster `pev->target`, advance `m_pGoalEnt` along the authored `path_corner`
  chain. Authored intent wins, which is how Half-Life already treats `pev->target`.
- Otherwise, the leader assigns new Posts spread around the last known position, using the existing
  `SquadMemberInRange(..., 128)` spacing rule so members do not stack.

Either way the monster settles at `MONSTERSTATE_ALERT` with a **permanently raised Suspicion floor** —
never back to IDLE. A room the player was spotted in stays harder for the rest of the level.

**Across a level change it does return to IDLE.** On restore with `fUseLandmark` set, Suspicion, the raised
floor and the Post all clear, and a monster with no valid enemy that is not in a script is pushed to
`MONSTERSTATE_IDLE`. A plain save/load changes nothing, so quickloading is not a "calm everyone down"
button.

### Squad coordination

**There is no squad-level Suspicion value.** One source of truth: each monster's own meter. The squad
channel is a set of writes the leader makes.

| Trigger | What the leader does |
| --- | --- |
| a member crosses the notice threshold | raise every member's Suspicion to a floor, with a voice line |
| a member crosses acquisition | `SquadMakeEnemy` — vanilla, unchanged |
| `m_fEnemyEluded` and enough time since `m_flLastEnemySightTime` | call a **Search**: distribute the LKP, spread the squad |

This reuses `m_fEnemyEluded` and `m_flLastEnemySightTime` where they already live and are already saved.

Killing the leader silently therefore removes the squad's entire coordination layer, permanently, because
the SDK has no leader promotion. That is a large stealth reward that costs nothing to build.

### Death, witnesses, and the Disturbance

Two mechanisms doing two different jobs.

**Seeing the kill.** At death — before `SquadRemove` — loop the victim's squadmates and nearby monsters.
Each one passing `FVisible` on the victim jumps to a high Suspicion floor and takes the death position as
its LKP. A monster that could not see it reacts to nothing.

**Finding the body.** A new sound type, `bits_SOUND_DISTURBANCE` (`1 << 7`, free), inserted at death with a
duration and **added to `FIsSound()`'s mask** so it behaves as a real sound — which buys
`bits_COND_HEAR_SOUND`, `PBestSound`, `MakeIdealYaw` and every existing schedule interrupt for nothing.
Only monsters whose Perception Profile opts in listen for it. It expires on its own, so a level does not
accumulate permanent distractions, and `Look` never has to look at a corpse.

Reaching a Disturbance raises the squad's Suspicion floor once. **This is the single exception to "only
sight fills the meter"**, and it is justified because a body is proof rather than a hint.

Watch `MAX_WORLD_SOUNDS`: it is 64 for the whole world. Keep the duration modest and insert only for
profiles that opt in, or a large firefight will crowd the pool.

### Perception Profiles — built 2026-09-01

`struct PerceptionProfile` (`dlls/perception.h`): a fill scale, a drain scale, and a `bUsesSuspicion` flag.
Every monster participates by default; **"dumber" means a worse profile, never a bypass**, so a dark room
works on a zombie too, just less.

| Profile | Fill | Drain | Who |
| --- | --- | --- | --- |
| `g_ProfileDefault` | 1.0 | 1.0 | everything not named below |
| `g_ProfileTrained` | 1.5 | 0.5 | human grunt, assassin, alien grunt, alien slave — the four primaries |
| `g_ProfileAlwaysAware` | — | — | `bUsesSuspicion` false: turret family, apache, osprey, barnacle, tentacle, nihilanth |

The scales are **constants, not cvars**, because the ratio between two profiles is a design statement — a
grunt notices sooner than a zombie — while the absolute rate is the tuning knob, and that is what
`suspicion_fill` and `suspicion_drain` are. Promote them if the ratio itself needs dialling in.

The opt-out list is the Backstab's exclusion list minus everything excluded for being *small*. Each entry
is a machine, an aircraft, or a monster with no eyes to fool — never one excluded for being dangerous. The
tentacle is the load-bearing one: it is blind, driven entirely by the sound list, and already the vanilla
game's one stealth encounter, so Concealment has nothing to add to it and could only break it.

**Reach it through a virtual, not a member set in `Spawn`.** `Spawn()` does not re-run on restore — only
`FCAP_MUST_SPAWN` entities get one, everything else gets `Restore()` and `Precache()` and nothing more
(`dlls/cbase.cpp:380-389`) — so anything set there and not saved comes back default-constructed after every
load. That is why Valve saves `m_flFieldOfView` despite every monster assigning it in `Spawn`. A profile is
a property of the monster's *type* and can never differ between two instances, so paying save-game bytes
for it would be wrong twice over. `CanBackstab()` already takes this shape; see
[ADR-0010](adr/0010-the-backstab-is-positional.md#why-a-virtual-and-not-a-flag).

### Scripted sequences — built 2026-09-01

A monster with `m_pCine` set accumulates no Suspicion; it is playing a sequence, not perceiving. When the
script releases it, the normal model applies from where the meter stood, so stealth works afterwards.

`SF_MONSTER_IGNORE_CONCEALMENT` (**1024**, `dlls/monsters.h`) additionally lets a mapper mark a monster as
ignoring Concealment entirely, for a set piece that must fire. Half-Life's pacing leans on monsters spawning
into a fight that is going to happen, and authored intent has to be able to win. It is 1024 rather than the
apparently-free 8 because `apache.cpp` already spends 8 on `SF_NOWRECKAGE`. It is in the FGD's `Monster`
base class as **"Ignore Concealment"**, in both `fgd/halflife.fgd` and the mod directory's `top_mod.fgd`.

### The readout

Three states — **Unseen**, **Noticed**, **Spotted** — derived server-side from the highest Suspicion among
every monster that can currently perceive the player, *including monsters the player cannot see*. It is a
warning, not a mirror.

Quantising to three states means the message fires on threshold crossings only, following `gmsgPulse`'s
precedent of sending on state change and letting the client run its own clock. The Unseen→Noticed edge gets
a soft cue; Noticed→Spotted gets a hard one.

### The Backstab — built 2026-08-31

Recorded here only because it is adjacent; it is **independent of everything above** and needs no meter, no
profile and no squad code. It is the only part of pillar 6 that exists today.

A Backstab is a melee hit landed in a monster's rear arc. **Positional only** — whether the victim has
noticed the player does not enter into it, and there is one tier. The full reasoning, the rejected
alternatives and the exclusion list's four separate justifications are in
[ADR-0010](adr/0010-the-backstab-is-positional.md).

`CBaseMonster::FInRearArc` (`dlls/combat.cpp`) is the test — the same 2D comparison against `pev->angles`
that `FInViewCone` makes, so "behind" is the exact complement of "in front". It deliberately does **not**
call `UTIL_MakeVectors`, because its caller is mid-attack and still needs `gpGlobals->v_forward`.
`CBaseMonster::CanBackstab()` is the opt-out, virtual and per class.

It is applied in `CCrowbar::Swing` between Crowbar Force and the Follow-Up, so every stage multiplies the
already-stronger hit and the largest number a player can produce is every bonus at once. Two cvars:
`backstab_damage_scale` (3) and `backstab_arc_dot` (-0.5, the rear 120°).

Not backstabbable: headcrab (and babycrab, which inherits — `dlls/headcrab.cpp:479`), snark, roach, rat,
leech, hornet, flyer, barnacle, tentacle, tentacle maw, controller, turret / miniturret / sentry, apache,
osprey, nihilanth, and big momma — whose `TakeDamage` clamps `pev->health = flDamage + 1` until her node
path finishes (`dlls/bigmomma.cpp:588-596`), making her unkillable by construction and any multiplier on
her meaningless.

**Gargantua is backstabbable, and its damage filter stays untouched.** `GARG_DAMAGE` is
`DMG_ENERGYBEAM | DMG_CRUSH | DMG_MORTAR | DMG_BLAST` (`dlls/gargantua.cpp:47`); the crowbar is `DMG_CLUB`,
so `TraceAttack` zeroes the damage and plays a ricochet (`:830-851`) and `TakeDamage` would multiply by
0.01 on top (`:858-871`). A crowbar therefore never hurts one, however large the multiplier. The
[Gauss Katana](ROADMAP.md#the-gauss-katana), already proposed as `DMG_ENERGYBEAM`, passes the filter — so
the endgame melee weapon is what makes a Gargantua stabbable at all.

---

## Deliberately not generalised

**Suspicion governs player acquisition only.** Every other hostile is acquired instantly, exactly as in the
base game. The scoping is a single `pSightEnt->IsPlayer()` branch in `Look`, and `ConcealmentOf` takes a
`CBaseEntity*` rather than reading the player directly, so generalising it later is deleting a branch rather
than a rewrite.

**Recorded for later review**, because most of the model already generalises for free and one part does not:

*Generalises free.* Angle and distance are computed from the target either way. `GETENTITYILLUM` works on
any entity, so light does too. Stance is player-specific, but monsters never duck, so it is a constant 1.0
for them.

*Does not generalise.* The muzzle-flash term. `m_iWeaponFlash` lives on `CBasePlayer`
(`dlls/player.h:113`) and only player weapons set it; monsters use the base `CBaseEntity::Illumination()`,
which is plain `GETENTITYILLUM`. So a grunt firing an MP5 in a dark room **illuminates nothing**, while the
player firing the same gun lights up for about a second. Generalise naively and monsters become stealthier
than the player while shooting, which is exactly backwards. Fixing it means giving monsters an equivalent
flash value set where they fire, or consciously accepting the asymmetry.

*What it would change.* The pacing of every alien-versus-marine set piece in the vanilla campaign. Those
fights happen in lit rooms at range and currently trigger the instant two hostiles see each other; under the
full model they would take a beat, and in a dark room two hostile groups could walk past one another.

*What to check when reviewing.* Whether faction fights still trigger at the same ranges; whether a dark
room letting two groups miss each other reads as intentional or as broken; and whether the muzzle-flash
asymmetry has been resolved before any of it is judged.

---

## Known weaknesses

Recorded now so they are not rediscovered as bugs.

- **A round trip across a level boundary launders a room's alert state.** The transition reset is
  deliberate, and this is its cost.
- **`MAX_WORLD_SOUNDS` is 64, shared by the whole world.** Disturbance markers compete with gunfire,
  grenades and the player's own footsteps for the pool.
- **Circle-strafing into the rear arc trivially Backstabs slow enemies** — zombies and headcrabs, which are
  exactly the monsters the Follow-Up is already tuned against.
- **Getting behind an acquired monster does nothing for gunfire.** Aim reads the LKP, not facing, so a
  monster you are standing behind still shoots you accurately whenever anything refreshes its LKP. Deferred,
  with the diagnosis, to [the post-aggro step](ROADMAP.md#the-post-aggro-step).
- **Light is nearly inert until custom maps exist.** Blocked on [Maps](ROADMAP.md#maps), like most of the
  mod.
- **Pillar 6's "measurably better off" criterion is not carried by the damage model.** The Backstab is
  positional and single-tier, so the stealth player's advantage is not fighting at all, and silent leader
  kills dissolving squads. If that turns out to be too thin in play, the second tier is the obvious lever.
- **A monster outside the player's PVS does not drain.** `RunAI` skips `Look` entirely in that case
  (`dlls/monsterstate.cpp:82`), so Suspicion freezes rather than decaying while the player is elsewhere.
  Judged correct rather than merely tolerable — forgetting should cost time *in the room* — but it does mean
  a monster can be left one step below acquisition indefinitely.
- **The meter advances once per think, not once per frame.** A monster in a state that thinks rarely fills
  more slowly per second than the cvar implies. Intentional; worth knowing before tuning against a stopwatch.
- **Corpses are still invisible to `Look`.** The Disturbance marker makes a body findable for a while; it
  does not make a body *visible*, and a monster standing next to one after the marker expires sees nothing.

---

## Tuning

Every number in Part 2 is a first guess and every one is a cvar, following the `pulse_*` and `infusion_*`
precedent in `dlls/game.cpp`. Nothing here has been judged in play.

The knobs the model needs, and which exist today: Concealment weights per term (`conceal_*`); Suspicion fill
and drain rates and the two thresholds (`suspicion_fill`, `suspicion_drain`, `suspicion_notice`,
`suspicion_acquire`); the
crouch and walk noise multipliers (`noise_stance_duck`, `noise_stance_walk`); and the Backstab's rear-arc
dot and multiplier (`backstab_*`). Still to come with the features that need them: the Search duration,
per-profile scales if the constants prove wrong, and Disturbance volume and duration.

A debug view of live Suspicion values shipped with the meter rather than after it — `debug_suspicion`,
described above. Two [TECH_DEBT.md](TECH_DEBT.md) entries already ask for debug visualization of custom
systems, and a meter nobody can see is a meter nobody can tune.
