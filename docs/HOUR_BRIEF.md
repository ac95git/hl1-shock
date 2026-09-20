# Hour brief — tech debt and shaped mechanics, 2026-09-20

Settled in a short preparation with Andrei on 2026-09-20. His sentence: *"a 1 hour independent coding
session, we address tech debt and shaped mechanics."* Four slices were proposed; he kept three and left
out **5b, the give-up** — not rejected as a design, just not this hour. This file is the record of what
was decided so the work survives a compaction or a fresh session, and the test rows are written into it
**as each slice lands**, so an absent row means an unbuilt slice.

## How the work lands

- **A commit per slice on `hl-shock`**, message ending **"Not verified in game"**. `git add` names the
  files; never `-A` or `.` — `models/v_pickaxe.mdl` in the tree is Andrei's hand-made build and stays out.
  Commit messages go through a file (`git commit -F`).
- **Both DLLs build clean before each commit**, with the MSBuild invocation the build memory records, and
  the build installs itself.
- **Docs move with the slice**: PILLARS gains what is built, the ROADMAP line shrinks, TECH_DEBT closes.
- **Sequential, one slice at a time**, no parallel agents.
- **Undecided questions are decided**, the option that makes the most sense, written under *Decisions I
  made* with the rejected alternative, so overturning one is a one-line instruction.
- **Stop freely.** If a slice turns into a slog, stop with a true checklist rather than finish a doubtful one.

### Where the work stops

1. **The Pulse**, in any form — the timing grill is booked for the morning of 2026-09-21 and must not be
   pre-empted. Slice 3 touches what *monsters* do after a deflect, never what the Pulse does.
2. **Retuning a system Andrei has verified**: the Dash's numbers, the Search, the Matrix, the katana.
3. **His own code slices** under ADR-0015: the friendly alien slave, `env_model`.
4. **Anything needing a decompile.**
5. **`topmap`**, and no roadmap entries from the agent's own ideas.

## The slices, in order

| # | Slice | Kind | Size |
| --- | --- | --- | --- |
| 1 | 5g — the level-change reset to IDLE | shaped mechanic, pillar 6 | small |
| 2 | Phase's effect — no damage during the Dash | shaped mechanic, Shinobi | small |
| 3 | The deflected melee root cause | tech debt, last open root cause | medium |
| R1 | *(reserve)* 5e — the captain's channel | shaped mechanic, pillar 6 | medium |
| R2 | *(reserve)* one deferred-text collector for the VGUI panels | tech debt fallback | small |

### 1. 5g — the level-change reset

**What it does in play.** A room the player was hunted in does not follow them through a level change: on
the far side, a monster with no enemy that is not in a script is idle again. A plain quicksave and load
changes nothing — quickloading is not a "calm everyone down" button.

