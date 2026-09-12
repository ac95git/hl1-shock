# Roadmap — intended work

Everything this mod means to build and has not built. [PILLARS.md](PILLARS.md) is the record of what exists
today; this is the record of what is wanted, why, and what it will cost.

Nothing here is a commitment to a design. Entries capture the intent, name the code that already exists to
build on, and list the questions that have to be answered before the first line is written. When an entry
is built, its content moves into PILLARS.md and the entry here is deleted — this file only ever shrinks
from the top.

**Last updated:** 2026-09-13 (branch `hl-shock` — the monster roster listed; the Panthereye explored)

## Shape legend

How settled a thing is, which is a different axis from PILLARS.md's build status.

| Label | Meaning |
| --- | --- |
| **Idea** | Named. Nothing decided, and the open questions below it are genuinely open. |
| **Shaped** | The design questions have answers written down. No code. |
| **Ready** | Shaped, and the first commit is obvious. |

## How this is organised

By pillar, because that is how the mod is already described and a feature that does not belong to a pillar
is a feature that needs justifying. Two structural calls were made in writing this, both of which change
PILLARS.md:

**Stealth becomes pillar 6.** It is not "enhanced combat". It is the alternative to combat, and it moves
enemy perception, player movement, weapon choice and level layout at the same time. Filing it under pillar 2
would make that pillar mean "everything you do to things that are alive", which is a category and not a
pillar. It gets a row in the PILLARS.md summary at **Not started**, the same as Exploration.

**Modules do not become a pillar.** A Module is a thing you find that changes what you can do, which is
what pillar 3 already is; the Pulse is the shape it takes, and the Modules tab is a pillar 5 concern. Making
it a pillar would split "things you find" across two of them. It is the largest single entry under Custom
items instead.

