# Roadmap — intended work

Everything this mod means to build and has not built. [PILLARS.md](PILLARS.md) is the record of what exists
today; this is the record of what is wanted, why, and what it will cost.

Nothing here is a commitment to a design. Entries capture the intent, name the code that already exists to
build on, and list the questions that have to be answered before the first line is written. When an entry
is built, its content moves into PILLARS.md and the entry here is deleted — this file only ever shrinks
from the top.

**Last updated:** 2026-09-20, curated. Every statement that a later addition had amended, reversed or
struck through now says the settled thing once; a rejected alternative stays only where it is a reason
not to propose it again. The day-by-day changelog that used to sit here is `git log -- docs/ROADMAP.md`.

**How to add to this file.** Change the sentence, do not append a correction to it. A decision that
reverses an earlier one replaces it, with the earlier one kept in a clause only if someone might propose
it again. Strike-through is for nothing.

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

**Stealth is pillar 6 for filing, and inside combat for ranking.** It moves enemy perception, player
movement, weapon choice and level layout at the same time, so its *work* files under a section of its own
here and in PILLARS.md rather than under pillar 2, which would otherwise mean "everything you do to
things that are alive". But it is not ranked beside the other pillars (settled 2026-09-17): the mod is
exploration first, by far, then RPG, then combat, and stealth is one of the three ways to solve an
encounter — fight it, sneak it, go around it. See [The shape of the game](#the-shape-of-the-game).

**Modules do not become a pillar.** A Module is a thing you find that changes what you can do, which is
what pillar 3 already is; the Pulse is the shape it takes, and the Modules tab is a pillar 5 concern. Making
it a pillar would split "things you find" across two of them. It is the largest single entry under Custom
items instead.

Everything else falls out: weapons, monsters and bosses are pillar 2; Transmissions, level design and
Stations are pillar 1; sounds and icons are not roadmap items at all — see [Art and audio](#art-and-audio).

## Contents

- [The shape of the game](#the-shape-of-the-game) — settled 2026-09-17; the rules every entry below answers to
- [The prediction problem](#the-prediction-problem) — solved for weapons; movement has its route, the physics key the Dash proved
- [Maps](#maps) — no longer a blocker
- [Pillar 6: Stealth](#pillar-6-stealth)
- [Pillar 2: Weapons](#pillar-2-weapons)
- [Pillar 2: Monsters and bosses](#pillar-2-monsters-and-bosses) — the cult, the maddened, Xen hell and where the bosses sit, 2026-09-17; the alien flyer, 2026-09-20
- [Pillar 2: Decapitation](#pillar-2-decapitation) — a design to port, already read
- [Pillar 3: Modules](#pillar-3-modules)
- [Pillar 4: Routes](#pillar-4-routes) — builds; all seven Routes shaped and built on the board, the Alien's ultimate and every number still open
- [Pillar 1: Records](#pillar-1-records) — built 2026-09-18: the Prompt on everything usable, Records read with +use and kept in a fourth tab; audio later
- [Pillar 1: The world](#pillar-1-the-world) — the facility, **the cold open (`shaft1`, grilled 2026-09-19)**, Xen, mining and Shards, Stations
- [Art and audio](#art-and-audio)
- [Proposed vocabulary](#proposed-vocabulary)
- [Open questions](#open-questions)

---

## The shape of the game

**Shape: Shaped 2026-09-17**, in a grilling session on the game as a whole rather than on a feature. The
full record, story included, is `docs/GAME_VISION.md` — a local working file that is deliberately not
committed. This section carries what the entries below depend on, so the roadmap stands without it.

### The rules

- **Work with what the engine offers, never fight it.** It has already decided: a lo-fi monitor instead of
  render-to-texture, `env_global` for anything that crosses maps, hub and spokes over a web, patterns over
  swarms, a time freeze as staging rather than as a mechanic.
- **Exploration first, by far; then RPG; then combat.** Stealth is inside combat.
- **Run, think, shoot, live.** Houndeyes, bullsquids, vortigaunts and grunts charging the player is still
  the game. Stealth adds variety and pays off for most of the game, not all of it.
- **Modules gate the critical path; Skills never do.** The tree is not completable, so no build may be
  needed to finish. Builds open optional spaces and make answers forgiving.
- **Gates are declared.** A *hard* gate is rare, exists only where the story would break, is sealed by
  construction and looks impassable (no power, flooded, sealed) — never a ledge a little too high. A
  *soft* gate is everything else: an intended key plus at least one deliberate alternative (a build, a
  resource spend, an observation), and unintended bypasses are left in. Scarcity prices a bypass. Every
  gate gets one line in the map brief: `hard`, or `soft: intended X, alternative Y`.
- **The game remembers from the first map.** Endings are multiple and read choices and secrets; designing
  them is deliberately the last step. Until then the rule is only this: a moment that might matter sets a
  named `env_global`. One entity now; a retrofit into finished maps later.
- **One telegraph language: a flash, then the discharge.** Unstable crystal teaches it in the first ten
  minutes; the slave's zap, the reactor's lasers, the Nihilanth's patterns and the last Pulse reuse it.
- **Start weak, and scarcity is placement and carrying, not damage numbers.** Enemy health and player
  damage stay near vanilla, which protects every tuning pass already made. Hour one is melee and the Pulse
  against enemies that are fair in melee; a sidearm early only by luck; **soldiers are the arsenal**, so a
  clean kill pays a full weapon; energy and alien weapons are late, authored finds. Ammunition is rare on
  the critical path and present in secrets.

### The premise, as far as the entries need it

A parallel universe that touches Half-Life's only through Xen. A facility that **mines Xen crystal and
processes it into technology**; the suit is adaptive equipment that is *fed* processed crystal, and
[Skill nodes are that fuel](#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate). The
Nihilanth is a collective that seeded the crystals long ago, has a case, and can do no more through them
than **plant ideas**. The player is a miner — silent, named, attuned by exposure, hearing it more clearly
with every node. Management knew about the exposure cases and buried them.

Human enemies are **soldiers** (the bulk, with Xen creatures), a **cult** with a leader, and **the
maddened**; see [Monsters and bosses](#pillar-2-monsters-and-bosses).

### The structure

- **Hub and spokes, in both halves.** Wings are mostly linear chains of maps that end by opening a shortcut
  home. Wings open by **Modules** and by **restored infrastructure** — power to a zone, an elevator, a
  pumped shaft — each one named global state, so the hub visibly comes alive. A wing is a finishable
  project; **the hub plus wing one is the vertical slice of the whole game.**
- **The spine.** A cold open in the deep shaft (the player cuts the **Heart** — provisional name — out of
  an arranged crystal chamber and sends it up; a stray cultist is the first fight), a ride as the prologue,
  the experiment watched on a monitor in a cafeteria, an escape unarmed or with only the Pulse. First half:
  restore the facility for the survivors at the hub. A brief Xen excursion, then the player runs the rig a
  second time on a doctored plan and **the whole facility is taken to Xen**. Second half: collect the
  pieces to go home, in a **soft order** — steered, but a Module *or a creative bypass* opens each. Then
  **the reactor**, then the Nihilanth, at the hub.
- **The reactor** is the facility's own, torn loose and venting, visible from the hub all half: the last
  restored infrastructure and the clearest hard gate, a platforming gauntlet of lasers, platforms and
  damaging walls, more frantic with height. It needs the Dash; the double jump unlocks the true difficulty
  curve; Air Dash the fastest lines. Stock entities only (`env_laser`, `func_train`, `trigger_hurt`,
  `multi_manager`) — the most mapper-heavy, least programmer-heavy set piece in the game. Routing its power
  is what brings the Nihilanth.
- **After the teleport the soldiers are stranded too**: three-way fights, thinning as pieces are collected,
  and the only source of conventional ammunition in Xen. 5f already keeps monster-on-monster kills out of
  witnesses and Disturbances, which is this case. Xenian presence escalates by pieces held, one global.
- **The hazard course** is a separate map launched like vanilla's, the company's safety induction, curated
  around the Pulse and the mining loop. Built late, grown out of `topmap`. The campaign still teaches each
  Module where it is found.

### What it made urgent, and what it did not

Exploration is ranked first, and PILLARS had it at **Not started** until Records landed. What the session
exposed as missing, roughly in the order the vertical slice needs it, with what has happened since: the
[Pulse's tail](#the-pulses-tail--settled-2026-09-17-built-2026-09-18-overnight-visual-open) and the base
Dash in the air (built 2026-09-18); [the Prompt and Records](#pillar-1-records) (built 2026-09-18);
[deposits, shards and Stations](#mining-and-crystal-shards) (built 2026-09-18); the maddened miner (built
2026-09-19); the slave boss and the hub's vortigaunt; the hub and wing one as maps. Not urgent: the Hook,
the double jump (second half), everything in Xen, the endings.

**The first real map is not the hub but [the cold open](#the-cold-open--shaft1-grilled-2026-09-19)**,
`shaft1` (2026-09-19): the most fully specified level in this document, needing no open question answered,
linear, and using only what is built. It is built, walked, and in Andrei's hands in J.A.C.K.; the hub and
wing one come after it.

---

## The prediction problem

**Shape: Solved for weapons; movement goes through the physics key, which the Dash proved.**

This was seven features on one blocker. The weapon half is done — see
[PILLARS pillar 4](PILLARS.md#4-skill-trees) for what was built and
[TECH_DEBT.md](TECH_DEBT.md#skill-state-never-reaches-client-side-weapon-code-—-resolved-2026-08-31) for
the record. `HUD_SetPredictedSkills` populates the client's `CBasePlayer::m_skills` from the mask the
server already sends, and `dlls/skill_tuning.h` lets predicted weapon code read the server's tuning cvars.
`FastReload` was unreserved on the back of it and is in the tree.

What that leaves:

| Feature | Where it lives | Still blocked on |
| --- | --- | --- |
| Dash | [Modules](#pillar-3-modules) | nothing — **built 2026-09-15** on the physics-key route below, in the air since 2026-09-18 |
| Hook | [Modules](#pillar-3-modules) | `pm_shared/` — the same route. Low priority since 2026-09-17 |
| Double jump | [Modules](#pillar-3-modules), new 2026-09-17 | `pm_shared/` — the same route: a physics key gating a second impulse in `PM_Jump`, the long jump's own shape |
| Sprint speed (`SprintSpeed`, id 6) | cut from the tree for good, 2026-09-13 | nothing — cut by design, see the [Routes](#pillar-4-routes) |
| High jump (`HighJump`, id 5) | cut from the tree for good, 2026-09-13 | nothing — cut by design, see the [Routes](#pillar-4-routes) |

Melee Speed (2026-09-14, with the halving rule dropped) and Quick Draw (2026-09-14, the animation sped up
to match on 2026-09-15) used to be rows here and are built; see [PILLARS pillar 4](PILLARS.md#4-skill-trees).

**Movement is a genuinely different problem.** `pm_shared/` runs from `playermove_t`, not from
`CBasePlayer`, so it cannot reach `m_skills` at all — the fix above does not extend to it.

**But a route into it exists, the base game uses it, and the Dash proved it.** The long jump module is a
predicted movement verb gated on player state: the server sets a physics key (`"slj"`, `dlls/items.cpp`)
and `PM_Jump` reads it from `pmove->physinfo`. The Dash rides the same route (`DASH_KEY_*`, written by
`CBasePlayer::DashSync`, read in `pm_shared/pm_shared.cpp`), and so does Phantom's speed. The Hook and
the double jump go the same way.

---

## Maps

**Shape: No longer a blocker, as of 2026-09-15.** This entry used to argue that six finished systems
(Skill Points, Reset Tokens, Syringes, Row Grants, exploration itself, Records) were code nobody could
reach in play, because vanilla maps cannot hold them. Two decisions closed it:

- **The Skill Point economy is a non-issue.** The numbers in [SKILL_TREE.md](SKILL_TREE.md#the-economy-renumbered)
  stand as set; nothing waits on judging them against a map, and no feature should be held for it.
- **The proving map is tested and closed.** It served its purpose as a showcase of coworking in mapping
  ([PROVING_MAP.md](PROVING_MAP.md)). **`topmap` is the default test map**, and it places Skill Points,
  Reset Tokens, Row Grants and Syringes, so every pickup is reachable in play there. `shaft1` is the
  campaign's first map ([the cold open](#the-cold-open--shaft1-grilled-2026-09-19)).

What the campaign's maps should contain is under [The world](#pillar-1-the-world).

---

## Pillar 6: Stealth

**Shape: Building. Steps 1–4 and 6 are done — everything that decides whether the player is noticed, and
telling them about it — and of step 5, everything after acquisition, the cost of a kill (5f) is built and
tested. What remains is 5b, 5e and 5g, sliced on 2026-09-17 around one decision: stealth is predator
first.**

**Finish, then freeze — settled 2026-09-17 with [the shape of the game](#the-shape-of-the-game).** Stealth
is one way to solve an encounter, inside combat, not a pillar ranked beside exploration. So: build 5b, 5e
and 5g — they are what stops the loop being sticky, and 5g matters more than it did now that the game is
hub and spokes and level transitions are constant — and then **no new stealth features until real maps
exist**. Every open question below is waiting on a map, not on code. What stealth gained in that
session is content, not features: ritual rooms (cultists busy, backs turned, perception live), the
military wing with the assassin at its end, the maddened as the untrained-profile tutorial enemy, and
soft gates a Stealth build can ghost. It already accommodates alien grunts and vortigaunts.

**Predator first — settled 2026-09-17.** The loop the pillar is judged on is *unseen, kill, unseen again*.
Ghosting a room stays possible and is not the measure, because every Stealth node that exists rewards a
strike on an unaware monster and none rewards passing one, corpses are invisible so one-by-one kills are
what the engine makes cheap, and in vanilla maps "I saved some ammo" is a reward nobody can feel. The
intended play, from which every number below was checked: stab one, swap to the silenced pistol, put a
round in the head of whoever saw it before its meter fills.

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
| 3 | **Suspicion** | Perception Profile, the meter, the `Look` gate, the `debug_suspicion` view (replaced by `debug_schedule` 2026-09-17), `SF_MONSTER_IGNORE_CONCEALMENT`, `adr/0009`. **Done 2026-09-01.** |
| 4 | **The readout** | `gmsgConceal` plus `CHudConceal`, following `CHudPulse`'s send-on-change pattern. **Done 2026-09-02.** |
| 5 | **[The post-aggro step](#the-post-aggro-step)** | Everything that happens *after* a monster acquires the player. Attempted 2026-09-02 and reverted; **re-sliced 2026-09-17**, cost of a kill first (witnesses, the Disturbance, the Search — **done 2026-09-17, tested 2026-09-18**), then the give-up, then the captain's channel — see below. Aim-versus-facing is dropped. |
| 6 | **Noise** | A deliberate multiplier on the computed noise volume for crouching and walking. **Done 2026-09-02**, pulled forward: without it a crouched player could not get within crowbar reach without being heard, so the Backstab's own approach did not work. |

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

**So the seam would be `ShootAtEnemy`, not the writers.** Clamp the shot to the monster's own cone — fire
along facing when the LKP falls outside it — and "get behind it" means something with no LKP bookkeeping
and no per-monster schedule surgery, for every monster that fires through it: the grunt's rifle and
shotgun, the assassin, the alien slave's zap, Barney, the ichthyosaur and the Gargantua's flame. It does
**not** carry the alien grunt, which aims its hornets straight at `m_vecEnemyLKP` itself
(`dlls/agrunt.cpp:437`) and whose hornets then home in regardless; nor grenades, which are thrown at the
LKP directly (`dlls/hgrunt.cpp:495-527`). Those would need their own answers. None of it is being built:
5a is dropped, below.

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

**Fixed and verified 2026-09-14**: `ChangeSchedule` clears the move-wait, so a freeze is scoped to the
schedule that set it, and the TECH_DEBT entry is closed. It had to come first, because post-aggro
behaviour could not be judged against a monster that might be frozen for 99 seconds; with it fixed, a
monster that loses the player actually turns around, and the aim seam above lost most of its urgency.

**How this step is sliced — settled 2026-09-17 under predator first.** A first slicing on 2026-09-12 put
aim (5a) first and the give-up (5b) second, in small commits. Both changed:

**5a is dropped.** Under predator first, being hunted means the loop has already failed and the player is
in vanilla combat, which the mod promises to leave byte-for-byte alone. Whether being behind a hunting
monster should be safer is a combat question, and clamping the shot to the cone would change every
gunfight in the vanilla campaign. Aim stays vanilla. The move-wait fix already removed the frozen grunt
shooting through its own back; if that is seen again it is a bug report, not a design question. The
diagnosis above is kept because it is true, not because anything will act on it.

**The cost of a kill comes before the recovery from a failure.** Today a kill costs nothing — kill one grunt
of four from behind and the other three do not react — so every kill is already a Silent Kill and the
Major rewards nothing. The loop needs tension before it needs mercy, and a witnessed kill ending in a
vanilla fight is a failure state the player understands and can be judged in any grunt squad in any
vanilla map. The order, each commit independently playable:

| | | Settled |
| --- | --- | --- |
| **5f** | **Witnesses, the Disturbance, the Search.** The cost of a kill | 2026-09-17, below. **Built the same day**, with the silencer item, the Headhunter helmet skip, Shroud, Silent Kill's effect, and the Ambush and fill cvar changes. **Tested 2026-09-18** against [STEALTH_CHECKLIST.md](STEALTH_CHECKLIST.md), whose results are at its end: every row passed once the Search was made to walk (its path ended inside the victim's hull). Recorded in [PILLARS pillar 6](PILLARS.md#the-cost-of-a-kill--built-2026-09-17-tested-2026-09-18). `topmap` has no `info_node`s, so the Search around corners is still unjudged |
| **5b** | **The give-up**, contact-keyed, into the same Search | 2026-09-17, below. Not built; its lines (`HG_LOST`) are already in `sound/sentences.txt` |
| **5e** | **The captain's channel** — notice propagation | 2026-09-17, below. **Built 2026-09-20**, not verified in game |
| 5g | The level-change reset | 2026-08-31, [PERCEPTION.md](PERCEPTION.md#losing-the-player--the-give-up-settled-2026-09-17-not-built). **Built 2026-09-20**, not verified in game |
| 5c, 5d | The Search, Posts | Absorbed into 5f: the SDK already has both |
| 5a | Monsters only shoot where they face | Dropped |

Alongside 5f, two things outside this pillar that the intended play depends on: **the silenced pistol
comes forward as a found item**, the first [Evolution](#weapon-evolutions), and **Headhunter ignores the
grunt's helmet** with Ambush retuned to ×1.5 / ×2 ([SKILL_TREE.md](SKILL_TREE.md#stealth) has the
arithmetic — without both, the stab-then-headshot play leaves a live grunt at 0.75 and rising).

**What 5f is.** The full model is in [PERCEPTION.md part 2](PERCEPTION.md#death-witnesses-and-the-disturbance--settled-and-built-2026-09-17-untested-in-game);
the decisions, in the order they were made:

- **A witness** is any hostile monster with a meter that passes a visibility trace on the victim as it
  dies, squad or not, **for player-dealt kills only** — otherwise every marines-versus-aliens set piece
  fills the sound pool with grunts searching bodies they shot themselves. It jumps to `suspicion_witness`
  (0.75), speaks, and takes the death spot as its last known position. **A jump, not a floor**: from
  there the meter fills or drains as normal, but never again below `suspicion_floor` (0.3) until a level
  change. 0.75 rather than outright acquisition because the difference is exactly the half-second the loop
  is made of — kill, duck, gone — and it keeps "only sight acquires" true with no new exception. The floor
  sits *under* the readout's Noticed line (0.35) on purpose: the room is primed, the icon stays dim.
- **The Disturbance** is the death spot entering the sound list: a new sound bit, `disturbance_volume`
  (512 units) for `disturbance_duration` (20 s), player-dealt kills only, heard by Trained profiles via a
  flag on the profile struct (soldiers care about bodies; a zombie does not; the bullsquid already smells a
  carcass through the existing scent bit and that stays untouched). It is a real sound, so the existing
  hear-and-turn machinery drives it for free.
- **The Search is the SDK's own.** `slInvestigateSound` (`dlls/defaultai.cpp:285`) stops, stores the
  monster's position, walks to the best sound, idles ten seconds, walks back and clears — a Search and a
  return to Post in one vanilla table that only the assassin ever picks. Grunts have every task it needs.
  So the Search and the Post are a schedule switch on hearing a Disturbance, and 5c and 5d cease to exist.
- **A squad sends one.** The leader picks its nearest member; the rest hold where they are, meters at the
  jump, turned toward the body. The predator's reward for a clean kill is the next isolated target.
- **Loners all go.** With no leader there is nobody to pick one, so every leaderless monster that hears a
  Disturbance walks to it and they arrive as a mob. **Kept deliberately** (2026-09-17, one-answers-by-claim
  was proposed and rejected): disciplined squads isolate, rabble converge, and the mob has an answer — kill
  a grunt, make a noise, drop a satchel, leave. [ADR-0014](adr/0014-a-body-draws-one-squad-member-or-every-loner.md).
- **Silent Kill is about the ears only.** On a victim below Spotted no Disturbance is inserted, so a
  squadmate around the corner never knows. A squadmate *in sight* reacts in full — soldiers are not blind,
  and the answer to the one who saw is the silenced headshot. The weapon's own noise is untouched: a
  gunshot is a sound that led to the kill, not one that results from it, and a quiet gun is the silencer.

**What 5b is.** No sight of the player and no player damage for `suspicion_giveup` seconds (first guess
10), then the monster nulls its enemy, sets its meter to the same 0.75 jump (it was pinned at 1.0, and left
there the next look would re-acquire on sight and no Search would ever happen), says "stay alert", runs the
same investigate schedule to the last known position, and settles at ALERT on the floor. **Contact is
squad-wide where there is a leader** — last sight by any member, last damage to any — via the
`m_flLastEnemySightTime` the leader already holds, or one grunt gives up and says "stay alert" while its
squadmate is trading fire. The damage clause stays, so shooting from cover and waiting does not work.

**What 5e is, and why the captain matters.** When any member crosses Noticed, the leader lifts every
member to the notice line with a line — "stay alert people". Loners never do this. So while he lives the
captain sends the searcher, primes the squad on a slip, and holds the contact clock; when he dies the SDK
dissolves the squad with no promotion, and the survivors lose acquisition sharing (a slip alerts the one
who saw you, not four), the attack slots (everyone fires and throws at once), the friendly-fire check, and
the propagation. Killing him first is encouraged by what he does alive, not by a node. He is already
marked — the leader wears the commander head, the beret, set in `StartMonster` — and already unhelmeted,
so the pistol headshot always worked on him. **Cut the Head is dropped** (id 129 retired): its "drop the
survivors' meters" was a footnote to a free reward. **Shroud** takes its cell, a flat ×0.8 on the fill.

**Sentences.** Every line above is composed from words the grunt already has, in a new mod `sentences.txt`
(the mod has none yet). First picks: `HG_ALERT3` "shit, we got hostiles" for the witness, `HG_QUEST5`
"sweep that sector" for the send, `HG_QUEST0` / `HG_QUEST11` "stay alert" for the give-up and the
propagation, `HG_CLEAR3` "no sign hostiles sir" for the walk back. The vanilla `HG_QUEST` group cannot be
played as a group — it also holds "echo mission is go".

**Fill at 2.0.** `suspicion_fill` doubles (from 1.0), settled 2026-09-17 with the Stealth numbers: a Trained
monster looking at a running, lit player at close range takes 0.35 s, and a fully invested player crouched
in front of a lit grunt at mid range has about three seconds, cover to cover. Stealth is meant to work
behind unaware enemies and in the dark; the lit room is generous enough at that.

**Two facts to record rather than fix.** The witness window in the open is about a sixth of a second at
fill 1.0 and less at 2.0 — the play exists only from cover, which is what "sufficiently concealed" meant
when it was said. And a searcher that hears a second Disturbance mid-walk restarts the schedule from where
it stands, so its walk-back position drifts; the vanilla schedule drifts the same way.

### Deliberately deferred

Recorded so they are not rediscovered as gaps.

**Silent weapons — no longer deferred: built with 5f, 2026-09-17.** The silencer is a found item,
permanent once attached, and the first [Evolution](#weapon-evolutions). It was dormant in the SDK: the
glock's silenced submodel, the `QUIET_GUN_VOLUME` and `DIM_GUN_FLASH` branch, and a real attach animation
in `v_9mmhandgun.mdl`. Exposing it had been deferred to Evolutions so as not to pre-decide the identity
question; the predator loop's intended play needed it, and it answers that question with the example it
was written around instead. The crossbow is already `QUIET_GUN_VOLUME` and is the mod's existing quiet
weapon whether anyone intended it or not.

**A second Backstab tier.** The Backstab is positional and single-tier, so the damage model cannot tell a
stealth kill from a flank. If "measurably better off" proves too thin in play, a larger multiplier when the
victim has never acquired the player is the obvious lever, and it costs one branch on a test already being
made.

**Stealth Skills — built 2026-09-16**, Night Vision Module included, and Silent Kill's effect with 5f the
day after; moved to
[PILLARS.md pillar 6](PILLARS.md#the-stealth-region-and-the-night-vision-module--built-2026-09-16-untested-in-game).
Cut the Head was dropped on 2026-09-17 and Shroud takes its cell.

**Generalising perception to monster-vs-monster.** Scoped to the player deliberately. The reasoning —
including the muzzle-flash asymmetry that makes a naive generalisation exactly backwards — is under
[Deliberately not generalised](PERCEPTION.md#deliberately-not-generalised). Marked for review, not for
building.

### Open questions

- **Stealth is optional everywhere, with declared exceptions, and nothing is unwinnable head-on**
  (2026-09-17). This is a run, think, shoot, live game; bosses, sieges and plain charging fights are
  fights, marked with the mapper's spawnflag. Everything else can be solved quietly, loudly or by going
  around.
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

**Rewritten 2026-09-17 for predator first.** A player can clear an occupied room one monster at a time,
through choices they made — light, speed, weapon, who to take first — with each kill leaving them unseen
again, and a kill that was seen costs them the room. Crossing a room unseen is a way to reach the first
kill, not the measure.

---

## Pillar 2: Weapons

### Custom weapons

**Shape: two built (the pickaxe and the katana), one Idea (the energy rifle).** The approach for all of
them is to import and reuse existing assets rather than author from scratch, and to accept placeholder
quality on the first pass; what Andrei makes by hand goes through the tier in
[MODEL_WORKFLOW.md](MODEL_WORKFLOW.md).

#### The Carbon Pickaxe

Heavier, slower, hits harder, beside the crowbar rather than instead of it.

**v1 built and verified in game 2026-09-18** — `weapon_pickaxe`, 25 damage at 1.5× the crowbar's swing time,
the only mining tool, the crowbar in black metal; recorded in [PILLARS pillar 2](PILLARS.md#2-enhanced-combat).
What is left here: a real pick head ([ART_DEBT.md](ART_DEBT.md#the-carbon-pickaxe--a-black-crowbar-and-the-crowbars-sounds-and-hud-icon),
the first model on the hand-made ladder in [CRAFT_LOG.md](CRAFT_LOG.md)), and becoming the starting tool,
which is campaign work.

**The starting tool, and the mining tool — settled 2026-09-17.** The player is a miner: the pickaxe is
what they hold in the cold open, check in at the end of the shift, and get back within the first 5–10
minutes; the crowbar becomes a find. It is also the only thing that breaks a
[deposit](#mining-and-crystal-shards), which is what keeps it worth three Cells once the katana exists.
The maddened miners carry one too.

**It joins the crowbar rather than replacing it — settled 2026-09-13**, as one of a melee roster on the
crowbar's base, each weapon leaning one way; see the [Melee Route](#melee). The mechanic is
`dlls/crowbar.cpp` with different numbers and a different model. The cost was in what the crowbar's name
was load-bearing for, and that is paid: the Skills named for it (Reach, Force, Speed, Follow-Up) went
generic in enumerator, display string and cvar with the Melee Route on 2026-09-14, ids unchanged, and
Valve's half-damage follow-up swing was dropped the day before, so a swing-rate change costs nothing. The
one crowbar-named thing left is `d_crowbar`, the icon for three Skills ([ART_DEBT.md](ART_DEBT.md)).

#### The Gauss Katana

**Shape: Built, v1, and reworked onto the Energy Route's rules (2026-09-12 to 2026-09-15); the model is
the open question.** A gauss weapon shaped as a katana. Slow, bulky, and frightening. What exists is in
[PILLARS pillar 2](PILLARS.md#2-enhanced-combat); this entry keeps the decisions, the numbers that are
only recorded here, and what is still open.

**The shape it settled into:**

- **A melee weapon on the crowbar's shape**: `dlls/crowbar.cpp` with different numbers and a different
  model, so Backstab, Melee Force and Reach and the Follow-Up come along unchanged — the point of
  subclassing rather than copying. `weapon_katana`, `dlls/katana.cpp`, `CCrowbar` with two hooks
  overridden: base damage (`sk_plr_katana1-3`, 60 since 2026-09-14) and swing time
  (`katana_swing_time_scale`, 2.4, so 0.6 s after a hit; it started at 2×, went to 1× when the wave still
  rode on the swing and the slow swing read as waiting, and is slow again now that the slash is the burst
  click and the wave the fast one). Both read from both DLLs because the delay is predicted. In the melee
  bucket beside the crowbar; `impulse 101` gives it; the FGD places it.
- **The wave is a crescent that flies and burns the wall it meets.** `EV_KatanaArc` in
  `cl_dll/ev_hldm.cpp`, fired by `events/katana_arc.sc`: a `)` standing in front of the player and tilted
  to the cut, a temp entity with no model and a per-frame callback (`EV_KatanaArcThink`) that moves it,
  redraws it as ten short beam segments bright at the belly and thin at the tips, and traces three points
  forward. The burn is the gauss glow sprite fading over six seconds, one per point of the crescent, so
  the wall shows the crescent's silhouette. Seven client cvars, all first guesses: `katana_arc` (on/off),
  `_range` (1200, which is also how far it shrinks and dims to nothing, so running out never looks like a
  cut), `_radius` (70), `_sweep` (150° of the circle), `_speed` (1200), `_roll` (30°, sign alternating
  swing to swing), `_lean` (90: how far the belly turns from facing the player toward the line of flight,
  so it leads with its belly). A floor or ceiling does not end the flight: a tip that meets one scrapes a
  glow along it and the wave flies on. Rejected on the way: a fan of lightning rays from the blade to the
  walls (the first look, replaced at Andrei's request the same day); gunshot decals under the glow (they
  read as bullet holes); ending the flight on the floor (aiming down at a headcrab killed the wave on
  the first floor tile).
- **The wave hurts, and its damage travels with it.** Decided 2026-09-12, when aiming down at headcrabs
  made a wave that only looked like an attack feel like a miss. `CKatanaWave` (`dlls/katana.cpp`) is an
  unseen server projectile whose look is the crescent: born 32 units past the blade on the aim, flying at
  `katana_wave_speed` (1200) to `katana_wave_range` (1200), each frame's step swept as a line then the
  small hull, and everything damageable it meets struck once (it pierces) with energy damage at
  `katana_wave_damage` (15; a plain number, not a share of the slash's, since the two are tuned apart),
  full out to `katana_wave_full_range` (800) and then falling off to nothing at the range. The client
  reads those three cvars by name for the crescent, so the drawn wave and the damage are one flight. It
  dies where the crescent's belly does, on a wall; a floor or ceiling the line meets is scraped and flown
  through, and a step spent wholly inside the world ends it. The crossbow bolt was the template for the
  entity, not for the look — a crescent cannot be a model, so it stays client-drawn from the event. An
  instant trace at the swing was the first version, kept for a day on the grounds that the match was good
  enough, until a target 800 units out taking damage at the swing showed it was not. The katana is
  therefore a **ranged melee weapon**: the blade for what is in reach, the wave for what is not, at half
  strength, and a crowd on the path is a crowd hit.
- **The blade goes hot on the swing.** Two parts: a dynamic light at the hand (`katana_glow_light`, 3 s,
  decaying over its life, which is the fade the eye reads) and the blade's own texture swapping to a hot
  one (`katana_glow_hot`, 0.7 s), cooling back over its last `katana_glow_fade` (0.5 s) — the viewmodel
  drawn a second time in the cold skin, blended in as the heat falls, so only the blade changes
  (`StudioRenderKatanaCooling`). The hot blade is a second state in the skin families — six now, glove
  colour × cold/hot, glove-major — with the blade metal split onto a material of its own so only it
  swaps, and the hot texture flagged **additive** by `utils/mdltool/mdlflags.py` after the compile. It
  is drawn as light over what is behind it and is a little transparent; it does **not** shine in the dark
  by itself (learned on the progression pickups, 2026-09-14 — additive is still multiplied by the room's
  light), which is why the hand light matters: the light is what lights the blade. Two
  approaches were built and rejected first: a beam entity between two viewmodel attachments (the engine
  draws beams before the viewmodel with last frame's attachments, so it trailed the swing) and quads drawn
  by the studio renderer in the viewmodel's pass (right place, wrong look). The attachments stay in the
  model for whatever wants them next. The glove half of the six families is picked by the player's Suit
  Variant in `cl_dll/view.cpp`; see PILLARS.md.
- **The blade leaves a trail on the swing, in first person. Built 2026-09-15, shaped in a grilling
  session.** `cl_dll/katana_trail.cpp`: the ribbon the blade sweeps, blade only — a quad strip between
  successive blade lines, each from where the blade leaves the guard (`$attachment 2`, written by the
  bend stage in MODEL_WORKFLOW.md) to the point (`$attachment 1`) — gauss orange, additive, textured
  with the crescent's own beam sprite so the trail and the wave read as one substance, fading and
  tapering (`katana_trail_taper`, 0.6 of the blade's length lost at the tail) over `katana_trail_life`
  (0.12 s); `katana_trail 0` turns it off. Sampled only while an attack animation is **playing** (the
  crowbar's sequences 3–8, judged by start time and frame count — the sequence stays *selected*,
  frozen on its last frame, for ten seconds after, and sampling through that turned the viewmodel's
  lag behind the camera into a smear on every mouse turn), both clicks alike, and kept in **view
  space** so only the swing's own motion leaves a trail. **Weighted by the blade's speed**
  (`katana_trail_speed`, 300 units/s of the point through view space for full strength, nothing
  below half): every attack is a short cut and a slow recovery — measured with
  `katana_swing_lead.py`, the point does 300–700 in the cut and 70–220 coming back — and the ribbon
  belongs to the cut, thinning away as the blade slows. Chosen over a fixed sequence-depth cutoff
  (the cut ends at 45%, 60% and 35% of the three swings, so one number fits none) and over QC
  animation events at the cut's frames, the engine's way for muzzle flashes, which is twice the work
  and stops hard at a frame. Drawn from **inside the viewmodel's studio draw**
  (`CStudioModelRenderer::StudioDrawModel`, after the model), which is the one place with this frame's
  attachments; the engine draws beams and the transparent-triangle hook before the viewmodel with
  last frame's, which is why the beam glow lagged. Triangles drawn there **do** inherit the viewmodel's
  narrowed depth range: the wall test passed, the ribbon stays whole where the blade does. But the
  triangle API's colour and render-mode calls do **not** take in that pass (white and unblended,
  through both colour calls, with the particle manager's own recipe), so the quads go through
  **OpenGL directly** — texture modulation and additive blend set by hand, the sprite still bound
  through the API, state restored after — and the client now links `opengl32` (`-lGL` on Linux).
  Hardware renderer only. Two more lessons from the first build, both in MODEL_WORKFLOW.md:
  `$attachment` coordinates are bone-local, and the katana's had been model-space since v1.
- **Viewmodel and world model are the mod's own.** The Dystopia blade on Half-Life's crowbar hands
  and the crowbar's eleven animations in their original order, so the crowbar's `CROWBAR_*` sequence
  indices drive it unchanged, and the Dystopia prop lying flat for the floor. Sources in
  `E:\CustomAssets\models\src\{v,w}_katana`, made by the loop in [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md).
  The `p_` model, the sounds and the HUD icon are the crowbar's; [ART_DEBT.md](ART_DEBT.md) has them.

**The two clicks — settled 2026-09-13 while shaping the [Energy Route](#energy), built 2026-09-14 with
Cleave:**

- **Left click is the slash.** Blade only, full damage, no ammo, Melee Speed applies, and it is the click
  that carries Cleave (an orange air shock on the katana). Every swing heats the blade, through
  `events/katana_swing.sc`.
- **Right click is the wave, for uranium.** The blade light (`katana_wave_blade_damage`, 10) and the
  wave thrown off it, at the crowbar's rate (`katana_wave_swing_time_scale`, 1.0), Melee Speed scaling it
  too for now. **The numbers, set 2026-09-14 against the gauss's two clicks**: slash 60 at 0.6 s is
  burst, blade 10 plus wave 15 per target at 0.25 s is DPS — 100 a second in reach, 60 per target down the
  path, for uranium. Cleave never spends on it. The cost is `katana_wave_cost` (5) uranium per wave,
  divided by Energy Efficiency (four with it), refused with the empty click and no swing when short. The
  katana carries uranium for it — `KATANA_DEFAULT_GIVE` (20) on pickup, the gauss's figure, the uranium
  counter on the HUD — and can always be drawn and is never switched away from empty, since the slash
  needs none. **The lore, settled the same day:** swinging heats the energy in the blade at no loss; a
  thrown wave spends some of it. The blade heats on both clicks.
- **No charge.** The rework first wrote the right click as "hold to charge, release a big wave"; the wave
  is thrown at the crowbar's rate instead, and Quick Charge was cut for it on 2026-09-15.
- **The katana always deals energy damage**, slash and wave (`SwingDamageType`, 2026-09-14), and
  **scales off both Melee and Energy bonuses**. `DMG_ENERGYBEAM` because nothing is immune to it, where
  the alien slave is immune to `DMG_SHOCK`.

**Still to tune, as one piece**: the cost's number, the wave's damage and the blade's share on the right
click against the slash's. Andrei tunes them against the gauss and the health table.

**Still open: the animations.** Whether the crowbar's one-armed swings are enough for a heavy weapon, or
the longsword moveset below takes over, is the identity question v1 in game raised and has not answered.
Still a candidate for [Evolutions](#weapon-evolutions).

**The longsword moveset — probe built 2026-09-16, not yet judged.** Age of Chivalry's `v_longsword`
viewmodel (decompiled by Andrei to `E:\CustomAssets\models\decompiled\aoc\v_longsword`) carries a
two-handed moveset that may suit the katana better than the crowbar's one-armed swings: idle, draw,
holster, two swings, a stab, a block and a deflect, on a 44-bone ValveBiped rig with both arms and
fingers. Shaped in a grilling session the same day:

- **The animations come to us; the katana is not re-skinned onto another rig for its own sake.** The
  literal request — Half-Life's hands and the katana weighted onto the longsword's bones — was the least
  accessible reading and was set aside.
- **First, a probe, judged in HLMV only.** `E:\CustomAssets\scripts\katana_aoc.py` compiles the
  longsword's rig and all eight animations *as decompiled*, nothing retargeted, with AoC's footman hands
  (the only AoC hand mesh under GoldSrc's 2048-vertex submodel limit; the knight's is 4,200) in flat
  stand-in colours and a `blade` bodygroup holding the decompiled longsword and the katana blade, so the
  same swing is seen with the sword it was made for and with ours. Output
  `E:\CustomAssets\models\src\v_katana_aoc\v_katana_aoc.mdl`; ships nowhere. **Seen in HLMV the same
  day — "has potential" — and rebuilt as a game drop-in** (`--install` copies it over the install's
  `v_katana.mdl`; the repo's `models/v_katana.mdl` restores it): the katana first in its bodygroup so
  body 0 shows it, the trail's three attachments bone-local on the sword bone, the blade's cold and hot
  textures laid out as the six glove-major skin families the client reads, no glove variants, no
  hitboxes of its own, no code change. **Fine-tuned after the first in-game look, the same day, before
  a definitive decision:** the katana slid 2 units out along its axis so the guard clears the right
  thumb (the thumb's tip reaches 5.3 up the sword bone, the guard's underside sat at 3.5), its handle
  slimmed to 0.45 of its width below the guard (the Dystopia handle is a flat card 4–6 units wide and
  the hands close on about 2; `--slide`, `--slim`), the clavicle and upper-arm geometry dropped since
  Half-Life shows the arm from the elbow, and the footman hands painted as an HEV glove by texture
  bands in the mod's three glove variants — its texture is one island stacked by height, fingers and
  hand at the top, forearm, upper arm — so the six families now change gloves and blade both.
  **Andrei's reading of the animations in game:** the swipe is a Cleave, the overhead a riposte, the
  stab a Backstab — a verb mapping not yet designed. And the open identity question: the katana as
  built is a swift blade, the AoC moveset a bulky heavy sword, which is what *slow, bulky, and
  frightening* above asked for. The slot table is the crowbar's twelve indices — idle_01 in the three idle slots,
  draw, holster, swing1 as attack1, swing2 as attack2 and as the Cleave, stab as attack3 — then block
  and deflect appended as 12 and 13, unplayed until a Guard verb exists. **None of the eight is
  dropped.** Measured: the katana's point reaches 25 units past the grip where the longsword's reaches
  45; each swing is 76 frames at 35 fps, about 0.35 s of wind-up, 0.4 s of cut and 1.4 s of return.
- **Deferred until the probe is judged, in this order.** *The hands:* either AoC's footman mesh painted
  as an HEV glove (no texture came out of the decompile and the mod is not installed here, so it is
  painted from the UV layout), or Valve's own gauss hands — the stock two-armed 32-bone rig, the
  crowbar's eleven bones under the same names plus a left arm — with the animations retargeted onto
  them by orientation matching, and a solver pass pinning the left hand to the grip if it floats.
  *The timing:* the animation trimmed to the weapon's settled 0.6 s slash and 0.25 s wave (start near
  the cut, raise the fps, keep the return for the last swing to play out), or the weapon slowed to the
  animation with a hit delay; explicitly left open in both directions.
- **Credit:** an [ART_DEBT.md](ART_DEBT.md) entry like the Cleave swing's, licence unchecked.

#### The energy rifle

**Shape: Idea, from Andrei 2026-09-18. Not grilled.** An alien rifle with a scope that fires green beams:
the game's sniper. It is **technological, not organic** — built, not grown — which sets it apart from the
hivehand and the [alien Module's](#alien) Core weapons.

**It is unlocked by alien progression**, past a threshold. Two ways proposed, not chosen:

- **A gift.** Enough Alien nodes taken and the [friendly vortigaunt](#friendly-alien-slave) hands it over
  through his machine — which fits the hand-over he already has and the rule that energy and alien weapons
  are late, authored finds ([The rules](#the-rules)). It is also the first hand-over gated on the build
  rather than on the story, and "Skills never gate the critical path" holds only while the rifle is
  optional.
- **Crafted**, at a [crafting Station](#crafting--idea-2026-09-18). Waits on crafting existing.

What it touches, for when it is grilled:

- **Green beams are presumably `DMG_ENERGYBEAM`**, which scales with the [Energy Route](#energy), passes the
  Gargantua's filter and is what a [Panthereye immune to all but energy](#panthereye) would demand. That
  makes it the ranged half of the Energy build beside the katana.
- **It fires Cores** — settled by Andrei 2026-09-18. The same ammunition as the alien Module's weapons,
  which the Ammunition Station already makes. The Core's look changes with it to a green battery (below,
  and in [ART_DEBT.md](ART_DEBT.md#the-alien-module--stand-in-models-and-the-summon-weapons-borrowed-everything)),
  which suits a technological weapon better than the organic reading of Cores did.
- **A scope is new code**: Half-Life's only zoom is the crossbow's and the 357's FOV change
  (`m_iFOV`), which is the whole of it — no overlay, no sway.
- **Beams at sniper range meet the katana wave's lesson**: a visible projectile drawn on the client around
  one server trace is how the wave already works; a beam is simpler, `TE_BEAMPOINTS` from the muzzle.
- Model, sounds and icon: [ART_DEBT.md](ART_DEBT.md) when built. The stated approach for custom weapons is
  to reuse existing assets.

**Cores become green batteries — Andrei, 2026-09-18.** The Core's world model is the HEV battery
reskinned from its blue to the vortigaunt beam's green. **The battery and the Core both give off light in
their own colour.** The light is cheap: `ProgressionLight` in `cl_dll/entity.cpp` already gives the
progression pickups a dynamic light keyed by model name, so each is one row in its table. Two things to
watch when it is built. The table is keyed by model, so the Core has to be its own `.mdl`, or the row has to
read the skin as well. And the Row Grant's light is already green (80, 255, 80), so the Core's green has to
be told apart from it. The battery's source is in the SDK (`Weapon Models/world_models/wrld_battery`,
[HL_SDK.md](HL_SDK.md)), so the reskin needs no decompile. **Built 2026-09-19**, not yet verified in game:
`models/w_core.mdl` from `core_world.py`, the light rows for both, the details in
[ART_DEBT.md](ART_DEBT.md#the-cores-replacement-decided-2026-09-18-built-2026-09-19).

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

**The silencer is the first Evolution — settled 2026-09-17**, pulled forward by
[pillar 6](#the-post-aggro-step) because the predator loop's intended play (stab, swap, silenced headshot)
has no second half without it. It is **found**, an item entity in the pattern of the Night Vision Module:
pick it up, the pistol plays its attach animation, and from then on it fires quiet and dim. **Permanent**
once attached. That answers the second question below for this Evolution by example, and gives mappers a
way to hand the loop's second half to the player where the level wants it. A Stealth node was rejected
because no other node changes a weapon's identity; always-on was rejected because it changes the vanilla
campaign for players who never touch stealth.

Consequences to settle before building the next one:

- Where does an Evolution **live**? It is durable per-weapon state, so it wants to sit with the weapon
  rather than in `CPlayerInventory` — but a dropped weapon keeps its clip today and would have to keep its
  Evolutions too, or dropping becomes lossy (the inventory design is explicit that dropping and retaking is
  exactly lossless). The silencer has to answer this for itself when built.
- Is an Evolution **found** or **bought**? The silencer is found. Whether every Evolution is, or whether
  the Armaments column buys some, is still open — the user's list put weapon handling upgrades under
  "upgrade points invested", which points at bought for at least some.
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
what is left is goals 4 and 5 below. **The codenames got their first job on 2026-09-15**: the Inventory
Panel's header reads `HEV MK IV  //  <codename>` for the suit worn ([SKILL_PANEL.md](SKILL_PANEL.md)).

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
5. **The gloves emit light.** Later, and harder than it looked. The engine **ignores
   `STUDIO_NF_FULLBRIGHT`** on studio textures — the katana tested it — and **honours
   `STUDIO_NF_ADDITIVE`**, drawing the texture as light over what is behind it at the cost of some
   transparency — but **additive does not shine in the dark** (progression pickups, 2026-09-14: the
   renderer still multiplies it by the room's light). So the seams can *look* lit by day and will go dark
   with the room, and making them genuinely glow means a dynamic light at the hands, which is a light
   the player carries everywhere and a real stealth cost. `utils/mdltool/mdlflags.py` patches either
   flag into a compiled `.mdl`. For the gloves' look: split the light channels onto a texture of their
   own in the generator, give those faces their own material in the reference SMD (a mesh edit, since
   the seams are painted on the sleeve's faces today), compile, patch additive. The transparency is the
   open question for a seam that is always on, where it was fine for a blade lit for under a second.
   PERCEPTION.md's light term reads the lightmap, not the viewmodel, so an additive seam costs nothing in
   Concealment; a dynamic light would not either as the code stands, and that would then be a lie worth
   deciding about.

Known issue from the first pass: on the crossbow the hand clips slightly through the stock. Not from the
textures; recorded in [ART_DEBT.md](ART_DEBT.md) for later.

---

## Pillar 2: Monsters and bosses

**Shape: a roster, listed 2026-09-12 and explored one entry at a time. Three are built — the Panthereye,
the melee alien grunt and the maddened miner; the rest are Shaped or Ideas.**

Import and reuse existing monsters with light polish, with occasional custom AI. **Kingpin is the boss of
Xen hell** (2026-09-17), and the roster gained the human side the same day:
[the cult, the maddened](#the-cult-and-the-maddened) and
[Xen hell's cut monsters](#xen-hell-and-the-cut-monsters). Where each boss sits is under
[Where the bosses sit](#where-the-bosses-sit).

### The roster

| Entry | Kind | Starts from | Shape |
| --- | --- | --- | --- |
| [Panthereye](#panthereye) | Enemy | Half-Life's cut model and sounds; AI written new | **v1 built** 2026-09-18 and played; the menace rework grilled 2026-09-23, not built |
| [Melee alien grunt](#melee-alien-grunt) | Enemy | `CAGrunt`, bare arm | **v1 built** 2026-09-18, verified |
| [Shelled headcrab](#shelled-headcrab) | Enemy | `CHeadCrab`, recoloured | **Shaped** |
| [Friendly alien slave](#friendly-alien-slave) | Non-combatant | The slave model on `CTalkMonster` | **Shaped** |
| [Alien slave boss](#the-alien-slave-boss) | Boss, freed to become the friendly slave, or killed | `CISlave` | **Shaped** 2026-09-23: the Ward, the Overcharge, three Bindings |
| [Assassin boss](#the-assassin-boss) | Boss | `CHAssassin` | Idea |
| [Alien grunt boss](#the-alien-grunt-boss) | Boss | `CAGrunt` | Idea |
| [Nihilanth](#the-nihilanth) | Boss | `CNihilanth`, new model | **Shaped** 2026-09-17: a pattern fight |
| [Soldiers](#soldiers) | Enemy | `CHGrunt`; fire on the move | Idea 2026-09-18 |
| [The maddened](#the-cult-and-the-maddened) | Enemy | A melee human on the player's rig, Ivan's body; a loner grunt for security | **v1 built** 2026-09-19, verified; Ivan's body 2026-09-20, not yet |
| [Cultists](#the-cult-and-the-maddened) | Enemy, passive in ritual scenes | The maddened, plus a ritual spawn state | **Shaped** 2026-09-17 |
| [Cult leader](#the-cult-and-the-maddened) | Boss | A melee human | Idea |
| [Kingpin](#xen-hell-and-the-cut-monsters) | Boss | `valve/models/kingpin.mdl`; no AI | Idea |
| [Xen hell's residents](#xen-hell-and-the-cut-monsters) | Enemies | Half-Life's cut models; no AI | Idea |
| [The alien flyer](#the-alien-flyer) | Enemy, or a moving surface | Decay's `flyer.mdl`; one idle sequence, no AI | Idea 2026-09-20 |

This also answers the old question of what a boss "moveset" meant: all four bosses are an existing monster
given **custom attacks**, and two add something more (a dash, and turning into an ally). None is a puzzle.

### Panthereye

**Shape: Shaped 2026-09-13. v1 built 2026-09-18 and played by Andrei the same day** — "super solid"
mechanically, the animations "goofy", which is the menace direction below. Recorded in
[PILLARS pillar 2](PILLARS.md#2-enhanced-combat): the stalk as a straight path (run off screen, crawl
close or on screen), the spotted test, the claws, the pounce, the Predator profile, a den in `minemap`.
What v1's grill settled beyond the rules below: spotted is within 30° of the crosshair with a
clear line to the centre or head, held 0.25 s, no light term and no range limit; hurt, spotted, **or its
first slash** end the stalk; the alert sounds when it is spotted, not when it acquires; the growl is
`pa_idle3` pitched to 75; the pounce is 150–450 units, capped at 900, 3 s apart, with a 0.55 s wind-up;
v1 is always the blue body, and the red is kept for the alpha so the look never lies. Stats are the
legacy cvars as `sk_panthereye_*` (health 60/70/80, claw 13/15/20, leap 20/25/35), unjudged against a
map; the alpha has no numbers.

**Left for later slices, in no set order:**

- **Cover to cover.** v1 stalks in a straight line. The node search that prefers hidden nodes *closer* to
  the threat is still unwritten (below, "What the settled behaviour costs").
- **Feeding**, as a spawn keyvalue and a looping eat schedule woken by `Use`, **together with the carcass
  scent** (decided 2026-09-18: yes, the bullsquid's `bits_SOUND_CARCASS`/`MEAT`, but only once eating
  exists). Together they make an emergent feeding ambush — a Panthereye found over a houndeye it killed —
  and the placeable keyvalue is then just "start already eating". The Disturbance (v1, through Predator)
  stays the player's-kills-only lead it hunts; the scent is any corpse, which it eats. A feeding
  Panthereye wakes three ways: a map trigger, being hurt, or noticing the player through its own
  Suspicion — which comes free as long as feeding is its own schedule and not a `scripted_sequence`
  ([How the bullsquid feeds](#how-the-bullsquid-feeds)). `eat_idle` and `eat_turn` exist in the model
  for it, untagged, so the schedule plays them by name like the crawl.
- **The alpha**: the red body, set by a keyvalue with its stats, the way `item_suit`'s `variant` is; leaps
  from further out, a shorter `panther_leap_windup`, and may skip the stalk entirely and fight head-on
  from the start ("may" is how it was put; the likely shape rather than a decision).
- **The glowing-eye light** at the head, so it can be seen in the dark (below).
- ~~Freeze when watched~~ — **dropped 2026-09-23** in the menace grill: Circling is the answer to being
  Glimpsed, and a Panthereye frozen in place cannot also be circling.

#### The menace rework — grilled 2026-09-23, not built

Andrei's direction after playing v1 (2026-09-18): the animations read goofy, and the Panthereye has to
become menacing. Grilled 2026-09-23 in twelve questions. Valve's source (`Monster Models/Diablo/diablo.qc`,
[HL_SDK.md](HL_SDK.md)) has two controllers, `$controller 0 "Bip01 Spine" YR 90 -90` and
`$controller 1 "Bip01 Spine" ZR 0 50`. Both turn everything above the spine, and the front legs hang off
the neck (bones 21 and 28 parent to 19).

**Words.** The Panthereye's one-way switch is now **Revealed**, not "spotted", which is the readout state
pointing the other way (`m_bSpotted` becomes `m_bRevealed`). On screen but not yet Revealed is **Glimpsed**.
The movement is **Circling**, and the pounce by way of a wall is the **Wall Pounce** (CONTEXT.md).

**Circling, in both modes.** One movement with two sets of numbers, and it replaces the straight crawl as
the answer to "what does it do when it might be seen". **Freeze-when-watched is dropped**: a Panthereye
frozen in place cannot also be circling. Cover to cover stays its own later slice.

- **Stalking.** Off screen it runs, as in v1: that is what keeps it from being too slow ever to arrive.
  Glimpsed, it **spirals toward the player's back**: each waypoint a little further round toward their rear
  and a little closer in. Turning to follow it moves the back, so it keeps sliding round the edge of the
  screen; looking straight at it Reveals it within the dwell, so the dance is the player's to end. In the
  rear arc and off screen it drops the spiral and closes straight in to the claws.
- **The spiral is the controller's clamp** (settled after the grill, same day). Andrei clamps controller 0
  at 60°, so to keep the upper body on the player the heading may never be more than 60° off the line to
  them: every Circling path closes, and a constant-radius ring is impossible. Circling is therefore a
  logarithmic spiral at a constant `panther_circle_angle` (60°, tied to the clamp) off that line — each
  unit travelled closes 0.5 and goes round 0.87, and the radius shrinks ×0.16 per half-turn. From 600 u a
  spiral to the player's back arrives about 100 u behind them, near claw range, so the stalk's spiral ends
  at the back and the claws by its own geometry. Widening the controller in the QC flattens it toward a
  circle with no code change.
- **The stalking gait is the crawl, slowed.** `crawl_on_belly` at `panther_crawl_rate` (0.6 to start, about
  26 u/s). `MoveExecute` multiplies ground speed by `pev->framerate`, so the feet and body slow together.
- **Combat.** Revealed, it runs to the top of the pounce band and spirals in upright at run speed, the same
  60° spiral. It pounces when a random time between `panther_circle_min` and `panther_circle_max` runs out
  **or** when it reaches the band's floor, whichever is first: 450 → 150 u is 600 u of path, about 3 s at
  run speed and about 110° round the player, so the geometry caps the timer. The direction is random per
  engagement and reverses when blocked. *Rejected:* a true ring with the upper body lagging 30° short
  (loses the facing that sold the controller), and weaving out and back in (it turns its back).
- **Blocked.** A spiral waypoint that fails `CheckLocalMove` falls back to v1's straight crawl for that
  step. Circling needs no nodes; `topmap` has none.
- *Rejected:* circling only in the stalk (the combat rush would stay as it is) or only in combat (the
  stalk would stay goofy, the original complaint). A pounce triggered by the player's aim drifting off it is
  **parked, not rejected**: it only works once the Panthereye is fast enough that aiming can't hold it off
  at a distance.

**Controller 0 turns the upper body toward the player while it circles**: clamped at ±60°, at a capped turn
rate, and released during the claws, the leap and flinches so they play untwisted. The head only follows the shoulders; fixing the head itself
needs a controller on the head bone (v2).

**The Wall Pounce.** When the circling timer runs out, with chance `panther_wall_chance` (0.5) it pounces
by way of a wall instead of straight, so the leap arrives from a direction the player wasn't covering.
Combat only, so it never pounces unRevealed still holds.

- **A usable wall.** Traces sideways (both perpendiculars to the line to the player, and ±30° off each) out
  to `panther_wall_reach` (200 u). The hit counts if the surface is near vertical, a second trace at head
  height also hits (tall enough, not a crate), the contact point has a line to the player's eyes, and the
  rebound comes in at least 45° off the direct approach. None qualifies: the plain pounce.
- **Leap, cling, rebound.** It leaps at the wall, clings for `panther_wall_cling` (0.25 s: velocity zeroed,
  gravity held, yaw snapped to face the player), then leaps at the player's eyes; damage is `LeapTouch` as
  today. The cling is the beat that lets the player see where it will come from, and the slot the ideal
  version's second crouch drops into. **The cling is not interruptible** in this build.
- **The first build's pose is plain `crouch_to_jump`**, to prove the trajectory is fun before a modelling
  session is spent on it. *Rejected:* a Wall Pounce every time a wall qualifies (learned in two fights), and
  once per encounter (a Panthereye that runs out of tricks works against the menace).

**Build order.** (1) Andrei's HLMV check: controller 0 settled at a 60° clamp; controller 1 across 0–50
still to be read, for v2's lean. (2) The code, one slice each: the rename, the stalking
spiral and the slowed crawl, combat Circling, the upper-body turn, the Wall Pounce.

**Built 2026-09-23 and shelved, not verified in game** — all of (2) in one commit, `d17f217`, on the
branch **`panthereye-menace`**, because Andrei was away from the PC. `hl-shock` and the installed
`hl.dll` are still v1. To test: `git checkout panthereye-menace`, build `hldll.vcxproj`, then play with
`panther_debug 1` (its readout gains a line: direction, timer, turn, wall state). Merge into `hl-shock`
once it passes; the rows:

1. **The turn's direction.** Controller 0's range is reversed in the QC, so which way it turns had to be
   left to the eye: if the shoulders turn *away* from the player, `panther_turn_sign -1`, then fix the sign
   in code.
2. Glimpsed at the screen's edge, it crawls round toward the back; following it keeps it sliding round.
3. Revealed, it spirals in at a run and pounces after 1.5–3 s, or at about 200 u.
4. Near a wall, about every other pounce is leap, cling, rebound (`panther_wall_chance 1` forces it).
5. v1 intact: the growl, the claws, Revealed by being hurt.

Built beyond the grill, as calls made while writing it: the pounce's facing test is gone (the attack
schedule turns it first), and it turns at 180°/s once Revealed so the turn out of a 60° spiral is not a
pause. Expected to need tuning: the rebound off the wall reuses the pounce's aim and may come out flat
and fast; the slowed crawl may be too slow on a long glimpse. A v1 save loads with the Panthereye
unRevealed (the save field was renamed).

**v2, recorded so it isn't re-grilled:**

- **The ideal Wall Pounce**: `crouch_to_jump` at the wall, a controller-1 lean against it, a second
  `crouch_to_jump` off it into the pounce. Probably a second controller on another bone for the horizontal
  turn, a QC edit and recompile (Andrei's). The wall-kick pose is in [ART_DEBT.md](ART_DEBT.md).
- **Knocking it off the wall** by hurting it during the cling. Needs the Panthereye tilted back to upright
  as it falls.
- **Head fixation**: a new controller on the head bone.
- **The aim-triggered pounce**, once it is fast enough.

**Kin to the Gargantua — Andrei, 2026-09-18. Idea, not grilled.** The Panthereye reads as the Gargantua's
little cousin: the same skin texture, the same red eye. Proposed to make the kinship mechanical:

- **The Panthereye's body is immune to anything but energy**, the Gargantua's own rule (`GARG_DAMAGE`,
  `dlls/gargantua.cpp:47`, zeroing everything else in `TraceAttack`). The cost to weigh: the player of the
  first hour has the pickaxe, the crowbar and the Pulse, none of them energy, so a Panthereye met before
  the katana or the [energy rifle](#the-energy-rifle) is unkillable and becomes a thing to avoid — which may
  be exactly right for a stalker, but then where it is placed is the design. Whether the head or eye is an
  exception (the Gargantua pattern below, in small) would keep it killable early by precision.
- **The Gargantua's red eye becomes a weak spot** — hitting it does not necessarily kill, but enough damage
  to the eye **stuns** it. Two things to build: the eye as a hitgroup (`CGargantua::TraceAttack` has an
  `// UNDONE: Hit group specific damage?` in it, and its hitboxes are unread — `mdlinfo.py` first), and the
  stun itself, since Half-Life has no stagger ([Shared by all of them](#shared-by-all-of-them)). Whether
  the eye takes non-energy damage — the natural answer to "how does anyone without the katana fight one" —
  is the question that decides the rest. The eye glow already exists as a sprite on attachment 1
  (`m_pEyeGlow`), which could dim while stunned.

**The model is Half-Life legacy content**, a monster Valve cut, not another mod's art. v1 took it from
*Half-Life: Extended* (`Half-Life/hl_extended`, per its `liblist.gam`): `models/panthereye.mdl` and ten
sounds, copied into the repo's `models/` and `sound/panthereye/`. Valve's own source, `Diablo`, was found
in the SDK afterwards ([HL_SDK.md](HL_SDK.md)), so any model change is a recompile rather than a
decompile.

**Its AI in HL: Extended is compiled only** (`hlex.dll` has `CPanthereye` and `CCinePanther`, no source),
so the mod's AI was written new. The strings showed what theirs did, and they set v1's numbers:

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
- **Its eyes are meant to glow.** Each of the four face textures has a `_Light` twin flagged additive and
  fullbright (`0x24`). The engine honours additive and ignores fullbright, and additive alone does
  **not** show in the dark (progression pickups, 2026-09-14) — so as shipped the eyes are drawn as light
  by day and go dark with the room. If a Panthereye is to be seen in a room the player cannot see into,
  which is a strong stealth fact worth having, it needs a small dynamic light at the head, the way the
  progression pickups get theirs (`cl_dll/entity.cpp`).
- **Two body models, red and blue.** Body 0 is red (`Panther_Bodyfull`), body 1 is blue
  (`Panther_Bodyfull_Blue`). The FGD calls them Diablo and Nightkin. Both use the same red glowing eyes.
- **The four skins are eyelid states, not variants.** Skin 0's eye is open, 1 half-shut, 2 narrowed,
  3 shut and dark. That is enough to blink, and a shut eye is an eye that does not glow.
- 49 bones including a tail, no attachments, 40 hitboxes.
- Sounds: two alert, one attack, one death, four idle, two pain.

#### The rules — settled 2026-09-13, and what v1 built them as

- **It stalks.** Once it knows about the player it closes in, crawling on its belly toward them, and
  growls quietly the whole time. The growl is the player's warning. It is the monster side of pillar 6:
  here the player is the one being hunted. The stalk starts when it acquires the player — `Look`, gated by
  Suspicion, gives it an enemy exactly as it does every other monster, so stalking is simply what it does
  with an enemy it has not been spotted by, and no map trigger is needed (one stays a later option). v1
  stalks in a straight line; cover to cover is a later slice, because `CBaseMonster::FindCover` finds a
  node the threat cannot see but never prefers one *closer* to the threat, and that is a new search on the
  same node graph.
- **Close up, it slashes.** In claw range it attacks whether or not it has been spotted.
- **Spotted, it rushes and pounces.** When the player sees it, it stops hiding, runs at them and leaps from
  moderate range. **It never pounces unspotted**: a leap the player could not have seen coming is not fun,
  so the pounce is only ever the answer to being seen; being attacked out of nowhere is limited to the
  slash, and the growl warns before that.
- **"Spotted" is the reverse of Suspicion**: the Panthereye asking whether *the player* can see *it*, a
  test this mod wrote (the numbers are in the header above). **Spotted is permanent** — the encounter has
  two modes, stalking and combat, and moves one way between them; looking away does not send it back.
  **Hurting it counts as being spotted.** Because spotted is one-way, a wrong answer from the test is
  permanent too, so the test leans strict: firing at the very edge of the screen costs the whole stalk.
- **Alone.** A `CBaseMonster`, not a `CSquadMonster`; the FGD's `SquadLeader` flag is dropped.
- **The crawl is picked by name.** `crawl_on_belly` carries no activity tag, so the base AI would never
  choose it; the Panthereye selects it, with `crouch_to_crawl` and `walk_to_stand_*` as the transitions,
  and feeding will be picked the same way. The growl is a cue for the player at low volume and short
  range and never enters `CSoundEnt`.
- **The red one is the alpha**, a keyvalue rather than a second classname, the body picking the stats.
  Both bodies have the same red eyes, so the eyes cannot tell them apart; the blue body is the normal one.
  Not in v1, above.
- **Both attacks are `DMG_SLASH`**, which is on the Shield's list
  ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)), so the Pulse counters them with no
  extra work. The leap is the headcrab's pattern: an anim event sets a velocity and a touch function deals
  the damage. Its Perception Profile is Predator, and it can be backstabbed.

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

**The Panthereye will also hunt for food like the bullsquid** — decided 2026-09-18, once eating exists
(the feeding bullet under *Left for later slices*). A trail of bodies brings the stalker to the player. It
overlaps the **Disturbance** ([PERCEPTION.md](PERCEPTION.md#death-witnesses-and-the-disturbance--settled-and-built-2026-09-17-untested-in-game)), and
the reconciliation is: the Disturbance is the player's-kills-only lead it hunts, the scent is any corpse,
which it eats.

### Melee alien grunt

**Shape: Shaped 2026-09-13. The first version is decided and needs no new art; the charge and the weapon
wait for model work. v1 built 2026-09-18 overnight, verified in game the same day** ("a very good base, it
will be extended and refined later") — `monster_alien_grunt_melee`,
recorded in [PILLARS pillar 2](PILLARS.md#2-enhanced-combat). The "classname or keyvalue?" question below is
answered: a classname, because alien military recruit across classnames, so mixed squads form either way,
and a classname shows by name in the editor and in `debug_schedule`. The deflected-punch cosmetic fix went
in with it, for every alien grunt. What is left: the charge, the chainsaw, an unarmoured body, and the
shielded variant.

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
  shoves. The stock punch already behaves like this: `CheckTraceHullAttack` returns the entity it hit
  whether or not the damage was taken, and the punch applies its shove to whatever comes back. Since
  2026-09-20 it also reports whether the blow landed (`pbLanded`), and the hit sound and blood are gated
  on that for every melee monster ([TECH_DEBT.md](TECH_DEBT.md), the deflected-melee entry, resolved).
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

- **The chainsaw's design**, from the ground up, and the charge animation. Both wait on model work.
- **The unarmoured body**, so the melee variant looks like what it is.
- **The shielded variant**, above, would be a third classname beside `monster_alien_grunt` and
  `monster_alien_grunt_melee`.

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
reward for the Skill or a hole in the design; `pulse_discharge_melee 0` exists to compare. **Answered
2026-09-23, not built:** the Discharge becomes innate and fires only off slave beams
([ADR-0016](adr/0016-the-discharge-is-innate-and-answers-only-slave-beams.md)), so a deflected leap vents
nothing and the swing is the only answer.

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

**Shape: Shaped 2026-09-23, in a grill.** Nothing built. A special alien slave under the Nihilanth's control,
who ends wing one. The fight tests the Pulse: guns carry most of it, and one signature attack, the
**Overcharge**, only a deflect answers. Freed, he is the [friendly alien slave](#friendly-alien-slave): one
character, in his lab, who helps the player through unlockables and information, never by fighting. Killed,
he is gone, and the alien Module drops from his body. **Freeing him is the fight's natural end; killing him
is a route a player has to pursue.** The vocabulary (Ward, Overcharge, Bindings, Stagger, Barrier) is in
[CONTEXT.md](../CONTEXT.md).

#### Settled before the grill

- **Two entities. 2026-09-13.** The boss and the lab slave are separate. Freed, the boss leaves and sets a
  global state (`env_global`), and the lab slave is present only once that state is on.
- **The collar and the bracelets are the Nihilanth's control. 2026-09-13.** They already show it on the stock
  model, and `collar1` and `collar2` tug at the collar. They are the three **Bindings** below.
- **The Module he gives is the alien Module** (2026-09-13, confirmed 2026-09-17): a platform for
  Core-powered alien weapons, whose first weapon summons ghost slaves, designed under the
  [Alien Route](#alien). Until his fight exists, `item_alienmodule` stands in for the hand-over.
- **Placed 2026-09-17: he ends wing one**, in the mine levels, having come through the tear at its deep
  end. Early, because everything downstream needs him: the hub's vortigaunt, the hand-over machine, the
  second half's advisor. The Pulse is found in the first minutes, so by his fight it may be assumed; after the
  teleport he and the surviving staff advise on the pieces.

#### The fight — settled 2026-09-23

**He fights alone, and he is mortal.** Allies are a later refinement, below.

**The Ward.** A green glow round his body (`kRenderFxGlowShell`) that is his protection. **All damage goes to
the Ward first, melee included**, and while it stands his health is untouched. **A Discharge does nothing to
him while the Ward is up** — no damage, no Stagger, no bounce; it is simply absorbed.

**The Overcharge is his attack**, part of each phase's rotation like any other, not a cue the Ward's break
sets off. It is the stock zap made bigger: a longer channel, more beams, and bolts at a multiple of the zap's
damage. **Only a deflected Overcharge advances the fight**, so it is in the rotation of every phase.

**The Ward down stays down until his next Overcharge resolves** — deflected, dodged or taken — and then
comes back up. So a break is always worth exactly one chance, and the lethal route below is still bounded by
one window of shooting per Overcharge. A fixed timer was rejected: a break with no Overcharge inside it
wastes ammunition in a fight tuned to be short of it.

**A Discharge off an Overcharge, with the Ward down, breaks one Binding** — left bracelet, right bracelet,
then the collar — **and deals no damage to his health.** It Staggers him long. A Discharge off his ordinary
zaps, with the Ward down, Staggers him and hurts like any Discharge. An Overcharge that is not deflected
breaks nothing. Because the Discharge goes to the crosshair, a panicked deflect while looking away wastes the
Overcharge; the Ward comes back and the loop runs again, so that is a cost, never a lock.

**The Bindings drive the escalation**, which the player can see, rather than a hidden health threshold:

| Bindings broken | His attacks |
| --- | --- |
| 0 | The stock zap and claws, and the Overcharge |
| 1 (left bracelet) | **The volley** replaces the stock zap: three or four quick zaps with a rhythm, each its own deflect; the short Recharge after a deflect is what makes the next one possible |
| 2 (right bracelet) | **The Overcharge winds up faster**, a tighter read on the one that frees him |

His claws stay his answer at close range throughout. He **never flees**: the stock slave's flee at low health
(`dlls/islave.cpp:723`, `health < 20`) is removed on him.

**Freed.** The third Binding breaks: the biggest effect of the fight at the collar, `env_shake`, the green
flash the Discharge uses, and the Ward goes out for good. Then **the collar animation** (`collar1` or
`collar2`, Andrei checking them in HLMV first), then he beams out — the Xen teleport sprite and sound, and he
is removed — and his target fires, which the map hangs the `env_global` and anything else on. The ending is
the collar animation for now; more animations chained after it, or one long sequence compiled from existing
ones, wait until it is seen.

**Killed.** Only by out-shooting him through the Ward's windows. His health is sized so that a player who
deflects every Overcharge always frees him first, and the arena's ammunition so that a player who refuses the
Pulse — breaking line of sight from each Overcharge, which is hitscan, and shooting through window after
window — has **barely enough** for the kill. He dies with his Bindings on. **The alien Module drops from his
body**, so the Alien Route stays open on both paths; what is lost is the character — the lab, the hand-overs,
the English, his advice, which the surviving staff carry alone. A global state records it (`slave_killed`,
beside the freed one) for later maps to acknowledge; nothing reads it yet. A kill window after the freeing,
the slave kneeling, was proposed and dropped: one lethal route, and it is a fight, not an execution.

#### What the Pulse gives up for him — settled 2026-09-23

**The Discharge becomes innate to the Pulse, and fires only off slave beams.** Recorded as
[ADR-0016](adr/0016-the-discharge-is-innate-and-answers-only-slave-beams.md), which amends ADR-0006. Every
player with the Pulse vents a deflected slave zap to the crosshair; every other negated hit — melee, the
Shield's other damage types — is only negated. The `PulseDischarge` Skill (id 16) is **retired**: its id is
never reused, and its node on the board needs a new occupant. In the fiction, the suit's mining shield
happens to answer vortigaunt energy. **A deflected beam flashes the screen green**, in place of the suit
colour's tint (`hud_pulse_tint`) for that one event.

#### The Barrier — settled 2026-09-23

The fight is taught before it. **Barriers** are brush entities of the Shield's own tech, mine safety equipment
on a power source, so they stand for as long as they are powered. The teaching scene is a slave zapping the
player through one, and Staggering himself instead.

- **Toggleable**: on until something turns it off (a button, a breaker, any trigger), with a start-off flag so
  a map can power one *up*. `func_wall_toggle` (`dlls/bmodels.cpp`) already switches a brush's solidity and
  visibility on each use; `func_barrier` is that plus the zap. Puzzles fall out: cut the power to pass,
  restore it to shelter, lure a slave into zapping through it.
- **Blocks everything**, both ways. A grunt's bullets just stop. **Slave beams go back to the slave who fired
  them**, with a Stagger — the one place a beam returns to its source, because a Barrier has no crosshair.
- **Its look is Andrei's texture**, painted into `topmod.wad` and drawn additive; a placeholder ships first, see
  [ART_DEBT.md](ART_DEBT.md).

#### What the base game already gives

Read from `dlls/islave.cpp` and the model's events on 2026-09-23:

- **The stock zap's wind-up** is `zapattack1`: the `ZAP_POWERUP` event four times (frames 0, 4, 10, 15 at
  15 fps), each adding an arm beam per hand — up to 8, the cap `ISLAVE_MAX_BEAMS` (`:39`) — raising
  `zap4.wav`'s pitch and brightening the beams (`BeamGlow`, `:815`). `ZAP_SHOOT` at frame 24 (about 1.6 s;
  1.07 s on Hard, where the framerate is ×1.5) clears them and fires **two bolts, one per hand**
  (`:486-487`), each a hitscan trace dealing `sk_islave_dmg_zap` as `DMG_SHOCK` (`:891`).
- **The beams come from attachments 1 and 2, the hands** (`:802`), which is where the bracelets are: effects
  at a broken bracelet cost nothing. What the collar has is to be checked in HLMV.
- **The revive** (`m_hDead`, `:459-480`): a slave zaps a dead slave and a new one spawns in its place. Unused
  while he fights alone.
- **Chained `scripted_sequence`s** play one after another through their `target`s, and the client blends each
  change of sequence over 0.2 s (`cl_dll/StudioModelRenderer.cpp:864-907`).

#### Traps

- **Arm beams need walls.** `ArmBeam` draws only if geometry is within 512 units of the hand (`:783-793`); in
  an open arena the stock wind-up shows nothing. The Overcharge's beams have to be his own — to the floor, or
  round his body.
- **He is immune to `DMG_SHOCK`** (`:585`); the Discharge is `DMG_ENERGYBEAM` for that reason. A Stagger is
  therefore not a side effect of damage and has to be forced.
- **The gap in a chain**: between two `scripted_sequence`s the monster is handed back to its AI. Whether that
  shows as a frame of idle or a turn is to be measured, not asserted.
- **Melee needs no ammunition**, so the ammunition bound on the lethal route is soft for a melee player.
  Accepted.

#### What is code and what is map

The code: the boss (a `CISlave` subclass) with the Ward's pool and glow, the Overcharge and the volley, the
Binding counter and its effects, the escalation, no flee, and firing a target on the third break; the forced
Stagger on any slave; the innate Discharge on beams only, the green flash, and the Skill's retirement;
`func_barrier`. The map: the arena and its ammunition, the ending's chain, the two global states, and every
Barrier.

#### Open

- **How do the lab slave's items arrive?** Handed straight into the Inventory, where a full Grid refuses them,
  or left in a Box, which is not built yet.
- **Does a Barrier's returned beam also hurt the slave**, or only Stagger him? The teaching scene needs only
  the Stagger.
- **The retired Discharge node's replacement** on the board ([SKILL_MAP.md](SKILL_MAP.md),
  [SKILL_TREE.md](SKILL_TREE.md)), and the two cross-Route links that scale the Discharge (Energy Damage,
  Weapon Mastery), which still apply to the innate one.
- **The arena**, in wing one's map, and its ammunition.
- **Every number**: the Ward's pool, the Overcharge's channel and damage, the Stagger's length, his health
  against the strongest wing-one weapon over three windows — if a well-armed player kills him by accident
  while deflecting, the pool is too small.

#### Later

- **Allies**, or **ghost summons** — the alien Module's first weapon, used on the player before it is handed
  to them, which introduces the Module.
- **Binding bodygroups** in place of the effects, and the lab slave's model without the hardware, from one
  decompile; see [ART_DEBT.md](ART_DEBT.md).

### The assassin boss

**Shape: Idea.** A human assassin with a dash and custom attacks.

`CHAssassin` already jumps (`ASSASSIN_AE_JUMP` sets a velocity, `dlls/hassassin.cpp:260`, with a 3-second
cooldown), cloaks by fading `renderamt` down to 20 when not attacking (`RunAI`, `:693`), and throws
grenades. A dash is a horizontal jump on the same event pattern.

**"Dash" is already a Module's name.** Either the assassin's move gets a different word, or the fight is
where the player first sees the Dash, and possibly where they win it.

**Placed 2026-09-17: the military's specialist, at the end of the military wing** — the stealth systems'
hardest test, ending in a fight with someone better at it than the player. The Dash Module being won
here is proposed, not confirmed.

**Its Perception Profile becomes Predator when the boss is built** (decided 2026-09-18, when the Predator
profile was made for the [Panthereye](#panthereye)): fill ×2.0 and drain ×0.25 where Trained is ×1.5 and
×0.5. Not before: the assassin is one of the four primaries 5f's numbers were checked against, and now
that [STEALTH_CHECKLIST.md](STEALTH_CHECKLIST.md) has its results (2026-09-18) nothing holds the change
but the boss itself. Today it is still Trained.

### The alien grunt boss

**Shape: Idea.** An alien grunt with custom attacks. Nothing beyond that is recorded. It shares a base with
the [melee alien grunt](#melee-alien-grunt), so whichever is built second gets the other's groundwork.
**Placed 2026-09-17: ends a Xen wing and guards a piece** — the collective's soldier caste at its peak,
and the test of whatever build the player has made.

### The Nihilanth

**Shape: Shaped 2026-09-17 — a pattern fight, and the sphere puzzle is gone.** Custom attacks and a
revamped model.

**`CNihilanth` is not schedule AI.** It runs on think functions (`HuntThink`, `Flight`, `NextActivity`),
with energy spheres it absorbs and throws (`CNihilanthHVR`), and it is a puzzle boss: the spheres protect
it (`AbsorbSphere`, `dlls/nihilanth.cpp:984`). New attacks go into `NextActivity`, not into schedules.

**A new model must keep the old sequence names**, or the code changes with it. It picks animations by name
(`float`, `walk_r/l/u/d`, `recharge`, `attack1_open`, `attack1`, `attack2`, `die1`,
`dlls/nihilanth.cpp:469-839`).

**The fight — shaped 2026-09-17.** Neither the sphere puzzle kept nor a straight fight like the other
three. It was asked for as a bullet hell. A first-person player sees a quarter
of the sky and the engine's projectiles are full networked entities against a 900-edict default, so it is
**patterns, not swarms**: a small vocabulary of readable shapes, each telegraphed (a flash, then the
discharge), each with an answer from the base kit. Skills make an answer forgiving and are never needed.

| Pattern | Answer | Note |
| --- | --- | --- |
| A wall with a gap, sweeping the arena | Dash through the gap; Phase through the wall | Drawn on the client around one server projectile, which is how the katana's wave already works |
| Ground marks that detonate | Leave; double jump the shockwave | |
| Slow sweeping beams | Break line of sight behind wreckage; dash across | They track where the player *was* |
| Homing spheres, few | A timed Pulse | Vanilla's `CNihilanthHVR`, kept as a deflectable projectile. Rehearses the finisher |

It is a **siege of the hub**, not an arena he waits in: he arrives because the player routed
[the reactor's](#the-shape-of-the-game) power to the rig. Three phases. **The ships**: he is distant,
firing walls and beams, and alien ships heal him with crystals; the player crosses the torn-up facility
and cuts them down — Air Dash and Reprisal chains for a Shinobi, the katana's wave at range for anyone
else. **Direct**: marks and spheres join, and he can be hurt. **The scripted ending**: the player is
pushed away, vortigaunts teleport in and fire, time freezes, and the one live input is the Pulse key — the
volley vents as one beam at the crosshair. That is the Pulse redirect the [Alien Route](#alien) moved out
of the loop to "one scripted set piece at the end of the game", here.

**The freeze is staging, not the mechanic this roadmap rejected twice.** The vortigaunts hold a charged
zap (they already charge visibly), projectiles have their velocity zeroed, the sound drops out, the screen
may tint. Nothing scales time.

After it he gives the backstory, in flashbacks. What follows is one of several endings, deliberately the
last thing to be designed.

**The finisher as a choice — Andrei, 2026-09-18. Idea, open to refine or dismantle.** In the scripted
ending the friendly vortigaunts fire *at the player*, and the Pulse reflects every attack into the
Nihilanth. Doing it grants one ending; **not deflecting** grants another, in which the Nihilanth is not
dealt a killing blow. That turns the one live input of the freeze into the first ending decision, which
the rule in [The rules](#the-rules) wants: a moment that might matter sets a named `env_global`. Questions
it raises: what *not deflecting* costs the player in the moment (the volley lands on them — is it lethal,
survivable, or stopped short), how the game tells a choice from a missed timing, and whether the
vortigaunts firing on the player reads as betrayal or as trust. Nothing waits on it; the endings are
still [the last thing designed](#open-questions).

### The cult and the maddened

**Shape: Shaped 2026-09-17.** The human enemies that are not soldiers. Both come from one fact: the
Nihilanth's reach through the crystals stops at **planting ideas**, so nobody is a puppet. These are staff
who spent years around crystal, heard the voice, and did something with it.

- **The maddened** are miners with pickaxes and some security members. A maddened miner is a melee human,
  **backstabbable**, slow to notice and lethal up close: the fair melee enemy of the first hour and the
  stealth tutorial before the soldiers arrive. A maddened security member is a grunt that spawns as a
  loner, which 5f already treats differently (the mob rule); not built.

  **The maddened miner: grilled 2026-09-19 evening, built and verified in game the same night ("it
  works"), given Ivan's body on 2026-09-20.** Recorded in [PILLARS pillar 2](PILLARS.md#2-enhanced-combat)
  and in depth in [MADDENED.md](MADDENED.md), which ends with the list for the grill that follows. What
  was built: `monster_maddened` (`dlls/maddened.cpp`), `models/maddened.mdl` from
  `E:\CustomAssets\scripts\maddened_build.py`, `CLASS_MADDENED` as row and column 15 of a relationship
  table grown to 16 (14 is the vehicle class, which Valve defined and never gave a row),
  `sk_maddened_health` 50/60/70 and `sk_maddened_dmg_swing` 10/15/20, the FGD entry, and shaft1's vein
  holds him instead of the zombie. His stand-ins are in
  [ART_DEBT.md](ART_DEBT.md#the-maddened-miner--ivan-in-blue-a-black-crowbar-no-flinch). The decisions:
  1. **Body: Ivan, on the player's rig from the SDK sources** (`Player Models/`, [HL_SDK.md](HL_SDK.md)),
     built by a script into `models/maddened.mdl`. Ivan is Half-Life's original protagonist as the 25th
     anniversary shipped him, decompiled by Andrei — "the look of a madman" in a bulkier suit of his own,
     washed to the work blue. **One body, no variants**: there is no suited keyvalue, so the one at
     shaft1's vein is not a choice. The weapon is a bodygroup (the pick: `reference_crowbar`, the crowbar
     mesh already skinned to the hand bone, in the pickaxe's black). The origin drops from the player's 36
     to 0. The swing is the player's crowbar swing, one pitch-blended sequence held level as Valve's QC
     has it, with a damage event on the frame it lands. Rejected: the zombie's rig (attacks exist, the
     mesh is a corpse, the miner would be hand modelling from minute one); the scientist's (a hundred
     animations of a man at work, no attack, no source, a Bip02 nothing else shares); and the two bodies
     that stood in first — the player's own with Gordon's face, verified in game, then the sealed-helmet
     suit and the blue-coated scientist from the SDK's deathmatch roster, because a face and clothes for
     an unsuited man are hand modelling Andrei cannot do yet. That roster stays the pool for later bodies
     (Barney's for the maddened security member). Ivan's body is not yet verified in game.
  2. **He walks unaware and runs when he chases.** The grill said walk always, the run being the cult's
     later, so that the first fight in the game would not be a sprint in the dark; the first build ran by
     accident and it worked, and the corrected walking one "looks like no threat" (Andrei). Reversed on
     play the same night.
  3. **Health 60, swing 15, reach 64, about 1.2 s a swing**, as skill cvars: three swings from the front,
     two with a Melee Damage node, one Backstab.
  4. **A relationship class of his own, everyone's enemy**, the player's, the soldiers', Xen's, allied with
     his own kind, which the cult will share. Rejected: Xen leaving the touched alone, which contradicts
     the rule that the voice plants ideas and controls nothing.
  5. **The default Perception Profile for now**; no Disturbances; `CanBackstab` true. A slower profile
     of his own (fill 0.75, drain 1.5) was proposed and deferred to when the cult needs faster ones.
  6. **Sounds, all stand-ins:** the road's whisper (the slave's words) as his idle every 4–8 s, so what the
     player heard down the tunnel is him and it is his tell in the dark; the crowbar's miss and body hit
     pitched down; Barney's pain and death.

  **Debt it created, in ART_DEBT:** one face for all of them; the black crowbar as the pick until the
  hand-made head exists; no flinch, since the player never flinches; a planted swing, since it is an
  upper-body animation; the voice.
- **The cult** is the organised form, with a leader. **Hostile everywhere, with one exception: ritual
  scenes**, where cultists stay passive until the player is noticed or crosses a map trigger.
- **The ritual has to be the cultist's own state**, not a `scripted_sequence`: Suspicion is frozen while a
  script holds a monster ([PERCEPTION.md](PERCEPTION.md), the `m_pCine` row), so a scripted ritual could
  never notice anyone. A keyvalue the monster spawns with, a looping schedule, woken by `Use` from the
  trigger — the [feeding Panthereye's](#how-the-bullsquid-feeds) answer, and worth building once for both.
- **The leader is a boss**, from the deep shifts, met once in a scripted scene before the fight: a
  monologue, then pickaxe against pickaxe. **The corrupted director** is the leader's highest-placed
  follower and **is not a fight** — found at the rig's console, dead or still listening.
- A true neutral faction (walk among them until provoked) was rejected: relationships are a static class
  table, neutral-until-provoked exists only as Barney's per-monster hack, and a faction-wide version is a
  second social layer on top of an untested 5f. One ritual room gives the feeling at a fraction of the cost.

**How a cultist looks — Andrei, 2026-09-23. Direction, not grilled in full.** The maddened is a miner who
broke; a cultist is a miner who joined something, so what marks him is belonging, not madness. One class:
a cultist is the maddened plus a keyvalue that sets his skin and bodygroup, plus the ritual state — which
answers MADDENED.md's "spawnflag or a class of its own".

- **The sign.** One glyph, drawn once by Andrei, used everywhere: a **decal on the tunnel walls**, met
  before the first cultist so the player learns the sign before the people; **on the suit's chest plate**,
  in place of Ivan's RESEARCH plate, as a skin family (texture only, the same pipeline as the
  [face variants](MADDENED.md)); and in the margins of cult Records. Unchecked: whether a custom decal
  means the mod ships a whole replacement `decals.wad`.
- **Crystal, worn**, as submodels (a bodygroup): a shard on a cord or taped over the lamp, in the
  deposits' purple or the Core's green, additive so it reads as glowing. The voice comes through crystal
  and they carry it. It also shows them in the dark before they see the player — the stealth pillar, and
  the Panthereye's opposite — and may drop a Crystal Shard. A modelling rung for Andrei after the beard.
- **The ritual pose.** The members kneel on the player rig's **crouch idle** to begin with, facing the
  crystal, backs turned; expected to need tuning. The chant is the mutter in unison. **The leader gets
  something else**, not chosen yet.
- *Rejected:* **hoods** — cheap, the most generic cult image there is, and they would hide the faces being
  varied. Dropped outright, not kept for rank.

**For a later grill — Andrei, 2026-09-23.** Two more variant axes, on cultists as well as the maddened:

- **Beard shapes.** Ivan's beard is geometry, a wedge down the neck, so a short beard, stubble or a shaved
  jaw is a head modelled by Andrei, shipped as a head bodygroup with its own hair mask so the colour skins
  apply to it too. To settle: which shapes, how many, and whether shape says anything (the cult's own
  grooming, a shaved head as initiation) or is only variety.
- **Helmets.** A miner's helmet as a bodygroup, some wearing it and some not. It meets the worn crystal
  above — a shard taped over the helmet's lamp is one of its two proposed places. To settle: whether the
  helmet marks miner against cultist, or rank, or nothing; whether its lamp is lit (a real light is a stealth
  fact, as the glowing shard is); whether it comes off the head, which the geometry of Ivan's hair decides;
  and whether it is modelled by Andrei or taken from an existing model (the SDK and Decay folders first,
  [HL_SDK.md](HL_SDK.md)).
- **A crystal trinket the player wears, to pass among cultists — Andrei, 2026-09-23. Idea.** The worn
  crystal is the cult's mark, so wearing one is a disguise. It runs into a rejection above: a true neutral
  faction was turned down because relationships are a static class table and neutral-until-provoked
  exists only as Barney's hack. The way round it is perception, not relationships: while the trinket is
  worn, cultists' Suspicion of the player fills slower or only at close range, through the same
  Concealment and Profile terms as everything else ([PERCEPTION.md](PERCEPTION.md)); once one notices,
  he is as hostile as ever. To settle: what breaks the disguise (drawn weapon, running, a body found, a
  kill witnessed, standing too close too long), whether the maddened are fooled too or only the organised
  cult, where it is worn (a Module Slot on the Status doll, or an inventory item), where it is found (off
  a cultist, which the Crystal Shard drop already half-builds), and whether it shows on the player at all.
- **How the axes combine.** Head shape × helmet × skin colour × crystal is many bodies from few parts;
  GoldSrc packs every bodygroup into `pev->body`, so which combinations a mapper sets and which are rolled
  at spawn is part of the same question.

### Soldiers

**Shape: Idea, from Andrei 2026-09-18. Not grilled.** Soldiers are the bulk of the human enemies and
[the arsenal](#the-rules), and they fight as vanilla's grunts do: stop, then shoot. Proposed:

- **They move while shooting**, less accurate while they do.
- **Stationary fire stays**, more accurate, and its burst may grow from 3 to 5.
- **The soldier decides which** — support fire on the move, or a still, accurate burst — and may still run
  to cover without shooting when cover is the bigger priority.

What it meets in the code:

- **The model has a move-and-shoot sequence, and it is not wired up** (Andrei, 2026-09-18, confirmed
  from Valve's source in the SDK, [HL_SDK.md](HL_SDK.md)). `strafeleft` and
  `straferight` are made from animations named `strafefire_l2` and `strafefire_r2`. The weapon visibly
  fires in them, they are tagged `ACT_STRAFE_LEFT`/`_RIGHT`, and they carry **no fire events**, where
  `standing_mp5` fires at frames 10, 12 and 14. Two ways to use them: add the events in the QC and recompile
  from the SDK source, or fire from code on a timer while the strafe plays. They only cover sideways
  movement, facing the enemy. A soldier advancing or retreating while firing needs more.
- **For any direction, split the body** — Andrei's second proposal was a new bone controller at the torso,
  with the firing animation on the upper body and the movement on the lower. A bone controller only turns a
  bone by a value, so it cannot carry an animation. But **the split exists already, for the player**:
  `CStudioModelRenderer` (`cl_dll/StudioModelRenderer.cpp:928-946`) plays the player's
  `gaitsequence` on the bones below `Bip01 Spine` and the main sequence above it. That is how a player
  model runs and shoots at once. Giving a monster the same split is a client renderer change plus a way to
  network a second sequence for a non-player entity. Whether that field arrives, and at what
  precision, is decided by the entity's encoder in `delta.lst`. It is also the general answer to "move while doing anything" for
  every monster, not only soldiers. The grunt's skeleton is a Biped too.
- **The burst is the animation.** Three fire events in the sequence (`HGRUNT_AE_BURST1`–`3`,
  `dlls/hgrunt.cpp:80`), and `GRUNT_CLIP_SIZE` 36 carries the note "3 round burst sound, so keep as
  3 * x". Five rounds means new events in the model or a loop in code, a new burst sound, and a clip size
  that is a multiple of five.
- **Accuracy is one cone**, `VECTOR_CONE_10DEGREES` in `CHGrunt::Shoot` (`dlls/hgrunt.cpp:855`). A moving
  cone is a branch there.
- **"Decides" is the schedule selection** in `CHGrunt::GetSchedule`, which today chooses between cover,
  suppress and attack; moving fire is a new schedule and a new task beside `SCHED_RANGE_ATTACK1`.
- **It feeds [the Pulse against sustained fire](#the-pulse-against-sustained-fire)**: more fire on the move is more bullets the Pulse is
  asked to answer.

### Xen hell and the cut monsters

**Shape: Idea.** The bottom of Xen, where the collective keeps what it has used up, populated by
Half-Life's cut monsters. On this machine, in `valve/models/`: `friendly.mdl` (Mr. Friendly),
`kingpin.mdl`, `stukabat.mdl`, `snapbug.mdl`, `archer.mdl`, `protozoa.mdl`, `boid.mdl`, `chumtoad.mdl`;
`panthereye.mdl` in `hl_extended`. **Valve's sources for most of them are in the Half-Life SDK** —
Archer, Bigrat, Floater, Gasbag, Kingpin, Mr. Friendly, Snapbug, Stukabat, the chumtoad, and the
Panthereye as `Diablo` — with QCs, so a change is a recompile, not a decompile ([HL_SDK.md](HL_SDK.md)).
**All are models without AI**, so each is the
[Panthereye's](#panthereye) cost again: read the sequences with `utils/mdltool/mdlinfo.py`, then write the
monster. **Kingpin is its boss** and guards a piece. Stukabats and any other flyer matter beyond Xen hell:
see [aerial melee](#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate) and
[the alien flyer](#the-alien-flyer).

### The alien flyer

**Shape: Idea, from Andrei 2026-09-20. Not grilled.** A large organic craft in the Xen sky. It starts from
the observation that vanilla's Xen flyers are brushwork on a track and therefore invulnerable — a thing that
crosses the view and cannot be answered. **The mod's version is a model, and it can be shot down.** Nothing
below the model facts is decided.

**The asset already exists, compiled.** `flyer.mdl` and `flyer_gibs.mdl` in Half-Life: Decay's folder on
this machine, `D:\GameLibrary\steam\steamapps\common\Half-Life\decay\models\` ([HL_SDK.md](HL_SDK.md#half-life-decay)).
Read with `utils/mdltool/mdlinfo.py` and a one-off header reader, 2026-09-20:

- **Seven bones, three chains off one root.** Bones 3–4 and 5–6 are mirror images of each other in their
  hitboxes — two two-segment wings. Bones 1–2 are the third chain, a head or a tail. 73 vertices, two
  meshes, one bodypart. It is a manta.
- **Two textures, `FLYER_TOP.BMP` and `FLYER_BOTTOM.BMP`, 128×64 each.** It was built to be read from above
  *and* below, which is a flying thing the player is meant to pass under.
- **One sequence, `idle1`**, 30 fps, 31 frames, activity 0. No walk, no turn, no attack, no death, no
  flinch. **Everything except the flap has to be authored**, which is the [Panthereye's](#panthereye) cost
  again and then some — the Panthereye at least had a full set.
- **It is large.** The reference pose spans 164 × 224 × 78 units; `idle1`'s own bounding box spans
  441 × 356 × 111, so the flap travels a long way. A player is 32 × 32 × 72.
- **Header flags `0x300`** = `STUDIO_DYNAMIC_LIGHT | STUDIO_TRACE_HITBOX`. Bullets already trace against its
  seven hitboxes rather than its bounding box, so **per-wing damage is free** — shooting a wing off is a
  question of code, not of the model.
- **`flyer_gibs.mdl` is four distinct pieces** (`flyer_gib_01`–`_04`, each at two scales; the largest is 174
  units across). **The thing was built to be destroyed.** That is the strongest evidence for what it is for.
- **Candidate sound:** `decay/sound/ambience/alienflyby2.wav`. Unverified as its own.

**How Decay used it, which is a shape and not a plan.** The 2007 community PC port's `decay.fgd` declares
`monster_alienflyer` on `models/flyer.mdl`: base `Monster` plus `RenderFields`, a **`death_target`**
keyvalue, a **Start Inactive** spawnflag, and a collision hull of `-32 -32 -32` to `32 32 32` — a 64-unit
cube under a model three times that wide. It rides `path_corner`s, and the port added two path_corner
spawnflags, **8 "Alienflyer laser"** and **16 "Alienflyer attack"**, so *the path says where it fires*. It
appears exactly once in the whole game, in `dy_fubar.bsp`, the last level: `spawnflags 64`, `target
af_wait`, `death_target previctory_mm`. A scripted, mortal, one-off set piece — not a monster you meet.

**There is no source for it.** It is not in the Half-Life SDK (which has `Aflock`, `Bird` and `Stukabat`,
and no flyer), and `decay.dll` is the port team's compiled code. So the AI is written new regardless, and
gaining sequences means a **Crowbar decompile of `flyer.mdl`**, which is Andrei's to run
([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md)). Nothing has been copied into `models/` yet.

**What it meets in the code.** A brush cannot be a monster: `SOLID_BSP` only pairs with `MOVETYPE_PUSH`,
which is a door or a train, and every schedule that advances on animation needs sequences a BSP model does
not have. A studio model on `MOVETYPE_FLY` with a hand-written think loop is the shape that works, and the
repo already has two worked examples of a big damageable flyer built that way — `dlls/apache.cpp` and
`dlls/osprey.cpp`. Neither uses schedules. The osprey also already gibs on death. Start there, not from
`CBaseMonster`.

Open, and the grill's to answer:

- **What is it?** A hazard that crosses the sky, an enemy that hunts, a transport whose cargo matters, or a
  moving surface the player lands on. Its size makes the last one real, and the reactor gauntlet
  ([where the bosses sit](#where-the-bosses-sit), 6) is the one entry that wants moving surfaces. These are
  different builds; picking one is the first question.
- **One set piece or a population?** Decay's answer was one. A thing seen twice and killed once reads
  differently from a thing in every Xen sky.
- **Shot down how?** Anything, or a specific answer — the wings, a lit organ, the [Core](#mining-and-crystal-shards)
  it runs on. The hitbox flag makes a weak point cheap. Falling, it is a hazard with a landing site, which
  is exploration content for free.
- **Does it fight back, and is the Pulse an answer?** An attack that is not on the Shield's damage list
  ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)) is an attack the Pulse is useless
  against.
- **Is this what the aerial tools are for?** The air Dash, the double jump, the katana and the pencilled
  Hook's one worthwhile form — a *pull* — all exist to reach something in the air, and the roster has no
  flyer worth reaching yet ([aerial melee](#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate)).
  A flyer that can only be shot wastes that.
- **The collision hull**, deliberately: Decay's 64-unit cube under a 224-unit model means shots pass through
  most of what is drawn. With `STUDIO_TRACE_HITBOX` set, bullets are fine; the hull is about where the
  player and the world collide with it, which matters a great deal if it is a surface.
- **Where it lives** — a Xen wing, Xen hell, or the sky over the facility as scenery first, earning its AI
  later.
- **Its name**, for [CONTEXT.md](../CONTEXT.md). "Flyer" is the filename and `monster_flyer_flock` already
  owns the word in the SDK.

### Where the bosses sit

**Settled 2026-09-17 as a base, open to small permutations.** Each ends a wing of the hub-and-spoke
[structure](#the-shape-of-the-game), and each tests one system — a boss that tests nothing is a big enemy.

| # | Boss | Where | Tests |
| --- | --- | --- | --- |
| 1 | Alien slave boss | Ends wing one, the mine levels | The Pulse |
| 2 | Assassin boss | Ends the military wing | Stealth, the Dash |
| 3 | Cult leader | Ends the first half, in the shrine around the cold open's chamber | Melee |
| — | The director | Not a fight | — |
| 4 | Alien grunt boss | Ends a Xen wing, guards a piece | The build |
| 5 | Kingpin | Xen hell, guards a piece | Aerial and ranged |
| — | The third Xen wing | **Open**: a gauntlet, a Xen sabotage operation, or pure exploration | — |
| 6 | The reactor | After every piece; a platforming gauntlet, no monster | Movement |
| 7 | The Nihilanth | The hub | Everything |

**The fallback:** the story needs only 1, 3 and 7, and that is the build order. The other three can ship
as tough variants of their base monster and gain custom attacks later, which is this section's approach
anyway.

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

A boss is a **fight**, not a puzzle: the slave, assassin and alien grunt bosses are fights with custom
attacks, and the Nihilanth is a pattern fight (2026-09-17, above).

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

Not nothing. Checked 2026-09-01:

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
- **The tree already touches it.** Headhunter (id 38, the Weapon Specialist's, built 2026-09-14) raises
  the head multiplier for player hits, so it raises how often the lethality test passes with no new hook.
  Whether a node should lower the damage floor as well is open, and whether a Headhunter rank makes
  decapitation more likely or only the hit bigger is a question the Route's entry also carries.
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
there is no loadout: finding one is permanent, and level design may assume any Module the player has
already passed. Each Module has a fixed **Slot** on the suit where the Status page
shows it (built 2026-09-16, [PILLARS pillar 5](PILLARS.md#5-inventory-management)) — a place, not a
choice; swappable Slots were rejected again, for now.

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
| **Pulse** | The existing Pulse, no longer available from the start. **Built 2026-09-16** as a found Module: `item_pulsemodule` opens its gate, and without it the key, the bar and every Pulse node are absent ([ADR-0013](adr/0013-the-pulse-is-a-found-module.md)); the pickup is a stand-in like the alien Module's. **Found at the hub or just after it** (2026-09-19), where the suit is put back on — not in a safety cabinet in the first minutes as 2026-09-17 had it, because the escape after the cafeteria is made with no suit and the Pulse needs one ([the cold open](#the-cold-open--shaft1-grilled-2026-09-19)). It is mining safety gear in the fiction, kept for crystal that flashes and then discharges. It gains [a half-damage tail](#the-pulses-tail--settled-2026-09-17-built-2026-09-18-overnight-visual-open) | Suit hardware — the Pulse came with the suit until 2026-09-16 |
| **Dash** | A short, fast movement burst. Built 2026-09-15. **Works in the air from the start — settled 2026-09-17, built 2026-09-18**: along the movement keys, the ground Dash's own rule, because a Dash glued to the ground cannot cross a gap. The *directional* dash stays the [Shinobi Major](#shinobi) | **The long jump module**, which serves the same purpose |
| **Hook** | A grappling hook, in the manner of Opposing Force's barnacle grapple. **Low priority since 2026-09-17**: kept as an alternative for navigation and combat, and nothing in the game is designed to need it | — |
| **Double jump** | A second jump in the air. **New 2026-09-17, the sixth Module**, found later in Xen, filling the **legs Slot** the Status page left open. It also unlocks the Air Dash node. Where exactly it is found is open (the vortigaunt's first hand-over after the teleport was proposed) | — |
| **The alien Module** (unnamed) | A platform for alien weapons that run on **Cores**; the summon weapon is its first. **Built 2026-09-16, untested in game, via a stand-in pickup** — the freed alien slave's actual hand-over is not built. | — |
| **Night Vision** | Opposing Force's night vision, adapted from `E:\Projects\halflife-op4-updated`. Gates the Stealth region of the Skill Tree. **Built 2026-09-16, untested in game.** | **The flashlight**, which stays until the Module is found |

The Opposing Force grapple code will be added to the project for reference; nothing about the Hook should
be designed against guesses until it is. With the air Dash, the long jump, the double jump and the Air
Dash, the player has four aerial tools where there were none when the Hook was pencilled in, and it is the
most expensive of them (predicted rope physics, a viewmodel, anchor entities). **It may make any space
easier and may be needed by none.** The one form worth the cost is a *pull* — a flyer down into katana
reach — rather than a swing.

**The double jump knowingly amends the 2026-09-13 cut of High Jump.** That cut had two halves: reaching is
a Module's job, and the normal jump rules stay untouched. The first survives; the second now ends the
moment the player finds the legs Module. High Jump stays `SKILL_RESERVED` — it is a Module, not a Skill.

The fourth Module was added 2026-09-13 and is designed under the [Alien Route](#alien); it is the one
Module carried as a *weapon*, which is a precedent for the Hook's "weapon or verb" question below.

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
Pulse, and both live in the [Juggernaut Route](#juggernaut--resilient):

- **Timing** — the Pulse as it is: press to raise a Shield, reward for reading an attack. Pulse Window
  (12), Pulse Recharge (15), Pulse Discharge (16), Pulse Rebound (17) and the Follow-Up (18) that hangs
  off a deflect.
- **The Defense Matrix** — for a player who does not want to parry. Proposed 2026-09-12 as a separate,
  passive, rechargeable health pool; shaped 2026-09-13 and built 2026-09-16 as neither separate nor
  passive: the Pulse key *held* for a second raises it, armour is the pool, nothing reaches health while
  it stands, the player is slowed 20% while it is up. There is no second bar and nothing refills by
  waiting.

**Settled 2026-09-12, as it stands today:**

- **The unskilled Pulse is the timed press**, with [the tail](#the-pulses-tail--settled-2026-09-17-built-2026-09-18-overnight-visual-open)
  since 2026-09-18 so that a player who is not into precise parrying is not put off the mod's first verb.
  Neither branch changes what the Module does before a Skill is spent.
- **The branches are not exclusive, but investing in both is meant to be inefficient.** No lockout rule,
  no new line style in the tree: the tree stays AND-only, and the cost of spreading points across both is
  carried by pricing. Both sit in one Route, so "splitting" is within the Juggernaut rather than across
  Routes.
- **The Pulse is found in the world, early.** It is the player's first Module, and the stretch of game
  that cannot assume it is short: at the hub or just after it, where the suit is put back on
  (2026-09-19, [the cold open](#the-cold-open--shaft1-grilled-2026-09-19)).
- **Order in `CBasePlayer::TakeDamage`**: a standing Shield refuses the hit outright, then the Matrix's
  armour share, then armour, then health.

**This reversed a recorded decision, and [ADR-0013](adr/0013-the-pulse-is-a-found-module.md) records it.**
PILLARS pillar 2 said the Pulse was suit hardware because *"Skills evolve a verb the player already has
rather than granting it, which lets level design assume it."* As a Module, level design may assume it
only after its acquisition point, and every Pulse node is absent until the Module is found, so a player
cannot spend on a verb they do not have.

### The Pulse's tail — settled 2026-09-17, built 2026-09-18 overnight, visual open

**Removed 2026-09-23**, when the window became one second: the tail filled 0.25–1.0 s, and the window now
fills all of it. See [The Pulse's timing](#the-pulses-timing--settled-2026-09-23-one-second). Kept below
as the record of what it was.

**Built and verified in game 2026-09-18: the mechanic is right, the visual is not** — and since
2026-09-20 the tail is **a candidate for removal**, not merely an unfinished visual. Recorded in
[PILLARS pillar 2](PILLARS.md#2-enhanced-combat).

Today the tail shows as a low clang on a braced hit and as recharge on the bar; its dim ring went with
every other ring when [the Shield in first person](#the-shield-in-first-person--settled-2026-09-20-built-and-seen)
was built. Andrei's verdict on 2026-09-18 — "the visual indicator needs to be addressed in order for the
mechanic to be clear and rewarding to the player" — was deferred the same day ("leave them for another
time"). When the first-person Shield could have carried it for free on 2026-09-20, a distinct vanish
telling "I parried" from "I braced", he declined: *"the tail is something that might not make it to the
finals so for this v1 lets not take it into consideration for extra treatment."* So nothing is built on
top of it, and [the Pulse's timing grill](#the-pulses-timing--settled-2026-09-23-one-second)
decides whether it merges into a longer window or goes.

If it stays, the proposals recorded 2026-09-18 and not chosen: (1) the bar gives the tail its own dimmer
segment that drains before the recharge starts, which needs a new client state; (2) a repeating, fading
cue across the whole tail, so its length reads in the world; (3) a braced hit gets its own payoff, a
half-bright flash and a brief screen-edge tint in the suit's colour beside the low clang — a small win,
clearly smaller than a parry. **Also open, lower priority:** whether a *successful* deflect should be
followed by a tail. Today it is not — a deflect ends as it always did, and the rest of that second is
unprotected.

**The rules, as built.** The deflect window stays (`pulse_window`, 0.25 s, full negation). The Pulse then
**persists to one second in total, and the tail takes half damage** (`pulse_tail_scale`, 0.5). Its length
is `skill_matrix_hold` rather than a cvar of its own, since rule 5 makes them one moment, and it follows
only a window that deflected nothing, so a deflect ends exactly as it did before the tail existed. The
rules that keep the skill ceiling where it is:

1. **Only the window is a deflect.** The short recharge (`pulse_recharge_hit`), the Discharge, the Rebound,
   the Follow-Up's prime and the [shelled headcrab's](#shelled-headcrab) punish window all stay on it. A
   hit taken in the tail is a miss with a discount: half damage, and the long recharge.
2. **The tail uses the Shield's damage list** ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)).
   Falls and drowning are not on it and are not halved.
3. **Pulse Window (12) widens the negate part inside the second**; it does not extend the total. The node
   converts tail into window.
4. **The two states must look and sound different**, so "I parried" and "I braced" are told apart without
   a number. Today only the sound differs, a duller clang. [ART_DEBT.md](ART_DEBT.md) already records the
   first Pulse sounds failing because two cues shared a timbre a tenth of a second apart.
5. **The tail runs exactly to the moment a hold raises the [Defense Matrix](#juggernaut--resilient)**
   (the key held for one second). Window, tail, Matrix: one motion, where today there is an unprotected gap.

Mashing the key buys about a second of cover per four-second cycle, most of it at half — roughly 12%
average reduction at today's `pulse_recharge_miss` of 3 s. A cushion, not a build.

### The Pulse's timing — settled 2026-09-23: one second

**Settled by Andrei on 2026-09-23**, coming out of the slave boss's grill: *"make the pulse last 1 second,
period."* Built the same day, not verified in game.

- **`pulse_window` is 1.0 s**, for everyone, and nothing extends it. It is the same second a hold takes to
  raise the Defense Matrix, so window and Matrix are one motion.
- **The tail is gone**, with `pulse_tail_scale`: the window now fills the second it used to.
- **Pulse Window (12) is inert**, and `pulse_window_bonus` is gone. The node stays on the board doing
  nothing.
- **Left for later:** a shorter parry window, gated behind a Skill and carrying extra bonuses — the shape
  the old 0.25 s window was reaching for — and the retired Discharge's node
  ([ADR-0016](adr/0016-the-discharge-is-innate-and-answers-only-slave-beams.md)). Pulse Window's slot is the
  natural home for the first.
- **What it does to cover:** mashing buys one second of full negation per four-second cycle at today's
  `pulse_recharge_miss` (3 s), 25% uptime, against about 12% at half damage before. Worth knowing when
  tuning.
- **Sustained fire** ([below](#the-pulse-against-sustained-fire)) is answered in the same stroke: a grunt's
  whole burst fits inside one window.
- **The Shield's sweep** (`pulse_shield_sweep`, 0.5 s, archived in Andrei's `config.cfg`) is still exactly
  half the window, so the Shield still has no hold. Anything under 0.5 buys one; 0.25 stands it for half a
  second. A call by eye.

The brief as it was written, questions and all, is kept below as the record.

**Raised by Andrei on 2026-09-20, the evening the first-person Shield first ran.** His words: *"the pulse
presentation is now gated by the actual mechanic: 0.25 is impossible to fit expansion and contraction of
the shield."*

**The observation, and why it is not a presentation problem.** The Shield's sweep is clamped to at most
half the window, so at `pulse_window` 0.25 s the entrance and the exit are 0.125 s each and there is no
hold between them at all. The Shield never *stands*; it arrives and immediately leaves. Raising
`pulse_shield_sweep` cannot fix that, because the clamp exists for a real reason — a sweep longer than
half the window would still be forming while the window closed. The presentation is not badly tuned. It
is correctly reporting that **the window is too short to be a state**, and it took drawing the thing
honestly for that to become visible.

This inverts the usual direction: presentation normally serves mechanics, and here the presentation has
produced evidence about the mechanic that no amount of playing it could.

**The numbers it runs into** (all defaults, `dlls/game.cpp`):

| | |
|---|---|
| `pulse_window` | 0.25 s, 0.40 s with the Pulse Window Skill |
| tail | to 1.0 s total, at `pulse_tail_scale` 0.5 |
| `pulse_recharge_hit` / `_miss` | 1.5 s / 3.0 s |
| `skill_matrix_hold` | 1.0 s — doubles as the tail's length |

**Questions to put, none of them answered:**

1. Is 0.25 s the right window, and what was it ever chosen against? It predates everything built on top
   of it, and nothing on record says it was measured.
2. Does the Shield want a **hold** at all — a state you are in — or is "arrives and leaves" the honest
   shape of an instant parry, with the presentation problem being that a sweep is the wrong motion for it?
   These are opposite conclusions and both are live.
3. If the window grows, what pays for it? A longer Recharge, a smaller damage list
   ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)), no Discharge on cheap hits?
4. What happens to the **tail**, which already occupies 0.25–1.0 s, is already a candidate for removal,
   and would be the obvious thing to merge into a longer window?
5. Does Pulse Window (Skill 12) still make sense if the base window changes?

**Fold in [The Pulse against sustained fire](#the-pulse-against-sustained-fire)**, which is the same grill
from the other end: grunts spam hitscan, a 0.25 s window catches part of a burst, and the verb teaches
badly against the enemy the player meets most. That entry has been open since 2026-09-18 with no answer
chosen. One session should close both or neither.

**Do not re-grill** the Shield's *look* — it is settled below and Andrei's verdict on it was "it looks
amazing". What is in question is the mechanic underneath it.

### The Shield in first person — settled 2026-09-20, built and seen

**Shape: settled by grill 2026-09-20, built the same day. Andrei's verdict on first sighting: "it looks
amazing."** But see [The Pulse's timing](#the-pulses-timing--settled-2026-09-23-one-second)
above — drawing it honestly exposed that the 0.25 s window is too short to fit an entrance and an exit,
which is a question about the *mechanic*, not about anything in this entry.

**Every call below is Andrei's.** Vocabulary note
first, because the grill did not obey it: the thing being drawn is the **Shield**
([CONTEXT.md](../CONTEXT.md)), and *bubble*, *barrier* and *forcefield* are on that entry's Avoid list.
Code, comments and commits say Shield.

**What it replaces.** `CHudPulse::Draw` (`cl_dll/hud_pulse.cpp:238-248`) fills the whole screen with a
flat `FillRGBA` rectangle at `hud_pulse_tint` 48 for as long as a Shield stands, and that is the entire
first-person treatment. Andrei: *"instead of 'seeing blue' like the current functionality, a Shield
rapidly forming around you and vanishing is much cooler."* **The motion is the point** — not the colour,
not a texture. Everything below serves that sentence.

**How it got here.** The session started on giving the Pulse a *model* animation, and both model routes
died on inspection. A holster-play-draw costs ~1 s against a 0.25 s window, disarms the player during the
second they pressed a defensive key, and breaks the Follow-Up. A per-weapon animation is three poses
(window, tail, Matrix) across the 16 viewmodels in `models/` — ~48 hand-animated sequences, against a
modelling ladder on rung one. A single off-hand model was killed by Andrei in one line: *"some weapons are
held with two hands, so a ghost arm cannot just appear"* — true of the MP5, shotgun, crossbow, gauss,
egon, RPG and tripmine, with the grenade and satchel showing the off hand mid-animation. The one surviving
model answer, a hard swap of `pev->viewmodel` with no holster time, is parked in the backlog below.

#### The settled shape

1. **A surface, not energised gear.** Two readings were put: the gear you hold lights up, or something
   stands between you and the world. Andrei took the second — *"b is the correct answer with a kept in
   backlog for maybe fitting better on the defense matrix."* That split is sharper than it looks: a
   surface in front of you for 0.25 s is fine, for the Matrix's 6 s it would be unbearable, and a glow on
   your gear is the reverse.
2. **It covers the whole view.** `CPlayerPulse::WouldNegate` (`dlls/player_pulse.cpp:975`) takes no
   direction at all — a shotgun in the back is negated exactly as one in the face. Anything held in front
   of the player would teach a facing rule the game will never reward. The player is *inside* the Shield.
3. **Clear at the crosshair, dense at the periphery.** The falloff is **authored**, and its justification
   is that the player must be able to see what they are shooting during the one quarter-second that
   matters. In v1 it is the *only* structural cue besides the travelling edge, so it carries the surface
   read alone. `pulse_shield_spread` and `pulse_shield_falloff` shape it, `pulse_shield_centre` sets what
   is left at the crosshair itself. Trap, recorded here and in the code header so nobody tunes against a
   model that does not exist: the grill argued this as physics — "what the inside of a sphere looks like,
   oblique at the edges" — and that is wrong. From the exact centre of a sphere every surface element is
   face-on, and the path length through a thin shell is identical in every direction; there is no fresnel
   to reproduce.
4. **The motion is a fill, not an inflation.** The sphere sits at a fixed radius and the *material*
   spreads across it behind a bright leading edge. Rejected: inflation (the surface crosses the camera on
   its way out) and facet assembly (most work, most generic, and it is a texture decision rather than a
   motion one). The geometry never moves, so nothing can ever pass through the eye.
5. **The origin is the crosshair, live.** Recommended against and overruled, correctly — Andrei: *"the
   frontal protection is the most important in the heat of the moment... the pulse happens quickly so I
   don't see a big issue with turning around."* The player's eyes are already at the crosshair, so
   feedback that begins there is perceived in the first frame, where a sweep rising from the screen edge
   costs perceptual time a 0.25 s window does not have. Latching the direction in world space at the press
   was offered as strictly more robust; it is *one member variable dearer*, not cheaper, so it ships as a
   cvar branch to judge by eye, the way `pulse_ring_style` was meant to be judged.
6. **The vanish is the fill reversed.** The boundary retreats back toward the crosshair and winks out
   where it began. A trailing-edge sweep (one wave crossing the view, arriving and departing) was the
   prettier option and was rejected for lying: it empties the centre of the view while the Shield is still
   mechanically up. The reverse empties the periphery first, which is the part the player is not watching,
   and its last frames are a bright ring shrinking to a point at the crosshair — an unmissable "you are
   unprotected now" cue delivered to the fovea, which an ability with a 1.5–3 s Recharge wants.
7. **Fixed sweep, variable hold — decided, and inert at today's numbers.** The decision: sweep in and
   sweep out are a fixed duration *always*, and `PulseWindowFor`'s extra 150 ms at Pulse Window
   (`dlls/player_pulse.cpp`, 0.25 s → 0.40 s) goes entirely into the hold, because sweep speed is a
   property of the suit, not of the player's build, and an entrance that looks identical every time is
   learnable as a confirmation that the press registered. What the tuning did: `pulse_shield_sweep` went
   from 0.08 to **0.5** on first sighting, and the sweep is clamped to at most half the window, so at the
   default window the Shield spends its *entire* life travelling — out for 0.125 s and back for 0.125 s —
   with no hold at all, and every duration scales with the window again, which is the option this one
   was chosen over. The structure reasserts itself the moment the window grows or the sweep drops back
   under half of it; until then nobody should read this rule and expect a hold. That is the observation
   [the timing grill](#the-pulses-timing--settled-2026-09-23-one-second) exists for.
8. **No texture in v1.** Pure vertex colour; the travelling edge and the obliquity falloff do all the
   work. Hex facets are an [ART_DEBT.md](ART_DEBT.md) entry, deliberately not a prerequisite — if the two
   procedural cues do not sell a surface on their own, that is worth learning for the cost of an evening
   rather than a sprite. Concentric ripples were rejected outright: rings travelling outward would fight a
   boundary travelling outward and read as two events.
9. **Suit Variant colour.** On the client that is `RGB_SUIT`, which `cl_dll/hud.h:36` defines as
   `gHUD.SuitColour()` and which already tracks the Variant — the same source the flat rectangle it
   replaces was already using, and the same the bar at `cl_dll/hud_pulse.cpp:35` uses. No new colour
   plumbing; the server's `GetSuitVariant` (`dlls/player_pulse.cpp:591`) is the world rings' equivalent
   and stays where it is.

#### The deflect flare

A deflect today is a sound and nothing else, and it *destroys information*: the negated hit returns at
`dlls/player.cpp:438` before `gmsgDamage` is ever sent, so a hit you successfully parried tells you
nothing about where it came from, while a hit that lands gives you the suit's damage compass. The flare
repairs that loss — the strongest justification an effect can have.

- **Vanilla's math verbatim**, Andrei's call: `CHudHealth::CalcDamageDirection` (`cl_dll/health.cpp:238`).
  Forward and right dots, 0.3 to store, 0.4 to draw, `V_max` accumulation, decay `m_flTimeDelta * 2`
  (~0.3 s, near enough `pulse_window`). Two gifts fall out free: several hits inside one window all stack,
  which is correct because the window never closes early (`dlls/player_pulse.cpp:614-621`); and inside 50
  units all four quadrants light at 1.0, so a melee deflect floods the whole Shield rather than one side,
  which is what a claw at arm's length deserves.
- **Painted as a broad quadrant gradient**, not as patches at vanilla's offsets. The player sees vanilla's
  real trapezoids in the same firefight whenever a hit actually lands; same shape and position would make
  "I was hit" and "I parried" hard to separate at a glance, which is the exact distinction the effect
  exists to teach. A region of a surface lighting says *object*; a patch at a fixed screen offset says
  *HUD*.
- **White-hot core** falling back to the Shield's hue. A contrasting accent was rejected on Suit Variants
  alone — a colour chosen to pop against blue can vanish against amber, and it would need retuning against
  every Variant forever. Same-hue-brighter has no headroom, since the periphery where most flares land is
  already bright and additive. Luminance is the only axis that works against every hue, and it keeps three
  signals distinct at once: the pain ramp is red/yellow (`GetPainColor`), the Shield is the suit's hue, a
  deflect is white-hot in that hue.
- Needs a small new message carrying the direction. `TryNegate`'s call site
  (`dlls/player.cpp:436`, inside `CBasePlayer::TakeDamage`) has `pevInflictor` and `pevAttacker` in scope.
  The state enum is *not* touched — `EPulseState` stays as it is.

#### Where it draws

`HUD_DrawTransparentTriangles` (`cl_dll/tri.cpp:47`, today only the particle manager), after the world and
after the viewmodel, **with no depth test**. Depth-testing was rejected on where this lives: `shaft1` and
`minemap` are tight corridors where walls sit nearer than any radius giving useful curvature, so the
Shield would not clip occasionally but *most of the time*, and a Shield sliced open by the wall it is
protecting you from reads as a bug. Drawing over the viewmodel is the accepted cost and a dividend: the
weapon is washed in the suit's colour on every press, on every weapon including the two-handed ones that
killed the ghost arm — a free preview of the energised-gear reading before the Matrix version is built.

**Built on raw GL, not `pTriAPI`**, for two reasons that only showed up in the writing: v1 has no
texture, so the API's one real service, binding a sprite, is not wanted; and turning the depth test *off*
is the entire point of drawing here, which the API gives no way to do. So the Shield is hardware-renderer only, like the katana trail and
the blade's cooling, and the software renderer draws nothing at all. That is precisely why
`hud_pulse_tint` was kept rather than deleted — it is not a courtesy to old configs, it is the software
path. When the hex facets land (ART_DEBT), the texture binding can come back through `pTriAPI` or through
GL directly; the depth-test requirement will still rule out the API on its own.

#### What goes, what stays

- **Every ring — deleted**, and with them `pulse_ring_style` and `pulse_ring_scale`, on the day the
  Shield was first seen. The `TE_BEAMCYLINDER` branch was the houndeye's floor blast from
  `dlls/houndeye.cpp` and went during the grill (*"as for the houndeye ripple delete it"*); the
  `TE_BEAMTORUS` pair was kept to be judged beside the Shield and lasted exactly as long as that took —
  *"disable or remove the torus/rings all together, they are ugly compared to what we have on our
  hand."* The tail's dim ring went with them, which costs nothing it was not already losing, since the
  tail was deliberately left out of v1 anyway. `sprites/shockwave.spr` is no longer precached or
  referenced; the Pulse draws no sprite in the world at all.
- `hud_pulse_tint` — **default 0, cvar kept**. It is `FCVAR_ARCHIVE` and already written into configs, so
  deleting it would make an existing setting silently do nothing; at 0 it costs four lines and gives an
  instant A/B in the console while judging the Shield in play.

  **The trap that follows from that, hit within minutes of the first build (2026-09-20):** changing an
  `FCVAR_ARCHIVE` cvar's default does nothing for anyone who already has it in their `config.cfg`, which
  the engine replays on every launch. Andrei's config pinned `hud_pulse_tint "48"`, so the first test
  showed the old flat rectangle drawing over the new Shield and read as "no change is evident" — the new
  code was running the whole time, provably, because the `pulse_shield_*` cvars had been written to that
  same config at their new defaults. `hud_pulse_tint 0` in the console once is the whole fix, and it
  persists on quit. Worth remembering before assuming a changed default has taken.
- `TE_DLIGHT` — **kept**, and it is now the *only* thing the Pulse puts in the world. This is the carve-out
  that matters, and it survived the rings being cut because the argument for it is different in kind. It
  throws suit-coloured light onto the walls, which is the one thing a first-person overlay fundamentally
  cannot do. Losing the rings costs a shape nobody likes; losing the dlight costs the Pulse its only
  physical presence in the room.

#### Deliberately not in v1

- **The tail gets nothing.** Andrei: *"the tail is something that might not make it to the finals so for
  this v1 lets not take it into consideration for extra treatment."* This leaves
  [the tail's visual](#the-pulses-tail--settled-2026-09-17-built-2026-09-18-overnight-visual-open) open
  where it already was, and removes it as a blocker here.
- **The Matrix is untouched**, keeping its edge bands. A hold therefore shows the Shield form and retreat
  over 0.25 s, then a **0.75 s hole**, then the bands at 1.0 s. Left honest on purpose: the Shield
  genuinely is down in that gap, and holding the visual through a closed window would teach the wrong
  timing for an ability whose whole skill expression is timing. How bad the gap feels in play is direct
  evidence for how urgently the Matrix needs its own treatment — evidence that does not exist today.
- Numbers ship as cvars, dialled by eye per `pulse_ring_scale`'s precedent: `pulse_shield` (off switch),
  `_sweep`, `_alpha`, `_edge`, `_edge_width`, `_spread`, `_falloff`, `_centre`, `_flare`, `_live`. The
  Shield respects `HIDEHUD_ALL` as `CHudPulse::Draw` does. **Radius and tessellation are constants, not
  cvars**: with the depth test off and the sphere centred on the eye, the radius has no visual effect at
  all — only each vertex's *direction* matters — and a cvar that does nothing is worse than none.

#### Backlog this created

- **Energised gear on the Defense Matrix** — the (a) reading, parked at question one for exactly the state
  it suits: 6 s, fought through, weapon must stay in hand. The edge bands at
  `cl_dll/hud_pulse.cpp:181` already keep the middle clear to fight in, so the instinct is half-built.
- **A hard `pev->viewmodel` swap** — the one model answer that survives two-handed weapons. No holster
  time, one `v_pulse.mdl`, covering the tap only. Its unknown is prediction: `cl_dll/hl/hl_weapons.cpp:756`
  reads `from->client.viewmodel` in and `:829` writes the predicted value back out, and whether a
  server-side swap survives that round trip is reasoning, not evidence. Two-line test before any model.
- **Hex facets** for the Shield's surface — [ART_DEBT.md](ART_DEBT.md).
- **Ripples from the impact point** across the surface, on top of the quadrant flare.
- **Rebuilding the world effect to agree with the Shield.** Andrei: *"the drawshieldring is more like a
  placeholder than actually desired mechanic."* The dependency now runs the other way — the first-person
  Shield is the primary, and the world's rings should be rebuilt to match it rather than the reverse.

#### Two traps to record

- `cl_dll/health.cpp:262-263` — `front` holds the **right** dot and `side` holds the **forward** dot. The
  names are backwards and the behaviour is correct. Somebody will try to fix it.
- The grill's working words were *bubble* and *barrier*. Both are on the Shield's Avoid list in
  [CONTEXT.md](../CONTEXT.md). They must not reach code, comments or commit messages.

### The Pulse against sustained fire

**Shape: Idea, from Andrei 2026-09-18. Needs refining; no answer chosen.** Grunts spam shots, and
deflecting bullets makes an awkward loop: it works *sometimes*, so a player concludes it is worth trying,
and the window is short enough that trying it again and again is frustrating. The Pulse teaches a verb
that does not hold up against the enemy the player meets most.

The shape of the problem, as far as the code goes: a burst is three hitscan rounds fired by the
sequence's own events, and the window (`pulse_window`, 0.25 s) is followed by the long recharge whenever
it catches nothing — how much of a burst a well-timed window covers has not been measured. [ADR-0006](adr/0006-the-discharge-vents-at-the-crosshair.md) records that the mechanic was
designed against exactly this hitscan. The tail (above) halves what lands after the window but does not
change the lesson. Directions worth putting to Andrei, none decided:

- **Tell the player plainly that bullets are not the Pulse's job** — the window only answers a telegraphed
  hit, and bullets are answered by cover. The honest version of the loop, and the cheapest.
- **Make a burst the unit**: a window that catches the first round covers the rest of that burst.
- **Give the grunt a telegraph** the Pulse can read — the flash-then-discharge language — so a deflect is
  earned by reading the soldier, not by guessing.
- **Deflect credit per window, not per round**: one success is one reward and the short recharge,
  however many rounds were stopped.

[Soldiers moving while shooting](#soldiers) makes this more pressing, not less.

### Open questions

- **How "inefficient to split" is priced.** Both branches sit in one Route, so "splitting" is within the
  Juggernaut rather than across Routes, and the economy is a non-issue (2026-09-15, [Maps](#maps));
  whether the pricing does it is a play question.
- **Is the Hook a weapon or a verb?** Opposing Force's grapple occupies a weapon slot. A Module as defined
  above is a verb, which argues for its own key — to be read against the reference code. Low priority
  since 2026-09-17; nothing waits on the answer.
- **Where is the double jump found**, and where the Dash? Proposed 2026-09-17, neither confirmed: the
  double jump as the vortigaunt's first hand-over after the teleport, the Dash from the assassin boss.

Answered, and recorded where the answer lives: the Dash's input is a tap of shift in the direction of
movement, walk rebound to ALT ([Shinobi](#shinobi)); `item_longjump` in vanilla maps keeps its classname
and gives both the long jump and the Dash (2026-09-15); every pickup is walk-over
([ADR-0011](adr/0011-pickups-are-walk-over.md)); the Dash is upgraded by Skill Points; Modules are shown
as fixed Slots on the Status tab, occupying no Cells ([STATUS_PANEL.md](STATUS_PANEL.md)); Pulse nodes
are absent until the Module is found (ADR-0013), so nobody spends on a verb they do not have; and there is
no passive pool — armour is the pool, under the [Juggernaut](#juggernaut--resilient).

---

## Pillar 4: Routes

**Shape: Shaped. The direction was settled 2026-09-13 and all seven Routes were shaped the same day, each
in a grilling session. What remains is the order of building, the numbers, and the infrastructure notes
below.** The agreed tree as one reference, with every node, the prerequisite structure and the
cross-Route links, is [SKILL_TREE.md](SKILL_TREE.md); this section is the reasoning behind it.

The tree started 2026-09-13 at 16 Skills and 35 points, wanting closer to 50–70, under two rules that
pull in opposite directions on purpose: **more Skills are definitely needed**, and **diluting the tree is
not good design** — a bigger tree of flat numbers, each a little more of the same, is the failure mode.

The resolution is **builds**. Skills are added so that *combining* specific bonuses scales exponentially,
not so that each one is worth a little on its own. The example given: several Backstab damage bonuses
stacked with several energy damage bonuses culminate in **backstabbing a Gargantua with the Gauss Katana**.
[ADR-0010](adr/0010-the-backstab-is-positional.md) already says this is intended, so the tree's job is to
make the path to it exist and cost most of a campaign.

**And the path is literal.** Since 2026-09-14 the tree is a matrix of small **Stat nodes** — one flat
bonus each, one point each, themed by Route — that are the roads between Skills and the whole of their
price; since 2026-09-15 it is a 15×15 board of nine regions with the suit as its one start, about 140
nodes against 100 findable points, deliberately not completable. Settled in
[SKILL_TREE.md](SKILL_TREE.md#the-matrix--settled-2026-09-14), which is the reference. The anti-dilution
rule is amended rather than broken — flat numbers as roads, never as destinations — and the amendment is
written beside the rule there.

A **Route** is a build path, a region of the board since 2026-09-15, and the seven below are what the tree
is curated toward.

### Settled 2026-09-13

- **Four Skills are cut, and the cuts are built.** Battery Regen (14) and Regeneration (10) rewarded
  idling; High Jump (5) and Sprint Speed (6) altered the normal movement rules, and reaching is a Module's
  job (Dash, Hook, see [Modules](#pillar-3-modules)). All four are `SKILL_RESERVED` with their ids frozen,
  `CPlayerRegen` is gone with its two Skills, and Med Expert (19) is a root again. The record is
  [SKILL_TREE.md](SKILL_TREE.md#cut-and-reserved) and [PILLARS pillar 4](PILLARS.md#4-skill-trees).
- **Stacking is the goal, not a hazard.** Every multiplier a Route adds is meant to multiply the others.
  The Backstab's current tuning (49.5 against a grunt's 50, so that a one-shot is *not* reliable at full
  melee investment) still describes the early tree. It does not describe the endgame, which is meant to
  one-shot things that are not grunts.

### Settled 2026-09-17 — the fuel, the processors, the Air Dash gate

From the grill on [the game as a whole](#the-shape-of-the-game). No code.

- **The tree has a fiction, and it was already drawn.** The facility processes crystal into technology;
  the suit is adaptive equipment that changes when fed processed crystal; **a Skill node is that fuel.**
  SKILL_PANEL.md's circuit, the processor as the only start and power spreading by adjacency already say
  it. `item_skillpoint` is a processed crystal cell, the facility's product (a model and a name,
  [ART_DEBT.md](ART_DEBT.md)); a gated region reads `No signal` because the suit cannot route fuel to
  hardware that is not installed; a Reset Token is a reflash, which is why it refunds. **Power and
  attunement are one curve**: every node makes the Nihilanth's voice clearer.
- **Fuel processors: a bounded way to make a point.** Some [Stations](#stations) turn a large stack of
  [crystal shards](#mining-and-crystal-shards) into **one Skill Point, once**, and are then spent. The
  mapper counts each among the 100 findable points — some 10–15 of them — so the settled economy is
  untouched; only the delivery changes. An open conversion at any Station was rejected: deposits are
  finite, but the ceiling would then depend on how each player splits shards between ammunition and points.
- **The directional Air Dash is gated behind the double jump Module.** With the base Dash working in the
  air, the upward dash is the one tool that out-reaches the double jump, and bought in the first half it
  would open every vertical gate in the game. The gate is already a per-row field (`EGate gate` in
  `k_SkillDefs`), so this is one new `EGate` value on one row inside a region gated by `DashModule` — the
  per-node gate the [Alien Route](#alien) set aside comes back as a table edit, not as machinery. Until the
  double jump exists the node is sealed; `skill_unlock_all` and `skill_open_gates` still reach it.
- **Aerial melee is a stated fantasy**: the upward dash with the katana in hand, against controllers, alien
  ships and other flyers in late Xen. **Reprisal already is the reset** — a one-shot melee kill refills a
  Dash, so in the air it chains. What it needs is flyers the katana one-shots, which is tuning, and
  stukabats are on the [cut-monster list](#xen-hell-and-the-cut-monsters).
- **Modules gate, Skills never do**, so no Route is needed to finish. What a Route buys is which
  [soft gates](#the-shape-of-the-game) open cheaply: a Juggernaut tanks a hazard corridor, a Stealth build
  ghosts a guarded shortcut, an Energy build cuts a crystal barrier. Routes are key rings.

### Built 2026-09-15, the same day

An implementation session with delegated agents built everything below that is code: the rule and the
Suit, the board with every region placed and the hidden ones gated, the panel of SKILL_PANEL.md, and
Egon Focus, Overdraw, Last Stand and Glass Cannon. Not built: the effects behind the hidden regions'
nodes and the Modules that open them; the art. What exists is in [PILLARS pillar 4](PILLARS.md#4-skill-trees).

### Settled 2026-09-15 — one start, open roads, the count, the keystone, the circuit

A grill held before the third Route, on the tree as a whole rather than on a Route. Every point below was
put as a question with a recommendation; the shape is in
[SKILL_TREE.md](SKILL_TREE.md#one-start-open-roads--settled-2026-09-15) and the reasoning for the first
in [ADR-0012](adr/0012-the-skill-tree-has-one-start-and-open-roads.md).

- **The matrix's "efficient pathing" did not exist.** With curated edges on every node and seven free
  roots, the set of nodes needed for any Skill was fixed and a seam was never cheaper than a fresh root.
  **Changed**: the suit at the centre is the only start, and **every node opens from any owned orthogonal
  neighbour** — no prerequisites, no AND gates; empty cells limit pathing and price the Majors. Path of
  Exile's rule, and simpler to state than the one it replaces. (A first cut the same morning kept curated
  gates on Skills; Andrei removed them entirely within the hour.)
- **Last Stand answered**, for the Medical Major and Glass Cannon alike: held at 1 health, invincible for
  3 s, the Syringe fires, 60 s cooldown.
- **Road stats**: Juggernaut Max Armour (continuing the hub's), Dash Recovery, Alien Hornet Replenish,
  Stealth Concealment (a multiplier on the Suspicion rate). The hub's are Max Health and Max Armour.
- **Hub corners**: Follow-Up in the Melee–Juggernaut corner, the other three open for now; Leech and
  Ricochet stay in their Routes. The Energy Major, **named Overdraw**, is placed on the Energy–Juggernaut
  seam as that pair's cross-Route node.
- **Hidden means impassable**: a blank pad cannot be bought, so a gated region is a wall until its Module.
  Seam nodes sit on the ungated side; the Pulse nodes on the far side of the Juggernaut's region.
- **The hub's stats face their Routes**: +5% max health north (Medical), +5% max armour south
  (Juggernaut), +5% bullet damage east (Specialist), +5% melee damage west (Melee), with a Minor of higher
  value on each side of the rim (first-cut reading: Fortitude, Battery Capacity, Marksman, Melee Force).
- **The Dash Route is named Shinobi.**
- **The Stealth Route is shaped**, seven nodes on Concealment roads, all reading a monster's own meter at
  an action: Soft Step, Ambush (22, the merged unaware-damage node, all weapons; ×1.5 below Spotted / ×2
  below Noticed since 2026-09-17, up from ×1.25 / ×1.5 as first built), Phantom (23, an Unseen Backstab
  kill buys 4 s at ×1.5 speed with silent movement; 2 s at ×1.2 on a hit was the first shape), Nightfall,
  Slip Away, Shroud (×0.8 on the fill; it took Cut the Head's cell when that was dropped on 2026-09-17),
  and the Major **Silent Kill** (a kill below Spotted is unheard — no Disturbance — and nothing more; a
  witness in sight reacts in full, settled 2026-09-17 and built with 5f). The Gargantua stack was
  checked: Stealth alone reaches 540 of 800 and needs Melee and Energy for the rest. See
  [SKILL_TREE.md](SKILL_TREE.md#stealth).
- **A fifth Module, Night Vision**, gates the Stealth region and replaces the flashlight when found; the
  flashlight stays until then. Adapted from Opposing Force's, source at `E:\Projects\halflife-op4-updated`.
- **Energy's two loose ends closed**: Egon Focus is the SDK's narrow beam on right click, as it is; Quick
  Charge (57) is cut, the wave having no charge. Glass Cannon's 50 is the ceiling after every health bonus;
  the hub's rim Minors are Fortitude, Battery Capacity, Marksman and Melee Force.
- **The panel's theme settled**, in [SKILL_PANEL.md](SKILL_PANEL.md): header `HEV MK IV // <codename>`
  following the Suit Variant; tab stays "Upgrades"; a gauge strip (segment counter, token pips) and a
  hazard-striped Reset switch fixed over the field; the board at 1:1 with drag to pan, opening on the
  suit; the region palette grounded in the game's objects; monochrome glyph icons in the HUD style;
  tooltip name and effect only; hidden pads say `No signal`; one unlock sound.
- **The board placed cell by cell**, in [SKILL_MAP.md](SKILL_MAP.md) and `docs/skill_map.csv`: seams are
  regions touching directly with empty cells curating the doors; the hub has health and armour on its
  diagonals and one melee and one bullet cell toward their Routes; every region's map was put as a drawn
  question and accepted; 154 nodes. Sizes for the 1:1 view: a 112 step, 36 / 50 / 62 / 74, the processor
  92; Mastery, Discharge, Rebound and Follow-Up keep the Major-sized frame.
- **Rejected while shaping Stealth**: Assassinate as a separate node (one verb with Ambush); Executioner
  (an unnoticed Backstab kills outright — hands out the one-shot the stacking is meant to earn); Sabotage
  (disabling turrets from behind); Follow Through and Shroud as the seventh node; a "never noticed" saved
  flag in favour of reading the meter at the hit.
- **Nine regions on 15×15.** Hub in the centre with the suit stats and the four old survivability
  Skills; ungated Routes on the edges (Melee W, Medical N, Specialist E, Juggernaut S); gated or late
  Routes in the corners (Dash NW, Stealth NE, Alien SE, Energy SW). Stealth is a corner because it may be
  gated behind a Stealth Module later; the price is that Dash and Stealth do not touch.
- **~140 nodes, 100 findable points**, 40 on the critical path and 60 in optional spaces. A thorough
  player owns 71%; raised as a concern and accepted knowingly over 200 nodes or 70 points.
- **One keystone, Glass Cannon**, on the Melee–Dash seam: max health 50, Last Stand permanently armed, no
  damage multiplier. Keystones are nodes with a downside; more only after this one is played.
- **The tree is drawn as the suit's circuit**: substrate tile, a colour wash per region, copper traces,
  a frame sprite per tier, red for the keystone, blank pads for gated regions. Brief in
  [ART_DEBT](ART_DEBT.md#the-skill-tree--the-circuit-substrate-traces-and-frames).
- **Rejected in the same grill**: per-Major downsides (reopens seven settled designs); jewels, masteries
  and per-point refunds from the model; a 200-node tree; region labels on the tree itself; curated gates
  of any kind; a damage multiplier on Glass Cannon; moving Leech and Ricochet into the hub.

### What the Gargantua example actually needs

Worth writing down because it is the acceptance test for the whole idea, and one fact about it is not what
the ADR assumed.

- **The Gargantua takes only energy, crush, mortar and blast** (`GARG_DAMAGE`, `dlls/gargantua.cpp:47`).
  Everything else is zeroed in `TraceAttack` and scaled by 0.01 in `TakeDamage`.
- **The katana's blade is energy damage outright** (built 2026-09-14): `CCrowbar::SwingDamageType` is
  the blade's type against a monster, `DMG_CLUB` by default and `DMG_ENERGYBEAM` on the katana, for the
  single hit and the Cleave arc alike. Anything that is not a monster still takes `DMG_CLUB` from every
  roster weapon, because `func_breakable` keys its crowbar rules on that bit and a blade through a crate is
  still a blow. A katana Backstab on a Gargantua passes the filter; ADR-0010's note is closed. A node that
  *made* the blade energy was rejected as leaving the katana half a weapon until bought.
- The Backstab is a multiplier on the blade only. The wave never reaches `CanBackstab`, and there is no
  reason it should.

### The seven Routes

Named on 2026-09-13, to be curated one at a time; the order is not set. Routes are meant to combine: the
Gargantua build is Melee × Energy, and a Route on its own is a starting point rather than a finished
character. Each is recorded with what exists
to build on, so that curation starts from the code rather than from the note.

#### Juggernaut — resilient

**Shape: Built 2026-09-16, whole, and tuned in play the same day.** The Defense Matrix, Matrix on Kill
and Decaying Armor were built as settled below, every number a first guess in a cvar
([instructions/04](../instructions/04-CUSTOM-FEATURES.md)); what exists is in
[PILLARS pillar 2](PILLARS.md#2-enhanced-combat). The Pulse key became a `+pulse`/`-pulse` pair, carrying
the press and the release as two impulses so the hold is timed on the server. The text below is kept as
the reasoning. Shaped 2026-09-13, in a grilling session. **Low mobility, high defense**, and it holds the
whole of the Pulse: the timing branch that exists, the Defense Matrix that replaces the passive branch,
and the armour both lean on.

##### The Defense Matrix — settled

The passive health pool from [Modules](#the-pulse-as-a-module--two-branches-instead-of-a-swap) went
through three shapes in one session and came out as none of them. What it is:

- **Hold the Pulse key for one second** and the Matrix comes up. Gated by its Skill, and needing the Pulse
  Module like everything else on the press. **The tap's Shield still fires at the front of the press**, so
  a hold begins with the deflect window and the Matrix follows it; deflect and Matrix are separate verbs on
  one key and neither invalidates the other.
- **While it is up, armour takes a far larger share of every hit** than its normal ratio. **Armour is the
  pool**: there is no second bar. Battery Capacity is its capacity, batteries and chargers are its refill,
  and **nothing refills by waiting**. Falls and drowning bypass it because the base game already skips
  armour for both, so no carve-out is needed.
- **The player is slowed 20% while it is up.** This is the Route's whole cost, and it is paid exactly when
  the protection is on, never by standing still. It is reachable without touching the movement code: the
  engine feeds the player's own `maxspeed` into `pmove->maxspeed`, which `pm_shared/` clamps against
  (`pm_shared/pm_shared.cpp:1144`, `:2940`), and the server sets that value per player. A cap change, not
  a rule change, which is why it survives the rule that cut Sprint and High Jump. To be measured in play.
- **It drops at 6 seconds or at zero armour**, whichever first, and **not on release**: the hold only
  raises it, and it keeps its own time (Andrei, 2026-09-16, after the first play; "drops on release" was
  the first shape). **10-second cooldown** after it drops.
- **The HUD tints the armour readout while it is up**, and nothing else changes.

Rejected on the way, so they are not proposed again: a passive pool that refills after 10 seconds without
damage (it is Battery Regen under another name); a pool that takes the whole hit (a second armour bar); a
toggle with two presses (state to forget); changing what a tap does for a Matrix player (the branches are
meant to mix); a movement penalty on the nodes themselves (a rule change); a lockout against the Dash
Route (the tree is AND-only). "Low mobility" enforced by pricing alone was the recommendation and was
overruled in favour of the slow, which is a better answer: the Juggernaut is slow *while being a
Juggernaut*.

**The word "Shield" is taken** ([CONTEXT.md](../CONTEXT.md): the field a Pulse raises) and the Matrix must
not be called one in code, docs or commits.

##### Ricochet — settled

**A chance per bullet to bounce it back at the attacker, negated for the player.** The bounced bullet does
nothing to the player; its full damage goes to the attacker as bullet damage, with a tracer drawn from the
player back to them. Only while the player has armour, since the plates do the bouncing, and only bullets:
explosions, melee and energy never ricochet. Ranks raise the chance. The "this hit was turned" half exists
in the alien grunt's plating (`dlls/agrunt.cpp:221-257`); the return trip is new, and the attacker is known
at the moment of damage and had a line to the player.

**Built 2026-09-14** as a single node (id 64) off Armor Expert, in the player's `TakeDamage` after the
Shield's answer and before the suit's report: `skill_ricochet_chance` (0.2) per bullet hit while armour is
above zero, the hit refused, the shooter dealt the full damage as `DMG_BULLET` with the player as
inflictor, a `TE_TRACER` from the player's centre to theirs and the stock ricochet spark at the player.
The tracer's look waits on the Route's region. It has no ranks: under the matrix a rank is a Stat node on
the road, and the Juggernaut's road stat is Max Armour, so Ricochet is one node at its cvar. A
Ricochet-chance stat of its own is a **low-priority reevaluation** once the Route has been played, not a
promise (settled 2026-09-16).

##### The nodes — settled

Eleven, the largest Route, because it holds the Pulse's existing four as well as its own. Ranks count as
one node each.

| Node | Effect | Note |
| --- | --- | --- |
| Fortitude (8) | +25 max health | Exists. Root |
| Armor Expert (9) | Less gets past armour | Exists. Ranks |
| Battery Capacity (13) | More max armour | Exists. Ranks |
| Ricochet | As above | New. One node, no ranks |
| Pulse Window (12) | Longer Shield | Exists. The timing branch, brought inside the Route |
| Pulse Recharge (15) | Shorter Recharge | Exists |
| Pulse Discharge (16) | Negated hits vent at the crosshair | Exists. **To be retired** (2026-09-23): the Discharge becomes innate, [ADR-0016](adr/0016-the-discharge-is-innate-and-answers-only-slave-beams.md); the node needs a new occupant |
| Pulse Rebound (17) | A deflect skips the Recharge | Exists |
| Defense Matrix | The gate: hold for 1 s | New. Needs the Pulse Module |
| Matrix on Kill | A kill while the Matrix is up restores some armour | New. The Route's one way to sustain, and the opposite of idling |
| Decaying Armor | **+100 decaying armour on activation** | New, the Major. Numbers to be toned down; the philosophy is below |

**The major node is the Energy tie.** The Energy Route's own major node makes energy weapons drain armour
as fuel. So a player with both raises the Matrix, gains a hundred decaying armour, and fires the egon into
it: **the armour is consumed at an alarming rate and the damage is enormous**. What was first read as the
one *conflict* between Routes is the intended build.

Two things held back on purpose: refill rate and refill delay are not nodes, because shortening a wait is
the idling lever returning through a side door; and no node touches the slow, because the slow is the
price.

**The Follow-Up (18) stays where it is**, gated on Crowbar Force and Pulse Recharge. It is now a Melee ×
Juggernaut link, which is exactly what a cross-gated node is for.

##### How it was built, 2026-09-16

- **A decaying armour grant has to sit above the cap.** Every armour ceiling goes through `PlayerMaxArmor`
  (the rule in [instructions/04](../instructions/04-CUSTOM-FEATURES.md)); a grant that respects it does
  nothing for a player at full armour, so the grant is explicitly allowed above it and decays back down.
  The armour bar is drawn against the maximum the client is sent (`gmsgBattery`'s second short), so an
  over-cap value needs a HUD answer; the tint is the start of one. **Built**: the grant is tracked as its
  own float beside the armour and fades over the Matrix's duration; a hit that eats through it shrinks
  what is left to fade, so it is fuel rather than a refill. The bar clamps at the cap and the number shows the excess.
- **The hold.** The Pulse is an impulse, which is edge-triggered and self-clearing; "held for one second"
  needs the press *and* the release, which an impulse does not carry. Either the client sends a second
  impulse on release, or the Pulse moves to a `+pulse` / `-pulse` command pair like `+inventory`. The
  second is the honest shape. **Built as both**: `+pulse`/`-pulse` on the client, sending impulse 150 on
  the press and 152 on the release (`game_shared/pulse_defs.h`), so the timing stays a button's and a bare
  `impulse 150` bind is still a tap. A release in the same frame as its press waits one command.
- **The share.** Armor Expert already scales `ARMOR_RATIO` (the share that gets *past* armour) through a
  cvar; the Matrix is a second, larger scale on the same number while it is up, applied in the same place.
  **Built that way, and reworked the same day after the first play**: scaling the stock split from 20% to
  5% through could not be read even with `debug_damage` on, since 80% of a hit already went to armour, and
  it was no defence. Now **nothing reaches health while the Matrix stands** and armour pays for the whole
  hit at `skill_matrix_armor_cost_scale` (0.5) per point — health frozen, the armour figure draining, a
  point of armour worth two of health. The alternative, a flat damage cut on top of the stock split, was
  not tried: it would have been readable only on the armour number too.
- **The slow** is one `pfnSetClientMaxspeed` call on raise and one on drop, restored on spawn and on
  restore. **Built** as a cap applied on change from the state, so spawn, restore and a cvar edit need no
  bookkeeping of their own.

##### Still open

- Every number: the share, the slow (20% is the starting guess), 6 s, 10 s, the grant (100 is "to be
  toned down"), Ricochet's chance, Matrix on Kill's amount (15, a first guess). All cvars. The grant's
  decay is derived — it fades over the Matrix's duration and is gone as it drops (tuned in play
  2026-09-16 from a 10-per-second first guess).
- **Low priority, later:** whether Ricochet wants a Stat node of its own to raise its chance.
- Whether Ricochet's tracer is the gauss's or its own.
- The Matrix's sounds and its raise light are placeholders ([ART_DEBT.md](ART_DEBT.md)), and it has no
  icon of its own. Its readout exists: a bar beside the Pulse's, an edge tint while up and a ready chime,
  since the first play.

Settled in the first play (Andrei, 2026-09-16), so not open: a save mid-Matrix comes back with it
standing, since the key is not held through it (up and its end time are saved); and the layout was
redrawn — Ricochet off the road to the Matrix, the Pulse block west, the Matrix trio east and
disconnected from it, in [SKILL_MAP.md](SKILL_MAP.md).

#### Medical

**Shape: Built, whole.** The region, Med Expert as its root, the four Healing Stat nodes (Potency's
ranks, as the matrix turns ranks into roads), Leech and Overheal on 2026-09-14, and Last Stand on
2026-09-15 once its one question was answered — the player is held at 1 health, invincible for 3 s, the
Syringe fires, 60 s cooldown; the killing blow does not land and race the Infusion. See
[SKILL_TREE.md](SKILL_TREE.md#medical) and [PILLARS.md](PILLARS.md). Shaped 2026-09-13, the last of the
seven. The smallest Route, and deliberately: with Regeneration (10) cut alongside Battery
Regen, **there is no passive healing in it at all**. Every node is on an action: using a thing, hitting a
thing, or being about to die.

What exists under it: Med Expert (19), the Infusion and its three cvars (`dlls/player_infusion.cpp`),
[ADR-0007](adr/0007-the-infusion-is-one-at-a-time.md), the medkit's single heal value
(`gSkillData.healthkitCapacity`), and the alien chainsaw as a design intent under the
[melee alien grunt](#melee-alien-grunt).

##### The nodes — settled

| Node | Effect | Note |
| --- | --- | --- |
| Med Expert (19) | An Infusion runs longer | Exists. **The root again**, where it started before Regeneration was made its parent |
| Potency | An Infusion heals more per second, **and** a medkit heals more | New. Ranks. `infusion_rate` where the tick lands, and the medkit's one value. Name provisional |
| Overheal | A Syringe used at full health raises health **above the maximum** for the Infusion's length, then it decays back | New. ADR-0007 already lets a Syringe be used at full health and today that is a waste; this makes it a decision. The Juggernaut's decaying armour grant is the same shape |
| Leech | Melee hits heal a fraction of the damage dealt | New. All melee weapons, not only the chainsaw: **the chainsaw's lifesteal becomes its own base property**, and Leech is what every melee weapon gets. Medical × Melee, the sustain the glass-cannon build lacks |
| Last Stand (49) | A hit that would kill the player leaves 1 health and 3 s of invincibility, **spends an unused Syringe from the Inventory automatically** and starts the Infusion at once, on a 60 s cooldown; and **all Infusion healing is doubled while health is below 50** | The Major. The only node in the tree that spends an item for the player. The below-50 doubling is what makes it felt before the day it saves anyone: a Syringe used while low is already the Route's best heal. Glass Cannon arms it permanently |

**Cut:** *Field Medic* (medkits and wall chargers heal more; folded into Potency for the medkit half, the
charger left alone) and a *reserved Station slot* ([Stations](#stations) do not exist; when they do, a node
is a table row).

**One rule Last Stand inherits, for the tooltip:** the Infusion is one at a time (ADR-0007), so Last Stand
fires only when no Infusion is running. A player already infusing when the killing hit lands is not saved
by it. That is the ADR's reasoning holding, not a gap: the alternative is stacking Infusions, which it
declined.

##### Still open

- Every number: Potency's ranks, Overheal's ceiling and decay, Leech's fraction, Last Stand's 3 s and
  60 s.
- Whether Last Stand should also fire for a Syringe that is *dropped* rather than in the Grid. No: it reads
  the Inventory, and that is the point of carrying one.

#### Alien

**Shape: Building since 2026-09-16.** Shaped 2026-09-13 in a grilling session that turned a speculative note
into the design below; the first slice — Cores, the Module, the Hive nodes and the summon's left click —
built 2026-09-16, untested in game ([PILLARS.md pillar 2 and 3](PILLARS.md#2-enhanced-combat)). The Route
absorbs the reserved [alien column](PILLARS.md#wanted-the-alien-column), ids 20 and 21. It is the
one Route built on a new **Module**, and the Module came out of the grill as the larger half of the design.

##### The alien Module — built 2026-09-16, untested in game

- **The summon is a Module**, the fourth after Pulse, Dash and Hook, and it is what the
  [freed alien slave](#friendly-alien-slave) gives (confirmed 2026-09-17). **Handed over by a stand-in
  pickup, `item_alienmodule`**, until his fight exists. A weapon whose right click does nothing reads as
  broken, so both verbs should work from the hand-over and the Route is what makes them grow — but only
  the left click exists so far.
- **The Module and its ammunition serve several weapons. Marked for refinement.** The Module is a platform:
  gaining it grants access to alien weapons that run on **Cores**, and the summon weapon below is the first
  of them. What the others are is not decided (the alien chainsaw is the obvious candidate). The
  consequence is that "the Module" and "the summon weapon" are two things, and the hidden-until rule below
  keys on the Module, not on any one weapon.
- **Cores are the resource: built.** A real **ammo type** (`CORE_MAX_CARRY` 6), so the HUD readout, the
  carry ceiling and the pickup all come from the engine's ammo path; `item_core` gives one, standing in on
  `w_gaussammo.mdl`. **Found in the world only, finite**; the hand-over gives `SUMMON_DEFAULT_GIVE` (3).
  Not made by the slave, so that "finite" stays true and the lab is not a place the player treks back to
  for ammo. A Station that converts something into a Core is the shape a renewable source would take, if
  one is wanted.
- **No new binds.** The verbs live on a weapon: **left click summons, right click is the ultimate.** Two
  impulse binds were considered and rejected as flooding the keyboard.

##### The summon weapon — left click built 2026-09-16, untested in game; right click not built

- **Left click summons one ghost** for one Core, on a cooldown (`summon_cooldown` 3), up to a maximum out
  at once (`summon_max_ghosts` 1) — built. A refusal costs nothing.
- **A ghost appears near the player on an eligible surface — built.** Spawning at the crosshair was
  rejected: it puts the ghost against a wall, and possibly far from the fight the player is in. The test
  is a human-hull ground trace at 64 then 96 units behind or beside the player, plus a clear line from the
  eye; with none found (the vent case) the summon is refused outright and the Core is kept.
- **A ghost is the stock alien slave on the player's side** with a lifetime (`summon_ghost_lifetime` 30)
  and no corpse — built, as `monster_ghost_slave`, translucent. **Following the player when idle is not
  built**, so the pack does not yet move between rooms with the player. The timer ends it; death ends it
  early, with no refund. `CISlave` gained `m_bAlly` and `m_bVanishOnDeath` as shared, saved flags so
  whatever makes the [friendly slave](#friendly-alien-slave) an ally, when it exists, can reuse them.
- **Right click is the ultimate — not built.** It **fills the pack to the maximum**, one Core per ghost it has to create,
  and **refuses if the player cannot afford the fill**, so it fires at full strength or not at all. Every
  ghost is **teleported to the player's left and right, never in front or behind**. They **charge a zap the
  way a slave does** and **hold it for 3–5 seconds**; **left click fires them all** through the player's own
  aim, or the timer does. **The volley is the ghosts' last act**: they vanish as it leaves, so the ultimate
  is "everything I have, now", and a player who wants a pack afterwards summons again.
- **The player is ethereal for the hold**: takes no damage, drawn translucent, can turn and walk to re-aim,
  cannot fire or switch (the weapon refuses to holster). Monsters still see and shoot; the shots do nothing.
  Ethereal ends the instant the volley leaves. The wider version, passing through hits and dropping off
  monsters' perception, was considered and rejected as engine work for no play gain.
- **No time freeze.** Dropped: GoldSrc has no time scale that spares the player, and the hold-and-release
  gives the aiming time a freeze was for. The Dash Route's bullet time no longer has a sibling here.
- **The Pulse redirect is out of the loop.** The idea (fire the volley at the player behind a standing
  Shield and let the Discharge vent it at the crosshair) is exactly [ADR-0006](adr/0006-the-discharge-vents-at-the-crosshair.md)
  with the cap lifted, and it was judged too complex for play. It becomes **one scripted set piece at the
  end of the game**, outside the loop. The [Nihilanth](#the-nihilanth) entry is where it belongs.

##### The Route's nodes — settled

Seven, sized like the other Routes. The Route sells *more ghosts, longer, sooner*, not the same number
bigger: ghost zap damage is deliberately not a node, because the Energy Route already sells damage.

| Node | What it does | Note |
| --- | --- | --- |
| Hive Capacity (20) | Hivehand holds more hornets | **Built 2026-09-16** |
| Hive Replenish (21) | Hornets replenish faster | **Built 2026-09-16**, as `HiveRegrowth`; display name changed, id did not |
| Hornet Replenish Stat ×11 (146–156) | +5% hornet replenish each, additive within the stat | **Built 2026-09-16**. The roads |
| Hive Attack Speed (141) | Hivehand fires faster | **Built 2026-09-16**. The cadence is predicted, so it is a both-sides node through `skill_tuning.h`, like Fast Reload; the fire animation itself is not sped up |
| Pack (142) | Maximum ghosts out, 1 → 2 → 3 | **Built 2026-09-16**: one rank, `+1` rather than scaled. The ranks shape is still one node today |
| Tether (143) | Ghost lifetime longer | **Built 2026-09-16** |
| Recall (144) | Summon cooldown shorter | **Built 2026-09-16** |
| Major node (145) | The volley is energy damage | Placed 2026-09-15; no effect — it depends on the ultimate, which is not built. See below |

**The major node is the cross-Route hook.** A slave's zap is `DMG_SHOCK` (`dlls/islave.cpp:831`), which the
Gargantua ignores and the alien slave itself is immune to. As energy damage the volley scales with the
Energy Route and passes the Gargantua filter, so **Alien × Energy is a second endgame build** beside
Melee × Energy.

**Dropped:** *Poise* (a longer hold window) and a *Snark node*. **Snarks may leave the mod entirely**: buggy,
frail, and outclassed by the ghosts. Not decided; recorded.

**The whole Route is hidden until the player gains the Module.** Hive nodes included: a player who found the
Hivehand hours earlier does not see them until the slave's hand-over. A gate per node (each Skill naming the
weapon that reveals it) was considered and set aside for the single gate. (It stays set aside *here*. The
Shinobi region has the first node gated differently from its neighbours, 2026-09-17: the Air Dash, behind
the double jump.)

##### Still open

- **Which other weapons the Module serves**, and what a Core-powered weapon is when it is not the summon.
  The refinement the Module is marked for.
- **The ultimate.** Right click, the teleport to left and right, the 3–5 s hold, ethereal, the volley — none
  of it is built; today right click idles.
- **The freed alien slave's actual hand-over.** `item_alienmodule` is a stand-in for it, and the boss fight
  it should follow is not built.
- **The summon weapon's name**, its HUD bucket, its viewmodel and world model. All [ART_DEBT.md](ART_DEBT.md)
  entries the day it exists — `sprites/weapon_summon.txt` still borrows the gauss ammo icon for a Core.
- **The ghost following the player when idle**, so the pack moves between rooms.
- **The endgame set piece** that inherits the Pulse redirect.
- **Do Snarks go?**
- Worth watching once this is played: a ghost's zap beam can hit the player standing in its line, and a
  ghost behind the player is solid and can shove them.

What already exists near the rest: `CTalkMonster` plus `CLASS_NONE` for the friendly slave, and the ally
relationship (`m_bAlly`, `m_bVanishOnDeath` on `CISlave`) built for the ghost should be one piece of code
shared with it; and the Pulse's `CanHolster`-style refusal for the ultimate's hold.

#### Energy

**Shape: Built, whole.** The region, Energy Damage as its root, the four Energy Damage Stat nodes (the
root's ranks, as the matrix turns ranks into roads), Energy Efficiency (Egon Efficiency until the
katana's wave spent uranium) and Insulation on 2026-09-14; Egon Focus (the SDK's narrow beam on right
click, as it is) and the Major, **Overdraw**, on 2026-09-15, when Quick Charge (57) was cut because the
wave has no charge. See [SKILL_TREE.md](SKILL_TREE.md#energy) and [PILLARS.md](PILLARS.md). Energy's
region is the south-west corner of the board, touching Melee's on the west edge ([SKILL_MAP.md](SKILL_MAP.md)).
Shaped 2026-09-13, in a grilling session. **The energy weapons are the katana and the egon**; the gauss
is probably removed. The Pulse's Discharge and the Alien Route's volley deal energy too.

##### What "energy" is — settled

**Energy is the damage type, `DMG_ENERGYBEAM`, and nothing else.** With the gauss gone every energy source
already carries it (the egon, `dlls/egon.cpp:271` and `:308`; the katana's wave, `dlls/katana.cpp:131`; the
Discharge, [ADR-0006](adr/0006-the-discharge-vents-at-the-crosshair.md); the Alien volley's major node) except
one, and that one changes: **the katana always deals energy damage**, slash and wave alike, and **scales off
both Melee and Energy bonuses**. That is the Gargantua build in one sentence, and it means the Route's
test is a damage-type check at the `ApplyMultiDamage` chokepoint, with no weapon list to maintain. A node
that *made* the blade energy was considered and rejected: it left the katana half a weapon until bought.

Two consequences, stated so they are not rediscovered: **an energy slash passes the alien grunt's
plating**, so the [melee grunt](#melee-alien-grunt) entry's "the crowbar cannot hurt an armoured grunt" no
longer describes the katana; and the type's identity is **energy ignores plating and is what the
Gargantua accepts**, which the Route's tooltips should say rather than a node.

##### The katana rework — settled, recorded under [the Gauss Katana](#the-gauss-katana)

Left click is a plain melee slash with no wave and no ammo; right click throws a piercing wave, for
uranium, whose damage travels with the projectile; there is no charge. Not a Route change, but the Route's
nodes are written against it.

##### The nodes — settled

| Node | Effect | Note |
| --- | --- | --- |
| Energy Damage | Energy damage dealt up | Root. Ranks 1 → 2 → 3. The katana, the egon, the Discharge and the Alien volley all read it |
| Egon Focus | Secondary fire unlocks the egon's **narrow beam** | Dormant code the SDK shipped: `CEgon::PrimaryAttack` hard-sets `FIRE_WIDE` and the narrow mode, single target, its own damage (`plrDmgEgonNarrow`) and ammo cadence, was complete and unreachable. Built as it is, 2026-09-15 |
| Energy Efficiency | Uranium drains slower | Ranks. One chokepoint, `CEgon::UseAmmo` (`:127`), the `DefaultReload` pattern. Also cheapens the katana's wave (built 2026-09-14), so it pays twice; renamed from Egon Efficiency that day |
| Insulation | Less energy **and shock** damage taken | Shock included so it means something in Xen: controller balls (`dlls/controller.cpp:1410`) and `env_laser`/`env_beam` hazards are energy; the slave's, controller's (`:1228`) and Nihilanth's (`dlls/nihilanth.cpp:1501`) zaps are shock |
| Overdraw | **Energy attacks drain armour as well, for bonus damage. Always on, never below a floor** | The Major. Below |

Cut: *Quick Charge* (57), which would have made the katana's charged wave charge faster; the wave has no
charge (2026-09-15).

**The major node.** Every energy hit spends armour and gets the bonus; the drain never takes the last
portion of the bar (20 is the starting guess), and below the floor the attack does base damage. No switch
and no mode: a player who does not want the trade does not buy the node. **It rewards two builds**, and
that is its purpose:

- **The Juggernaut**, with a deep armour bar and the Matrix's decaying grant to burn: raise the Matrix,
  fire the egon, and armour goes at an alarming rate for enormous damage. What was first read as the one
  *conflict* between Routes is the intended build; see the [Juggernaut](#juggernaut--resilient).
- **The glass-cannon "ninja"**, who dashes and slashes with the katana: all damage, no defence, and the
  little armour they carry is fuel. Melee × Energy × the Dash Route. *"Ninja"* is the first word anyone has
  used for the Dash Route and is noted as a candidate name, not a decision.

Held back on purpose: nothing that touches the Discharge specifically (Energy Damage already scales it);
nothing that changes what the Pulse does (the Juggernaut's); no "wave pierces" node (piercing is what the
wave does). Dropped from the idea list: a Siphon (chargers refilling uranium) and Discharge-as-Route-weapon.

`PlayerMaxArmor` still governs the drain's ceiling and the floor is the drain's other bound; both live
where the armour is spent.

#### Shinobi

**Shape: Shaped 2026-09-13**, in a grilling session that shaped the Dash Module with it, the way the Alien
grill shaped its Module; the Dash, Reprisal and the Air Dash built 2026-09-15, the base Dash in the air
2026-09-18. Named **Shinobi** on 2026-09-15: it was *Agility* until that collided with the Suit Variant
codename ([CONTEXT.md](../CONTEXT.md#the-suit)), which keeps the word, and *Ninja*, the glass-cannon
dash-and-slash build, was the candidate in between.

##### The Dash — settled here, for the [Modules](#pillar-3-modules) entry

- **A burst in the direction of movement, on a key press.** Not the long jump renumbered: the long jump
  throws the player along their *view* (`pm_shared/pm_shared.cpp`, 560 units/s forward with a vertical
  kick, jump-during-duck on the ground while moving); the Dash goes where the player is *moving*, so
  sideways and backwards dashes exist and it is a dodge as much as a crossing. **On the ground and in the
  air alike, from the start** (2026-09-17, built 2026-09-18): a Dash glued to the ground cannot cross a
  gap. "Ground only at first" was the 2026-09-13 shape.
- **The key is shift, tap only. Walk is rebound.** Shift is `+speed` in Half-Life, and this mod made
  walking matter (the noise multiplier, Concealment's stance term), so the Dash key is the stealth key. A
  tap-to-dash / hold-to-walk split was considered and rejected; walk gets a new default in the mod's
  config, and everything that says "hold shift to sneak" says the new key. On the wire it needs no button
  bit: the per-tick command's impulse field reaches the movement code, so a dash can ride the same packet
  the Pulse does and still be predicted. To be verified when built.
- **Charges and a cooldown.** One charge at the base, a cooldown between dashes: combat pacing, like the
  Pulse's Recharge, not idling. Built as a count from the start, the `m_iRebounds` shape, so a node raises
  the ceiling with one edit.
- **The directional Air Dash is the Major**: through it, in the air the Dash goes **where the player
  aims, upward included**. That is how high places get reached without touching jump height, which is
  what High Jump's cut asked for. Because the upward dash is the one tool that out-reaches the double
  jump, the node is [gated behind the double jump Module](#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate)
  (2026-09-17). The Hook remains the other answer, and is its own Module.
- **No bullet time.** Dropped: the engine's only time scale slows the player with the world, so a "bullet
  time" node would be the tree's first tooltip that lies, and a world-slows-player-does-not version is the
  freeze's cost again. If the feel is wanted it is presentation on the air dash, not a node.

##### The nodes — settled

| Node | Effect | Note |
| --- | --- | --- |
| Sure Footing (7) | Falls deal half damage | Exists. The root: a Route about being airborne starts with landing |
| Dash Reach | The Dash goes further | Ranks |
| Dash Recovery | The Dash comes back sooner | Ranks. The Alien Route's Recall, same shape |
| Second Wind | A second Dash charge | The count raised from 1 to 2 |
| Reprisal | **A one-shot melee kill refills a Dash** | A single melee hit that kills a monster that had not been hurt before: a Backstab kill counts, a Cleave opener counts, finishing a wounded grunt does not. Name provisional |
| Phase | **No damage taken during the Dash itself** | A dodge, not immunity: the dash lasts a fraction of a second. Where the glass cannon's defence comes from. One flag for the dash's duration. **The first to cut if it proves too strong**, since a dodge with no damage window makes every melee enemy a free hit |
| Major node | **Air Dash**: in the air the Dash goes where the player aims, upward included | The evolution, and the thing that reaches high places. Gated behind the double jump |

Held back on purpose: nothing that changes ground speed or jump height, since the Route's premise is that
the normal movement rules stay; nothing that touches the Hook. Kill-replenish alternatives considered and
set aside: a kill shortly after a dash (build-agnostic, but rewards nothing the Dash itself makes
special), a Backstab kill (Melee-only), any melee kill.

##### Still open

- The burst's speed and length and the cooldown are cvars (built 2026-09-15, see
  [PILLARS](PILLARS.md#the-dash--built-2026-09-15-untested-in-game)), every one a first guess.
- **The Air Dash, settled 2026-09-15:**
  - **It stops dead.** Gravity is off during the burst, and at the end speed drops to run speed with no
    upward carry. One straight up reaches about 120 units, the burst's length.
  - **Any ready charge can be spent in the air**, one after another, including a charge Reprisal refilled.
    Height is capped by charges, not by a once-per-jump rule.
  - **Downward is allowed and takes fall damage.** A dive into the floor lands as a fall at that speed, so
    Sure Footing matters. **To review:** straight down at the full 800 lands for about 60 (about 30 with
    Sure Footing), which proved brutal on first look, 2026-09-15. Candidates: a separate, slower air speed; a
    cap on the fall speed an Air Dash leaves behind; or no fall damage from a dive after all.
  - **Direction:** along the crosshair only; the movement keys do nothing in the air (revised 2026-09-15 —
    the key rotation was built and dropped). The ground Dash keeps following the movement keys.
- **Walk's new default key is ALT** (2026-09-15). The mod ships no `default.cfg`, so it lives only in the
  install's `config.cfg` for now.
- **Phase is built** (2026-09-20, not verified in game): every hit during the burst is refused except
  falls and drowning, so the dive's damage above is still a separate question. Recorded in
  [PILLARS pillar 4](PILLARS.md#4-skill-trees).

Settled, so not open: `item_longjump` in stock maps keeps its classname and gives both the long jump and
the Dash (2026-09-15); acquisition is walk-over, like every pickup since
[ADR-0011](adr/0011-pickups-are-walk-over.md).

#### Weapon Specialist

**Shape: Built 2026-09-14, whole**, node by node in five commits, on the layout in
[SKILL_TREE.md](SKILL_TREE.md#weapon-specialist); what is built is in
[PILLARS.md](PILLARS.md#2-enhanced-combat). Every number is a first guess in a cvar
(`instructions/04`), Swap Surge has no readout yet, and the placement on the matrix was the agent's call
from the graph below, to be judged by eye. The text below is kept as the reasoning. Shaped 2026-09-13, in
a grilling session. The Route for the player who uses the whole arsenal: handling speed, typed damage,
and a major node that makes swapping weapons the way to fight.

##### The nodes — settled

Seven. Two exist, one is unblocked, four are new.

| Node | Effect | Note |
| --- | --- | --- |
| Marksman | Bullet damage up | New. **The root**: bullet damage is what most of the arsenal does and the natural first buy. `DMG_BULLET` at the `ApplyMultiDamage` chokepoint. Ranks |
| Fast Reload (3) | Reloads quicker | Exists. Ranks. **Rank one covers the shotgun**, which feeds shell by shell and dodges `DefaultReload` today; a reload Skill that skips one gun reads as broken |
| Quick Draw | Weapons come up faster | New. `DefaultDeploy` is the same chokepoint shape as `DefaultReload` and is predicted, so both sides through `skill_tuning.h`. Ranks |
| Weapon Mastery (4) | All weapons +10% | Exists. Moved **deeper** into the Route: the "everything" node after the typed ones, not the toll gate in front of them |
| Demolitions | Explosive damage dealt up, **explosive damage taken down** | New. `DMG_BLAST` at the `RadiusDamage` direct branch, and the same type on the way in. Ranks. The resistance covers the player's own grenades, which is how the "Mastery makes your own explosives hurt you more" accident is answered: not with a guard at the chokepoint, but with a node worth buying for the resistance alone |
| Headhunter | Headshot damage up | New. One place, the head hitgroup multiplier at `dlls/combat.cpp:1582`, for player-inflicted hits. Ranks. **Designed together with [Decapitation](#pillar-2-decapitation)**, which keys on the same hitgroup; the Panthereye's head is hitgroup 2 and needs remapping first |
| Major node | **Swap Surge**: for **1–2 seconds after a weapon swap, everything the weapon deals lands harder**, on an internal cooldown | New. A window rather than a single empowered shot so that the egon and MP5 get their burst as much as the shotgun and python get a big first shot. It is what makes Quick Draw a build rather than a convenience: the specialist juggles weapons, and every swap is a hit. Name provisional |

**Cut:** *Bandolier* (ammo carry ceilings). Ammo scarcity is a level-design lever and a Skill that loosens
it works against the mapper the moment maps exist.

##### Filed elsewhere

- **The animations.** A faster reload or draw with the stock animation reads as the animation being cut
  off, which is what happens at 0.8× today ([weapon handling](#weapon-handling)). Per-tier reload and draw
  sequences are model work per weapon per tier. They are an [ART_DEBT.md](ART_DEBT.md) entry the nodes
  create, not a promise the nodes make; the nodes ship first and read as "faster" until the art exists.
- **Skills or Evolutions?** Answered by this Route: **numbers are Skills, identity is an Evolution.** A
  faster reload is a node here; a silencer or a second barrel is an [Evolution](#weapon-evolutions).

##### Still open

- Every number, and the Surge's window and cooldown.
- Headhunter against Decapitation's damage floor: whether a Headhunter rank makes decapitation more
  likely, or only the hit bigger.

#### Melee

**Shape: Built 2026-09-14, whole.** The first Route on the matrix and the worked example for the other
six: Reach, Force, Speed, the Backstab node, Cleave, nine Melee Damage Stat nodes as its roads, the crowbar
names retired, the halving rule dropped. What is built is in [PILLARS.md](PILLARS.md#2-enhanced-combat)
and the layout is in the comment above `k_SkillDefs`. What remains here is the roster weapons below, which
are weapons rather than Skills, and Cleave's numbers, which are first guesses. The text below is kept as
the reasoning.

##### The roster — settled

**The crowbar stays**, and the roster grows on its base: several melee weapons, each cheap, each leaning
one way.

| Weapon | Lean | State |
| --- | --- | --- |
| Crowbar | All-round, good stats | Exists |
| Gauss Katana | The ultimate melee weapon; always energy damage, scales off Melee *and* Energy | Exists, [reworked](#the-gauss-katana) |
| Carbon Pickaxe | Slower, stronger | Exists, v1 ([entry above](#the-carbon-pickaxe)) |
| Knife | Higher Backstab base | New |
| Pipe wrench, others | Maybe | Named as the kind of thing, not committed |

Each is `CCrowbar` with a few numbers overridden, which the katana already proved cheap: `BaseDamage`,
`SwingDelayScale`, and now a **per-weapon Backstab base** beside them, so the knife's lean is one hook. The
Inventory makes the roster self-limiting for free: every weapon costs three Cells, so carrying four melee
weapons is a Row decision, which is pillar 5 doing its job.

**The nodes went generic in name**: Melee Reach, Melee Force, Melee Speed. Ids 1, 2 and 11 are frozen; the
C++ enumerators and display strings changed, which was free. The crowbar is not replaced, so the rename
happened once, for the roster, not for the pickaxe.

##### Valve's rapid-swing halving is dropped — settled

The base game halves every crowbar swing that comes within about a second of the last one
(`dlls/crowbar.cpp:259-269`, "subsequent swings do half"). The katana inherits it, so chained katana
swings are 20 rather than 40. **Every swing now does full damage.** Three reasons, in the order they came
up:

- **It made the tree unreadable.** A player checking whether Force worked sees 15, then 7.
- **It made the speed node dishonest.** Speed only shortens the gap *inside* a chain, and every swing
  inside a chain was half, so the node bought faster half-hits and never a faster full hit; a player who
  fights approach-and-strike got nothing from it.
- **It would have interacted with the katana's right click by accident**, since the halving keys on the
  left-click timer alone.

Sustained melee damage per second roughly doubles for a player holding the button; that is absorbed by
retuning base damage and swing time, both already cvars. The roadmap's earlier claim that a faster cadence
"silently makes every swing a follow-up" was wrong: Valve's test is "was there a pause", which a shorter
interval does not change. **The word "follow-up" for Valve's subsequent swing is retired**; in this mod
**Follow-Up** means Skill 18 and nothing else.

##### The nodes — settled

| Node | Effect | Note |
| --- | --- | --- |
| Melee Reach (1) | Swings connect from further | Exists as Crowbar Reach. Unranked |
| Melee Force (2) | Hits land harder | Exists as Crowbar Force. Ranks |
| Melee Speed (11) | Swings come faster | Back from reserve; with the halving gone there is nothing to untangle. Unranked |
| Backstab | The rear-arc multiplier: **3× base, ranks raise it to about 5×** | New. Multiplies each weapon's own Backstab base, so the knife climbs highest. 3× stays free for a player with no Melee nodes, so getting behind things is worth doing from hour one |
| Follow-Up (18) | After a deflect, the next hit lands far harder | Exists, gated on Force and Pulse Recharge. The Melee × Juggernaut link |
| Major node | **Cleave**: the first hit after an internal cooldown hits everything in its arc **and lands harder**; every hit until the cooldown elapses is normal | New. An opener, which rewards approach-and-strike over holding the button, like the rest of the Route |

**The never-noticed Backstab tier** (a larger multiplier when the victim never acquired the player, deferred
under [stealth](#deliberately-deferred)) is **a Stealth node, not a Melee one**: it gives the reserved
stealth ids their first real Skill, and a Melee × Stealth build beside Melee × Energy.

Held back: a chain reward (dropping the halving gives that for free) and cross-Route payoffs, which belong
to the Route that receives them. A Backstab kill refilling the Dash is the Dash Route's "under certain
circumstances", not this one.

##### The Gargantua build, restated against the Route

Force × Backstab × Mastery × Follow-Up is 49.5 today, tuned to *not* one-shot a grunt at 50. The Backstab
ranks and the Energy Route's ranks are where that ceiling climbs, on a katana that is energy and multiplies
off both. [ADR-0010](adr/0010-the-backstab-is-positional.md)'s note about the blade being club is now
history the moment the rework lands.

### Seven Routes, seven columns, and what is not in one

The count matches the column count by accident and should not be read as a layout. What is outside every
Route:

- Nothing, as of 2026-09-13. Sure Footing (7) is the Dash Route's root and Fortitude (8) the Juggernaut's.
- **The Pulse's timing branch** (Window 12, Recharge 15, Discharge 16, Rebound 17) was the other candidate
  and is **inside the Juggernaut** as of 2026-09-13, so that one Route holds the whole of the Pulse.

### Infrastructure notes from the same session

Facts found while sizing a bigger tree, so they are not found twice. All of the infrastructure is built
and recorded in [PILLARS pillar 4](PILLARS.md#4-skill-trees); what stands as a rule:

- **The id ceiling is 256** (`k_SkillIdCeiling`, 2026-09-14; 96 the day before). It sizes the saved
  array (`m_bUnlocked256`, so an older save resets rather than over-reads) and the 32-byte sync mask,
  length-checked on both sides from the same constant, and adding a Skill changes neither. Ranks cost ids,
  since under the matrix a rank is a Stat node on the road drawn like any other.
- **The economy.** One pickup is one point. The tree is deliberately not completable (2026-09-14): about
  140 nodes against 100 findable points, 40 on the critical path and 60 in optional spaces, which
  reverses the stance PILLARS pillar 4 first recorded and PILLARS says so. The numbers stand as set, and
  no map is waited on to judge them (2026-09-15, [Maps](#maps)).
- **The sum, once all seven were shaped**: Juggernaut 11, Alien 7, Weapon Specialist 7, Dash 7, Energy 6,
  Melee 6, Medical 5, 49 Skills with ranks counted once, before the Stealth Route added seven.
- **The board.** 15×15 at 1:1 with drag to pan; one 112-pixel step on both axes, square nodes of 36 / 50
  / 62 / 74 with the processor at 92; `ENodeTier::Stat` below Minor; icons fitted into layout-sized
  nodes, so node size follows the layout rather than the art; `skilltree_step` and
  `skilltree_preview_cols` / `_rows` for judging a footprint by eye. A `static_assert` that no two rows
  share a cell (`SkillDefsOnePerCell`) and the `skilltree_debug_edges` overlay guard the hand-placed rows
  in `k_SkillDefs`, which is where the mistakes will live.

### Open questions

- **The other three hub corners.** Follow-Up takes the Melee–Juggernaut corner; the Juggernaut–Specialist,
  Specialist–Medical and Medical–Melee corners are open for cross-Route Skills.
- **The hub's two stat sizes**, to be set when the hub's Minors are judged in play.
- **What is left to build in the Routes**: the Alien ultimate and the ghost following the player, the
  Air Dash's dive damage, the charge and chainsaw the melee grunt waits on, and every number.

Answered, so not open: a Route is drawn as a region with a faint colour wash over a circuit substrate,
its name in the tooltip only (2026-09-15); every road stat is chosen (SKILL_TREE.md); the suit node costs
0 against the cost-one `static_assert` and is held on spawn and after a Reset; the build order was Melee
then Weapon Specialist (2026-09-14), the rest as their needs allowed; the Dash Route is Shinobi; bullet
time is dropped (2026-09-13); the Pulse's timing branch lives inside the Juggernaut (2026-09-13).

### Done when

A player can describe their build in a sentence, and two players with different builds kill the same
Gargantua in visibly different ways, one of them with a knife.

---

## Pillar 1: Records

**Shape: Built, whole, 2026-09-18, and verified in game the same day** — the Prompt on everything usable,
Records read in the world and kept in a fourth tab, Guidance, locks and the mapper's controls; recorded in
[PILLARS pillar 1](PILLARS.md#1-exploration). Shaped 2026-09-17 in two grills, the game as a whole and then
this feature on its own. What is left here: audio, the reader's styling, and the stand-in document model
([ART_DEBT.md](ART_DEBT.md)). Renamed from *Transmissions* the same day:
the feature stopped being audio logs, and a clipboard is not transmitted. **Entry** was the working word
and is taken ([CONTEXT.md](../CONTEXT.md): one occupant of an Inventory), so the thing is a **Record**,
the tab is **Records**, the suit **registers** a Record in its memory, and *Transmission* narrows to a
category of Record — what was radioed, intercepted or, later, voiced.

**Text first, audio later.** Audio logs and subtitles are wanted as a later step; a Record's format leaves
room for a `sound` field, and its body is then the transcript — which is also the answer to the engine
having no subtitle system. **Show, don't tell comes first**: the world carries the big facts, Records the
specific ones. **The Nihilanth's contact is never a Record** — not registered, not re-readable.

### The Prompt

**A rule for the whole game, 2026-09-17: everything that can be interacted with gets text on screen** — a
title (*Battery*, *Terminal*, *Door lock*) and the action or actions under it (`[E] Take`,
`[E] Enter code 4471`). It generalises the Pickup Prompt and keeps its best property.

- **It appears in the vicinity, exactly as the Pickup Prompt does**: inside use reach
  (`INV_PICKUP_RADIUS`, 64, matching `PLAYER_SEARCH_RADIUS`) and the narrow view cone.
  `FindLookedAtPickup` (`dlls/player_inventory.cpp`) mirrors `PlayerUse`'s aim test and **already weighs
  every usable entity** — when one wins it reports nothing, so that a use press at a button never grabs the
  medkit behind it. The rule is mostly reporting that winner. The server still answers "what would a use
  press do here?" once, so the Prompt and the press cannot disagree.
- **Titles default by class** (*Button* / `Press`, *Health station* / `Heal`, *HEV charger* / `Charge`, a
  scientist / `Talk`), **a mapper overrides them** with `prompt_title` and `prompt_action` on any entity
  (*Pump control* / `Start pumps` — short strings fit a 192-byte message), **and can suppress the Prompt**.
  Suppression is not optional: vanilla's classic secret is the unmarked usable panel, exploration is ranked
  first, and a label on every hidden switch spoils it.
  **Built 2026-09-18.** All three keys work on every entity, read in `DispatchKeyValue` rather than in
  `CBaseEntity::KeyValue`, because too many KeyValue overrides in this SDK never chain to their base.
  Suppression hides the label only — the thing still works when pressed, which is the entire point of
  being able to hide one.
- **A state line is how a hard gate looks impassable**: *Elevator — No power*, *Door lock — Code required*.
  **Built 2026-09-18** with `record_lock`, the first thing that needed one: a third field on
  `PromptClassDef`, drawn *instead of* the bound key and its action. A door that offers `[E] Open` and
  then does nothing teaches the player that use presses are unreliable; one that says *Code required*
  teaches them to go and find the code.
- **The key shown is the real binding** of `+use`, which the Status page's tooltips already look up.
- Walk-over pickups keep [ADR-0011](adr/0011-pickups-are-walk-over.md) and their `Take`. Breakables,
  monsters and scenery get nothing. A [deposit](#mining-and-crystal-shards) gets a hint line — *Crystal
  deposit — Mining tool required* — because that teaches the mechanic.
- The Pickup Prompt is drawn in the engine's console font (PILLARS lists it as unstyled); widening it is
  the reason to style it. **Prompt** goes to CONTEXT.md when built, with the Pickup Prompt as its oldest
  case; the glossary's "avoid *use prompt*" is why the word is bare.

### What a Record is

- **The text lives in a data file in the mod directory, read by the client**: `records.txt`, keyed blocks
  in the manner of `titles.txt` and `hud_additions.txt`; no JSON, the client has no parser for it. A user
  message caps at 192 bytes, so text cannot cross the wire. A map entity carries an id; **the server owns
  a saved found-set and syncs it as a mask**, the Skill pattern. **512 ids, sized once** — the Skill
  ceiling moved twice and reset saves each time. Ids are stable once added and never reused. A missing id
  draws as "record not found" with a console warning; a parse error names its line and skips the block.
  The repo copy is the source of truth, copied to the install by hand like `sprites/`. A compiled table
  like `skill_defs.h` was rejected: prose gets rewritten constantly and should not cost a rebuild of two
  DLLs. Text in map keyvalues was rejected: chunked messages, keyvalue limits, and a typo costs a compile.
- **Per Record**: a numeric id (the bit), a string id, a category, a title, a one-line **source**
  (*Medical file, Level 3 infirmary*), an optional `revocable` flag, a plain-text body. **One emphasis
  marker**, drawn in the suit's colour, for codes and for `[REDACTED]` — how the cover-up shows in the
  documents. No images in the first version. **Short documents**: they are read in real time in an unsafe
  world, so a long record is a series found apart.
- They occupy no Cells — the Reset Token pattern argued for below.

### Reading one

- **Two forms, both read with +use**: a loose document (a model) and a fixed source — a terminal, a wall
  notice, a roster — which is the mapper's brushwork. One entity class, two looks.
- **Reading must be intentional.** A document absorbed by walking over it might never be read. This is a
  **deliberate exception to [ADR-0011](adr/0011-pickups-are-walk-over.md)**, with its own reason: that ADR
  is about *taking*, and a Record is *read* — attention, which is what +use already means for a scientist.
  Written down with the build, so nobody fixes it back.
- **A press registers the Record and opens the reader at once, in real time.** The Inventory Panel does
  not pause the game and neither does this. **Taking damage closes the reader**; the Record is already
  registered and nothing is lost. One reader serves the world and the tab. When it closes, a short
  "Record registered" line in the suit's voice. Bank-and-notify was rejected as the walk-over problem one
  step removed; a pause was rejected because GoldSrc only offers the console's.
- **Both forms stay in the world** — the suit scans, nothing is taken — and re-open on every use.
  **An unread Record glows**, and the glow goes out once registered. If it glows, it is unread. The
  word was settled 2026-09-18, in build: *glint* was tried and describes the wrong thing — a glint is a
  spark catching the light, and this is the document itself softly lit. The progression pickups' dynamic
  light (`cl_dll/entity.cpp`) was the first attempt and was **rejected**: a dlight lights the room, which
  reads as a beacon. A loose document wears a `kRenderFxGlowShell`; a brush one, which the studio
  renderer never sees, gets a `kRenderGlow` halo sprite. The found-set lights
  it, so the entity has no state of its own to save.

### The tab

Categories on the left, from the file, found order inside each; the reader on the right; a pinned
**Guidance** section on top. A fourth row in the tab table in `cl_dll/vgui_inventory.cpp`.

- **Guidance lines are ordinary Records in a `Guidance` category**, granted and revoked by a triggered
  point entity, `record_grant`. "Done" is a revoke, optionally granting the next; nothing fails. Written in
  the advisor's voice, so the vortigaunt's improving English shows here too. **Revoke is for Guidance
  only**: a found document never leaves the suit's memory. A separate objectives system was rejected as a
  second thing to save, sync and debug.
  **Built 2026-09-18, and the revoke rule is structural rather than a convention a mapper has to keep.**
  The server cannot ask `records.txt` which category a Record is in — that file is the client's — so
  `CPlayerRecords` carries a second saved mask of which ids arrived *by grant*. `Forget` refuses anything
  else; a grant that adds nothing marks nothing, so it cannot launder a read document into a revocable
  one; and reading clears the granted bit, because reading is finding. Pointing a revoke at a real
  document therefore does nothing at all, without a parser and without a rule to remember.
- The same entity hands over any Record at a scripted moment, and **a Record's first read can fire a
  target** — how one sets a remembered global for the endings with no code that knows about endings.

### A Record can open something

`record_lock` names a Record. On use it fires its target if the suit has that Record; the Prompt reads
*Code required* without it and `[E] Enter code 4471` with it. **No typing**: GoldSrc has no keypad and
VGUI1 text entry is awkward enough that the Inventory Panel avoids it. It is the
[soft-gate rule's](#the-shape-of-the-game) first reusable form — `soft: intended Record X, alternative Y`,
the alternative being the mapper's vent or window. It accepts a granted Record as readily as a found one.
Knowledge costs no Cells, where a keycard item would tax a scarce Inventory. Accepted knowingly: a code
remembered from a previous run still has to be found again; a typed keypad can be added beside the lock
later without changing it.

**Built 2026-09-18** as `record_lock` and `record_lock_brush` — the brush form is the keypad a mapper
builds, and it is the one that will get used. A press without the Record plays vanilla's access-denied
beep; with it, the target fires. **The state line arrived with it**, as a third field on
`PromptClassDef`, drawn instead of the bound key and action: *Lock / Code required*, then *Lock /
`[E] Enter code`*. The server picks which face to show, because only it holds the found-set, so a lock
can never offer an action it will not honour — and that is now the general mechanism every hard gate can
use, which slice 4's keyvalues hang off rather than invent.

**A door needs no `multisource`.** `CBaseDoor` already refuses to be walked into once it has a
`targetname` (`dlls/doors.cpp:540`, "if door is somebody's target, then touching does nothing"), and it
only gains `FCAP_IMPULSE_USE` when its *Use Only* flag is set. So a named door with that flag off is
inert: no touch, no use press, no Prompt of its own. The keypad beside it is the only way through, which
is two entities for a locked door instead of four.

### How it was built, 2026-09-18

Four slices, each judged in game on its own, all verified by Andrei the same day.

1. **The Prompt, widened** — defaults by class, the real key, judged in a vanilla map.
   `game_shared/prompt_defs.h` is the table, the server classifies in `dlls/player_inventory.cpp`, the
   message gained a third byte. The mod's own pickups that never reach the Grid (Skill Point, Reset Token,
   Row Grant, the Modules, the silencer, the Core) and all ammunition get a Prompt and a use press takes
   them, through a new `EEntryKind::Pickup`. Found on the way: `Key_LookupBinding` wants the command
   without its `+`, which had the Status page's Pulse tooltip reading `[UNBOUND]` — fixed. Still the
   console font.
2. **Records, the core** — the file and parser, the found-set and its sync, the world entity and its
   glow, the reader, the tab; the first code pillar 1 ever had.
   `records.txt` at the repo root, copied to the mod directory by hand like `sprites/`,
   parsed line-by-line by `cl_dll/records.cpp` (a tokeniser eats the blank lines that make paragraphs)
   and reloadable in place with `records_reload`. The found-set is `CPlayerRecords`
   (`dlls/player_records.*`), 512 ids, saved as a `"RECORDS"` block and synced as a 64-byte mask.
   `dlls/record.cpp` is the entity, under two classnames — `record` and `record_brush` — because an FGD
   cannot declare one class both `@PointClass` and `@SolidClass`; a loose document drops to the floor
   unless its Fixed spawnflag is set. The reader is one implementation
   (`cl_dll/vgui_record_reader.h`) with two hosts: a centred panel that takes **no input at all**, so the
   player keeps moving and looking while reading, and the Records tab. Prompt row is *Record* / `Read`.
   Cheats: `record_spawn <id>`, `record_forget`. Found on the way: **the glow is the object, not the
   room** (three attempts — see "Reading one"), and tuning cvars have to be re-read on a clock, because a
   look cannot be judged through a console command.
3. **`record_grant`, Guidance, `record_lock`**, and the first-read target. All three, plus the Prompt's
   **state line**, which the lock needed and which every
   hard gate now inherits (see "The Prompt" and "A Record can open something"). `record_grant` grants,
   revokes, or both in one trigger, with an Only-once flag, and fires its target whether or not anything
   changed so a Guidance chain cannot stall on a line the player already had. The first-read target is on
   `record` and `record_brush` and fires once, because a Record stays in the world and is re-readable.
   Cheats: `record_grant <id>`, `record_revoke <id>`.
4. **The mapper's controls** — the three prompt keyvalues, the FGD (and its sync rule), CONTEXT terms
   (Record, Prompt, Guidance), the ADR-0011 exception written into the ADR itself, PILLARS pillar 1, an
   ART_DEBT line for the stand-in document model.
   `prompt_title`, `prompt_action` and `prompt_suppress` work on **every** entity: they are read in
   `DispatchKeyValue`, the one chokepoint every keyvalue passes through, because a good many KeyValue
   overrides in this SDK answer their own keys and return without chaining to their base — which would
   have dropped the override on exactly the classes a mapper most wants to rename. They live on
   `CBaseEntity` and are saved, since a restored entity is never re-read from the map. The FGD carries a
   `Prompt` base class on the fourteen classes most likely to want it, and says in a comment that the
   keys work everywhere else too, through SmartEdit. **This is the one place strings cross the wire**
   (`gmsgPickupPrompt` is variable-length now, clamped at 48 characters each) and it is affordable for
   the reason the Prompt was cheap to begin with: it is sent when what the player is looking at changes,
   not per frame. The change test gained the entity index with it, or two renamed buttons would have
   looked identical. Ammunition and the vanilla pickups that are not Item Types name themselves —
   *Shotgun shells*, *Uranium*, *HEV battery* — instead of reading *Ammunition* and *Item*; the Prompt
   names a document by its title; the lock says `Enter code` rather than the digits, which stays.

### Open

- **Audio.** A Record's format leaves room for a `sound` field, its body then the transcript. The sentence
  system (`!SENTENCE`), `ambient_generic` and `CHudStatusIcons` for a "playing" icon are the machinery,
  all existing.
- **The reader and the Prompt are drawn in the engine's console font.** Widening the Prompt was the
  reason to style it; it is not styled.
- **The stand-in document model**, in [ART_DEBT.md](ART_DEBT.md).

Answered 2026-09-17, so not open: Records are re-readable, read in the world and kept in a tab, text
stands in for subtitles, and they carry information. A Record occupying a Cell was rejected because a
lore pickup that competes with a medkit for space is never picked up; it banks like a Reset Token.

---

## Pillar 1: The world

**Shape: Shaped in structure 2026-09-17; the places themselves are Ideas, except the cold open, which is
built.** Most of it *is* maps. The structure — hub and spokes, hard and soft gates, restored
infrastructure as global states, the reactor, the hazard course — is under
[The shape of the game](#the-shape-of-the-game).

### The facility

**Settled 2026-09-17:** a hub with wings; shortcuts home at each wing's end; zones opened by Modules and by
restored infrastructure (power, elevators, pumps), each an `env_global` so a switch thrown in one map opens
a door in another. It brims with secrets, and **ammunition, deposits, entries and Skill Points live in them
rather than on the critical path** — exploring is how the player arms themselves. After the midpoint the
same hub stands in Xen and the same wings are torn open, so the second half reuses geometry the player
knows. Wing identities are open.

Secret areas, vents, shortcuts, elevators and trains. Structurally this is level design rather than code —
Half-Life already ships `func_train`, `func_tracktrain`, `func_door`, `func_plat`, `func_button` and
`func_breakable`, all working, all used by the base campaign.

The Skill Point economy is a non-issue as of 2026-09-15 (see [Maps](#maps)), so these areas are level
design for its own sake, not a test of the numbers; the brief stays roughly one Skill Point per optional
space and 5–10 Reset Tokens across the campaign, the first around 20% in.

### The cold open — `shaft1`, grilled 2026-09-19

**Shape: Built through three cuts on 2026-09-19, walked and confirmed by Andrei, and his since in
J.A.C.K.** The spec is `maps/shaft1.rooms.txt`, in the loop [MAP_WORKFLOW.md](MAP_WORKFLOW.md) sets:
plan first (`maps/shaft1.plan.png`), then the `.map` from the generator, then J.A.C.K., where the layout
is frozen and the generator is not run again. Since 2026-09-20 his hand-built work is in it — the
chamber's pillars, the crystal deposit units — with each session in [CRAFT_LOG.md](CRAFT_LOG.md). Why
this map first: the session opened on feeling blocked, and the diagnosis was a design-complete, play-zero
project — five pillars Playable and no level anyone has played. The cold open is the most fully specified
level in this document, needs none of the open questions answered, is linear, and uses only what is
built. The decisions, in the order they were made:

1. **Miners wear suits.** The player's own is **Agility**, with the pickaxe. The player lands in overalls
   and takes both from their own open locker in the dry, on the station's mezzanine, thirty seconds from
   the cage; the HUD wakes there (the third cut, on Andrei's ask for a locker room; the first two cuts
   gave both at the spawn). The same locker is where the kit is handed in at the end (decision 12). No
   code either way.
2. **The suit choice is made at the hub**, under an hour in, not in a changeroom at minute one and not in
   the escape. Suit Variants are meant to carry specialization (stat bonuses or Skill unlocks — Andrei,
   2026-09-19; PILLARS still records them as cosmetic with codenames), so the choice has to be informed.
3. **The escape is unequipped**: no suit, no pick. It sets the atmosphere, introduces Records and a locked
   door, maybe a Panthereye chase. So the Pulse moves from the safety cabinet to the hub or after it.
4. **The map ends at the cage.** The ride is map two: the title sequence has to show the facility working,
   which a greybox cannot. The strip of suit, pick and Shards is map two's, at the pithead.
5. **NPCs now, as stand-ins, only the ones the beats need**: the crew at the face, security at the hoist,
   scientist and Barney models, scripted, no voice.
6. **Theme in words now, the look later.** The brief's paragraph shapes the volumes (haulage way 192 by
   144, drifts 128 by 104–112, the chamber taller than wide); textures, light and detail wait for
   J.A.C.K., stock rock until then.
7. **Crystal is amber**, as in vanilla's own amber crystal, and the chamber inherits it. The Shard's shell
   and the arcs already are; the deposit code's "unsettled" comment can go.
8. **The Heart is carried in the Grid**, not on a cart: a unique Entry three Cells wide, **not
   droppable** (a soft-lock otherwise, and the Grid cannot be freed of it, which is the point of it being
   big). Deposits encase it — eight units of crystal since 2026-09-20, built by Andrei, north and south
   unstable; four slabs before — and mining at least one reaches it. The hoist is a Station's cousin that
   takes a named Entry and fires the cage.
9. **The way back collapses** when the Heart is taken (a quake; the one hard gate), and the old workings,
   unlit, lead out to the landing from the other side. The map is a loop.
10. **One attacker in the dark**, at a vein in the old workings: the [maddened miner](#the-cult-and-the-maddened),
    written that same evening and in the map since (a zombie stood in for the third cut's walk).
11. **Two secrets**: a stope with two deposits off the haulage way, and a wordless shrine in a dead end.
    No Skill Point: the tree is the suit's fuel and the suit comes off at the top of the shaft.
12. **Mined Shards are handed in with the kit in the dry at the pit bottom, in this map** (the third
    cut; the pithead in the first two): after the hoist, security sends the player on break, the hand-in
    button on the locker strips suit, pick and Shards, and only then does the cage button work; the ride
    and map two begin with no suit. The company owning the ore is the first showing of who owns what.
    **Later option, recorded**: a stash — hide Shards, or deposit them for retrieval later in the
    campaign.
13. **Start at the shaft bottom**, the cage just landed, and walk in under the lamps, so the lit half is
    seen before the dark half and the loop closes where it opened.
14. The foreman fires the blast; the player's own act is cutting the Heart.
15. The name is `shaft1`; `topmap` and `minemap` stay test maps.

**Code this map needs, all small — written 2026-09-19, verified in game by Andrei the same evening on the
second cut (the Heart into the Grid, the hoist, the cage):** `item_heart` and its Item
Type row (`game_shared/inventory_defs.h`, id 7, three Cells, unique, with a new `droppable` column the
server's Drop verb and the client's Drop buttons both read); `func_station` stationtype 3, the hoist —
`StationDef` gained an `input` Item Type and a `None` output, so the hoist takes the Heart once and fires
its target, with its own Prompt rows; and for map two, spawnflag 1 on `player_weaponstrip`, which also
removes the suit and the Grid's Entries one by one (not `Clear()`, which would forget granted Rows). The
plan was accepted by Andrei ("looks really good"), the first `.map` walked the same evening ("solid"),
and his five notes became the second cut, all in the spec: the haulage way three legs long with two
bends, descending 192 on shallow steps; the station grown to a pit bottom with a lamp room, a tool crib
and the foreman's office off it; four blind branches behind chainlink and one ending in a rockfall; steel
sets down every drift, timber in the old workings, from a new `sets` line in the generator's grammar; and
the way back climbing 352 to a gallery over the station's south wall, out through a gate seen locked at
the start and opened from behind. Props, posters, signage and the non-box shapes are his in J.A.C.K.,
once the layout stops moving. **Debt it creates** is in [ART_DEBT.md](ART_DEBT.md#shaft1--the-cold-opens-stand-ins).

**The third cut, built unattended the same evening; walked by Andrei on his return and every row below
confirmed, the hand-in and the strip flag with them. The layout is frozen from here: the map is his in
J.A.C.K. and the generator is not run on it again.** Andrei's notes on the second:
way bigger, in both senses — larger volumes so there is room for storytelling by hand, and more places
with a reason each — the locker room, and a road before the first enemy long enough to foreshadow him
("this guy is missing for some time already"). What the spec now holds beyond the second cut: the
station on two storeys with the dry and the showers on the mezzanine (decisions 1 and 12 amended above);
a junction on the first leg with a second working drift north to an abandoned face, roof down and two
deposits left, where the missing man was last seen, and a pump room south over a sump; the ventilation
raise behind mesh, its fan heard down the leg; a 640-by-640 chamber 704 tall with crystal pillars; the crew
scripted with vanilla sentence groups as stand-ins, the worried line, the foreman's word, then the blast;
the old workings' 1500-unit road before the vein, with drips and wind, three triggered whispers at growing
volume (alien voices and the slaves' words standing in for the incantations), a flooded dip, the horror
under the shrine, and past the vein a ladderway up to the missing man's camp; the cage button dead until
the hoist and the hand-in have both fired, through two multisources. Every stand-in is in ART_DEBT.

**Walk it — the test rows for the third cut**, in order. **All confirmed by Andrei, 2026-09-19.** They
stay as the regression list for the editor pass; the chamber row has changed since (crystal units, and
the maddened at the vein), and [map-check](MAP_WORKFLOW.md) reads back each export:

| Row | What should happen |
| --- | --- |
| Start | On the cage floor, no HUD. The cage button says nothing and does nothing (mastered) |
| The dry | Up either stair to the mezzanine, through the door: the open locker holds the suit and the pick; the HUD wakes; the guard is there |
| The station | Lamp room and crib under the mezzanine, office off the south, the hoist by the cage door, the gallery gate seen locked above the south wall; a hum |
| Leg A | 256 wide, sets every 192, rails and two cars; the chainlink branch south with a lamp inside |
| The junction | North: the drift to the abandoned face, its roof down at the end, two deposits, a lamp. South: the pump room, the sump's water, the pump running |
| The descent | Three shallow ramps, two bends; the fan heard from the raise behind mesh on leg B; two more chainlink branches on leg C |
| The face | The worried line, the foreman's word, the blast at about six seconds; the rock face breaks |
| The chamber | Amber, tall, four pillars; mine a deposit, the unstable one flashes then arcs; the Heart walks over into the Grid as a three-wide Entry with no Drop button |
| The quake | Rubble fills the breach; the south wall opens; autosave |
| The road | Drips, then wind; a whisper far off, a second nearer, a third close; the flooded dip is ankle deep; the shrine's dead end and its sound; the vein and the maddened miner |
| The camp | Past the vein, a ladder in the ceiling; a bedroll, a lamp, three Shards |
| The climbs | Five ramps; out on the gallery; the gate opens from the button beside it; the drop into the station |
| The hoist | Takes the Heart, the cage rises empty and returns; the line about the dry |
| The hand-in | At the dry the guard speaks; the locker button strips everything and the Grid empties; the cage button now works and the top of the shaft ends the map |

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

**Shaped in outline 2026-09-17.** A brief
excursion before the midpoint (one or two maps, an errand for the survivors' plan, arriving at the far end
of the anchor: the same arranged crystal, intact and enormous). After the teleport, Xen wings off the same
hub, one per piece, in a soft order, with Xenian presence escalating by pieces held. Xen introduces
platforming on the Dash, then the double jump; `trigger_gravity` is a free fourth tool. **Underground Xen
is Xen hell** — the bottom, [the cut monsters](#xen-hell-and-the-cut-monsters), Kingpin — and is original
level design rather than a revisit. How many pieces (three proposed) and the third wing's ending are open.

### Mining and crystal shards

**Shape: Shaped 2026-09-17. Built 2026-09-18 overnight, verified in game the same day** — the Shard, `func_deposit`
and its unstable form are recorded in [PILLARS pillar 1](PILLARS.md#1-exploration), with the grill that
settled the details in [OVERNIGHT_BRIEF.md](OVERNIGHT_BRIEF.md). What stays here is the design and the
one open question at the end. The mod's resource loop — small, finite, authored.

- **Deposits** are placed by hand, mostly off the critical path, and never respawn; a few dozen in the
  game. **Only a mining tool breaks one**, and the [pickaxe](#the-carbon-pickaxe) is that tool.
  `func_breakable` already keys special rules on the crowbar's damage type, so this is an extension of an
  existing rule rather than a new system.
- They drop **crystal shards**, an ordinary stackable Item Type in the Grid. Shards are the input to
  [Stations](#stations).
- **Unstable deposits flash and then discharge.** Mining and the Pulse in the same seconds, and a hazard on
  nobody's side: a discharge hurts whatever is near, so a player who knows the rhythm can lure a zombie
  past a vein. A flash is a sprite or a light; a discharge is a timed beam and a hurt. It is also the
  parry's first tutor — no enemy, a fully readable telegraph.
- **The risk, named.** Harvesting loops rot immersive sims: once walls can be hit for loot, players stop
  looking at rooms and start looking for nodes. A deposit here is *a secret that happens to be made of
  crystal*. If mining ever feels like farming, the fix is fewer deposits, not more systems.
- Shards as a second faucet of Skill Points was rejected except for the bounded
  [fuel processors](#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate).

**A deposit is brushwork, as a pasted unit** (2026-09-20): seven crystals in a 64 cube, built by Andrei
in J.A.C.K. and pasted around the Heart, each unit its own `func_deposit`; a `light_surface` makes every
crystal face emit. Brush rather than a model because a face can emit light and a model cannot, because
the entity already traces its real shape, and because Valve's crystals are brushes. The record is in
[CRAFT_LOG.md](CRAFT_LOG.md). A model stays possible for small wall outcrops later. A Shard is one Cell,
a Stack of ten (2026-09-18).

Open: whether any second-half deposit is remembered for the endings (stripping Xen's crystal, which the
Xenians are desperate for).

### Stations

**Shape: Shaped 2026-09-17. The first two kinds built 2026-09-18 overnight, verified in game the same day,
refusals for Grid space and uranium included** — the Fuel
processor and the Ammunition station, recorded in [PILLARS pillar 1](PILLARS.md#1-exploration). The room
question below was answered by refusing: nothing is taken unless the whole output fits. What stays here:
recycling, a Syringe or other outputs, and recipes found as Records. Crafting and recycling stations placed
in levels, with item inputs and item outputs. **The input is [crystal shards](#mining-and-crystal-shards)**; the outputs are what the player
cannot find enough of — Cores (which the [Alien Route](#alien) already said a Station would make),
energy ammunition, perhaps a Syringe. Some are **fuel processors** that make one Skill Point, once. The
choice a Station poses is ammunition now or power for good.

The precedent is exact: `func_recharge` (`dlls/h_battery.cpp:64`) and `func_healthcharger`
(`dlls/healthkit.cpp:157`) are wall-mounted entities with a `+use` interaction, a finite budget, a sound
response and a HUD effect. A Station is that, plus a transaction against the player's Inventory.

The good news is that the transaction is safe by construction:
[ADR-0004](adr/0004-the-server-owns-the-inventory.md) makes the server own Inventory contents *and*
placement, so a Station is server-side code calling into `CPlayerInventory` directly — no new authority
question, no new sync path. `TryAdd` already handles first-fit placement and Stack top-up, and already
refuses when there is no room.

Which surfaced the one genuinely new rule, **what happens when the output does not fit**, answered by
construction on 2026-09-18: the Station refuses, and nothing is taken unless the whole output fits. A
Station that consumed inputs and then could not deliver would have destroyed the player's belongings,
the same class of bug as the three `CWeaponBox` hazards PILLARS records under "Deliberately deferred".

#### Crafting — Idea, 2026-09-18

**From Andrei, not grilled.** Some Stations get a **VGUI panel**: a list of recipes, each consuming
ingredients for a result. The ingredients are **scrap or junk components**, found throughout the facility
or looted from enemies.

This reverses two calls made for the overnight build, on purpose now that it is asked for:
[OVERNIGHT_BRIEF.md](OVERNIGHT_BRIEF.md) kept Stations to one press per trade with no panel, and called
free-form recipes "a crafting system, which nobody asked for". What it asks of the design:

- **A second material.** Crystal was settled as "the one material" (answered below). Scrap as an ordinary
  stackable Item Type keeps the answer's substance — item → item, no new identity space, no save change —
  but crystal stops being the only input, and what scrap is *for* has to differ from what Shards are for.
- **Loot from enemies** is new: nothing drops items into the Grid today except a weapon from a soldier.
- **The panel** is a new VGUI page, on the pattern of the Inventory Panel's tabs, and a server message for
  the recipe list and a client command to craft; the transaction stays server-side under
  [ADR-0004](adr/0004-the-server-owns-the-inventory.md), with room checked before anything is consumed.
- **Recipes known, found, or both.** A panel lists what a Station can make, which is "known"; recipes found
  as [Records](#pillar-1-records) could unlock rows.
- **The risk named under [Mining](#mining-and-crystal-shards) applies twice**: junk to loot is the
  harvesting loop in its most common form. Scarce, authored placement is the answer there and here.
- A crafted [energy rifle](#the-energy-rifle) is one proposed customer.

#### Vending machines — Idea, 2026-09-18

**From Andrei, not grilled.** Some Stations **sell**: a list of products and their prices, paid in a
**currency** found or earned through the game. The simplest case is a drinks machine, which vanilla
half-supports: `env_beverage` (`dlls/effects.cpp:2147`) is a point entity that, triggered, drops an
`item_sodacan` (one health on touch) from a finite stock, one can at a time, with no price and no panel.

- **Currency is new** — a third thing to collect beside Shards and Skill Points. Whether it is an Item Type
  occupying Cells, like Shards, or a counter occupying none, like a Record, is the first question; the
  two answers price inventory space very differently.
- **Earned how** — found, looted from enemies, paid for Shards at a Station, or all three. Shards bought
  with currency or currency bought with Shards would make them exchange rates of each other.
- A vending machine and a crafting Station may be **one panel** with different inputs; worth deciding
  before either is built.
- No noun is proposed for the currency or the scrap yet, per [Proposed vocabulary](#proposed-vocabulary):
  naming them first would settle the design by accident.

### Open questions

- **Whether Stations also *recycle*** — break an unwanted item down into Shards. Not decided. What is
  decided (2026-09-17): a Station trades **item → item**, crystal is the one material and it is an Item
  Type — no new identity space, no new UI, no save change — which [crafting](#crafting--idea-2026-09-18)
  above would reopen with scrap.
- **Recipes found as Records**, for a Station type added later. Today every recipe is known: the Prompt
  states each trade (2026-09-18).

Answered, so not open: Stations are fixed in the world, a brush built into a wall (2026-09-18), the
stronger answer for pillar 1 at no extra cost.

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

**Icons must clear a bar the register already specifies.** The Skill Tree entry in ART_DEBT.md is marked
*blocking rather than cosmetic*, and its brief sets the constraints: **one fixed size, not a
resolution-bucketed set**, fitted into the board's node frames (36 to 74 pixels at 1:1, the draw fitted
since 2026-09-14); greyscale for anything drawn additively and tinted. The genuinely hard requirement is
that each icon be **distinguishable from every other at 20×20**, which is where generated icon sets
usually fail — they come back stylistically consistent and mutually indistinct, which is exactly today's
problem with `suit_full` on five Skills. The Inventory Grid's Icons are full-colour world-model renders
(PILLARS pillar 5), a different bar.

**On sounds.** Settled 2026-09-18 with the art workflow: Andrei makes them, in FL Studio, and
[SOUND_WORKFLOW.md](SOUND_WORKFLOW.md) is the loop that ships them.

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
| **Module** | A mechanic the player does not start with, found partway through the game and kept for good — never swapped. | Deliberately echoes "the longjump module", which is the model. Five are built and the word is still not in CONTEXT.md; it should graduate. |
| **Hook** | One of the six Modules (Pulse, Dash, Hook, the alien Module, Night Vision, Double jump); the Dash is in CONTEXT.md already. The Dash comes with the long jump rather than replacing it (2026-09-15). The Hook is low priority since 2026-09-17. | Plain, and hard to improve on. |
| **Shinobi** | The Route built on the Dash Module. | Named 2026-09-15; *Ninja* was the candidate, *Agility* stays with the suit. |
| **Night Vision** | The fifth Module: replaces the flashlight when found, gates the Stealth region. | Settled 2026-09-15. Adapted from Opposing Force. |
| **Overdraw** | The Energy Route's Major: energy attacks drain armour as well, for bonus damage. | Named 2026-09-15. |
| **Evolution** | A durable alteration to a weapon that keeps the weapon's identity — silencer, second barrel, extended magazine. | Avoid *attachment* and *mod*; the first implies removable hardware, the second collides with "the mod". |
| **Transmission** | A *category* of Record: what was radioed, intercepted or, later, voiced. It was the whole feature's name until 2026-09-17. | Avoid *log*, *tape*, *audio diary*, *datapad*. |
| **Decapitation** | A lethal head hit that removes the head: headless submodel, thrown skull, blood from the stump. | Distinct from *gibbing*, which is the whole body and already means something in this codebase. **Headless** names the resulting state. |
| **Route** | A build path through the Skill Tree: the set of Skills whose bonuses multiply into one way of playing. A region of the tree since 2026-09-15. | Named 2026-09-13. Avoid *class*, *spec* and *tree* — the tree is the whole thing. |
| **Hub** | The centre region of the Skill Tree: the suit, the generic suit stats, and the cross-Route Skills in its corner cells. | Settled 2026-09-15. Avoid *core* (the alien ammo) and *centre* alone. |
| **Seam** | The shared border of two Routes' regions, where a cross-Route build pays in the neighbour's stat. | Settled 2026-09-15. Avoid *bridge*, *link* (a link is an effect that reads another Route). |
| **Keystone** | A Skill Tree node with a real downside. Glass Cannon is the first. | Settled 2026-09-15, Path of Exile's word kept on purpose. Avoid *curse*, *trade-off node*. |
| **Trace** | A drawn connector in the Skill Tree: thin between neighbours, thick for a Skill's gate, lit when powered. | Presentation term, 2026-09-15. Avoid *edge* in player-facing text (kept in code and the debug overlay). |
| **The Heart** | The purest crystal of the anchor under the mine, cut free by the player in the cold open and seated in the rig. | Provisional, 2026-09-17. The grill called it the *keystone*; **Keystone** is taken, two rows up. |
| **Double jump** | The sixth Module; fills the legs Slot. | 2026-09-17. Name as plain as Dash and Hook; a fiction name for the hardware is open. |
| **Wing**, **Piece** | A spoke of the hub; one of the things collected in the second half to take the facility home. | 2026-09-17. How many Pieces is open (three proposed). |
| **Hard gate**, **Soft gate** | Sealed by construction and looking it; or an intended key plus at least one deliberate alternative. | 2026-09-17. Map-brief terms, not player-facing. |

**Graduated** into [CONTEXT.md](../CONTEXT.md), each when the thing it names was built, so no longer
proposed here: Concealment, Backstab, Suspicion, Search, Post, Perception Profile, Disturbance and the
Unseen / Noticed / Spotted states (2026-08-31; Backstab's meaning changed on the way across — the proposal
required the victim to be unaware, and the settled term is purely a matter of where the attacker stands);
Stat node (2026-09-14); Core and Ghost (2026-09-16; *ghost slave* lost the second word because the
classname already says slave once); Defense Matrix and Decaying Armor; the Dash; Station, Fuel processor,
Deposit, Shard, Tail, Carbon Pickaxe, Record, Guidance and Prompt (2026-09-18); the maddened and the cult
(2026-09-19).

Note what is deliberately *absent*: there is no proposed term for the scrap that
[crafting](#crafting--idea-2026-09-18) would consume or the currency
[vending machines](#vending-machines--idea-2026-09-18) would take. Inventing a noun before the design is
grilled would settle it by accident.

---

## Open questions

Ranked by how much else is waiting on the answer.

1. **The Pulse's timing** — the [grill booked for 2026-09-21](#the-pulses-timing--settled-2026-09-23-one-second),
   with [the Pulse against sustained fire](#the-pulse-against-sustained-fire) folded in. Whether 0.25 s
   is a state at all; what happens to the tail.
2. **How many Pieces, and what are the wings?** Three Pieces were proposed and not confirmed; no wing in
   either half has an identity yet, and the vertical slice — the hub and wing one — needs the first.
3. **Where are the Dash and the double jump found?** Proposed: the assassin boss, and the vortigaunt's
   first hand-over after the teleport.
4. **The names**: the player, the facility, the company, the Heart, and whether *miner* is the final word.
5. **The decompiled stock maps** (2026-09-19, `E:\CustomAssets\maps\decompiled\`, the tram ride and
   Anomalous Materials): a reference for proportions and construction to learn from, or brushwork to
   lift, which is a licensing question first. See [MAP_WORKFLOW.md](MAP_WORKFLOW.md#decompiled-half-life-maps).
   Not used until grilled.
6. **The endings.** Multiple, read from choices and secrets, and deliberately the last thing designed
   (2026-09-17). Nothing waits on it except the rule that a moment which might matter sets a global.

Answered, with the answer recorded in its entry: Modules are found and kept, never swapped, and level
design may assume one past where it is found (2026-09-12); the Pulse is a Module with a timing branch
and the Defense Matrix (2026-09-12, 2026-09-13); numbers are Skills and identity is an Evolution
(2026-09-13); the pickaxe joins the crowbar rather than replacing it (2026-09-13); stealth is optional
everywhere with declared exceptions (2026-09-17); Stations trade item for item (2026-09-17); a boss
moveset is an existing monster with custom attacks (2026-09-12); sounds are Andrei's to make
(2026-09-18).