**Where.** `CBaseMonster::Restore`, `dlls/monsters.cpp` — the landmark branch that already clears the
meter and the floor says in its own comment that pushing an enemyless monster back to IDLE "is 5g's".
Spec: [PERCEPTION.md, "Losing the player"](PERCEPTION.md#losing-the-player--the-give-up-settled-2026-09-17-not-built),
the last paragraph. Also clear the stored Post — whatever the Search remembers (`m_vecLastSearch`,
`m_vecSearchTarget`), so the far side does not answer a Disturbance from the near side.

**The guard.** Only when `m_hEnemy` is null, the monster has no `m_pCine`, and its state is not
`MONSTERSTATE_SCRIPT`; set both the state and the ideal state. `CBasePlayer::Restore` tells a level change
from a load the same way, by `fUseLandmark`.

**Docs.** PERCEPTION.md's "not built" on this paragraph; PILLARS pillar 6; the ROADMAP table row for 5g.

### 2. Phase's effect

**What it does in play.** With Phase bought (Shinobi, id 111, already placed and gated behind the Dash
Module, tooltip *"No damage while Dashing."*), an attack that lands during the Dash's burst does nothing.
A dodge, not immunity: the burst is a fraction of a second, and the roadmap marks it *the first to cut if
it proves too strong*.

**Where.** `CBasePlayer::TakeDamage`, `dlls/player.cpp`, beside Last Stand's window — after it, before
Ricochet, with the same `debug_damage` line so a refused hit is visible in the console. The server sees
the burst in `pev->fuser1` (non-zero while it runs, positive flat, negative Air Dash — `DashAfterMove`
already reads it), so no new state and no new message.

**Decided in advance:** falls and drowning (`DMG_FALL`, `DMG_DROWN`) still land. Rejected: refusing every
type as Last Stand does — an Air Dash dive that meets the floor inside its burst would then be a free fall,
and the dive's damage is an open review of Andrei's that Phase must not answer by accident. Everything else,
time-based ticks included, is refused: a poison tick in a 200 ms burst is noise either way, and one rule
beats a list.

**Docs.** PILLARS pillar 4 (Shinobi — "Phase's node has no effect" goes), ROADMAP's Shinobi "Still open"
bullet on Phase and the Routes open question that lists it, SKILL_TREE.md if it says the node is inert.

### 3. The deflected melee root cause

**What it does in play.** A Pulse that turns a claw away no longer hears the claw connect: no hit-flesh
sound from the zombie, the alien slave, the bullsquid, the maddened miner or the Panthereye. The alien
grunt already behaves this way through a helper of its own; this makes it one mechanism for every melee
monster.

**The bug.** `CBaseMonster::CheckTraceHullAttack` (`dlls/combat.cpp`) calls `TakeDamage`, discards its
result, and returns the entity it struck. Every caller treats non-null as "I hit them" and plays the hit
sound. [TECH_DEBT.md](TECH_DEBT.md#a-deflected-melee-attack-still-reports-as-a-hit-to-the-attacker--mitigated-2026-08-02)
has the entry.

**The fix.** An optional out-parameter (`bool* pbLanded = nullptr`) on `CheckTraceHullAttack`, set from
`TakeDamage`'s return when damage is dealt and `true` when the call asked for none (the bullsquid's
zero-damage trace). Each caller gates its hit sound on it. Sites: `zombie.cpp` ×3, `islave.cpp` ×2,
`bullsquid.cpp` ×3, `maddened.cpp`, `panthereye.cpp`, `agrunt.cpp` ×2 (replace `AGruntPunchDeflected`
with the flag), and `CGargantua::GargantuaCheckTraceHullAttack`, which is a copy — give it the same
parameter and check whether its slash plays a hit sound on the return.

**Decided in advance, reconciling the entry with later decisions:**

- **The shove and the view kick stay.** Settled 2026-09-13 in the melee grunt entry: *the Pulse blocks the
  damage, not the blow*; the kick is scaled by `pulse_deflect_punch`, which the entry's mitigation already
  judged the better outcome. So the entry's original acceptance line ("no view punch") is superseded, and
  the closure says so.
- **No miss sound either.** A deflected blow connected with a Shield; the Pulse's own clang is the sound of
  it. Rejected: returning `NULL` on a refused hit (the entry's step 1) — it would drop the shove and play the
  miss whoosh for a blow that landed.
- **Audit each site for non-damage uses first.** A `false` from `TakeDamage` also comes from Last Stand,
  which is correct (no downstream system sees a hit that never landed), and from a victim that refuses
  damage outright; both should also silence the hit sound.

**Docs.** TECH_DEBT entry to RESOLVED with the reconciled criteria; the melee grunt entry's "the one thing
to fix while there" line; PILLARS pillar 2 or 3 wherever the deflect's cues are described.

### R1. 5e — the captain's channel *(reserve)*

Spec: [PERCEPTION.md, "The captain's channel"](PERCEPTION.md#the-captains-channel--settled-2026-09-17-not-built),
first row of the table. When a squad member **crosses** `suspicion_notice` (the edge, not the level —
`UpdateSuspicion` in `dlls/perception.cpp` already has the notice band), its leader lifts every member
*below* the line to it (a lift, never a lowering; `SuspicionJump`) and speaks `HG_LOST0`, "stay alert
people", through a virtual hook on `CHGrunt` in the pattern of the three `OnSearch*` hooks. Loners never do
it. The sentences are already in `sound/sentences.txt`; check the install's copy is current. Docs: the
"not built" sentence in PERCEPTION.md, PILLARS pillar 6, the ROADMAP row.

### R2. One deferred-text collector *(reserve)*

TECH_DEBT's fallback step for the VGUI draw-order rule: a small shared type in `cl_dll/` that collects
text during the sprite pass and flushes it at the end of `paintBackground`, replacing `DeferredText` in
`vgui_inventory.cpp` and `DeferredCountLabel` in `vgui_inventory_grid.cpp`. No behaviour change; the
entry stays open (the cause is still unknown) and gains a line saying the workaround is now one type.

## What landed

Commits on `hl-shock` after `1395a1b` (the pickaxe), each ending "Not verified in game", both DLLs
building clean.

| Commit | Slice | Rows |
| --- | --- | --- |
| `fb310a9` | 5g — the level-change reset | L |
| `873734d` | Phase's effect | H |
| slice 3 | The deflected melee root cause | F |

## Test rows

Written as each slice lands. Report "pass", "fail" and what was seen instead. Debug build, installed.
`sv_cheats 1`, `debug_schedule 1` (state and meter over every monster you aim at), `debug_damage 1`.

### L. 5g — the level-change reset

Needs a vanilla map with a level transition a monster can cross: any `trigger_changelevel` whose
`trigger_transition` volume holds the monster when you cross (a monster following you through the door is
enough). `topmap` has no transition.

| # | Do | Expect |
| --- | --- | --- |
| L1 | Aggro a grunt or zombie, run through a transition with it close behind, aim at it on the far side | `debug_schedule` shows state **Idle**, meter 0; it does not hunt you until it sees you again |
| L2 | Same setup, but quicksave and quickload instead of crossing | Still Combat or Alert, meter as it was: a load changes nothing |
| L3 | Be Noticed by a grunt (icon dim) but not acquired, cross | Meter 0 on the far side, the floor gone: a second kill there is not primed |
| L4 | A scientist mid-script (talking, or on a scripted sequence) crosses with you | The script plays as it always did |
| L5 | A monster killed before the crossing whose body crosses | Still a corpse, not standing up |

### H. Phase

`give item_longjump` for the Dash, `skill_unlock_all` or buy Phase (Shinobi, behind the Dash gate), a
zombie or a headcrab as the attacker, `debug_damage 1`.

| # | Do | Expect |
| --- | --- | --- |
| H1 | Dash through a zombie's swing so the claw lands mid-burst | No damage; the console prints `phase: dodged` with the milliseconds of burst left; the shove and the small view kick still happen |
| H2 | Stand still and take the same swing | Full damage, no `phase` line |
| H3 | Take a hit just after the burst ends (the ~200 ms after the Dash) | Full damage: the dodge is only the burst |
| H4 | Dash across a tripmine's beam or into a grenade's blast timed to the burst | No damage from the blast if it lands inside the burst |
| H5 | Air Dash straight down into the floor from height, with Phase | Fall damage as before: falls are not dodged |
| H6 | Without Phase (respec or a fresh save), H1 again | Damage lands; nothing prints |
| H7 | Judge it | Does a dodge with no damage window make melee enemies a free hit? The roadmap marks Phase the first to cut if so |

### F. The deflected melee root cause

With the Pulse Module (`give item_pulsemodule`). One attacker of each kind; `minemap` has zombies,
headcrabs and alien grunts, `topmap` the rest, or `give monster_zombie` etc. Listen with the game sound
up.

| # | Do | Expect |
| --- | --- | --- |
| F1 | Pulse into a zombie's slash | The Pulse's clang, the small (scaled) view kick, the sideways shove; **no** claw-strike hit sound from the zombie, and no miss whoosh either |
| F2 | Let the same zombie hit you without a Pulse | The claw-strike hit sound as always, full kick, damage |
| F3 | Zombie swings and misses you | The miss whoosh as always |
| F4 | F1 against an alien slave's claw, the maddened miner's swing, a Panthereye's claw | Same as F1: clang, kick, no hit-flesh sound from the monster |
| F5 | F1 against an alien grunt's punch | Unchanged from before: shove, no hit sound, no blood |
| F6 | F1 against a Gargantua's slash (`give monster_gargantua`, `god` if you like) | Clang, the big shove and kick, no hit sound |
| F7 | A zombie slashes a scientist or a grunt | The hit sound plays: a blow that lands still sounds |
| F8 | Under Last Stand's window, take a zombie's slash | Silent from the zombie, like a deflect: nothing landed |
| F9 | Pulse into a bullsquid's bite or tail whip | As before (those attacks have no hit sound of their own): shove, no damage |

## Decisions I made

*The pre-made ones are above under each slice; anything decided during the hour goes here with the
alternative rejected.*
