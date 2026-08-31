# Roadmap — intended work

Everything this mod means to build and has not built. [PILLARS.md](PILLARS.md) is the record of what exists
today; this is the record of what is wanted, why, and what it will cost.

Nothing here is a commitment to a design. Entries capture the intent, name the code that already exists to
build on, and list the questions that have to be answered before the first line is written. When an entry
is built, its content moves into PILLARS.md and the entry here is deleted — this file only ever shrinks
from the top.

**Last updated:** 2026-08-31 (branch `hl-shock`, at `ff03310` — stealth moved from Shaped to Ready)

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
| Dash | [Modules](#pillar-3-modules) | `pm_shared/` — movement, not weapons |
| Hook | [Modules](#pillar-3-modules) | `pm_shared/` |
| Sprint speed (`SprintSpeed`, id 6) | reserved, cut from the tree | `pm_shared/` |
| High jump (`HighJump`, id 5) | reserved, cut from the tree | `pm_shared/` |
| Crowbar swing speed (`CrowbarSpeed`, id 11) | reserved, cut from the tree | the crowbar's own damage rule, below |
| Draw speed | [Weapon handling](#weapon-handling) | nothing — unblocked |

**The movement four are a genuinely different problem.** `pm_shared/` runs from `playermove_t`, not from
`CBasePlayer`, so it cannot reach `m_skills` at all — the fix above does not extend to it, and the honest
next step there is to carry the mask into `playermove_t` rather than to reuse anything built here.

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

**Shape: Ready. The design is settled and the first commit is obvious.**

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
| 3 | **Suspicion** | Perception Profile, the meter, the `Look` gate, and a **debug view** built alongside rather than after it. `adr/0009`. |
| 4 | **The readout** | `gmsgConceal` plus a HUD element, following `CHudPulse`'s send-on-change pattern. |
| 5 | **The squad half** | De-escalation, the Search, Posts, death witnesses, the Disturbance marker, the level-transition reset. |
| 6 | **Noise** | A deliberate multiplier on the computed noise volume for crouching and walking. |

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

### Open questions

- Is stealth **optional** everywhere, or are there encounters designed to be unwinnable head-on? The
  answer changes level design more than it changes code, and nothing settled above touches it.
- **How dark is dark?** Vanilla Half-Life maps are lit for readability rather than for hiding, so the light
  term may do almost nothing until there are custom maps with dark places in them. What light level counts
  as concealing is a question only [Maps](#maps) can answer.
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

A gauss weapon shaped as a katana. Slow, bulky, and frightening. Explicitly planned in two passes:

- **v1** — model plus decent animations. Playable, unpolished.
- **v2** — sprites and polish.

Marked as a candidate for [Evolutions](#weapon-evolutions).

Open, and all of it: does it consume uranium, like the Gauss and Egon? Does it charge, the way
`GAUSS_PRIMARY_CHARGE_VOLUME` implies for the gun? Is it melee that deals energy damage, or does it
project? `DMG_ENERGYBEAM` is the natural damage type and PILLARS records why (the alien slave is the only
thing immune to `DMG_SHOCK`, and `DMG_ENERGYBEAM` has no immunity anywhere) — that reasoning applies here
unchanged.

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

### Viewmodel hands

**Shape: Idea. The largest art task in this document, and the least visible.**

Every `v_*.mdl` in Half-Life bakes its own hands into the model. There is no shared hand mesh, so
consistency means touching every viewmodel the mod ships — the vanilla set, the Carbon Pickaxe, the Gauss
Katana, and every Evolution that changes a viewmodel.

Worth being honest about the trade: a player who does not go looking will never consciously notice
consistent hands, and will absolutely notice inconsistent ones. It is a floor, not a feature. It is also
the sort of thing that gets cheaper the earlier it is decided and much more expensive once there are ten
custom viewmodels to redo.

---

## Pillar 2: Monsters and bosses

**Shape: Idea.**

Import and reuse existing monsters with light polish, with occasional custom AI. Named candidates:
**Panthereye** and **Kingpin** — both cut Half-Life monsters. Neither exists in this codebase; there is no
`panthereye.cpp` or `kingpin.cpp` and no reference anywhere in `dlls/`. Both would be imports of model and
behaviour, not revivals of dormant code.

**Bosses** — more difficult encounters with specific movesets.

> The user's note on bosses ends mid-example: *"specific movesets (e.g."*. What the intended example was
> is not recorded. Worth filling in — the example probably carries the actual design intent.

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

Open: is a boss a **combat** encounter or a **puzzle**? Half-Life's own answer is mostly the latter
(Gargantua, Nihilanth, the tentacles), and a mod adding movesets is proposing the former.

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

**Shape: Idea, with one large open question the user has already flagged.**

Findable items that grant the player a new verb, managed from their own tab in the Inventory Panel.
Proposed set:

| Module | What it does |
| --- | --- |
| **Dash** | A short dash. Upgradeable. |
| **Hook** | A grappling hook, in the manner of Opposing Force's barnacle grapple. |
| **Pulse** | The existing Pulse, converted from suit hardware into a Module. |

### The precedent is already in the game

`m_fLongJump` (`dlls/player.h:176`) is exactly this shape: a bool on the player, saved
(`dlls/player.cpp:107`), set by walking over `item_longjump` (`dlls/items.cpp:452`), and gating a movement
verb thereafter. It is a found item that permanently grants a new way to move, and it is in the fiction as
suit hardware. Modules are that, generalised, with a UI.

Note the collision: `models/w_longjump.mdl` is currently the **Skill Point** placeholder
(`dlls/items.cpp:381`). [ART_DEBT.md](ART_DEBT.md) already calls that misleading; Modules make it worse,
because the longjump module will read as a Module to anyone who sees one. That entry gets more urgent, not
less, if this is built.

### The three hard parts

**Movement Modules are client-predicted.** Dash and Hook both move the player, so both hit
[the prediction problem](#the-prediction-problem) head-on — and the weapon-side fix does not help them:
`pm_shared/` runs from `playermove_t` and cannot see `m_skills` at all. A dash that resolves server-side
only will rubber-band on any latency. This is the single largest cost in the Module idea and it should be
costed before the tab is designed.

Opposing Force's grapple is not in this codebase. The barnacle's tongue is the nearest existing
beam-plus-pull behaviour to read for reference, but a player-driven grapple is new movement code in
`pm_shared/`, which is shared, predicted, and the most dangerous place in the codebase to be wrong.

**Limited or unlimited is not a UI question.** The user's own note leaves this open — "limited (swapable)
or unlimited". It decides the whole feature:

- *Unlimited* makes a Module a permanent unlock, identical in kind to the longjump module, and the tab is
  a display of what you have found. Cheap, and level design can assume any Module the player has passed.
- *Limited* makes Modules a loadout, with slots, swapping, and a real choice at every Station. Much more
  interesting, and it means **level design can never assume a Module** — every gap crossable by Dash needs
  another way across, or the player who swapped it out is stuck.

The second is a commitment across every map in the mod. It should be decided before any map is built, not
after.

**Converting the Pulse costs something specific.** PILLARS pillar 2 is explicit about why the Pulse is
suit hardware: *"Skills evolve a verb the player already has rather than granting it, which lets level
design assume it."* Making it a Module withdraws that guarantee. Worse, four Skills hang off it
(`PulseWindow`, `PulseRecharge`, `PulseDischarge`, `PulseRebound`) plus `CrowbarFollowUp`, so a player
could spend points on Skills for a Module they are not carrying. Under *unlimited* Modules this is nearly
harmless; under *limited* it is a live problem needing an answer — grey the branch out, refuse the unlock,
or accept it.

### Open questions

- Does a Module occupy **Cells**? Its own tab suggests no, which puts it with Reset Tokens (banked, not
  carried, occupying no Cells and undroppable) rather than with Item Types.
- What **acquires** one — walking over it, the Pickup Prompt, or a Station?
- "Dash: upgradeable" — by Skill Points, by Module-specific upgrades, or by finding a better Dash? The
  first is the cheapest and reuses the whole tree; the third is the most exploration-flavoured.
- Does the Inventory Panel's existing **Upgrades** tab become the Modules tab, or is this a third tab?

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
today's problem with `suit_full` on five Skills.

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
| **Module** | A found thing that grants the player a new verb, managed from its own Inventory Panel tab. | Collides with "the longjump module", which is in-fiction and helps rather than hurts. |
| **Dash**, **Hook** | The first two Modules. | Plain, and hard to improve on. |
| **Evolution** | A durable alteration to a weapon that keeps the weapon's identity — silencer, second barrel, extended magazine. | Avoid *attachment* and *mod*; the first implies removable hardware, the second collides with "the mod". |
| **Transmission** | A recorded log found in a level and played back. | Avoid *log*, *tape*, *audio diary*, *datapad*. |
| **Station** | A world entity that takes items in and gives items out. | Avoid *bench*, *workbench*, *terminal*, *fabricator*. *Terminal* especially — it will be wanted for Transmissions. |
| **Decapitation** | A lethal head hit that removes the head: headless submodel, thrown skull, blood from the stump. | Distinct from *gibbing*, which is the whole body and already means something in this codebase. **Headless** names the resulting state. |
| **Carbon Pickaxe**, **Gauss Katana** | The two custom weapons. | Named already; recorded here so they are used consistently. |

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

1. **Are Modules limited or unlimited?** Decides whether level design may ever assume a Module, which is a
   commitment across every map in the mod. Must be answered before the first map is built.
2. **Does the Pulse become a Module?** Withdraws the guarantee PILLARS pillar 2 relies on, and strands four
   Skills plus the Follow-Up if the answer is yes and Modules are limited.
3. **Are weapon handling upgrades Skills or Evolutions?** Two systems currently want to make the same gun
   faster. Building both is the failure mode.
4. **Does the Carbon Pickaxe replace the crowbar entirely?** Decides whether five Skill enumerators, three
   icons and a CONTEXT.md entry get renamed once or never.
5. **Is stealth optional everywhere?** Level design consequence, not a code one, and the answer shapes
   every encounter.
6. **Recycling: item → materials, or item → item?** The first introduces a whole new identity space
   alongside `WeaponId` and `EItemTypeId` ([ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md)).
7. **What was the boss moveset example?** The note ends at *"specific movesets (e.g."* — the example is
   likely where the actual intent is.
8. **Sounds — offering help or asking for it?** *"here I have little experience and I can help"* reads
   both ways, and the two readings imply very different plans.