Everything else falls out: weapons, monsters and bosses are pillar 2; Transmissions, level design and
Stations are pillar 1; sounds and icons are not roadmap items at all — see [Art and audio](#art-and-audio).

## Contents

- [The prediction problem](#the-prediction-problem) — solved for weapons, still open for movement
- [Maps](#maps) — the other one
- [Pillar 6: Stealth](#pillar-6-stealth)
- [Pillar 2: Weapons](#pillar-2-weapons)
- [Pillar 2: Monsters and bosses](#pillar-2-monsters-and-bosses)
- [Pillar 2: Decapitation](#pillar-2-decapitation) — a design to port, already read
- [Pillar 3: Modules](#pillar-3-modules)
- [Pillar 1: Transmissions](#pillar-1-transmissions)
- [Pillar 1: The world](#pillar-1-the-world)
- [Art and audio](#art-and-audio)
- [Proposed vocabulary](#proposed-vocabulary)
- [Open questions](#open-questions)

---

## The prediction problem

**Shape: Solved for weapons. Still open for movement.**

This was seven features on one blocker. The weapon half is done — see
[PILLARS pillar 4](PILLARS.md#4-skill-trees) for what was built and
[TECH_DEBT.md](TECH_DEBT.md#skill-state-never-reaches-client-side-weapon-code-—-resolved-2026-08-31) for
the record. `HUD_SetPredictedSkills` populates the client's `CBasePlayer::m_skills` from the mask the
server already sends, and `dlls/skill_tuning.h` lets predicted weapon code read the server's tuning cvars.
`FastReload` was unreserved on the back of it and is in the tree.

What that leaves:

| Feature | Where it lives | Still blocked on |
| --- | --- | --- |
| Dash | [Modules](#pillar-3-modules) | `pm_shared/` — but the long jump's physics-key route reaches it |
| Hook | [Modules](#pillar-3-modules) | `pm_shared/` — same route |
| Sprint speed (`SprintSpeed`, id 6) | reserved, cut from the tree | `pm_shared/` |
| High jump (`HighJump`, id 5) | reserved, cut from the tree | `pm_shared/` |
| Crowbar swing speed (`CrowbarSpeed`, id 11) | reserved, cut from the tree | the crowbar's own damage rule, below |
| Draw speed | [Weapon handling](#weapon-handling) | nothing — unblocked |

**The movement four are a genuinely different problem.** `pm_shared/` runs from `playermove_t`, not from
`CBasePlayer`, so it cannot reach `m_skills` at all — the fix above does not extend to it.

**But a route into it already exists, and the base game uses it.** The long jump module is a predicted
movement verb gated on player state: the server sets a physics key (`"slj"`, `dlls/items.cpp:584`) and
`PM_Jump` reads it from `pmove->physinfo` (`pm_shared/pm_shared.cpp:2662`). The same mechanism can carry a
Module or a movement Skill. Noticed 2026-09-12 while shaping Modules; not yet tried for anything new.

**Crowbar swing speed is no longer a prediction problem.** The cadence is predicted and now reachable, but
`CCrowbar::Swing` reads `m_flNextPrimaryAttack` to decide whether a swing is a first swing (full damage) or
a follow-up (half), so shortening the interval silently makes every swing a follow-up. Separating those two
uses of the same timer is the work, and it is small.

**Draw speed is unblocked and unstarted.** `CBasePlayerWeapon::DefaultDeploy` is the same kind of shared
chokepoint `DefaultReload` turned out to be, and Fast Reload is the worked example to copy.

---

## Maps

**Shape: Idea. The most-shared blocker in the mod.**

Not a feature, but it belongs here because six completed systems are unreachable without it. Everything in
this list is **finished code with no way to encounter it in play**:

| System | Placeable entity | Reachable today only via |
| --- | --- | --- |
| Skill Points | `item_skillpoint` | `skill_addpoints` |
| Reset Tokens | `item_resettoken` | `skill_addtokens` |
| Health Syringe | `item_syringe` | `give item_syringe` |
| Row Grants | `item_rowgrant` | `inv_addrows` |
| Exploration (pillar 1) | — | nothing |
| Transmissions | *(not written)* | nothing |

Vanilla Half-Life maps cannot be edited to hold any of it, so the economy in PILLARS pillar 4 — a tree
completable only by near-exhaustive exploration — is entirely theoretical. Nobody has ever played it.

**As of 2026-08-31 there is no code left in the way.** `item_rowgrant` was the last unwritten entity
(see [MAP_BRIEF.md](MAP_BRIEF.md#the-one-code-dependency), now resolved), so every row in the table
above except Transmissions is placeable today. What remains is entirely map work.

This does not need to be the full campaign. **One map** that places Skill Points off the critical path, a
Syringe, and a Row Grant would move pillars 1, 3, 4 and 5 from "designed" to "judged", and every tuning
question in PILLARS.md is waiting on exactly that. It is the cheapest way to learn the most.

See [The world](#pillar-1-the-world) for what the maps should eventually contain.

---

## Pillar 6: Stealth

**Shape: Building. Steps 1–4 and 6 are done — everything that decides whether the player is noticed, and
telling them about it. Step 5, everything after acquisition, is all that remains.**

Half-Life has a working perception model that the vanilla game barely uses and never rewards. Stealth here
is not a new system — it is finishing one Valve left half-connected and then giving the player tools to
work against it.

**The whole model now lives in [PERCEPTION.md](PERCEPTION.md)**, written 2026-08-31. Part 1 documents the
base SDK exactly as it is — sight, hearing, scent, acquisition, the state machine, squads, deaths, and what
survives a save versus a level change. Part 2 is what this mod adds. That file is the reference; this entry
is only the build order and what is still open.

Two corrections it surfaced about the **base game**, worth knowing before reading anything written before
it: the claim that grunts investigate player noise is **wrong** — that block is commented out
(`dlls/hgrunt.cpp:1983-1988`) and has never run — and deaths and corpses are **entirely imperceptible**,
so nothing reacts to a squadmate dying and no body is ever noticed. The design addresses the first two
thirds of that (a witness check at the moment of death, and a **Disturbance** marker at the place it
happened) and deliberately leaves corpses themselves invisible to `Look`.

### Build order

Six commits, each independently playable, with the riskiest AI work last and sitting on top of a document
that already describes what it changes.

| # | Commit | Notes |
| --- | --- | --- |
| 1 | **Docs** | PERCEPTION.md, the CONTEXT.md terms, the two corrections, the CLAUDE.md index row. **Done 2026-08-31.** No code. |
| 2 | **The Backstab** | `CanBackstab()`, the curated exclusion list, `FInRearArc`, two cvars, `adr/0010`, and the headshot entry under [pillar 2](#headshots-and-how-they-reconcile-with-this). **Done 2026-08-31.** |
| 3 | **Suspicion** | Perception Profile, the meter, the `Look` gate, the `debug_suspicion` view, `SF_MONSTER_IGNORE_CONCEALMENT`, `adr/0009`. **Done 2026-09-01.** |
| 4 | ~~**The readout**~~ | `gmsgConceal` plus `CHudConceal`, following `CHudPulse`'s send-on-change pattern. **Done 2026-09-02.** |
| 5 | **[The post-aggro step](#the-post-aggro-step)** | Everything that happens *after* a monster acquires the player: de-escalation, the Search, Posts, aim-versus-facing, death witnesses, the Disturbance marker, the level-transition reset. Attempted 2026-09-02 and reverted — see below. |
| 6 | ~~**Noise**~~ | A deliberate multiplier on the computed noise volume for crouching and walking. **Done 2026-09-02**, pulled forward: without it a crouched player could not get within crowbar reach without being heard, so the Backstab's own approach did not work. |

### The post-aggro step

Renamed from "the squad half", because the squad work turned out to be the smaller part of it.

Everything up to acquisition is built and behaves. Everything after it is the base game, and the base game
was never built for a player who can choose *not* to be seen. This step was attempted on 2026-09-02 and
**reverted the same day** — not because the code was wrong, but because it was being written as patches to
symptoms in a commit that was supposed to be about detection. The diagnosis is worth more than the code was,
so it is kept here.

**Aim is not facing.** `ShootAtEnemy` (`dlls/monsters.cpp:3234`) aims from the gun position at
`m_vecEnemyLKP`. `pev->angles` is not consulted; `SetBlending` blends pitch only; there is no yaw blend and
no check that the target is in front. A monster with a live LKP fires at full accuracy through its own back
while its model faces elsewhere. **The turn is cosmetic — only the LKP is real.**

**The LKP has four writers**, and the first attempt gated exactly one of them:

| Writer | Notes |
| --- | --- |
| `CheckEnemy`, enemy in cone and visible | legitimate |
| `CheckEnemy` "behind or beside", ≤256 units | the anti-cheese hack this mod wants to *undo* |
| `TakeDamage` → `m_vecEnemyLKP = pevInflictor->origin` | shoot a monster from behind and it gets a perfect fix |
| `SquadCopyEnemyInfo` | one squadmate with eyes on you feeds the whole squad |

**So the seam is `ShootAtEnemy`, not the writers.** Clamp the shot to the monster's own cone — fire along
facing when the LKP falls outside it — and "get behind it" means something for every monster at once, with
no LKP bookkeeping and no per-monster schedule surgery. That is the change to try first.

**But not "every monster at once" — that claim was wrong and is corrected here.** `ShootAtEnemy` carries the
grunt's rifle and shotgun, the assassin, the alien slave's zap, Barney, the ichthyosaur and the Gargantua's
flame. It does **not** carry the alien grunt, which aims its hornets straight at `m_vecEnemyLKP` itself
(`dlls/agrunt.cpp:437`) and whose hornets then home in regardless; nor grenades, which are thrown at the LKP
directly (`dlls/hgrunt.cpp:495-527`). Those need their own answers.

**Diagnose before designing.** `debug_monster_aim` (throwaway, `dlls/combat.cpp`) prints per shot: which
monster fired, its schedule and task index, the yaw it is *at* versus the yaw it is *turning toward*, how
far its LKP is from the player, whether the player is inside the 0.5 arc, and whether the shot is actually
travelling at the player — plus a line naming whoever last hit the player, so an unseen squadmate cannot be
mistaken for the monster on screen. It exists because a monster facing away while the player takes hits has
three different possible causes (a squadmate shooting, the monster correctly shooting a stale position while
something else hits, or the turn genuinely failing) that look identical on screen and cannot be told apart
by reading the code.

**And nothing de-escalates.** `GetIdealState`'s only exit from `MONSTERSTATE_COMBAT` is a null enemy. A
monster that loses the player therefore cycles
`ESTABLISH_LINE_OF_FIRE → ELOF_FAIL → TAKE_COVER_FROM_ENEMY → SCHED_FAIL` for as long as the player stays in
its PVS, hidden and unreachable — idle two seconds at a time, throwing a debug spark on every failed cover
search (`dlls/schedule.cpp:189`, `#ifdef DEBUG` only), and walking to cover nodes 384 units away in between.
This is **vanilla**, reachable in the base game by aggroing a grunt and hiding; the mod only makes it a
normal thing to do rather than a freak one. Leaving PVS parks the monster silently in `TASK_WAIT_PVS`
instead — still stuck, just quiet.

**The cause of that stuck state is now known, and it is not pathing.** Diagnosed 2026-09-12 from in-game
capture: a grunt's grenade-cover schedule sets a 99-second "freeze in place" and dies two tasks before the
task that releases it, and nothing scopes that freeze to the schedule that set it. The monster is then held
motionless — not turning, still playing and firing its previous animation, still aiming at the player's exact
position — for up to a minute and a half. It is a **base-game movement bug**, it accounts for the shooting
backwards *and* the freezing *and* the sparks as one fault rather than three, and it is written up with
evidence and a one-line fix in
[TECH_DEBT.md](TECH_DEBT.md#a-leaked-move-wait-freezes-a-monster-for-up-to-99-seconds).

**Fix it before judging anything else here.** Post-aggro stealth behaviour cannot be evaluated against a
monster that may be frozen for 99 seconds, and the aim seam above may look less urgent once a monster that
loses the player actually turns around.

**How this step is sliced** — agreed 2026-09-12, small commits with aim first, each independently playable:

| | |
| --- | --- |
| 5a | Monsters only shoot where they face |
| 5b | Give up the chase (contact-keyed, not meter-keyed) |
| 5c | The Search |
| 5d | Posts |
| 5e | The squad channel — the captain's *"call for search"* from the original brief |
| 5f | Death witnesses and the Disturbance marker |
| 5g | The level-change reset |

Aim went first because it is the smallest piece, and because while monsters can still shoot behind them a
give-up test is confounded — you cannot tell which fault you are watching.

**5a's design is still open.** The question was put on 2026-09-12 and set aside in favour of diagnosis:
*should being behind a monster that is already hunting you be safer than being in front of it?* Three seams
were identified, and the choice depends on the answer:

1. **Clamp the shot to the firing arc** (`ShootAtEnemy`) — being behind it becomes genuinely safe.
2. **Withhold the free last-known-position** (`CanSenseUnseenEnemy`, built and reverted 2026-09-02) — it
   misses rather than stops, and still fires through its own back.
3. **Leave aim alone; make monsters turn much faster** so facing catches up with aim. Fixes the *look*,
   keeps vanilla tracking, and gives repositioning almost no value.

Answer it **after** the move-wait fix, not before: a monster that actually turns around may make the
question look different.

The give-up that fixes it must key on **contact** (last sight, or last damage) rather than on the meter, or
a monster under fire from an unseen attacker will quietly time out mid-firefight.

Step 2 goes first despite not being the pillar's centrepiece, because it is the only part judgeable in
vanilla maps today — it needs no meter, no profile and no squad code.

### Deliberately deferred

Recorded so they are not rediscovered as gaps.

**Silent weapons.** `dlls/glock.cpp:77` is `// pev->body = 1;`. Set it and the model switches to its
silenced submodel, the shot drops to `QUIET_GUN_VOLUME` and `DIM_GUN_FLASH` (`dlls/glock.cpp:120-130`), and
`GLOCK_ADD_SILENCER` (`dlls/weapons.h:490`) is a real attach animation already in `v_9mmhandgun.mdl`.
Everything except the decision to expose it is done — and that decision belongs to
[Evolutions](#weapon-evolutions), where a silencer is the canonical example of "base weapon plus a small
alteration". Uncommenting it now would pre-decide the Evolutions identity question. The crossbow is already
`QUIET_GUN_VOLUME` (`dlls/crossbow.cpp:322`) and is the mod's existing quiet weapon whether anyone intended
it or not.

**A second Backstab tier.** The Backstab is positional and single-tier, so the damage model cannot tell a
stealth kill from a flank. If "measurably better off" proves too thin in play, a larger multiplier when the
victim has never acquired the player is the obvious lever, and it costs one branch on a test already being
made.

**Stealth Skills.** Ids **22** and **23** are reserved via `SKILL_RESERVED` so a stealth column can open
later without an id shuffle — the same move already made for the alien column. Nothing gets priced until
the mechanic has been played. Note the constraint: column 7 is spoken for by the alien branch, and
`CSkillTreeView` scales the whole tree to fit down to a `k_MinScale` floor of 0.55, so a ninth column risks
clipping icons that [ART_DEBT.md](ART_DEBT.md) already calls blocking rather than cosmetic.

**Generalising perception to monster-vs-monster.** Scoped to the player deliberately. The reasoning —
including the muzzle-flash asymmetry that makes a naive generalisation exactly backwards — is under
[Deliberately not generalised](PERCEPTION.md#deliberately-not-generalised). Marked for review, not for
building.

**The flashlight, and night vision.** Proposed 2026-09-12 and not decided: remove the flashlight entirely
and replace it with a night-vision device. Recorded because it is a stealth-motivated *item* change that
would otherwise live only in conversation. Two verified facts bear on it. `GETENTITYILLUM` reads the baked
lightmap only, so **the flashlight does not register in Concealment at all** — walking a dark corridor with
it on is currently exactly as concealing as walking the same corridor dark, which is the one obvious hole in
the light term and the cheapest thing night vision would close. And whatever icon night vision takes, it
must not be `flash_full`, which the Concealment readout is already borrowing as a placeholder
([ART_DEBT.md](ART_DEBT.md#the-concealment-readout--icon)).

### Open questions

- Is stealth **optional** everywhere, or are there encounters designed to be unwinnable head-on? The
  answer changes level design more than it changes code, and nothing settled above touches it.
- **How dark is dark?** Vanilla Half-Life maps are lit for readability rather than for hiding, so the light
  term may do almost nothing until there are custom maps with dark places in them. What light level counts
  as concealing is a question only [Maps](#maps) can answer. **Tuning the light term was explicitly deferred
on 2026-09-12** until there are maps worth tuning against; `conceal_light_dark` stays at its first guess of
0.5 and remains the mildest of the four terms on purpose.
- **Which encounters should stealth not be able to skip?** The mapper spawnflag exists to say so; nothing
  has decided when it ought to be used.

Everything else that used to be open here is settled and recorded in
[PERCEPTION.md part 2](PERCEPTION.md#part-2--the-model-this-mod-adds): whether monsters lose the player,
whether the player gets a readout, and how scripted sequences interact.

### Done when

A player can cross an occupied room without being seen, using choices they made — light, speed, weapon —
and is measurably better off for it than a player who fought through.

---

## Pillar 2: Weapons

### Custom weapons

**Shape: Idea.** The stated approach for all of these is to import and reuse existing assets — vanilla or
other mods — rather than author from scratch, and to accept placeholder quality on the first pass.

#### The Carbon Pickaxe

Replaces the crowbar. Heavier, slower, hits harder.

The mechanic is trivial — it is `dlls/crowbar.cpp` with different numbers and a different model. The cost
is entirely in what the crowbar's name is load-bearing for:

- **Five Skill ids are named for it**: `CrowbarRange` (1), `CrowbarDamage` (2), `CrowbarSpeed` (11,
  reserved), `CrowbarFollowUp` (18), and id 12 which was `CrowbarParry` before it became `PulseWindow`.
  Ids are frozen and must stay frozen; the C++ enumerator names and the display strings ("Crowbar Reach",
  "Crowbar Force") are free to change, and would have to.
- **CONTEXT.md's Follow-Up entry says "crowbar swing"** and would need rewording. Any rename here is a
  CONTEXT.md change in the same commit, per the glossary rule in CLAUDE.md.
- **`d_crowbar` is the icon for three Skills** (see [ART_DEBT.md](ART_DEBT.md)).
- `crowbar.cpp` compiles into both DLLs for prediction. That is no longer a blocker — `m_skills` is
  populated on the client — but the swing-rate change still has to reckon with the first-swing/follow-up
  damage rule, which reads the same timer. See [the prediction problem](#the-prediction-problem).

Open: does the player still find a crowbar somewhere, or is the pickaxe simply what melee *is* in this mod?
"Replaces" reads as the latter, which is cleaner — one melee weapon, one identity, and the vocabulary
problem gets solved once instead of twice.

#### The Gauss Katana

**Shape: Shaped, model first.** A gauss weapon shaped as a katana. Slow, bulky, and frightening.

**Settled 2026-09-12:**

- **Slower attack speed, big damage.** A melee weapon, on the crowbar's shape: `dlls/crowbar.cpp` with
  different numbers, a different model, and one new thing below.
- **Swings create gauss arcs that leave "burning" decals on walls, the way the gauss gun does. Built,
  v1, visual only.** `EV_KatanaArc` in `cl_dll/ev_hldm.cpp`, fired by `events/katana_arc.sc` from
  `CKatana::PrimaryAttack` once per swing. It copies the gauss gun's wall hit exactly rather than
  approximating it: the mark is the ordinary gunshot decal (`EV_HLDM_DecalGunshot` with
  `BULLET_MONSTER_12MM`) and the *burning* is the gauss glow sprite fading over six seconds on top of it.
  The arc itself is a crescent, a `)` standing in front of the player and tilted to the cut, that flies
  forward and burns the first wall its belly or either tip meets. It is a temp entity with no model and a
  per-frame callback (`EV_KatanaArcThink`) that moves it, redraws it as ten short beam segments bright at
  the belly and thin at the tips, and traces three points forward. The first version was a fan of
  lightning rays from the blade to the walls; the crescent replaced it at the user's request the same
  day. Seven client cvars, all first guesses: `katana_arc` (on/off), `_range` (1200, which is also how far
  it shrinks and dims to nothing, so running out never looks like a cut), `_radius` (70), `_sweep` (150°
  of the circle), `_speed` (1200), `_roll` (30°, sign alternating swing to swing), `_lean` (90: how far
  the belly turns from facing the player toward the line of flight, so it leads with its belly). The burn is glow only, one per
  point of the crescent, so the wall shows the crescent's silhouette along its flight; the gunshot
  decals were dropped because they read as bullet holes. A floor or ceiling does not end the flight; a
  tip that meets one scrapes a glow along it and the wave flies on, because aiming down at a headcrab
  used to kill the wave on the first floor tile.
- **The wave hurts. Decided 2026-09-12, when aiming down at headcrabs made a wave that only looked
  like an attack feel like a miss.** `CKatana::WaveAttack`, server-side: energy damage
  (`DMG_ENERGYBEAM`) to the first damageable thing on the aim line beyond the blade's own 32 units, at
  `katana_wave_damage_scale` (0.5) of base damage, falling off to nothing at `katana_wave_range` (1200),
  the same distance the drawn wave fades over, so what the player sees arrive and what arrives agree. A
  line trace first, then the small hull; never the large one, which finds the floor before a headcrab.
  The katana is therefore a **ranged melee weapon**: the blade for what is in reach, the wave for what is
  not, at half strength and less with distance. One target per wave for now; the drawn wave flies
  through monsters, and whether it should hit everything on its path is the next question.
- **The blade goes hot on the swing.** Two parts: a dynamic light at the hand (`katana_glow_light`, 0.9 s,
  decaying over its life, which is the fade the eye reads) and the blade's own texture swapping to a hot
  one (`katana_glow_hot`, 0.9 s). The hot blade is a second state in the skin families — six now, glove
  colour × cold/hot, glove-major — with the blade metal split onto a material of its own so only it
  swaps, and the hot texture flagged **additive** by `utils/mdltool/mdlflags.py` after the compile. It
  is drawn as light over what is behind it, so it shines in the dark and is a little transparent. Two
  approaches were built and rejected first: a beam entity between two viewmodel attachments (the engine
  draws beams before the viewmodel with last frame's attachments, so it trailed the swing) and quads drawn
  by the studio renderer in the viewmodel's pass (right place, wrong look). The attachments stay in the
  model for whatever wants them next. The glove half of the six families is picked by the player's Suit
  Variant in `cl_dll/view.cpp`; see PILLARS.md.
- **The weapon exists, v1.** `weapon_katana`, `dlls/katana.cpp`: `CCrowbar` with two hooks overridden,
  base damage (`sk_plr_katana1-3`, 40) and swing time (`katana_swing_time_scale`, now 1.0, the crowbar's
  own rate — it started at 2× and the wave made the slow swing feel like waiting; read from both DLLs
  because it is predicted), and its own models. Backstab, Crowbar Force and Reach, and
  the Follow-Up come along unchanged, which is the point of subclassing rather than copying. In the
  melee bucket beside the crowbar; `impulse 101` gives it; the FGD places it. See PILLARS.md.
- **Viewmodel and world model are the mod's own.** The Dystopia blade on Half-Life's crowbar hands
  and the crowbar's eleven animations in their original order, so the crowbar's `CROWBAR_*` sequence
  indices drive it unchanged, and the Dystopia prop lying flat for the floor. Sources in
  `E:\CustomAssets\models\src\{v,w}_katana`, made by the loop in [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md).
  The `p_` model, the sounds and the HUD icon are the crowbar's; [ART_DEBT.md](ART_DEBT.md) has them.

Still open: does it consume uranium, like the Gauss and Egon? Does it charge, the way
`GAUSS_PRIMARY_CHARGE_VOLUME` implies for the gun? `DMG_ENERGYBEAM` is the natural damage type and PILLARS
records why (the alien slave is the only thing immune to `DMG_SHOCK`, and `DMG_ENERGYBEAM` has no immunity
anywhere) — that reasoning applies here unchanged. Whether the crowbar's own swing animations are enough
for a heavy weapon, or the Dystopia swings get retargeted onto the stock rig, is the first thing v1 in
game will answer. Still a candidate for [Evolutions](#weapon-evolutions).

### Weapon evolutions

**Shape: Shaped, and the answer is already in the codebase.**

A base weapon plus a small alteration — a silencer, a second barrel, an extended magazine.

The design question that matters is identity: is an Evolution a *new weapon* or *state on an existing one*?
[ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md) puts weapons in Half-Life's own
`WeaponId` space, and Half-Life has a finite number of slots, an `ItemInfo` row per weapon and a HUD bucket
layout that assumes them. Doubling the weapon count to express "same gun, plus a tube" would spend that
budget badly.

State on the existing weapon is the answer, and **the glock already ships it**: `pev->body == 1` selects
the silenced submodel and the fire code branches on it (`dlls/glock.cpp:120-130`). That is an Evolution,
built, in the base game, on the exact weapon the user identified. It needs to become durable player state
that saves and syncs rather than a body value nobody sets.

Consequences to settle before building:

- Where does an Evolution **live**? It is durable per-weapon state, so it wants to sit with the weapon
  rather than in `CPlayerInventory` — but a dropped weapon keeps its clip today and would have to keep its
  Evolutions too, or dropping becomes lossy (the inventory design is explicit that dropping and retaking is
  exactly lossless).
- Is an Evolution **found** or **bought**? Found argues for an Item Type and a pillar 1 reward; bought
  argues for Skill Points and makes the Armaments column mean something. The user's list puts weapon
  handling upgrades under "upgrade points invested", which points at bought.
- Does the **viewmodel** have to change per Evolution? A silencer does (submodel), a magazine may not.
  This is the cheapest possible Evolution to ship first for exactly that reason.

### Weapon handling

**Shape: Started.** Upgrades that increase reload speed and draw speed, and swap in more skillful handling
animations as they go — the weapon visibly getting better in the player's hands.

The first tier of the reload half is **built**: `FastReload` (id 3) is unreserved and in the Armaments
column, scaling `DefaultReload`'s delay by `skill_reload_time_scale`. What remains here is draw speed
(`DefaultDeploy`, the same shape and unblocked), the shotgun's shell-by-shell reload, which never calls
`DefaultReload` and so is untouched by Fast Reload, and further tiers.

The animation half is separate and is now the *visible* gap rather than a future one: a faster reload with
the same animation reads as the animation being cut off, which is exactly what happens today at 0.8×. Each
tier wants its own sequence. That is model work per weapon per tier, and it is the part that decides
whether "more skillful" reads at all.

The user's note suggests folding these into [Evolutions](#weapon-evolutions) rather than keeping them as
Skills. Worth deciding early — they are the same feature from two directions, and building both would
mean two systems making the same gun faster.

### Viewmodel hands and the custom HEV suit

**Shape: Shaped. Three of its five goals are built** — the glove textures, their rollout across every
viewmodel, and the Suit Variant that picks between them. What is built is in [PILLARS.md](PILLARS.md);
what is left is goals 4 and 5 below.

Every `v_*.mdl` in Half-Life bakes its own hands into the model. There is no shared hand mesh, so
consistency means touching every viewmodel the mod ships — the vanilla set, the Carbon Pickaxe, the Gauss
Katana, and every Evolution that changes a viewmodel.

Worth being honest about the trade: a player who does not go looking will never consciously notice
consistent hands, and will absolutely notice inconsistent ones. It is a floor, not a feature. It is also
the sort of thing that gets cheaper the earlier it is decided and much more expensive once there are ten
custom viewmodels to redo.

**The philosophy:** the mod has its own HEV suit, and the viewmodels reflect it. The goals, in the order
they build on each other:

1. **Three glove colour variants: cyan, red, purple. Built.** Grey plates with light channels in the
   seams and a readout on the back of the hand, in the variant's colour, from
   `E:\CustomAssets\scripts\hev_gloves.py`, generated per model from that model's own stock textures.
2. **Every vanilla viewmodel gets the new glove textures. Built, as skins.** Fourteen stock viewmodels
   (every one that has a glove: the hivehand has none, the chumtoad is unused) plus the katana compile
   with all three sets as skin families, cyan as skin 0. Compiled files live in the repo's `models/`
   and the install's `topmod/models/`.
3. **Picking a suit variant sets the glove colour of every viewmodel, and the HUD's. Built** — the Suit
   Variant, a world pickup carried in the player's `pev->skin`. See [PILLARS.md](PILLARS.md) for what it
   does and [CONTEXT.md](../CONTEXT.md#the-suit) for the term.
4. **A custom HEV suit 3D model, in the same three variants.** The suit the player sees — on a pickup, a
   charger, a mirror, the player model — matching the gloves. **The one goal here still unbuilt, and it
   is design work before it is model work: it is not designed yet.** Until it is, the pickup is the stock
   `w_suit` recompiled with three colour-washed skins (`E:\CustomAssets\scripts\suit_world.py`), which is
   in [ART_DEBT.md](ART_DEBT.md) and which also gives the real suit three references to be designed
   against. Note what the stand-in does *not* cover: the wall chargers, and the player model, which still
   shows the stock orange suit in a mirror or in third person.
5. **The gloves emit light.** Later, and the route is now proven on the katana's hot blade. The engine
   **ignores `STUDIO_NF_FULLBRIGHT`** on studio textures — the katana tested it — and **honours
   `STUDIO_NF_ADDITIVE`**, drawing the texture as light over what is behind it, which shines in the dark
   at the cost of some transparency. `utils/mdltool/mdlflags.py` patches either flag into a compiled
   `.mdl`. For the gloves: split the light channels onto a texture of their own in the generator, give
   those faces their own material in the reference SMD (a mesh edit, since the seams are painted on the
   sleeve's faces today), compile, patch additive. The transparency is the open question for a seam that
   is always on, where it was fine for a blade lit for under a second. A model that glows in the dark is
   also a stealth question: PERCEPTION.md's light term reads the lightmap, not the viewmodel, so it costs
   nothing in Concealment, and that is worth stating when it lands.

Known issue from the first pass: on the crossbow the hand clips slightly through the stock. Not from the
textures; recorded in [ART_DEBT.md](ART_DEBT.md) for later.

---

## Pillar 2: Monsters and bosses

**Shape: Idea. A roster was listed on 2026-09-12, to be explored one entry at a time; the Panthereye is
first and is the only one explored so far.**

Import and reuse existing monsters with light polish, with occasional custom AI. **Kingpin**, a cut
Half-Life monster, was named as a candidate before the roster and is not on it; it stays here until
someone says whether it was dropped.

### The roster

| Entry | Kind | Starts from | Shape |
| --- | --- | --- | --- |
| [Panthereye](#panthereye) | Enemy | Half-Life's cut model and sounds; no AI | **Shaped** |
| [Melee alien grunt](#melee-alien-grunt) | Enemy | `CAGrunt`, bare arm | **Shaped** |
| [Shelled headcrab](#shelled-headcrab) | Enemy | `CHeadCrab`, recoloured | **Shaped** |
| [Friendly alien slave](#friendly-alien-slave) | Non-combatant | The slave model on `CTalkMonster` | **Shaped** |
| [Alien slave boss](#the-alien-slave-boss) | Boss, freed to become the friendly slave | `CISlave` | Idea |
| [Assassin boss](#the-assassin-boss) | Boss | `CHAssassin` | Idea |
| [Alien grunt boss](#the-alien-grunt-boss) | Boss | `CAGrunt` | Idea |
| [Nihilanth](#the-nihilanth) | Boss | `CNihilanth`, new model | Idea |

This also answers the old question of what a boss "moveset" meant: all four bosses are an existing monster
given **custom attacks**, and two add something more (a dash, and turning into an ally). None is a puzzle.

### Panthereye

**Shape: Shaped 2026-09-13. The behaviour is decided (below); no code.**

**The model is Half-Life legacy content**, a monster Valve cut, not another mod's art. The copy found on
this machine is in *Half-Life: Extended* (`Half-Life/hl_extended`, per its `liblist.gam`), which ships
`models/panthereye.mdl`, ten sounds in `sound/panthereye/`, an FGD entry and skill cvars. None of it is in
this repo, `E:\CustomAssets` or `topmod` yet. Importing means copying the model into the repo's `models/`
and `topmod/models/`, and the sounds into `topmod/sound/panthereye/`, which does not exist yet.

**Its AI exists, but only compiled.** `hl_extended/dlls/hlex.dll` contains the classes `CPanthereye` and
`CCinePanther` (`monster_cine_panther`, on `models/cine-panther.mdl`, which is not in the folder). There is
no source, so "needs AI" means writing it. The strings still show what the AI does:

| Evidence | What it says |
| --- | --- |
| `sk_panthereye_health` 60 / 70 / 80 / 100 | Four difficulty levels, where this mod has three. Somewhere between a grunt and an alien grunt |
| `sk_panthereye_dmg_claw` 13 / 15 / 20 / 25 | A claw attack |
| `sk_panthereye_dmg_leap` 20 / 25 / 35 / 35 | A leap attack, harder than the claw |
| FGD: `size(-32 -32 0, 32 32 64)`, `SquadLeader` flag, `body` Diablo / Nightkin | A large hull, squads, two looks |

**What the model gives the AI to work with** — read with `utils/mdltool/mdlinfo.py` plus a sequence dump:

| Sequences | Tagged as | Notes |
| --- | --- | --- |
| `get_bug`, `itch`, `shakes` | `ACT_IDLE` | Three idles |
| `subtle_motion`, `idle_figit` | `ACT_CROUCHIDLE` | It rests crouched |
| `walk`, `run` | `ACT_WALK`, `ACT_RUN` | About 48 and 200 units/s |
| `turn_left`, `turn_right` | `ACT_TURN_*` | |
| `walk_to_crouch` | `ACT_CROUCH` | |
| `crouch_to_jump` | `ACT_MELEE_ATTACK1` | **The leap.** Event 5 at frame 11 |
| `attack_primary`, `attack_main_claw`, `attack_simple_claw` | `ACT_MELEE_ATTACK2` | Three claws. `attack_primary` is weighted 5 and hits twice (events 1 and 2); the others hit once (events 3 and 1) |
| `flinch_light`, `flinch_hard` | small and big flinch | |
| `death_drop`, `death_simple`, `death_violent` | `ACT_DIESIMPLE` ×2, `ACT_DIEVIOLENT` | |
| `crawl_on_belly`, `crouch_to_crawl`, `walk_to_stand_1/2`, `flinch` | nothing | **A stalking crawl**, about 43 units/s, with the transitions in and out |
| `eat_idle`, `eat_turn` | nothing | Feeding on a corpse, with chewing sounds baked in as events. `ACT_EAT` exists and is not used |
| `wgh_drag` | nothing | 212 frames. Probably a scripted sequence for the cinematic version |

Other facts about the model:

- **Its head is hitgroup 2, not 1.** Half-Life's head is 1, which is what doubles damage
  (`dlls/combat.cpp:1337`) and what [Decapitation](#pillar-2-decapitation) keys on. Every other hitbox is 0.
  As shipped, a headshot counts as a chest hit. `TraceAttack` can remap it in one line, with no recompile.
- **Its eyes glow.** Each of the four face textures has a `_Light` twin flagged additive and fullbright
  (`0x24`). The engine honours additive (see the katana's hot blade), so the eyes are drawn as light and
  show in the dark. That is a stealth fact: the player can see a Panthereye in a room they cannot see into.
- **Two body models, red and blue.** Body 0 is red (`Panther_Bodyfull`), body 1 is blue
  (`Panther_Bodyfull_Blue`). The FGD calls them Diablo and Nightkin. Both use the same red glowing eyes.
- **The four skins are eyelid states, not variants.** Skin 0's eye is open, 1 half-shut, 2 narrowed,
  3 shut and dark. That is enough to blink, and a shut eye is an eye that does not glow.
- 49 bones including a tail, no attachments, 40 hitboxes.
- Sounds: two alert, one attack, one death, four idle, two pain.

**What exists to write the AI from.** The headcrab's leap is the template for the jump: an anim event
(`HC_AE_JUMPATTACK`, `dlls/headcrab.cpp:29`) sets a velocity, and a touch function deals the damage
(`LeapTouch`, `:341`). Bullsquids and houndeyes are the squad-capable animal pattern. Both attacks would be
`DMG_SLASH`, which is on the Shield's list ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)),
so the Pulse counters them with no extra work. It needs a Perception Profile (every monster has one) and an
answer to whether it can be backstabbed.

#### Settled 2026-09-13

- **It stalks.** Once it knows about the player it closes in from cover to cover, crawling on its belly
  toward them, and growls quietly the whole time. The growl is the player's warning. It is the monster side
  of pillar 6: here the player is the one being hunted.
- **Close up, it slashes.** In claw range it attacks whether or not it has been spotted.
- **Spotted, it rushes and pounces.** When the player sees it, it stops hiding, runs at them and leaps from
  moderate range.
- **It never pounces unspotted.** A leap the player could not have seen coming is not fun, so the pounce is
  only ever the answer to being seen. Being attacked out of nowhere is limited to the slash, and the growl
  warns before that.
- **Alone.** No squad code for now: a `CBaseMonster`, not a `CSquadMonster`, and the FGD's `SquadLeader`
  flag is dropped.
- **Feeding is a placeable ambush.** A mapper can place it eating, using `eat_idle` and `eat_turn`, which
  exist for this. The player can come across it busy and choose to sneak past, strike first, or leave.
- **The red one is the alpha.** Different stats, and it leaps more aggressively. *Assumed:* "the red eye"
  means body 0, the red body. Both bodies have the same red eyes, so the eyes cannot tell them apart, and
  the blue body is the normal one.

#### What the settled behaviour costs

- **"Spotted" is a new test, the reverse of Suspicion.** Everything in [PERCEPTION.md](PERCEPTION.md) is a
  monster perceiving the player. This is the Panthereye asking whether *the player* can see *it*: is it
  inside the player's view cone, with a clear line from the player's eyes. It has to be written, and its
  tuning decides the whole encounter. Too wide and it pounces the moment it enters the edge of the screen;
  too narrow and it is never spotted at all. Its glowing eyes help the test be fair in the dark.
- **Cover to cover does not exist yet.** `CBaseMonster::FindCover` (`dlls/monsters.cpp:2270`) finds a node
  the threat cannot see within a distance band, but it does not prefer nodes *closer* to the threat. Getting
  closer through hidden nodes is a new search, built from the same node graph.
- **The crawl has no activity tag**, so the base AI will never choose it. Movement plays `ACT_WALK` or
  `ACT_RUN` (`dlls/schedule.cpp:1128-1151`). The Panthereye picks `crawl_on_belly` by name while stalking,
  with `crouch_to_crawl` and `walk_to_stand_*` as the transitions. Feeding is picked by name the same way.
- **There is no growl sound, as far as the names go.** The ten sounds are alert, attack, death, idle and
  pain. An idle might pass for a growl; if not, it is a new sound and an [ART_DEBT.md](ART_DEBT.md) entry.
  It plays at low volume and short range. It is a cue for the player and never enters `CSoundEnt`, which a
  monster's own sounds do not anyway.
- **The alpha is a keyvalue, not a second classname**: the body picks the stats, the way `item_suit`'s
  `variant` does.

#### Also settled 2026-09-13

- **Spotted is permanent.** Once spotted, it switches to full combat for good: rush, slash, pounce. Looking
  away does not send it back to stalking. The encounter has two modes, **stalking** and **combat**, and
  moves one way between them.
- **Hurting it counts as being spotted.** A Panthereye the player damages goes straight to combat.
- **The alpha leaps from further away**, and may skip the stalk entirely and fight head-on from the start.
  "May" is how it was put; it is the likely shape rather than a decision.
- **The stalk starts the easy way: when it acquires the player.** Its own perception already exists: `Look`,
  gated by Suspicion, gives it an enemy exactly as it does every other monster. Acquiring the player *is*
  the start of the stalk, since stalking is simply what it does with an enemy it has not been spotted by.
  That needs no code, where a map trigger would need a `Use` handler. The trigger stays a later option if
  mappers want one.
- **A feeding Panthereye wakes three ways:** a map trigger, being hurt, or noticing the player through its
  own Suspicion. Noticing was only wanted if it came cheap, and it does, as long as feeding is the
  Panthereye's own behaviour rather than a map's `scripted_sequence`. See
  [How the bullsquid feeds](#how-the-bullsquid-feeds).

Because spotted is one-way, a wrong answer from the "spotted" test is permanent too. A test that fires when
the Panthereye is only at the very edge of the screen costs the whole stalk, so the test should lean strict.

**Stats are deferred**, by decision: they are set later, against a map. The legacy cvars (60–100 health,
claw 13–25, leap 20–35) are where the normal one starts; the alpha has no numbers.

#### How the bullsquid feeds

Read 2026-09-13, because it is the base game's only monster that already eats, and the Panthereye's feeding
should be built on it rather than beside it.

**The bullsquid goes looking for food; nobody places it eating.** Food is a **scent** in `CSoundEnt`, and
two things leave one:

| Scent | Left by | Lasts |
| --- | --- | --- |
| `bits_SOUND_CARCASS` | A monster's corpse, once it has finished dying (`dlls/schedule.cpp:475`). Not a monstermaker's, which fades out instead | 30 s, radius 384 |
| `bits_SOUND_MEAT` | A gib that comes to rest, if it bleeds (`dlls/combat.cpp:899`) | 25 s, radius 384 |

When it smells either (`bits_COND_SMELL_FOOD`), `CBullsquid::GetSchedule` (`dlls/bullsquid.cpp:1021-1035`)
sends it to eat. If the food is behind it or out of sight, it plays a sniff first
(`SCHED_SQUID_SNIFF_AND_EAT`). The schedule (`tlSquidEat`, `:898`) remembers where it was, walks to the scent,
plays `ACT_EAT` three times, is marked full for 50 seconds (`TASK_EAT`, which only sets
`m_flHungryTime`, `dlls/monsters.cpp:149`), and walks back to where it started. It does this in combat as well
as when alert (`:1072`). Garbage gets the same treatment with an inspect animation instead (`SCHED_SQUID_WALLOW`).

**What ends a meal: damage, or a new enemy** (the schedule's interrupt mask, `:920-922`). That second one is
what makes Suspicion free. `RunAI` runs `Look` every think for any monster in the player's PVS, whatever
schedule it is in (`dlls/monsterstate.cpp:82-90`). So Suspicion keeps filling while it eats, and the moment it
completes the monster gains an enemy and the meal breaks. No perception code is needed.

**The exception is a `scripted_sequence`.** Suspicion is frozen while a script holds a monster
([PERCEPTION.md](PERCEPTION.md), the `m_pCine` row). A Panthereye placed feeding through the map's own
scripting would be blind until released. Feeding therefore has to be a state of the Panthereye itself: a
keyvalue it spawns with, starting a looping eat schedule, woken by a `Use` from the map trigger.

Three traps it shows up:

- **`ACT_EAT` finds nothing on the Panthereye.** `eat_idle` and `eat_turn` carry no activity tag, so
  `TASK_PLAY_SEQUENCE ACT_EAT` fails. The eat schedule plays them by name, like the crawl.
- **A scent registers only if the current schedule's sound mask includes it.** Every bullsquid schedule that
  should notice food lists `bits_SOUND_MEAT | bits_SOUND_CARCASS` for exactly that reason, and the code says
  so in a comment (`:924-927`). A Panthereye schedule that forgets it never smells anything.
- **`CBullsquid::IgnoreConditions` overwrites the base ignores rather than adding to them** (`iIgnore =`, not
  `|=`, `:249` and `:257`), and its comment says the opposite of what the test does: it ignores smells for 20
  seconds *after* being hurt. Vanilla behaviour, not worth copying as written.

**Not decided: should the Panthereye also hunt for food like the bullsquid?** The machinery is there for
free. A Panthereye drawn to corpse scents would come to the player's kills, so a trail of bodies would bring
the stalker to the player. That is a strong stealth idea, but nobody has asked for it. It also overlaps the
planned **Disturbance** marker ([PERCEPTION.md](PERCEPTION.md#death-witnesses-and-the-disturbance)), the
mod's own version of "a death leaves something behind". The two should be reconciled before either is used
for this.

### Melee alien grunt

**Shape: Shaped 2026-09-13. The first version is decided and needs no new art; the charge and the weapon
wait for model work.**

In the Half-Life alpha and beta the alien grunt was planned as a melee monster carrying an **alien
chainsaw**. This entry brings that back in two stages: a bare-handed brawler now, the chainsaw later.

#### Settled

- **Melee only, on the animations it already has.** `CAGrunt` already punches: `mattack2` and `mattack3`,
  both `ACT_MELEE_ATTACK1`, fire `AGRUNT_AE_RIGHT_PUNCH` (13) and `AGRUNT_AE_LEFT_PUNCH` (12). Each is a hull
  trace out to `AGRUNT_MELEE_DIST` (100) for `sk_agrunt_dmg_punch` as `DMG_CLUB`, shoving a player 250 units
  sideways (`dlls/agrunt.cpp:523-581`). The hornet attack is switched off: `CheckRangeAttack1` answers false.
  (`AGRUNT_AE_PUNCH` and `AGRUNT_AE_BITE` are defined at `:58-59` but nothing uses them; the model never fires
  them.)
- **No hivehand on its arm.** The model's `arm` bodypart has two submodels: `Gun_arm` (0) and `Bare_arm`
  (1), so `pev->body = 1` removes the gun. The one attachment is on `Bip01 R Hand`, which is where punch blood
  is drawn, so it still lands on the fist.
- **A charge is wanted, but needs an animation that does not exist.** No alien grunt model on this machine has
  one: stock, HD, Half-Life: Extended's four (`agrunt`, `_2`, `_3`, `_noarmor`) and Half-Life: Echoes' all
  carry the same 32 sequences, reskinned. A charge is new animation on the stock rig, which starts with a
  decompile.
- **The alien chainsaw is designed from the ground up, later.** Until it exists the bare arm stands in, and
  that stand-in is an [ART_DEBT.md](ART_DEBT.md) entry when this is built.

#### What the base AI does to a melee-only grunt

**Only one grunt in a squad will chase.** `GetSchedule` (`dlls/agrunt.cpp:1097-1102`) sends a grunt after
the player only if it can take `bits_SLOT_AGRUNT_CHASE`, which is a single bit (`dlls/squadmonster.h:40`).
Every other squad member gets `SCHED_STANDOFF`: stop, stand idle, face the enemy for two seconds, repeat
(`:690-708`). With hornets, the ones standing still were shooting. Without them, a squad of four melee grunts
is one brawler and three spectators who fight only if the player walks into reach. **This is the one change
the first version cannot skip.**

**Its armour cancels the player's melee.** Hitgroup 10 (pelvis, feet, upper spine, back of the head, upper
arms) takes 20 off any `DMG_BULLET`, `DMG_SLASH` or `DMG_CLUB` hit and ricochets it (`:221-257`). The
crowbar's 10 becomes 0.1. A full-investment Backstab (49.5) becomes 29.5, and the katana's 40 becomes 20. The
katana's wave is `DMG_ENERGYBEAM` and ignores the plates. So a monster built to be met in melee is, on most
of its body, immune to the crowbar: a real design decision, not a detail.

Smaller things worth knowing:

- **The Pulse counters the punch.** `DMG_CLUB` is on the Shield's list
  ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)), so a deflect primes the Follow-Up. The
  Follow-Up's swing then meets the armour above.
- **Hitboxes do not follow submodels.** The right forearm's hitbox (group 5) reaches 51 units, the length of
  the gun, so a shot through the empty air where the hivehand would be still hits.
- **The aim problem goes away for this variant.** Hornets were the one ranged attack
  [the post-aggro step](#the-post-aggro-step) had no answer for; a grunt without them has nothing to aim.
- `FCanCheckAttacks` is overridden so alien grunts can attack enemies they cannot see (`:877`). Harmless
  here: `CheckMeleeAttack1` still requires sight (`:895`).
- It keeps the alien grunt's Perception Profile, `g_ProfileTrained`.
- Every alien grunt model has two unused sequences, `attack3_1` and `attack3_2` (16 frames, looping, no
  events, no activity). What they show is unknown: worth a look in a model viewer before a charge animation
  is made from nothing.

#### Also settled 2026-09-13

**The purpose is monster variety.** Each alien grunt variant is there to make an encounter play differently,
not to replace the stock grunt.

- **Mixed squads.** Melee and hornet grunts fight in the same squad: hornet grunts hold back and shoot, melee
  grunts close in. That settles the chase slot: a melee grunt chases without competing for
  `bits_SLOT_AGRUNT_CHASE`, so every melee grunt in a squad is in the fight. Hornet grunts keep the slot
  rules they have.
- **No armour on the melee variant.** Its `TraceAttack` skips the hitgroup 10 ricochet, so the crowbar,
  the Backstab and the Follow-Up all work on it. *Visual gap:* the stock model still wears the plates, so it
  looks armoured and is not. An unarmoured body is art, and a second ART_DEBT line. Half-Life: Extended's
  `agrunt_noarmor.mdl` shows the idea exists; where it came from is unknown.
- **The charge knocks the player back, and the Pulse blocks only its damage.** A deflected charge still
  shoves. The stock punch already behaves like this: `CheckTraceHullAttack` calls `TakeDamage` but ignores
  its result and returns the entity it hit (`dlls/combat.cpp:1363-1372`), and the punch applies its shove to
  whatever comes back. The one thing to fix while there is cosmetic: a deflected punch still plays the hit
  sound and draws blood on the player, when nothing landed.
- **The alien chainsaw should, ideally, be a weapon the player can take.** That makes it a new player weapon
  in Half-Life's `WeaponId` space ([ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md)), with
  a viewmodel, and a relative of the Carbon Pickaxe and the Gauss Katana: a third melee weapon, which forces
  the question of what the melee bucket holds.

#### Wanted: a shielded variant

**Shape: Idea**, raised 2026-09-13. An alien grunt that carries a shield of some kind. Nothing is decided,
including whether it is melee, hornet or both.

**The word is taken.** **Shield** is settled vocabulary in [CONTEXT.md](../CONTEXT.md) for the field a Pulse
raises, and CONTEXT.md exists to stop one word meaning two things. This needs its own name before it is
written into code or commits.

What it would build on: the armour code this entry removes from the melee grunt is already a working
"this part of me stops hits" (`CAGrunt::TraceAttack`). A shield is the same test keyed on **direction**
rather than hitgroup: hits from in front are stopped, hits from behind land. That is exactly the rear-arc
test the Backstab already makes (`CBaseMonster::FInRearArc`, `dlls/combat.cpp`), so a shielded grunt is a
monster the player has to get behind, and pillar 6 already gives them the tools.

Open: carried in the hand or worn? Can it be broken? Does it block the katana's wave and the Pulse's
Discharge, which are energy? A model with a shield is new art, like the chainsaw.

#### Still open

- **Classname or keyvalue?** To be decided. A keyvalue on `monster_alien_grunt` keeps stock maps unchanged,
  and the bare arm itself could be that switch. With a shielded variant coming as well, the answer covers
  three kinds of grunt rather than two.
- **The chainsaw's design**, from the ground up, and the charge animation. Both wait on model work.

### Shelled headcrab

**Shape: Shaped 2026-09-13. The loop is decided; how the belly is shown and found is not.**

A headcrab with an armoured shell and a soft belly. Proof of concept on a recoloured stock headcrab.

#### Settled

- **It exists to teach the Pulse.** Parry the leap, then hit the exposed belly. The shell is what makes
  parrying the good answer rather than one answer among several.
- **The shell works like the alien grunt's armour**: ricochet, tracer, and a cut to the hit, the
  `CAGrunt::TraceAttack` pattern (`dlls/agrunt.cpp:221-257`), applied everywhere except the belly.

#### Why the loop is cheap

**The headcrab can already tell it was parried.** Its leap damages through `LeapTouch`
(`dlls/headcrab.cpp:341-362`), which calls the player's `TakeDamage` and ignores the result. The player's
`TakeDamage` returns **false** when a Shield turns the hit away (`dlls/player.cpp:413-416`). Reading that
return value is the whole detection: a false from a player means *deflected*, and the shelled crab reacts.
(`TakeDamage` also returns false when the game rules refuse damage, `:403-406`. That never happens to a
monster hitting a single-player player, but a strict version asks the Pulse, `WouldNegate`.)

**The leap is a fair thing to parry.** It is telegraphed twice: the attack sound plays when the leap starts
(`StartTask`, `:384`) and the jump animation winds up before the launch event. The Pulse has already been
confirmed to feel good against headcrabs ([PILLARS.md](PILLARS.md), pillar 2).

**A deflect already rewards a melee hit.** The Follow-Up, primed by that same deflect, empowers the next
connecting crowbar swing. The shelled crab makes that loop *required* rather than merely good. The flip
below works without the Skill; the Skill makes the punish hit harder.

#### What has to be built or decided

**The belly is already exposed by a deflect. Observed in play, 2026-09-13.** A deflected headcrab is
still in its lunge animation, belly toward the player, and the Pulse leaves it right in front of them. So
the punish window needs no new state, no flip and no animation: it is **the moment between the deflect and
the end of the lunge**. (A flipped-over crab was considered first and is not needed. No headcrab model has
such an animation anyway.)

**Where the window ends** has two natural candidates, both already tracked by the headcrab: the lunge
sequence finishing (`m_fSequenceFinished`, which is where `RunTask` ends the attack, `dlls/headcrab.cpp:322`)
or the crab landing (`FL_ONGROUND`). The window is short, which is right for a parry reward, and its exact
length is a tuning question for play.

**How a hit knows it found the belly.** Two routes, cheapest first:

1. **Any hit inside the window counts as a belly hit.** The deflect sets a short timer on the crab; hits
   while it runs skip the shell. No hitboxes, no geometry. It matches what the player sees, since a crab
   deflected in front of them is showing its belly. The obvious first version.
2. **The hit direction**: a hit travelling into the crab's underside counts, window or not. It also rewards
   shooting a crab out of the air with no parry, which may or may not be wanted: the entry exists to teach
   the Pulse, and this route teaches aiming instead.

A belly hitbox compiled into the model is a third route and the most expensive; the stock headcrab's
thirteen hitboxes are all hitgroup 0.

**What the shell does to a hit, with numbers.** The alien grunt's cut is a flat 20, and a headcrab has 10,
10 or 20 health (`sk_headcrab_health`, `valve/skill.cfg`). A flat 20 therefore makes every weapon under 20
per hit do nothing: the crowbar, the glock, each shotgun pellet. Anything over it kills in one: the magnum,
the crossbow, the katana's 40. That is "immune except to big guns", which undercuts the point. The cut wants
to be scaled to the crab, or full immunity, and it needs to say which damage types it stops. The alien
grunt's stops only bullets, slashes and clubs, so explosions, the katana's energy wave and the Pulse's own
Discharge would all go through.

**The Discharge may skip the lesson.** A melee deflect fires a Discharge at the crosshair
([ADR-0006](adr/0006-the-discharge-vents-at-the-crosshair.md)), and a player aiming at a leaping crab is
aiming at it. If the Discharge kills a flipped crab outright, the player never swings. That is either a fine
reward for the Skill or a hole in the design; `pulse_discharge_melee 0` exists to compare.

**Traps:**

- **The Follow-Up's knockback only throws headcrabs by classname** (`monster_headcrab`, `monster_babycrab`,
  `dlls/player_pulse.cpp:373-374`). A shelled crab with its own classname is not thrown unless it is added
  there. The knockback is applied *after* the Follow-Up's damage, so the punish hit lands first and
  the throw only moves what survives it.
- **It cannot be backstabbed**: `CHeadCrab` opts out, and a subclass inherits that. Correct here.

#### Art

- **The recolour is a recompile.** `headcrab.mdl` has no skin families, so the shelled crab is a new model
  file, starting with a decompile.
- **Half-Life: Extended's headcrab has a belly texture that opens.** Its model carries two skin families,
  `bottom.bmp` and `bottom_open.bmp`: a precedent for the belly visibly changing when it becomes the target.
- An ART_DEBT.md line when built, naming the recolour as the stand-in for a real shell.

#### Open

- **Window's end:** the lunge sequence finishing, or the crab landing?
- **Before the Pulse is found.** The Pulse becomes a found Module ([Modules](#pillar-3-modules)). Before
  that point a shelled crab can only be beaten by big hits, so maps should not place one before the Pulse,
  or there should be a second, harder way to flip it.
- **Can anything else open the window?** An explosion, a shot that interrupts the leap. Or is the Pulse the
  only key, by design?

### Friendly alien slave

**Shape: Shaped 2026-09-13.** An alien slave who lives in his lab and talks to the player. Not an ally in
combat.

#### Settled

**Who he is.** He is the [alien slave boss](#the-alien-slave-boss), freed. The player fights a special alien
slave under the Nihilanth's control; defeating it breaks that control, and from then on he helps the player
progress the game **through unlockables and information, never through combat**. This entry is what he is
after the fight. Settled 2026-09-13.

- **There is one of him.** A single character, not a kind of monster a mapper places.
- **He does not fight.** No following the player, no guarding a zone, no zap, no claws.
- **He stays in his lab**: talks to the player, now and then walks from one place to another, and performs
  animations.
- **His speech grows up with the game.** At first he speaks only alien words. As time passes and the player
  progresses, he sometimes says English words among them. The English is imported voice lines, from
  Half-Life 2's vortigaunts and/or Black Mesa.
- **Shooting him ends the game**, the way killing critical personnel does in Half-Life, but on the first
  hit rather than on death. He may flinch as it happens.
- **+use gets one line.** No conversations on the use key. Conversations are map-authored: a trigger when the
  player enters his room plays them.
- **He hands things over by working a machine.** He goes to a device, operates it, and it "prepares" the item;
  then a door on the device opens and the player picks the item up.
- **The loop is fixed.** Stops in a set order, because either was acceptable and fixed is cheaper.
- **He moves between points of interest in a loop**, stopping at each. The scientists were checked for such a
  thing first, and do not have one (below).

#### What the base game already gives

**Most of it is `CTalkMonster` with the fighting left out.** Barney and the scientists get idle chatter, a
hello, looking at the player, noticing being stared at and answering +use from `CTalkMonster`
(`dlls/talkmonster.cpp`). Following is **one function** on top of that, `FollowerUse` (`:1398`), attached by
`SetUse` in each monster's `Spawn` (`dlls/scientist.cpp:691`). A talker that answers +use with speech instead
of following is the same class with a different use function. The slave has none of this today: `CISlave` is a
`CSquadMonster` built to fight, so the friendly one is a new talk monster on the slave's model, not a
modified `CISlave`.

**The model already talks and already performs.** Read with the sequence dump:

| Sequences | Use here |
| --- | --- |
| `jibber`, `jabber` | **Talking.** Looping, untagged, with alien words (`aslave/slv_word3/4/5/7`) baked in as sound events |
| `collar1`, `collar2` | Tugging at his collar |
| `pushup`, `grab`, `updown`, `downup` | Untagged set pieces, what they show is worth a look in a model viewer |
| `idle1`–`3`, `walk1`–`2`, `left`, `right`, `crouch` | Standing, moving and turning |

**His voice is alien words, not speech.** `sentences.txt` has `SLV_IDLE0`–`10` and `SLV_ALERT0`–`5`, each a
string of `slv_word` samples. There are no English lines for a slave anywhere. "Talk" is gibberish unless a
translation is shown; see below.

**Scientists have no points of interest.** Checked 2026-09-13: their schedules are follow, heal, panic,
idle stand, cover, hide, startle and fear (`dlls/scientist.cpp:149-393`), and the sitting scientist only
turns its head and chats in place (`CSittingScientist::SittingThink`, `:1346`). Nothing in Half-Life gives an
idle monster places to visit; that is a Half-Life 2 idea.

**What does exist is the `path_corner` loop, and it does not stop.** A monster whose `target` names a
`path_corner` walks the chain on `SCHED_IDLE_WALK` (`dlls/monsters.cpp:2141-2176`), advancing to each
corner's own target as it passes it (`:1504-1505`). Point the last corner back at the first and it loops
forever. But **a monster ignores the corner's `wait`**: `CPathCorner::GetDelay` is read only by trains and
platforms (`dlls/plats.cpp:784`, `dlls/triggers.cpp:2311`), so a monster on a loop never pauses. Out of the
box that is a slave pacing a circuit, not one who drifts between places.

**So a point of interest is small new code on top of the loop.** At each corner: stop, honour its `wait`, and
optionally play a set piece named on the corner (`collar1`, `pushup`, `jibber`), then walk on. The loop, the
save/restore of the corner chain and the pathing are all already there. A random next corner instead of the
fixed order is a further small step, if the loop reads as mechanical.

`scripted_sequence` and `aiscripted_sequence` can also walk him to a spot and play a set piece there, driven
by the map rather than by him.

**The game over is map setup, as in vanilla.** Any monster takes a `TriggerTarget` and a `TriggerCondition`
(`dlls/monsters.cpp:3021-3026`), fired once when the condition is met (`:3130-3131`). Pointed at a
`player_loadsaved` (`CRevertSaved`, `dlls/player.cpp:5236`), that fades the screen, shows a message and
reloads the last save (`:5299-5327`). That is how Half-Life fails the player for a critical scientist, and it
needs no code. **The condition is `AITRIGGER_TAKEDAMAGE`** (`dlls/monsters.h:130`), not `AITRIGGER_DEATH`,
since the first hit ends the game.

Two things to know about that condition. It fires on **any** damage, not only the player's: a grenade the
player throws near him counts, which is right, but so would a stray explosion from anything else in the lab,
so the lab should hold nothing that can hurt him. And a monster holds only **one** trigger condition, so the
take-damage trigger is the only automatic one he has.

**The flinch is free** as long as the reload waits for it: `player_loadsaved`'s fade and message times are
keyvalues, and the stock slave has flinch animations (`flinch2`, and one per limb).

**The hand-over is map setup too.** A `scripted_sequence` walks him to the device and plays a set piece; when
it finishes it fires its own target (`CCineMonster::SequenceDone`, `dlls/scripted.cpp:550-568`), which opens
the device's door (`func_door`, or any brush entity that opens). The item waits inside from the start, so
nothing is spawned. The Pickup Prompt reaches it once the door is open. What *starts* the sequence, the player
reaching a point in the game or pressing something, is a map choice.

**The room conversation is `scripted_sentence`**: a sentence, the speaker, a listener, a radius, whether it
can refire, and it fires its own target when it plays (`dlls/scripted.cpp:968-1197`). Fired by a
`trigger_once` at the door.

**Progress has somewhere to live across levels.** `env_global` sets a named state (on, off or dead) in the
global state table (`CEnvGlobal`, `dlls/buttons.cpp:37-134`), which survives level transitions. One named
state per stage of his English is enough: a map turns on `slave_english_1` when the player reaches it, and
he reads the highest one that is on. A counter of his own is the alternative, but he lives in one map, and
state that has to change while the player is elsewhere belongs in the global table.

**The English lines are imports.** Each needs converting to a sound GoldSrc plays, a `sentences.txt` entry to
be mixed with his `slv_word` samples, and an [ART_DEBT.md](ART_DEBT.md) line naming where it came from, since
the voice lines come from other games (Half-Life 2, and Black Mesa, which is Crowbar Collective's work, not
Valve's). Whether mixing them into one line reads as learning a language or as a glitch is a play question.

**Nobody fights him, and he fights nobody**, with `Classify` returning `CLASS_NONE`: every entry in that row of
the relationship table is `R_NO` (`dlls/monsters.cpp:2239`), and so is that column in the rows checked. So
Suspicion ([ADR-0009](adr/0009-suspicion-gates-the-relationship-bits.md)) has nothing to gate either.

**+use needs saying explicitly.** A monster answers the use key only if its `ObjectCaps` includes
`FCAP_IMPULSE_USE`, as Barney's and the scientist's do (`dlls/barney.cpp:57`, `dlls/scientist.cpp:83`). The
Pickup Prompt already lets a usable entity win over a pickup behind it
(`FindLookedAtPickup`, `dlls/player_inventory.cpp:769-792`), so talking to him in a lab full of loot does not
pick anything up.

#### Traps

- **The talking animations already speak.** `jibber` and `jabber` fire their own word sounds. Playing a
  `SLV_` sentence over them doubles the voice. Either the animation talks and there is no sentence, or the
  sentence talks and the animation's sound events are skipped.
- **`CTalkMonster` turns on a player who keeps shooting it.** Barney warns, then fights back. A non-combatant
  needs a different answer.
- **Perception is not a concern**: at `CLASS_NONE` there is nothing to perceive for.

#### What is code and what is map

Almost everything about him is map setup on stock entities. The code is:

- **The monster itself**: a `CTalkMonster` on the slave's model, `CLASS_NONE`, `FCAP_IMPULSE_USE`.
- **+use plays one line**, picked from his current stage of English.
- **Stopping at points of interest** on the loop: honour the corner's `wait`, play the corner's set piece.
- **Reading the English stage** from the global states.
- **Never turning hostile.** The take-damage trigger ends the game first, but the class must not answer being
  shot with a fight in the frame before the fade.

#### Open

- **What does he hand over, and when?** Which unlockables, which information, and what starts each hand-over:
  reaching a point in the game, or the player asking. Information spoken in a mostly-alien voice argues for
  text on screen (`game_text`, `dlls/maprules.cpp:239`; the mod has no subtitle system).
- **A player walking in mid-hand-over.** A `scripted_sequence` holds a monster until it ends. A +use press or a
  room conversation arriving while he is at the machine needs an answer: wait, or refuse.

### The alien slave boss

**Shape: Idea, with its purpose settled 2026-09-13.** A special alien slave under the Nihilanth's control,
with custom attacks and AI. Defeating it frees it from that control, and from then on it is the
[friendly alien slave](#friendly-alien-slave): one character, in his lab, who helps the player progress
through unlockables and information, never by fighting. Its reward was first written as "a Module and some
items"; that now falls under how the lab slave hands things over.

- **Defeated is not killed.** It needs a health floor where the fight ends: the boss stops and is spared.
  Shooting the freed slave later ends the game, so the fight has to make "spared" unmistakable.
- **Two entities. Settled 2026-09-13.** The boss and the lab slave are separate. The boss leaves when defeated
  and sets a global state (`env_global`), and the lab slave is present only once that state is on.
- **The collar and the bracelets are the Nihilanth's control. Settled.** They already show it on the stock
  model, and `collar1` and `collar2` tug at the collar.
- **The end of the fight is scripted**, so that the slave being freed is noticeable. Settled. It is a
  sequence the player watches rather than a monster that simply stops. What it shows is open.

**What that asks of the art.** In the stock model the collar and bracelets are part of the one body mesh: the
model has a single body submodel, and the metal is most likely its chrome texture (`Chrome_1.bmp`, the one
chrome-flagged texture in `islaveT.mdl`). So a freed slave without them is a mesh edit and a recompile,
starting with a decompile. The cheap alternatives, if that waits: a second skin where the metal is dark or
broken, which is a texture edit and still a recompile, or a freed slave who keeps the hardware and shows his
freedom only in how he behaves. The boss and the lab slave being two entities makes this easy: they can be two
models.

**What the scripted ending can use.** `scripted_sequence` for the set piece (the stock `collar1` and
`collar2` are already a slave fighting his collar), `env_beam` or sprite effects for the control breaking,
`env_shake` and `env_fade`, and a `scripted_sentence` for a first free word. When it finishes, the boss
removes itself and the global state turns on. All of it is map entities; the only code in the ending is the
boss knowing its health floor has been reached and firing a target instead of dying.
- **Which Module?** The set is Pulse, Dash and Hook ([Modules](#pillar-3-modules)). The Pulse is meant to be
  found early, which suggests Dash or Hook. The fiction has the slave teaching an alien ability, which fits
  neither obviously.
- **How do the items arrive?** Handed straight into the Inventory, where a full Grid refuses them, or left in
  a Box, which is not built yet.
- Custom attacks: none written down yet.

### The assassin boss

**Shape: Idea.** A human assassin with a dash and custom attacks.

`CHAssassin` already jumps (`ASSASSIN_AE_JUMP` sets a velocity, `dlls/hassassin.cpp:260`, with a 3-second
cooldown), cloaks by fading `renderamt` down to 20 when not attacking (`RunAI`, `:693`), and throws
grenades. A dash is a horizontal jump on the same event pattern.

**"Dash" is already a Module's name.** Either the assassin's move gets a different word, or the fight is
where the player first sees the Dash, and possibly where they win it.

### The alien grunt boss

**Shape: Idea.** An alien grunt with custom attacks. Nothing beyond that is recorded. It shares a base with
the [melee alien grunt](#melee-alien-grunt), so whichever is built second gets the other's groundwork.

### The Nihilanth

**Shape: Idea.** Custom attacks and a revamped model.

**`CNihilanth` is not schedule AI.** It runs on think functions (`HuntThink`, `Flight`, `NextActivity`),
with energy spheres it absorbs and throws (`CNihilanthHVR`), and it is a puzzle boss: the spheres protect
it (`AbsorbSphere`, `dlls/nihilanth.cpp:984`). New attacks go into `NextActivity`, not into schedules.

**A new model must keep the old sequence names**, or the code changes with it. It picks animations by name
(`float`, `walk_r/l/u/d`, `recharge`, `attack1_open`, `attack1`, `attack2`, `die1`,
`dlls/nihilanth.cpp:469-839`).

Open: does "custom attacks" keep the sphere puzzle, or turn it into a straight fight like the other three?

### Shared by all of them

What exists to build on: Half-Life's schedule/task AI (`dlls/schedule.cpp`, `dlls/defaultai.cpp`) is a real
state machine and adding a monster with a custom moveset means new schedules, not new engine work. The
Gargantua (`dlls/gargantua.cpp`) is the closest thing to a boss the base game has and is the reference for
a large monster with multiple distinct attacks.

Two things this pillar should not rediscover:

- Half-Life **does not knock monsters back from damage** — the one place it happens
  (`dlls/combat.cpp:891`) is gated on `MOVETYPE_WALK`, which is the player. PILLARS records this under the
  Follow-Up, where knockback had to be restricted to headcrabs to look right. Any boss moveset that assumes
  stagger has to build it.
- The Pulse's damage list ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)) decides which
  boss attacks are counterable. A new boss whose signature attack is not on that list is a boss the Pulse
  is useless against — which may be the point, but should be a choice.

~~Open: is a boss a **combat** encounter or a **puzzle**?~~ The roster answers it for three of the four:
the slave, assassin and alien grunt bosses are fights. The Nihilanth is the one still open, above.

---

## Pillar 2: Decapitation

**Shape: Shaped. The only entry here with a reference implementation that has been read end to end.**

A killing head hit takes the head off: the model switches to a headless submodel, a skull gib is thrown
from the neck, blood jets from the stump, and the body drops. It is the clearest possible answer to "did
that shot land where I aimed it", and it is the piece of feedback pillar 2 most obviously lacks — the
Follow-Up and Weapon Mastery both make hits *stronger* without making them *legible*.

This is not a new gib system. `CGib` already throws heads (`CGib::SpawnHeadGib`, `dlls/combat.cpp:122`),
already picks the skull submodel, already has the 5%-chance-it-flies-at-your-face joke, and hitgroup 1
already doubles damage (`dlls/combat.cpp:1337`, `sk_monster_head` = 2 at `dlls/game.cpp:416`). What is
missing is a **partial** destruction path: today a monster is either intact or entirely gibbed
(`CBaseMonster::GibMonster`, `dlls/combat.cpp:300`), with nothing in between.

### Where the design comes from

**Delta Particles** — sources at `E:\Projects\dp_allSources(maps+code)_v13\_codeSRC`, which ships this
across twelve monsters. References below are marked **`dp:`** to keep them apart from ours.

Port the *design*, not the code. The design is sound; the implementation is the same forty-line block
copy-pasted into `zombie.cpp`, `hgrunt.cpp`, `barney.cpp`, `otis.cpp`, `scientist.cpp`, `technician.cpp`,
`agrunt.cpp`, `bullsquid.cpp`, `hassassin.cpp`, `houndeye.cpp`, `islave.cpp` and `controller.cpp`, with the
constants drifting between copies. Ours should be one thing on `CBaseMonster` that a monster opts into.

**One shared FX function** — `CBaseMonster::GibHeadMonster(Vector headPosition, BOOL Head)`
(`dp: dlls/combat.cpp:429`). Plays `common/bodysplat.wav`, throws either the skull (`Head == TRUE`) or two
random gibs for aliens with no separable head, then emits one `TE_BLOODSPRITE` and three `TE_BLOODSTREAM`
jets at the neck. It briefly swaps `m_bloodColor` red → 71 around the temp entities so the spray reads as
gib-blood rather than surface blood, and swaps it back. That palette trick is their own addition, applied
in their `TraceAttack` too (`dp: dlls/combat.cpp:1565-1568`); ours does none of it and just calls
`SpawnBlood` (`dlls/combat.cpp:1357`), so it is a separate small decision to copy or not.

**A trigger in `TraceAttack`, on hitgroup 1** (`dp: dlls/zombie.cpp:399`):

```cpp
if ( !HeadGibbed && pev->health <= flDamage * gSkillData.monHead && flDamage >= 10 )
{
    pev->body = 2;                          // headless submodel
    GibHeadMonster( ptr->vecEndPos, TRUE );
    HeadGibbed = TRUE;
}
```

The test is *"this head hit is lethal"*, not *"the head has taken enough damage"* — decapitation is a
killing-blow flourish, not independent limb damage. The damage floor is per-monster and hand-tuned: zombie
10, alien grunt 8, Barney and Otis 20, assassin 30, bullsquid 75.

**A second trigger in `TakeDamage`, for the shotgun** (`dp: dlls/zombie.cpp:432`). Buckshot arrives as N
separate `TraceAttack` calls, each individually too weak to clear the floor, so `TraceAttack` only counts
head pellets and stashes the impact point; `TakeDamage` then fires the same block once the accumulated hit
is lethal. Delta Particles' own comments call this a hack twice (`dp: dlls/barney.cpp:677`,
`dp: dlls/agrunt.cpp:293`). It is, but it is the *right* hack — Half-Life's multi-damage accumulation gives
no other hook, and any port needs the equivalent or the shotgun is the one weapon that can never behead
anything.

### What it costs, honestly

**Every monster needs a model edit, and that is the whole cost.** No vanilla model has a headless
submodel. `hgrunt.mdl`'s head bodygroup is four entries — grunt, commander, shotgun, M203
(`dlls/hgrunt.cpp:64-68`) — and Delta Particles added a fifth (`dp: dlls/hgrunt.cpp:74`,
`HEAD_HEADLESS 4`). `zombie.mdl` has no head bodygroup at all; theirs uses raw `pev->body` 2 and 3
(`dp: dlls/zombie.cpp:401-404`), which means custom `zombie.mdl`, `zombie_barney.mdl` and
`zombie_soldier.mdl`. So this is **per-monster opt-in gated on art**, and the code is cheap enough that the
art is the schedule. An [ART_DEBT.md](ART_DEBT.md) entry per monster, not one for the feature.

The upside of that shape: it ships incrementally. One monster with a headless submodel is a complete,
judgeable version of this feature.

### What to fix while porting

Four defects in the reference implementation, all worth not inheriting:

- **No save/restore.** `HeadGibbed`, `BuckshotCount` and the beheader pointer have no `DEFINE_FIELD`
  entries anywhere in Delta Particles. `pev->body` survives a save/load but `HeadGibbed` resets to false, so
  a beheaded monster can be beheaded a second time after a reload — second skull, second achievement tick.
  Ours saves `m_LastHitGroup` already (`dlls/monsters.cpp:59`); these belong in the same table.
- **Two skulls.** Our `GibMonster` calls `SpawnHeadGib` unconditionally (`dlls/combat.cpp:312`), so a
  monster beheaded and *then* gibbed throws two heads. Delta Particles solved this by commenting the call
  out entirely and making every NPC responsible for its own head (`dp: dlls/combat.cpp:400`), which is why
  each of their twelve `GibMonster` overrides carries an `if (!HeadGibbed)` guard. A single guard inside
  `GibMonster` is strictly better and keeps the vanilla path intact for monsters that never opt in.
- **The gore cvars are bypassed.** `GibHeadMonster` checks neither `violence_hgibs` nor `g_Language`, and
  calls the two-argument `SpawnHeadGib` overload with `models/hgibs.mdl` hardcoded — so a low-violence or
  German install still gets a flying skull, and the wrong gib set. Route through the language check.
- **The duplication itself.** Twelve copies of three members and two blocks. `CBaseMonster` wants the
  state plus a small descriptor — can this monster be beheaded, which bodygroup and submodel, what the
  damage floor is — and the per-monster code becomes one line. The interface wants to be *"a monster can
  lose its head"*, with hitgroups, buckshot counting and temp entities all behind it.

### What it would touch here

- **The Follow-Up.** A primed Follow-Up swing to the head is exactly the moment this should pay off, and
  PILLARS already records that the Follow-Up's knockback applies *after* damage so a killed headcrab is
  still thrown. Decapitation slots into the same ordering.
- **Weapon Mastery**, which raises damage, therefore raises how often the lethality test passes — for free,
  with no new hook. Same for a future [Backstab](#pillar-6-stealth).
- **No prediction risk.** All of it is server-side damage response, so nothing here is blocked on
  [the prediction problem](#the-prediction-problem). Rare, in this document, and it is the main argument for
  doing it sooner than its size suggests.

### Headshots, and how they reconcile with this

**Shape: Shaped. Documented 2026-08-31 at the user's request; deliberately not built.**

A head hit that does *not* take the head off should still be legible: a distinct sound, and **more blood
spurting out of the wound**. Today a headshot is indistinguishable from a body shot except that the monster
dies sooner — `TraceAttack` multiplies by `gSkillData.monHead` (`dlls/combat.cpp:1337`) and then produces
exactly the same `SpawnBlood` and `TraceBleed` as any other hit (`:1357-1358`).

**The reconciliation rule, which is the reason this is filed here:**

| Head hit | Response |
| --- | --- |
| Lethal, and the monster has a headless submodel | **Decapitation** — `common/bodysplat.wav`, skull gib, neck jets |
| Everything else | **The headshot cue** — an understated sound plus a heavier blood spray |

The two can never both fire. Decapitation is the loud, gory, once-per-monster event; the headshot cue is
the quiet constant one. Ordering them this way also means a monster that has *not* been given a headless
submodel still gets feedback for a head hit, so the cue is useful long before the art exists — which is
the opposite of Decapitation's problem, where the art is the schedule.

**"Discreet" means understated feedback to the attacker, not quieter in the world.** The cue is played to
the player and never enters `CSoundEnt`. Damage never does — what monsters hear is the *gun*, via
`UpdatePlayerSound`'s weapon volume — so a quieter head hit would do nothing while the weapon is loud, and
would put a second system in charge of how loud a kill is. Weapon noise stays entirely owned by
`m_iWeaponVolume`. See [PERCEPTION.md](PERCEPTION.md).

**Both halves land in `TraceAttack`**, which already has `ptr->iHitgroup`, already records `m_LastHitGroup`,
and already calls `SpawnBlood` and `TraceBleed` (`dlls/combat.cpp:1326-1360`). It is the one place that
knows the hitgroup, the damage and the impact point at once.

#### What the extra blood actually costs

Not nothing, and an earlier draft of this entry implied otherwise. Checked 2026-09-01:

**Damage does not visibly scale the existing spray.** `SpawnBlood` (`dlls/weapons.cpp:132`) emits exactly
one `TE_BLOODSPRITE` through `UTIL_BloodDrips` (`dlls/util.cpp:1185`), and the only thing damage controls
is the sprite's *size*: `V_min(V_max(3, amount / 10), 16)`. At melee and pistol damage that expression sits
**on its floor of 3** — a 10-damage crowbar hit and a 30-damage Backstab produce an identical puff, and
nothing changes until 40 damage. So a head hit doubling damage buys no visible blood at all.

**And that sprite cannot spurt.** `UTIL_BloodDrips` takes a `direction` argument and **never writes it to
the message** — `TE_BLOODSPRITE` carries position, two sprite models, colour and size, and nothing else.
It is a non-directional puff by construction. A jet is `UTIL_BloodStream` (`dlls/util.cpp:1163`), which
does write a direction and produces `TE_BLOODSTREAM`.

So "more blood spurting" means **additional `UTIL_BloodStream` calls at the wound**, thrown along the shot
direction with some spread — new code, not a tuned constant.

Which is the strongest argument yet for building this with Decapitation rather than after it: the reference
implementation's `GibHeadMonster` is *already* one `TE_BLOODSPRITE` plus three `TE_BLOODSTREAM` jets at the
neck. **The two features want one shared helper at two intensities** — a few jets at the head for a
survivable hit, more at the stump for a decapitation — rather than two separate blood routines that drift
apart the way the reference's twelve copies did.

Whatever that helper is, it must route through `UTIL_ShouldShowBlood` and the `g_Language` check.
Bypassing the gore cvars is one of the four defects listed above as not worth inheriting, and a new blood
effect is exactly where it would be reintroduced.

Two things to settle before writing it:

- **The shotgun fires the cue N times.** Buckshot arrives as separate `TraceAttack` calls, one per pellet,
  so a face full of shot would stack a dozen overlapping cues. This is the same problem Decapitation has
  and needs the same answer — count head pellets in `TraceAttack`, fire once from `TakeDamage` — which is
  an argument for building the two together rather than in sequence.
- **Player hits only, or monster-on-monster too?** It is player feedback, so gating on the attacker being
  the player is almost certainly right, and it avoids a firefight between grunts and aliens turning into a
  percussion section.

It will need an [ART_DEBT.md](ART_DEBT.md) entry when built, under the same constraint the Backstab cue
already carries: it must not share a timbre with the sound landing in the same instant.

### Open questions

- **Killing blow only, or can something survive it?** Delta Particles' zombie survives decapitation *by
  accident* — `CZombie::TakeDamage` cuts bullet damage to 30% (`dp: dlls/zombie.cpp:419-426`) but the
  lethality test in `TraceAttack` compares against the full pre-reduction figure, so the test passes while
  the damage does not, and you get a headless zombie still walking. They then built on the accident: the
  zombie tracks who took its head and awards an achievement if it dies headless (`dp: dlls/zombie.cpp:449-464`,
  via an `OnDying` virtual fired from the state machine at `dp: dlls/monsterstate.cpp:53-56`). Deliberate
  headless-but-alive is a much larger feature — a blinded monster needs different AI, not just a different
  model — and it is by far the more memorable one. Decide before writing the test, because the answer
  changes what the test compares.
- **Which monsters, and in what order?** Zombies first is the obvious call: they are the mod's most-fought
  melee enemy, the Follow-Up is already tuned against them, and a shambling headless zombie is the image
  that sells the feature.
- **Does the tree touch it?** A Skill that lowers the damage floor or raises the head multiplier is a
  server-side damage modifier read where the effect is computed — the exact pattern every existing Skill
  uses. Tempting, and it would give the Armaments column something that is not a flat number. Against it:
  the tree is at 15 Skills across seven columns and was deliberately curated down to that.
- **Does a decapitated monster still `DeathSound`?** Half-Life's death sounds are voiced. A headless
  scientist screaming is a bug the player will find in the first ten minutes.

### Done when

A player who aims for the head can see that they did, on at least one monster, and nothing about the
result is repeated in twelve files.

---

## Pillar 3: Modules

**Shape: Shaped in intent, 2026-09-12. What a Module is, and the set, are decided; the Pulse's two
branches and each Module's details are not.**

### What a Module is — decided

**A Module is a mechanic the player does not have at the start, found partway through the game and kept
from then on.** Half-Life's long jump module is the model, exactly. Modules are **not swappable** and
there are no slots: finding one is permanent, and level design may assume any Module the player has
already passed.

That makes Modules the one kind of reward that changes **what the player can do**, where Skill Points,
Row Grants and items change how well they do it. It is what gives exploration an access reward — a ledge
passed early becomes a reason to come back — and it is the missing reason for the backtracking the
Inventory design already commits maps to.

Rejected on the way: *limited, swappable Modules*. The swap was proposed so a player uncomfortable with
parrying could leave the Pulse out entirely. It was dropped because a swappable Module is one level design
can never assume, and because the same comfort goal is met better inside the Pulse's own Skills — below.

### The set

| Module | What it does | Replaces |
| --- | --- | --- |
| **Pulse** | The existing Pulse, no longer available from the start. | Suit hardware — the Pulse today comes with the suit at Anomalous Materials |
| **Dash** | A short, fast movement burst. | **The long jump module**, which serves the same purpose |
| **Hook** | A grappling hook, in the manner of Opposing Force's barnacle grapple. | — |

The Opposing Force grapple code will be added to the project for reference; nothing about the Hook should
be designed against guesses until it is.

### The precedent is already in the game — including the prediction route

`m_fLongJump` (`dlls/player.h:176`) is exactly this shape: a bool on the player, saved
(`dlls/player.cpp:107`), set by walking over `item_longjump` (`dlls/items.cpp:582`), and gating a movement
verb thereafter. Modules are that, generalised, with a UI.

**And it already solves the hard part.** The long jump is a movement verb, so it runs in `pm_shared/` and
is predicted — and the flag reaches it without `playermove_t` ever seeing `CBasePlayer`. The server writes
a **physics key** (`pfnSetPhysicsKeyValue(edict(), "slj", "1")`, `dlls/items.cpp:584`, re-sent on restore
at `dlls/player.cpp:3265-3271`) and `PM_Jump` reads it back from `pmove->physinfo`
(`pm_shared/pm_shared.cpp:2662`). Physinfo is networked to the client for prediction by the engine.
That is the route for Dash and Hook, and quite possibly for `SprintSpeed` and `HighJump` too — see
[the prediction problem](#the-prediction-problem), which predates this being noticed.

Dash replacing the long jump means the thing to extend is already the right shape: `CItemLongJump`, the
`slj` key and the `PM_Jump` branch, rather than a new entity and a new route beside them.

### The Pulse as a Module — two branches instead of a swap

The Pulse's Skills split into **two branches** that answer the parry-comfort problem without removing the
Pulse:

- **Timing** — the Pulse as it is now: press to raise a Shield, reward for reading an attack. The four
  existing Skills (`PulseWindow` 12, `PulseRecharge` 15, `PulseDischarge` 16, `PulseRebound` 17) and the
  `CrowbarFollowUp` (18) that hangs off a deflect all belong here.
- **Passive** — a **separate, rechargeable health pool** that protects without being timed, for a player
  who does not want to parry. New Skills, new ids.

**Settled 2026-09-12:**

- **The unskilled Pulse is the Pulse as it is today** — the timed press. Neither branch changes what the
  Module does before a Skill is spent.
- **The branches are not exclusive, but investing in both is meant to be inefficient.** No lockout rule,
  no new line style in the tree: the tree stays AND-only, and the cost of spreading points across both is
  carried by pricing. A player who goes deep in one branch should be clearly better served than one who
  splits.
- **The passive branch is a separate rechargeable health pool.** Damage lands on the pool before armour
  and health, and the pool refills on its own.
- **The Pulse is found in the world, early.** It is the player's first Module, and the stretch of game
  that cannot assume it is short.

**The pool sits next to two things that already absorb damage**, and the design has to say how it differs
from each:

- **Armour.** The HEV battery is already a second pool in front of health, and `BatteryRegen` (id 14)
  already makes it refill passively. What separates the Pulse pool from armour today is only the curated
  damage list and the recharge rule — worth making sharper than that, or the passive branch reads as
  "more armour".
- **The timed Shield.** Order in `CBasePlayer::TakeDamage` becomes: a standing Shield refuses the hit
  outright, then the pool, then armour, then health.

This **reverses a recorded decision**. PILLARS pillar 2 says the Pulse is suit hardware because *"Skills
evolve a verb the player already has rather than granting it, which lets level design assume it."* As a
Module, level design may assume it only after its acquisition point. That wants an ADR when it is built,
since the reasoning it replaces is written down and the new reasoning should be too.

### Open questions

- **What the pool absorbs.** Only the Shield's curated list
  ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)), so both branches protect against the
  same things and differ in *how* — or everything, which makes it the clearest difference from armour?
- **How the pool recharges.** After a delay since the last hit, at a rate — or does it share the Pulse's
  Recharge clock? And does pressing the Pulse still do anything useful for a passive-branch player?
- **How the pool is shown.** `CHudPulse` already has a charge bar beside the armour readout; a pool is a
  second quantity for it to carry, or a readout of its own.
- **How "inefficient to split" is priced.** The 35-point tree and a 60–70% critical-path budget are the
  lever; a proving map is what tells whether the pricing does it.
- **Pulse Skills before the Module is found.** A player could spend points on a branch for a verb they do
  not have. The [alien column](PILLARS.md#wanted-the-alien-column) already wants "hidden until the player
  carries it" — the Pulse branch is a second customer for the same rendering machinery, and a reason to
  build it once.
- **Dash's input.** The long jump is duck + jump while moving. Does Dash keep that, take its own key, or
  work in the air?
- **Is the Hook a weapon or a verb?** Opposing Force's grapple occupies a weapon slot. A Module as defined
  above is a verb, which argues for its own key — to be read against the reference code.
- **Vanilla maps place `item_longjump`.** Keeping the classname means stock maps hand out Dash with no
  edits; renaming it means they hand out nothing.
- **Acquisition** — walk-over like the long jump, or the Pickup Prompt the suit moved to?
- **"Dash: upgradeable"** — by Skill Points, which reuses the whole tree and is the pattern the Pulse
  already follows, or by finding a better Dash?
- **Where are they shown?** Not a loadout, so the tab is a record of what has been found. The Inventory
  Panel's second tab is already the Skill Tree ("Upgrades"), so this is a third tab or a section of one.
  A Module occupies no Cells, like a Reset Token.

---

## Pillar 1: Transmissions

**Shape: Idea.** Logs found in the level, played back by the player.

The closest existing machinery: Half-Life's sentence system (`sentences.txt` and the `!SENTENCE` form used
throughout, e.g. the Syringe's `!HEV_HEAL7`), `ambient_generic` for placed sound sources, and
`CHudTextMessage` / `gmsgTextMsg` for on-screen text. A "currently playing" indicator is nearly free —
`CHudStatusIcons` is wired up as of pillar 3, so a durable status icon is one `MESSAGE_BEGIN`.

The interesting question is not playback, it is **where a found Transmission goes.** Two shapes, and the
mod already has one of each:

- *Like an Item Type* — it occupies a Cell, can be dropped, can be lost. Wrong, almost certainly: a lore
  pickup that competes with a medkit for space will simply never be picked up.
- *Like a Reset Token* — banked as a count, occupying no Cells, spent or replayed from a panel. This is
  the right precedent, and the token already proves the pattern works end to end.

Which means a Transmissions list is a new tab or a new panel, not an Inventory change.

### Open questions

- Are Transmissions **replayable** after the first listen, or heard once? Replayable implies a list UI;
  once implies they are pure flavour and a much smaller feature.
- Do they play **in the world** (the player triggers a terminal and it plays aloud, interruptible by
  combat) or **from the panel** (a menu the player reads at leisure)? The first is far better for pacing
  and far worse for anyone who walks away mid-log.
- **Subtitles?** Half-Life has no subtitle system to speak of. Without one, a Transmission playing during
  a firefight is lost, and audio the player cannot re-hear is content that was never delivered.
- Do they carry **information** — a door code, a Station recipe, the location of a cache — or only fiction?
  Information makes them worth finding and makes missing one punishing.

---

## Pillar 1: The world

**Shape: Idea.** Everything here is blocked on [Maps](#maps), and most of it *is* maps.

### The facility

Secret areas, vents, shortcuts, elevators and trains. Structurally this is level design rather than code —
Half-Life already ships `func_train`, `func_tracktrain`, `func_door`, `func_plat`, `func_button` and
`func_breakable`, all working, all used by the base campaign.

The reason it belongs in this document rather than a level editor is that pillar 1's reward loop is
designed and unbuilt: PILLARS states the target as a tree completable only by near-exhaustive exploration,
with roughly one Skill Point per optional space and 5–10 Reset Tokens across the campaign, the first around
20% in. Those numbers are a level-design brief. Nobody has written a map against them.

### Interactable props

Crates and panels as **custom models rather than brushwork**. Worth flagging the cost: Half-Life's
`func_breakable` (`dlls/func_break.cpp:134`) is a brush entity. A model-based interactable prop is not a
`func_breakable` with a model set on it; it is new entity work — bounds, damage response, gibs, and a use
interaction. `CBreakable`'s existing material and gib handling is the thing to reuse, not the entity class.

This is also where the Pickup Prompt earns its keep. `FindLookedAtPickup` already answers "what would a use
press take?" and deliberately weighs ordinary usable entities so that pressing use at a button never grabs
a medkit behind it — so props and pickups can share a space without fighting, which is the hard part and
it is done.

### Xen and underground Xen

No detail recorded beyond the intent to build them as areas. Underground Xen is not a place Half-Life has,
so it is original level design rather than a revisit.

### Stations

**Shape: Idea.** Crafting and recycling stations placed in levels, with item inputs and item outputs.

The precedent is exact: `func_recharge` (`dlls/h_battery.cpp:64`) and `func_healthcharger`
(`dlls/healthkit.cpp:157`) are wall-mounted entities with a `+use` interaction, a finite budget, a sound
response and a HUD effect. A Station is that, plus a transaction against the player's Inventory.

The good news is that the transaction is safe by construction:
[ADR-0004](adr/0004-the-server-owns-the-inventory.md) makes the server own Inventory contents *and*
placement, so a Station is server-side code calling into `CPlayerInventory` directly — no new authority
question, no new sync path. `TryAdd` already handles first-fit placement and Stack top-up, and already
refuses when there is no room.

Which surfaces the one genuinely new rule: **what happens when the output does not fit?** The Inventory
refuses politely everywhere else. A Station that consumes inputs and then cannot deliver the output has
destroyed the player's belongings, which is the same class of bug as the three `CWeaponBox` hazards PILLARS
records under "Deliberately deferred". The transaction has to check for room *before* consuming, or spawn
the output on the floor.

### Open questions

- Is recycling **item → materials** (a new resource type, and therefore a new identity space) or
  **item → item**? The first is a much larger feature than it looks; the second needs no new nouns.
- Are recipes **known** from the start, found as [Transmissions](#pillar-1-transmissions), or discovered by
  experiment?
- Are Stations **fixed in the world** (a reason to backtrack, which the persistence design already
  anticipates — "maps are designed with backtracking in mind") or **carried**? Fixed is much stronger for
  pillar 1 and costs nothing extra.

---

## Art and audio

**Not roadmap entries.** Sounds and icons are already tracked, per-asset with acceptance criteria, in
[ART_DEBT.md](ART_DEBT.md) — the Pulse's Shield sprite and sound set, the Health Syringe's icon/model/
sounds, the Skill Point and Reset Token models, and the Skill Tree's node icons. That register is the right
place for them and duplicating it here would let the two drift.

Three things this roadmap adds to it:

**Everything here creates more art debt.** The Carbon Pickaxe, the Gauss Katana, Panthereye, Kingpin, every
Evolution, every Station and every Module needs a model, an icon and a sound set. The register will grow
faster than it is paid down for the foreseeable future, and that is fine as long as each entry says what is
wrong with its stand-in.

**Generated icons must clear a bar the register already specifies.** The Skill Tree entry in ART_DEBT.md is
marked *blocking rather than cosmetic*, and it sets hard constraints that any generation approach has to
respect: **one fixed size, not a resolution-bucketed set**; small enough that seven columns fit
`panelW - 264` (roughly ≤64px); greyscale for anything drawn additively and tinted. The genuinely hard
requirement is that each icon be **distinguishable from every other at 20×20**, which is where generated
icon sets usually fail — they come back stylistically consistent and mutually indistinct, which is exactly
today's problem with `suit_full` on five Skills. The size constraints hold only while the tree draws its
icons at native size; the Inventory Grid no longer does (PILLARS pillar 5, where the Inventory's Icons —
decided and built 2026-09-12 as full-colour world-model renders — are recorded), and the tree could follow.

**On sounds.** The user's note reads: *"sounds: here I have little experience and I can help"* — which is
ambiguous between offering help and asking for it. See [Open questions](#open-questions).

What the register already establishes about this mod's audio, and what any new sound work should inherit:
the Pulse entry records that the first attempt failed not because the samples were bad but because the
Pulse and the deflect **shared a timbre** and landed 0.1s apart, so the deflect was inaudible. That is the
transferable lesson — sounds in this mod are judged in the sequence they actually occur, not in isolation.

---

## Proposed vocabulary

Names used in this document that are **not yet in [CONTEXT.md](../CONTEXT.md)** and are deliberately
provisional. CONTEXT.md is the glossary of settled terms; these graduate into it when the feature they name
is designed, and may well change name first.

| Provisional term | Proposed meaning | Notes |
| --- | --- | --- |
| **Module** | A mechanic the player does not start with, found partway through the game and kept for good — never swapped. | Deliberately echoes "the longjump module", which is the model. Graduates to CONTEXT.md when the first one is built. |
| **Dash**, **Hook** | Two of the three Modules; the Pulse is the third. Dash replaces the long jump. | Plain, and hard to improve on. |
| **Evolution** | A durable alteration to a weapon that keeps the weapon's identity — silencer, second barrel, extended magazine. | Avoid *attachment* and *mod*; the first implies removable hardware, the second collides with "the mod". |
| **Transmission** | A recorded log found in a level and played back. | Avoid *log*, *tape*, *audio diary*, *datapad*. |
| **Station** | A world entity that takes items in and gives items out. | Avoid *bench*, *workbench*, *terminal*, *fabricator*. *Terminal* especially — it will be wanted for Transmissions. |
| **Decapitation** | A lethal head hit that removes the head: headless submodel, thrown skull, blood from the stump. | Distinct from *gibbing*, which is the whole body and already means something in this codebase. **Headless** names the resulting state. |
| **Carbon Pickaxe** | The other custom weapon. | Named already; recorded here so it is used consistently. **Gauss Katana** graduated to CONTEXT.md on 2026-09-12 when the weapon was built. |

**Graduated 2026-08-31**, when the stealth design was settled: **Concealment** and **Backstab** are now in
[CONTEXT.md](../CONTEXT.md), joined there by **Suspicion**, **Search**, **Post**, **Perception Profile**,
**Disturbance**, and the **Unseen / Noticed / Spotted** readout states. Backstab's meaning changed on the
way across — the proposal here required the victim to be unaware, and the settled term does not; it is
purely a matter of where the attacker stands.

Note what is deliberately *absent*: there is no proposed term for whatever a recycling Station consumes or
produces. That is the [open question](#open-questions) about materials, and inventing a noun before
answering it would settle the design by accident.

---

## Open questions

Ranked by how much else is waiting on the answer.

1. ~~**Are Modules limited or unlimited?**~~ **Answered 2026-09-12: neither swappable nor available from
   the start** — found through the game and kept, like the long jump. Level design may assume a Module
   past the point it is found. See [Modules](#pillar-3-modules).
2. ~~**Does the Pulse become a Module?**~~ **Answered 2026-09-12: yes**, with its Skills split into a timing
   branch and a passive health-pool branch so a player need not parry. Found early; branches not exclusive
   but priced so splitting is inefficient. The pool's details are open under [Modules](#pillar-3-modules).
3. **Are weapon handling upgrades Skills or Evolutions?** Two systems currently want to make the same gun
   faster. Building both is the failure mode.
4. **Does the Carbon Pickaxe replace the crowbar entirely?** Decides whether five Skill enumerators, three
   icons and a CONTEXT.md entry get renamed once or never.
5. **Is stealth optional everywhere?** Level design consequence, not a code one, and the answer shapes
   every encounter.
6. **Recycling: item → materials, or item → item?** The first introduces a whole new identity space
   alongside `WeaponId` and `EItemTypeId` ([ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md)).
7. ~~**What was the boss moveset example?**~~ **Answered 2026-09-12 by the roster**: four bosses, each an
   existing monster with custom attacks. See [Monsters and bosses](#pillar-2-monsters-and-bosses).
8. **Sounds — offering help or asking for it?** *"here I have little experience and I can help"* reads
   both ways, and the two readings imply very different plans.
