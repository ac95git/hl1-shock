# Gameplay Pillars — Progress

The things this mod is actually about. Everything inherited from the base SDK is scaffolding; this
document tracks the custom gameplay on top of it.

This is a living document. When a pillar's state changes, update its section and the summary table in the
same commit as the code change.

This file records **what exists today**. Intended work that has not been built lives in
[ROADMAP.md](ROADMAP.md), and each pillar below links to its entries there.

**Last updated:** 2026-09-17 (no code: the game was shaped as a whole and
[ROADMAP.md](ROADMAP.md#the-shape-of-the-game) reconciled to it. Here that changes only framing — the
pillars are ranked, exploration has a definition, and stealth is a way to solve an encounter rather than a
pillar ranked beside combat). Before that, 2026-09-16 (branch `hl-shock`, one wave from five parallel agents on the Juggernaut
foundation: the Stealth region's effects and the Night Vision Module that reveals it, the Hive nodes, and
the Alien Route's first slice — Cores, a ghost, the summon's left click; none of it verified in game yet.
Earlier the same day, the Defense Matrix built on a held Pulse key, and the Status tab before it; 2026-09-12
before that, after the Suit Variant)

## Status legend

| Label | Meaning |
| --- | --- |
| **Not started** | No code exists. |
| **Scaffolded** | Data structures, networking, or UI exist, but the player cannot feel it in-game. |
| **Playable** | The player can use it in a normal play session, rough edges accepted. |
| **Done** | Behaviour is final and covered by the acceptance criteria in this document. |

## Summary

**Ranked 2026-09-17: exploration first, by far; then RPG (pillars 3, 4 and 5); then combat (pillar 2, with
pillar 6 inside it).** The numbers below are names, not the ranking — and the first-ranked pillar is the
one at **Not started**. It is still a run, think, shoot, live game: stealth is one of three ways to solve
an encounter (fight it, sneak it, go around it). See [ROADMAP.md](ROADMAP.md#the-shape-of-the-game).

| # | Pillar | Status | One-line state |
| --- | --- | --- | --- |
| 1 | [Exploration](#1-exploration) | **In progress** | Its rewards exist — Row Grants, Skill Points, Reset Tokens are all findable entities, and `topmap`, the default test map, places them. Since 2026-09-18 it has its first code: the Prompt on everything usable, and Records read in the world and kept in a fourth Inventory tab. What it still lacks is a map with spaces to explore *for* any of it. |
| 2 | [Enhanced combat](#2-enhanced-combat) | **Playable** | The Pulse is complete and plays well — Shield, Recharge, Discharge, three Skills, readiness bar — and since 2026-09-16 the same key held is the Defense Matrix, the Juggernaut Route's last three nodes, untested in game. Melee Skills land, and the Backstab gives melee its first positional decision. The hivehand's three Hive nodes (capacity, replenish, fire rate) followed the same day, also untested. Numbers untuned. |
| 3 | [Custom items](#3-custom-items) | **Playable** | The Health Syringe works end to end — Item Type, world entity, the Infusion, a status icon and a Skill. No map places one yet. The Dash, the first Module, is built on SHIFT and untested in game. The Night Vision and alien Modules followed on 2026-09-16, also untested. |
| 4 | [Skill trees](#4-skill-trees) | **Playable** | 52 nodes, **all with effects**: the Melee and Weapon Specialist Routes built whole on the matrix (Stat nodes as their roads, every node one point, a major at the end of each), the Medical and Energy Routes built to all but their open nodes, Ricochet ahead of the Juggernaut, and the Dash and Alien columns waiting for their Modules. Points and Reset Tokens are earned and spent, the tree fits any screen, and nothing in it lies about what it does. Numbers untuned; `topmap`, the default test map, places Skill Points, and the economy is a non-issue. |
| 5 | [Inventory management](#5-inventory-management) | **Playable** | Grid, drag-drop, and context actions work over a server-owned model. Row Grants are now placeable; Boxes are the remaining gap. |
| 6 | [Stealth](#6-stealth) | **Partial** | Concealment and Suspicion are live: monsters no longer acquire the player on sight, they fill a meter at a rate set by angle, distance, stance and light, and the player is warned by `CHudConceal`. Quiet movement is deliberate. Nothing after acquisition has changed — once acquired, a monster stays acquired. Since 2026-09-16 the Stealth Skill Tree region acts on that meter — Soft Step, Nightfall, Slip Away, Ambush, Phantom and ten Concealment Stat nodes — behind the Night Vision Module, which also closes the flashlight's hole in the light term; none of it is tested in game yet. |

---

## 1. Exploration

**Status: In progress**

**Planned:** the rest of [Records](ROADMAP.md#pillar-1-records) — `record_grant`, Guidance, `record_lock`,
the mapper's Prompt keyvalues — and [the world](ROADMAP.md#pillar-1-the-world): the facility,
interactable props, Xen, and Stations. That last part is still downstream of [maps](ROADMAP.md#maps).

### What exists

**The Prompt** (2026-09-18). Everything a use press can act on says what it is and what the press will
do — a title and `[E] Action`, in the Pickup Prompt's vicinity, in the engine's console font. Defaults
by class (`game_shared/prompt_defs.h`); the server classifies, the client resolves, no strings cross the
wire. The mod's own pickups that never reach the Grid, and all ammunition, are taken by a use press
through it. A **state line** replaces the key and action where something is in the way — *Lock / Code
required* — so a hard gate looks impassable instead of offering a press it will not honour. A mapper
overrides any of it with `prompt_title` and `prompt_action` on any entity, and hides it entirely with
`prompt_suppress` — which hides the label, not the interaction, so vanilla's unmarked secret panel still
works. Ammunition and the vanilla pickups that are not Item Types name themselves. Still the engine's
console font, which is the reason to style it.

**Records** (2026-09-18) — the first code this pillar has ever had. Documents read in the world with
`+use`: a `record` (a model) or a `record_brush` (a terminal, a notice, a roster), both of which stay
where they are and re-open on every press. Reading is deliberately not walk-over, an exception to
[ADR-0011](adr/0011-pickups-are-walk-over.md) with its own reason. The text is `records.txt` in the mod
directory, read by the client alone and reloadable in place with `records_reload`; the server owns a
saved found-set of 512 ids and syncs it as a mask. An unread Record glows and goes dark once registered
— a glow shell on a model, a halo sprite on brushwork, never a light in the room. The reader opens on
the press, takes no input so the player keeps moving, and closes on damage or on walking away
(`record_read_range`). A fourth **Records** tab in the Inventory Panel lists what has been read, by
category with Guidance pinned, and shows the same reader.

**Guidance and locks** (2026-09-18). `record_grant` hands a Record over or takes one back when triggered,
which is all an objectives system needs: a Guidance line is an ordinary Record in the pinned category,
and "done" is a revoke that grants the next. Only a *granted* Record can be revoked, enforced by a second
saved mask rather than by a rule a mapper has to keep, so a document the player read can never be taken
away. `record_lock` fires its target if the suit has the Record it names — the soft-gate rule's first
reusable form, and the first thing in the mod that gates on knowledge rather than on an item. A Record's
first read can fire a target, which is how a remembered global gets set without any code knowing what it
is for.

**Mining** (2026-09-18, overnight; **verified in game the same day** — deposits, unstable deposits and
Stations, the Stations' refusals for Grid space and uranium included) — the resource loop the roadmap shaped on 2026-09-17,
small, finite and authored. **Crystal Shards** are an ordinary Item Type (`EItemTypeId::Shard`, id 6): one
Cell, a Stack of ten — exactly a Fuel Processor's price, so it reads off the Grid — no Use, picked up by
walking over `item_shard`. The pickup is a stand-in (Half-Life's glass gib in an amber glow shell); the HUD
icon is the mod's own, `utils/sprtool/icons/shard.py`. The only tool that mines is the
[Carbon Pickaxe](#2-enhanced-combat).

**Deposits** are `func_deposit` (`dlls/deposit.cpp`), a brush the mapper carves: `CBreakable` underneath,
so strength, break sound, gibs and the fired target come from it, and a pickaxe hit counts double as every
blow on a breakable does — 50 against a default strength of 120, three swings. **Only a mining tool breaks
one**: a hit whose attacker's active item answers `IsMiningTool()` and which was struck directly, so a
grenade thrown before a swap to the pickaxe does not count. Everything else glances off in a ricochet
spark and deals nothing — blades, bullets, explosions, the katana's wave. The Prompt names it *Crystal
deposit* with a state line by what is in hand, *Strike to mine* or *Requires a mining tool*; a use press
does nothing. Broken, it scatters its `yield` (default 3, at most 20) as loose `item_shard`s from the point
nearest the striker, stepped toward them, so a vein in a wall breaks into the room. Triggered by name it
breaks the same way, Shards and all. It never respawns.

**Unstable deposits** (spawnflag 8) discharge on a free-running cycle, in the mod's one telegraph language —
a flash, then the discharge. For the last `deposit_arc_warn` (1.2 s) of every `deposit_arc_period` (4 s),
small arcs crawl over the vein, thickening, and a light swells around it, with one crackle as it starts;
then **one arc per victim**: every living thing within `deposit_arc_radius` (192) that the vein can see
from its own surface takes `deposit_arc_damage` (20) as `DMG_ENERGYBEAM`, the arc drawn to it, so what is
seen is what hurts and cover works. Player and monsters alike, a hazard on nobody's side; with nobody in
reach it arcs into the walls. Arcs start from points on the vein's exposed surface, found once by tracing
in from outside, so none leaves solid rock. Energy is on the Shield's list, so the Pulse parries a
discharge — the parry's first tutor, with no enemy attached. Veins start out of step with each other,
keep their place in the rhythm across a save, and stop when broken. Vanilla's lightning sprite and zaps,
in the Shard's amber pushed toward white.

**Stations** are `func_station` (`dlls/station.cpp`), a brush on `func_recharge`'s shape: one use press is
one trade of Shards for something the player cannot find enough of. The recipe is a row of
`game_shared/station_defs.h` chosen by `stationtype`, not free-form keys — a **Fuel processor** takes ten
Shards, a full Stack, for one Skill Point, exactly once; an **Ammunition station** takes three for 20
uranium or for 2 Cores, `uses` times (default 3, `-1` unlimited, `0` placed spent). The Prompt states the
trade in the table's own words — *Fuel processor / [E] Insert 10 Shards for a Skill Point* — and a spent
one shows a state line, *Spent* or *Empty*, so recipes are known rather than discovered. **Nothing is taken
unless the whole output can be given**: too few Shards, or ammunition that would not all fit under the
carry ceiling, is refused with the charger's refusal and the reason on the centre line, before a Shard
moves. A Skill Point is banked, so it never needs room. Each trade fires the Station's target. The server
owns the Inventory, so a trade is server code calling into it — no new message.

What no custom code touches yet: level traversal, secrets, map flow, navigation aids.

### What's missing

~~The pillar has no definition yet, let alone an implementation.~~ **It has a definition since
2026-09-17**, in [ROADMAP.md](ROADMAP.md#the-shape-of-the-game): a hub with wings that open by Modules
and by restored infrastructure, hard and soft gates, and secrets that hold the ammunition, the crystal
deposits, the text entries and the Skill Points — exploring is how the player arms themselves. The hub
plus wing one is the vertical slice. None of it is built. The three questions below are answered there:
both; new entities (deposits, Stations, collectible entries) *and* new maps; no HUD affordance beyond an
information tab in the Inventory Panel. Kept for the record:

- Is exploration rewarded through **items** (pillar 3), **skill points** (pillar 4), or both?
- Does it need new entities (discoverable caches, lore pickups, optional-area triggers), new map work, or
  only tuning of existing HL maps?
- Is there any navigation/HUD affordance (compass, objective marker, map notes), or is it purely level design?

### Next step

The reward loop has two concrete answers now, and the question at the top of this section is settled: it is
**both** items and Skill Points.

**Row Grants.** Inventory capacity grows from things found in the world rather than from Skills (see
pillar 5), so a hidden cache off the critical path permanently increases what the player can carry.
`item_rowgrant` is a placeable entity as of 2026-08-31, which closed the last gap here — all three
exploration rewards are now things a map can hold.

**Skill Points and Reset Tokens.** Both are found in the world and nowhere else — `skill_points_start`
defaults to 0, so a player who explores nothing unlocks nothing (see pillar 4). `item_skillpoint` and
`item_resettoken` are placeable entities today. ~~The tree is deliberately sized so it is completable only
by near-exhaustive exploration~~ — **reversed 2026-09-14**: under the matrix design in
[SKILL_TREE.md](SKILL_TREE.md#the-matrix--settled-2026-09-14) the tree is deliberately *not* completable,
and ~~50–70 findable points buy a third to a half of it~~ **(renumbered 2026-09-15)** 100 findable points
buy about 71% of its ~140 nodes, 40 of them on the critical path and 60 in optional spaces. Reach is still
what exploration buys; it now buys more of a build rather than the last of the tree.

~~What is still missing is the same thing in every case: **maps**. Every mechanism now exists and nothing
places one, because vanilla Half-Life maps cannot be edited to hold them.~~ **Corrected 2026-09-15**:
`topmap`, the default test map, places all three rewards, so every loop is reachable in play there as well
as through `inv_addrows`, `skill_addpoints` and `skill_addtokens`. The proving map
([PROVING_MAP.md](PROVING_MAP.md)) is tested and closed, and the Skill Point economy is a non-issue, so
nothing here waits on a map to judge the numbers. What the pillar still lacks is level design with
something to explore.

### Acceptance criteria (draft)

- A player who explores off the critical path is measurably better off than one who does not.
- The reward is visible in the UI at the moment it is earned.

---

## 2. Enhanced combat

**Status: Playable**

**Planned:** [weapons](ROADMAP.md#pillar-2-weapons) — the Carbon Pickaxe, the Gauss Katana, Evolutions,
weapon handling and viewmodel hands — and [monsters and bosses](ROADMAP.md#pillar-2-monsters-and-bosses).

The Pulse is the first custom mechanic in the mod that changes how the game plays, and the first Skill
effect of any kind. **Since 2026-09-16 it is a found Module, not suit hardware**
([ADR-0013](adr/0013-the-pulse-is-a-found-module.md)): `item_pulsemodule` opens `EGate::PulseModule`,
and without it the key does nothing, the bar is absent (`PULSE_NONE` on `gmsgPulse`) and every Pulse
node is a blank pad. Vanilla maps place no pickup, so the Pulse exists there only through
`give item_pulsemodule` or `skill_open_gates 1`. Untested in game.

### What exists

**The Pulse** — `dlls/player_pulse.cpp` / `.h`, a `CPlayerPulse` held by value in `CBasePlayer` alongside
`m_skills`. Four hooks, all server-side:

- `CBasePlayer::ImpulseCommands()` — `impulse 150` raises a Shield.
- `CBasePlayer::TakeDamage()` — a standing Shield refuses qualifying damage before armour, before the
  suit's damage report, before `m_lastDamageAmount`.
- `CBasePlayer::PreThink()` — closes a window that has run out and picks the Recharge; completes the
  Recharge and chimes.
- `CBasePlayer::Spawn()` / `Save` / `Restore` — reset on spawn, `FIELD_TIME` timers across saves.

**Four Skills** that actually do something: `PulseWindow` (id 12, renamed from `CrowbarParry`) →
`PulseRecharge` (15) → then a branch into `PulseDischarge` (16) and `PulseRebound` (17). Placed in
column 11, with Rebound in column 12 so the branch has room to grow.

**The Rebound** — a window that deflected something skips its Recharge outright. Held as a *count*
(`m_iRebounds`), spent at window close, and all of them restored by sitting through a normal Recharge.
That caps the chain at two Shields back to back, then a real wait.

It is a count and not a flag on purpose: a later Skill raising the ceiling to 2 gives a three-Shield burst
and changes nothing else. `PulseMaxRebounds()` is the only place that would need editing.

**Why a Rebound cannot overlap.** It is granted when the window *closes*, never at the moment of the
deflect. The player therefore can never be Ready while a Shield is still standing, so two Shields cannot
coexist and the "window always runs its full duration" invariant holds. Granting it at deflect time was
considered and rejected for exactly that reason.

**The tail** (2026-09-18, overnight; the mechanic **verified in game** the same day, **the visual not
accepted**: Andrei, "the visual indicator needs to be addressed in order for the mechanic to be clear and
rewarding to the player" — see [ROADMAP](ROADMAP.md#the-pulses-tail--settled-2026-09-17-built-2026-09-18-overnight-visual-open)). A window that deflected **nothing** does not
fall; the Pulse stands on, braced, until `skill_matrix_hold` (1 s) after the press — the moment a hold
would raise the Defense Matrix, so window, tail and Matrix are one motion with no unprotected gap. A hit
in the tail on the Shield's damage list lands at `pulse_tail_scale` (0.5); falls and drowning are not
halved. Only the window is a deflect: a hit in the tail earns no Discharge, no Follow-Up, no Rebound and
no short Recharge, and the long Recharge waits for the tail to end. A window that **did** deflect ends
exactly as before, so the parry, the Rebound and the short Recharge are untouched. Pulse Window widens
the window inside the second and never extends it. The tail begins with a dimmer, smaller ring, and a
braced hit sounds like the deflect pitched far down; the bar counts the tail and the Recharge after it as
one wait. Its length is the Matrix's own cvar rather than one of its own, which could only ever disagree
with it. Mashing the key buys about a second of cover per four-second cycle, most of it at half.

**Sixteen tuning cvars** (`pulse_tail_scale` the sixteenth), registered in `dlls/game.cpp` — fourteen for behaviour, two for the ring's
look — plus `hud_pulse_tint`, which is client-side and `FCVAR_ARCHIVE` because it is a comfort setting
rather than a tuning knob.

**Feedback** — nested rings plus a `TE_DLIGHT` flash, and sounds on Pulse, on each deflect, on a denied
press, and on Recharge completing. Stock placeholder assets, except the Recharge cue, `player/recharged.wav`,
imported from Team Fortress 2 (2026-09-15). The Pulse is `weapons/cbar_miss1.wav` and
a deflect is `weapons/cbar_hit1/2.wav`, randomised and pitched: a swing and an impact, because the first
attempt used two electrical samples and the deflect landed a fraction of a second after the Pulse's own in
the same timbre and was simply not heard. The set is placeholder but the *property* is not — see
[ART_DEBT.md](ART_DEBT.md), which makes "the Pulse and the deflect must not share a timbre" the bar any
replacement has to clear.

The ring geometry is cvar-driven — `pulse_ring_style` picks `TE_BEAMCYLINDER` (a ring expanding along the
ground, borrowed from the houndeye) or `TE_BEAMTORUS` (screen-aligned and centred on the player, so it
reads as a bubble), and `pulse_ring_scale` sizes it. Nothing else in the SDK uses the torus, so there was
no existing call site to take a correct scale from; these exist to be dialled in by eye.

**The readiness readout** — `CHudPulse` (`cl_dll/hud_pulse.cpp`), sitting immediately right of the armour
readout on the same baseline. The `suit_full` sprite with a vertical charge bar beside it filling
bottom-up. Geometry mirrors `CHudBattery` so the two stay aligned at any resolution. It used to have a
private cyan, because it shares the armour's sprite and colour was the only thing telling two identical
icons apart; it gave that up when the HUD started following the Suit Variant, since a cyan suit would
have erased the difference anyway. The charge bar carries it meanwhile, and a sprite of its own is in
[ART_DEBT.md](ART_DEBT.md).

While a Shield stands the screen is tinted in the suit's colour, alpha via `hud_pulse_tint` (0 disables),
and the Shield's own rings and light are that colour too.
**`m_Pulse.Init()` is registered first in `CHud::Init` on purpose:** `AddHudElem` appends, so Init order is
draw order, and the tint has to go under every other readout rather than over it.

`gmsgPulse` carries `(state, duration in tenths)` and is sent **only on a state change** — the client runs
the fill off its own clock from the duration it was given. A whole Pulse costs three 2-byte messages
instead of one per frame. `CPlayerPulse::ForgetSentState()`, called where `m_fInitHUD` is handled in
`UpdateClientData`, forces a resend after the client's HUD is reset so the bar cannot go stale.

**The Defense Matrix — built 2026-09-16, untested in game.** The Juggernaut Route's stance, shaped
2026-09-13 ([ROADMAP](ROADMAP.md#juggernaut--resilient), [SKILL_TREE](SKILL_TREE.md#juggernaut)), and
the Route's last three nodes: Defense Matrix (id 157), Matrix on Kill (158) and the Major, Decaying Armor
(159). All of it lives in `CPlayerPulse` beside the Shield, because it is the same key.

- **Hold the Pulse key for `skill_matrix_hold` (1 s) and the Matrix comes up.** The tap's Shield still
  fires at the front of the press (`OnPress` is `TryPulse` plus the start of the hold), so a hold begins
  with the deflect window and the Matrix follows it; the two are separate verbs on one key. One attempt per
  press: a hold that finds the Matrix on cooldown, or a player with no armour and no Major, gets the denied
  buzz once rather than every frame. Without the Skill a held key is just a held key.
- **While it stands, nothing reaches health.** `CBasePlayer::TakeDamage` replaces the armour split: armour
  pays for the whole hit at `skill_matrix_armor_cost_scale` (0.5) per point of damage, so a 20 hit costs 10
  AP and 0 HP and a point of armour buys two of health; a hit the pool cannot cover spends the rest on
  health. **Reworked the same day it was built.** The first shape scaled the stock split's ratio to 5%
  and could not be read in play even with `debug_damage` on, because the stock split already sends 80% of
  a hit to armour; moving the last 15% was invisible and no defence at all. Health frozen while the armour
  figure drains is the readable version. **Armour is the pool** — no second bar, and nothing refills by
  waiting. Falls and drowning skip armour in the base game and so skip the Matrix without a carve-out.
- **The slow is the Route's whole cost**, paid only while the protection is on: `pev->maxspeed` is set to
  `sv_maxspeed` × `skill_matrix_speed_scale` (0.8) through `pfnSetClientMaxspeed`, which the engine hands
  the movement code as `pmove->clientmaxspeed` and `PM_CheckParamters` clamps against. A cap, not a rule
  change, so it survives the rule that cut Sprint and High Jump. Applied on change from the state rather
  than at raise and drop, so a save, a spawn and a cvar edit all come out right.
- **It stands for `skill_matrix_duration` (6 s) or until zero armour**, whichever first, then waits
  `skill_matrix_cooldown` (10 s). **The key is held only to raise it**: the first shape dropped the Matrix
  on release and tied a hand up for its whole life, and Andrei took that out after the first play. A save
  mid-Matrix comes back with it standing and the right time left, as a Shield or an Infusion does.
- **Matrix on Kill**: a monster the player kills while it stands restores `skill_matrix_kill_armor` (15) up
  to `PlayerMaxArmor` and no further. `CBaseMonster::Killed` tells the player through
  `CBasePlayer::OnMonsterKilled`, once per monster, whatever the weapon; `CLASS_NONE`, `CLASS_PLAYER` and
  `CLASS_PLAYER_ALLY` do not count, so killing a scientist for armour is not the Route's way to sustain.
- **Decaying Armor, the Major**: raising the Matrix grants `skill_matrix_grant` (100, set "to be toned
  down") **above the cap**, on purpose — a grant that respected `PlayerMaxArmor` would do nothing at full
  armour. The grant fades over the Matrix's own duration — grant over `skill_matrix_duration` per second,
  derived rather than a knob, so it is gone in the moment the Matrix drops and the two read as one thing
  (Andrei's tuning after the first play; 10 per second was the first guess) — and is spent like any armour
  before then, so it is fuel: Overdraw's drain and the Matrix's own share eat it first, and a hit that ate through
  it shrinks what is left to fade. It is the Energy tie: raise the Matrix, gain a hundred armour, fire the
  egon into it.
- **What the player sees and hears.** `gmsgMatrix` carries an `EMatrixState` (none, ready, up, cooldown)
  and a duration, sent on change like `gmsgPulse`; *none* is a player without the Skill, so they see
  nothing. With it: **a Matrix bar** right of the Pulse's charge bar in the same vocabulary — full in the
  suit's colour when ready, white and draining while it stands, dim and refilling on cooldown — and the
  Concealment icon lays itself out after it. **The screen's edges tint** in the suit's colour while it
  stands, six bands fading inward (`hud_matrix_tint`, the edge alpha, 110; `hud_matrix_tint_width`, the
  depth as a fraction of the screen's height, 0.12), so the frame reads as armoured and the middle stays
  clear. The armour readout goes white at full strength too; its bar still clamps at the cap and the number
  is what shows the grant above it. Sounds: a suit-coloured light and the slave's zap on raise, the zap
  again on drop, and **the Pulse's ready chime pitched down when the cooldown ends** — the readiness cue
  for a player not looking at the bar. All placeholders, in [ART_DEBT.md](ART_DEBT.md). Under
  `debug_damage` every raise, drop, kill, hit split and the grant's end prints a `matrix:` line.
- **Where it sits.** Since the same day, the Matrix trio runs down the Juggernaut's east column off the
  armour road, touching no Pulse node; Ricochet is a spur beside the Alien door, and the Pulse block is on
  the west side ([SKILL_MAP.md](SKILL_MAP.md)).
- **Nothing else changes.** No node touches the slow, refill rate and delay are not nodes, and the Status
  page's Armor efficiency still reports the standing ratio: the Matrix is situational, like Swap Surge.

**The Gauss Katana, v1** — `weapon_katana`, `dlls/katana.cpp`. The mod's first custom weapon, built as
`CCrowbar` with two hooks overridden rather than as a copy: `BaseDamage()` reads `sk_plr_katana1-3` (60,
against the crowbar's 10) and `SwingDelayScale()` reads `katana_swing_time_scale` (2.4, so 0.6 s after a
hit) for the slash and `katana_wave_swing_time_scale` (1.0, the crowbar's rate) for the right click,
through `skill_tuning.h`, because the delay it scales is predicted and the client must see the same
number. **Burst and DPS, set 2026-09-14 against the gauss's two clicks**: the slash is one heavy cut, the
right click a light blade plus the wave at four a second for uranium. The
Backstab, Melee Reach, Force and Speed, the Melee Damage Stat nodes, Cleave and the Follow-Up therefore apply to it with no code of their own,
which is what subclassing buys. **Its blade is energy damage, since 2026-09-14**: a third hook,
`SwingDamageType`, is `DMG_CLUB` on the crowbar and `DMG_ENERGYBEAM` on the katana, and every monster the
blade or the Cleave arc reaches takes that type, so a katana Backstab passes the Gargantua's filter and
the alien grunt's plating. Anything that is not a monster still takes `DMG_CLUB` from every roster
weapon, so the crowbar rules on breakables (instant on a crowbar-sensitive crate, double damage) hold for
a blade too. It sits in the melee bucket beside the crowbar, `impulse 101` gives it,
and `weapon_katana` is in the FGD. Its viewmodel and world model are the mod's own (`models/v_katana.mdl`
on the crowbar's hands and animations, `models/w_katana.mdl` lying flat); the third-person model, the
sounds and the HUD icon are the crowbar's and are in [ART_DEBT.md](ART_DEBT.md).

**Two clicks, since 2026-09-14** (the first step of the rework in ROADMAP.md, taken with Cleave). **The
left click is the slash**: the blade alone at full damage, Melee Speed on it, and the click that carries
Cleave, whose air shock is gauss-orange on the katana (`CleaveSweepStyle`). **The right click is the old
swing entire**: the blade light (`katana_wave_blade_damage`, 10, so the left click stays the melee verb;
the melee Skills still multiply it) and the wave thrown off it. **The wave is a projectile, since 2026-09-14**: `CKatanaWave`
(`dlls/katana.cpp`), unseen, born 32 units past the blade and stepping along the aim at `katana_wave_speed`
(1200) to `katana_wave_range` (1200), each step swept as a line then the small hull, striking everything
damageable it meets once with `katana_wave_damage` (15) of energy damage, full out to
`katana_wave_full_range` (800) and then falling off to nothing at the range. Its look is the crescent, a
`)` tilted to the cut (`EV_KatanaArc`, client-side from `events/katana_arc.sc`), which reads those three
cvars by name so the two are one flight, and burns a line of glows where its belly meets a wall. Only the
belly ends the flight, and only on a wall: floors and ceilings are scraped and flown through, on both
sides, and the tips scrape whatever they meet, so a doorframe or a ceiling beam no longer eats the wave in
a low room. It makes the katana a ranged melee weapon, and a crowd on the path is a crowd hit. **The wave
spends uranium**: `katana_wave_cost` (5), divided by Energy Efficiency (four with it), checked on both sides
because it gates a predicted swing, and refused with the empty click when short. The katana carries
uranium for it (`KATANA_DEFAULT_GIVE`, 20, on pickup, and the counter on the HUD from
`sprites/weapon_katana.txt`), and since the slash needs none it can always be drawn and is never switched
away from empty (`CanDeploy`, `IsUseable`, and the select-on-empty and no-auto-switch item flags). Both
clicks share one cadence, and Cleave never spends on the right.
**Every swing heats the blade** — the light at the hand and the hot skin, from `events/katana_swing.sc` on
the left and from the arc event on the right — and the lore is that swinging heats the energy in the blade
at no loss while a thrown wave spends some of it. **The swing leaves a trail** in first person
(`cl_dll/katana_trail.cpp`, since 2026-09-15): the ribbon the blade sweeps, guard to point, gauss orange
and additive in the crescent's sprite, fading over `katana_trail_life` (0.12 s), sampled only while an
attack animation is playing, weighted by the blade's speed (`katana_trail_speed`) so it belongs to the
cut and thins away in the recovery, kept in view space so a mouse turn never smears it, and drawn
through OpenGL from inside the viewmodel's studio draw so it sits on the blade this frame and stays
whole against walls as the blade does. Details and the open questions are in
[ROADMAP.md](ROADMAP.md#the-gauss-katana).

**The Carbon Pickaxe, v1** — `weapon_pickaxe`, `dlls/pickaxe.cpp`, built 2026-09-18, verified in game the
same day.
The mining tool and the third weapon on the crowbar's swing, `CCrowbar` with two hooks overridden like the
katana: `BaseDamage()` reads `sk_plr_pickaxe1-3` (25) and `SwingDelayScale()` reads
`pickaxe_swing_time_scale` (1.5, so 0.375 s after a hit and 0.75 s after a miss) — 2.5× the crowbar's hit
at 1.5× its time, better burst and better sustained damage than the free crowbar, well under the katana.
Every melee Skill, the Backstab, Cleave and the Follow-Up come with the subclass. **It alone mines**:
`IsMiningTool()` on `CBasePlayerItem` is false everywhere else, and a crystal deposit asks the striking
player's active item rather than reading a damage bit. Melee bucket, position 2; `impulse 101` gives it;
the FGD places it. It is **the crowbar in black metal** (`models/{v,w}_pickaxe.mdl`, a palette remap
compiled by `E:\CustomAssets\scripts\pickaxe_black.py`, with its Grid Icon rendered from the new world
model); its sounds, third-person model and HUD icon are still the crowbar's — see
[ART_DEBT.md](ART_DEBT.md#the-carbon-pickaxe--a-black-crowbar-and-the-crowbars-sounds-and-hud-icon). Not
yet the starting tool; that is a campaign decision.

**Custom HEV gloves on every viewmodel.** Fourteen stock viewmodels plus the katana compile with three
glove skin families — grey plates with cyan, red or purple light channels. Which one the player sees is
the Suit Variant they wear, below.

**The Suit Variant** — which of the mod's three HEV suits the player wears, defined once for both DLLs in
`game_shared/suit_defs.h`. Three values, each a codename for a specialization the suit does not yet have:
**Agility** (cyan, 0), **Strength** (red, 1), **Intelligence** (purple, 2). Cosmetic today; the codenames
are there so the names survive the day it is not, and what happens to a suit being *taken off* is
deliberately unanswered until then. Vocabulary in [CONTEXT.md](../CONTEXT.md#the-suit).

- **The choice is a world pickup. Switching is use-only; the first suit is walk-over.** `item_suit`
  gains a `variant` keyvalue (0/1/2, with FGD choices); a vanilla `item_suit` with no keyvalue is
  Agility, so stock maps are unchanged and there is no second classname. A suitless player walks into
  the locker at Anomalous Materials exactly as in vanilla. A player already wearing a suit has to look
  at the other one and press use — the Pickup Prompt (`FindLookedAtPickup`,
  `dlls/player_inventory.cpp`) reads "HEV Suit (Strength)" before they commit — because the old variant
  does not drop and a brush past a locker must never switch it
  ([ADR-0011](adr/0011-pickups-are-walk-over.md)). It never enters the Grid — it is worn, not carried.
- **Any suit pickup switches.** Using a suit of another variant changes the player's variant, plays the
  short logon line, leaves armour untouched, and consumes the pickup. A suit of the variant already worn
  is refused, exactly as the stock item refuses a second suit. Nothing drops.
- **The value is the player's `pev->skin`.** The engine saves it with the entity and networks it in
  entity state (`entity_state_player_t` carries `skin`, `network/delta.lst`), so the whole feature costs
  **no new save field and no new user message**. `CHud::UpdateSuitVariant` reads it back off the local
  player once a frame at the top of `Redraw`; everything drawn after that asks `gHUD`. It is also
  literally the skin a future three-variant player model would use. When the suit gains mechanics, that
  is the moment to add a named field and derive the skin from it.
- **The gloves follow it.** `V_SetViewModelSkin` (`cl_dll/view.cpp`) picks the viewmodel's skin family
  from the variant — three families are the suit alone, six are suit × the katana's cold/hot. The
  `cl_suit_variant` stand-in cvar is gone.
- **The HUD follows it too.** `RGB_SUIT` in `cl_dll/hud.h` replaced `RGB_YELLOWISH` at every readout that
  was drawing in the HUD's amber: health, armour, ammo, the ammo history, the flashlight, the train
  controls, the damage icons, the Concealment readout's resting colour, the Pulse, and the Inventory
  panel and Grid. It resolves through `gHUD.SuitColour()` at every draw, which is exactly what the HUD
  already did with `RGB_YELLOWISH`. Three derived shades — `RGB_SUIT_DIM`, `_LIT`, `_OFF` — cover where
  the amber palette had a darker fill, a lighter label and a switched-off one; they are computed from the
  accent so the relationships survive the hue changing.
- **State colours deliberately stay fixed**: red for low health and for a Spotted player, amber for
  Noticed, the Inventory's red close button and its red for a weapon with no ammo. They carry meaning the
  suit colour must not override. The ammo reserves panel keeps its cyan for the same reason — it is what
  tells that section apart from the two above it.
- **The accent colours are the glove generator's**, copied into `suit_defs.h` from `VARIANTS` in
  `E:\CustomAssets\scripts\hev_gloves.py` (cyan 60/220/255, red 255/70/50, purple 200/90/255), so gloves
  and HUD agree by construction rather than by two tables being kept in step by hand.
- **The pickup model is a stand-in** — the stock `w_suit` recompiled with three skin families from a
  colour wash, by `E:\CustomAssets\scripts\suit_world.py`; the entity's skin is set from the keyvalue on
  spawn, so the three are tellable apart on a floor. Filed in [ART_DEBT.md](ART_DEBT.md).
- **`cl_suit_debug 1`** prints the raw skin off the local player's entity state next to what the HUD made
  of it. It exists because the value travels a route nothing else in this mod uses, and two things about
  that route are worth watching rather than assuming: that it reaches the client in single player, and
  that it survives a `changelevel`.

There is **no console command** to set the variant. Variants are tested by placing `item_suit` entities
of each variant in the test map — see [MAP_BRIEF.md](MAP_BRIEF.md).

**The Melee Route, and two damage Skills.** All of them follow the `PulseWindowFor` pattern — the
modifier is read from `m_skills` where the value is computed, rather than through a hook of its own. The
melee ones live in `CCrowbar::Swing`, which every weapon on the melee roster (crowbar, katana) goes
through, so "melee" is true by construction rather than by a damage-type list:

- **Melee Reach** (id 1, *Crowbar Reach* until 2026-09-14) scales the swing trace by
  `skill_melee_reach_scale` (1.25), on **both** sides. The server still decides whether a hit landed; the
  client's copy of the trace only picks which swing animation plays, and reaches as far as the server's.
- **Melee Speed** (id 11, back from reserve 2026-09-14) scales the miss and hit delays by
  `skill_melee_speed_scale` (0.7), on both sides too, since the delay is predicted. It could return because
  **Valve's rapid-swing halving is gone**: any swing within about a second of the last used to do half
  damage, which made a speed Skill buy only faster half-hits. Every swing does full damage now; sustained
  melee damage roughly doubles for a player holding the button, absorbed by the base-damage and swing-time
  cvars.
- **Melee Force** (id 2, *Crowbar Force* until 2026-09-14) scales melee damage by `skill_melee_force_scale`
  (1.5), applied *before* the Follow-Up so a primed swing multiplies the already-stronger hit rather than a
  base one.
- **Nine Melee Damage Stat nodes** (ids 24–32), the Route's roads. Each adds `skill_stat_melee_damage`
  (0.05) to one multiplier — additive within the stat, so five are ×1.25 — applied after Force. The count is
  `CPlayerSkills::CountStat(EStat::MeleeDamage)`; there is no per-node code.
- **Leech** (id 48, the Medical Route's, 2026-09-14) heals the player `skill_leech_fraction` (0.1) of a
  melee hit's damage, on a living monster only — crates do not bleed — for every roster weapon, and per
  victim in a Cleave. The figure is the swing's damage before Weapon Mastery and the hitgroup, which is
  what the tree lets the player reason about. `TakeHealth` refuses at full health, so a Leech at full
  does nothing. Medical × Melee, the sustain the glass-cannon build lacks.
- **The Backstab node** (id 33) multiplies a Backstab by `skill_backstab_bonus_scale` (1.5) on top of the
  weapon's own Backstab base, `CCrowbar::BackstabScale()` (the plain `backstab_damage_scale`, 3, until a
  weapon that leans on it — the knife — overrides it). 3× becomes 4.5× with the node.
- **Cleave** (id 34), the Route's major. **While it is ready, the swing is the arc**: everything within
  `cleave_radius` (160) of the eyes, inside the arc (`cleave_arc_dot` 0.77, so 40° either side of the
  aim; the first guess of 80 and 60° read as short and wide), with a clear line, and able to take damage —
  a crate as much as a zombie, exactly the set the line trace could have hit — takes the swing's damage
  times `cleave_damage_scale` (1.5). **The point tested is the nearest point of the victim's box to the
  eyes, since 2026-09-14**, not its centre: the drawn wave is the region's edge, so a body the wave
  visibly reaches must be hit, and on the centre a monster at the edge or the side was reached and
  missed. Standing inside a monster's box counts, straight ahead. Force and the Stat nodes are
  in that number; the Backstab is tested per victim, monsters only; a primed Follow-Up multiplies every
  victim and is spent once. The line trace still runs for the wall decal and sound, and does no damage of
  its own on a Cleave swing. **Spent on the swing, hit or not**, then `cleave_cooldown` (4 s; 8 read as too
  long). The first shape, tried and rejected the same day, fired only off a landed primary hit, so a swarm
  had to line up and the middle one be struck. The cooldown is the player's, not the weapon's
  (`m_flCleaveReadyTime`, saved as a time), so a swap to the katana does not hand out a second.

  **What the player sees and hears.** On the swing: **an air shock** — the front edge of the region as a
  bow, born at the weapon and travelling out to the radius, widening as the sector widens, at full
  strength until the last quarter of the radius and then out, so it dies exactly where the hit test ends
  — from `events/cleave.sc` → `EV_Cleave` (`cl_dll/ev_hldm.cpp`), with the radius and half-angle carried
  in the event from the server's cvars; and the crowbar's miss sound pitched down, from the same event.
  Its look is **per roster weapon** (`CCrowbar::CleaveSweepStyle()`, carried in the event): white air for
  the crowbar, gauss-orange for the katana. A stationary gold bow was the first build and read as a
  fence; motion is what says "air". Client cvars shape it: `cleave_wave_time` (0.12 s to cross the
  region whatever its radius; a quarter second was the first guess and visibly trailed the hit, which
  lands at the swing, and a fade from birth left the bow all but gone by three quarters of the way, so
  the region read as shorter than the test), `cleave_wave_segments` (32; each segment is one quad of
  texture, and too few read as a row of tiles), `cleave_wave_lag` (0.04 s between the right end of the
  bow leaving the weapon and the left, so the front crosses the arc the way the swing did rather than
  ringing out of it; 0.08 at the old speed) and
  `cleave_wave_height` (2 units at the far edge, from about a third of that at birth; 28 was the first
  guess and read as heavy, 14 still did). **The wave dies against walls and passes through bodies**:
  every point of the bow is traced a step ahead each frame and judged the way the katana's crescent
  judges its probes: only a hit on a brush model is a wall. A point that meets a world brush, a door or
  a crate stops there and is drawn no further; a monster or the player is not a wall, and the point flies
  on through them. So a Cleave down a corridor shows the wave hitting the walls either side and the middle
  running on. A wall plays an impact once per wave at the first contact. **The wave is cosmetic, decided
  2026-09-14**: the damage landed at the swing and each victim shows it in its own blood and flinch, so
  the wave carries no hit test and no body sound. The first shape broke on bodies too, and the bow is born
  inside the player's own box, so it broke on the player as it left them. The trace flags were tried first
  (`PM_STUDIO_IGNORE`) and did not keep the player out, because a player's physent is a plain box rather
  than a studio model; judging the hit is what works. On ready: the status icon at the left edge (through
  `gmsgStatusIcon` like the Infusion's, from `CBasePlayer::CleaveThink`, re-sent after a HUD reset) and a
  quiet cue when it comes back from a cooldown. **All placeholders** ([ART_DEBT.md](ART_DEBT.md)) except
  the swing itself, which is half done. **The Cleave swing has its own sequence, since 2026-09-15**:
  `CROWBAR_CLEAVE`, appended as sequence 11 to `v_crowbar.mdl` and `v_katana.mdl` alike, so the katana
  swings it too. In the slot is **HL Extended's `attack_swing_miss3`**, retargeted onto Valve's
  eleven bones by `utils/mdltool/smd_retarget.py` (its rig carries them under other names with the
  same rest values, so the retarget is a rename and a prune): a low horizontal swipe right to left
  over the first ten frames with the bar flat across the view and the fork leading, then a long return
  where the bar comes up, stands and settles — 36 frames at 30 fps, 1.2 s. It replaced the mod's own
  script-authored swipe (`E:\CustomAssets\scripts\crowbar_cleave.py`) after five cuts judged in HLMV
  got the positioning right and never quite the bar's attitude; the lessons are in
  [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md), *What the Cleave swipe taught*. The animation is imported,
  not made: [ART_DEBT.md](ART_DEBT.md) has its own entry (*The Cleave swing — imported from Half-Life:
  Extended*) with the two ways out, credit it or replace it, and it does not ship until one is taken.
  **Cleave-ready reaches the client** for it — a flag in clientdata `fuser4`
  (`UpdateClientData` → `HUD_WeaponsPostThink`, cleared by the predicted swing that spends it; the
  field's entry in `network/delta.lst` had to be widened, since Valve's 2 bits at ×128 carried nothing
  above 0.008 and the flag arrived as 0, which is why the first install played the stock swing) — because
  the swing animation is predicted and the miss animation is played only by the crowbar's own event,
  which now carries the sequence in `iparam1`. **A Cleave swing recovers in `cleave_swing_time`** (1.2 s,
  the animation's length; a first guess, since the swipe itself is over in the first third and the
  next click could be allowed to cut the return) times Melee Speed, hit or miss, and not the weapon's
  own swing scale, so the swipe is seen whole on both models; 0 means the stock delays. `CCrowbar::CleaveSequence()` returns it
  and `FollowUpSequence()` still returns −1: the Follow-Up has no animation and its primed state is
  still server-only; the Follow-Up's wins when a swing is both. `debug_damage` prints one `cleave ->`
  line per victim, with its Backstab and Follow-Up flags.
- **The Follow-Up shows itself too.** A primed Follow-Up puts its own icon at the screen edge for the
  window it is primed (`CPlayerPulse::SyncFollowUpIcon`, the Follow-Up's tree icon as a placeholder), and
  the swing that spends it plays `CCrowbar::FollowUpSound()`, a placeholder per roster weapon. On a plain
  swing it is still spent by the hit that connects, as before.
- **Weapon Mastery** (id 4) scales every player weapon by `skill_weapon_damage_scale` (1.1), at two
  chokepoints rather than per weapon — `ApplyMultiDamage` and the direct-`TakeDamage` branch of
  `RadiusDamage`. Why it is two, and the two consequences that fall out of it, are under
  [pillar 4](#4-skill-trees).
- **Marksman** (id 35, the Weapon Specialist's root, 2026-09-14) multiplies player `DMG_BULLET` damage by
  `skill_marksman_scale` (1.15) at the same chokepoints, and each **Bullet Damage Stat node** (ids 40–46,
  the Route's roads) adds `skill_stat_bullet_damage` (0.05) to one multiplier on it. A damage-type test,
  not a weapon list: the glock, MP5, shotgun and python are bullets; the crossbow's bolt is not.
- **Energy Damage** (id 54, the Energy Route's root, 2026-09-14) is the same test on `DMG_ENERGYBEAM`,
  ×`skill_energy_damage_scale` (1.15), and each **Energy Damage Stat node** (ids 60–63) adds
  `skill_stat_energy_damage` (0.05) to one multiplier on it. The katana's slash and wave, the egon and
  the Discharge all read it, so the katana scales off Melee (in its Swing) and Energy (here) both — the
  Gargantua build in one sentence. **Insulation** (id 58) scales `DMG_ENERGYBEAM` and `DMG_SHOCK` taken
  by `skill_insulation_scale` (0.7) in `CBasePlayer::TakeDamage`, shock included so it means something in
  Xen. **Energy Efficiency** (id 56; Egon Efficiency until the katana's wave spent uranium, 2026-09-14)
  scales the interval between the egon's ammo ticks by `skill_energy_efficiency_scale` (1.33),
  server-side, where `CEgon::Fire` spends them, and divides the katana's wave's uranium cost by the same
  number, on both sides through `skill_tuning.h` because that check gates a predicted swing.
- **Ricochet** (id 64, the Juggernaut's, 2026-09-14, off Armor Expert until the Route's region exists).
  In `CBasePlayer::TakeDamage`, after the Shield's answer and before the suit's report: a bullet hit
  while armour is above zero has `skill_ricochet_chance` (0.2) of being refused outright, the shooter
  taking the full damage as `DMG_BULLET` with the player as inflictor, a `TE_TRACER` and the stock
  ricochet spark from a point just in front of the player toward them (at the player's own centre the
  spark sat inside the view model and was never seen), and one of the stock `weapons/ric*.wav` from the
  player at full volume. Bullets only, armour only: the first test had no armour on and read as the node
  doing nothing. Under `debug_damage` every bullet hit prints a `ricochet:` console line with the skill,
  armour, roll and chance that decided it. Ranks later.
- **Demolitions** (id 37, 2026-09-14) is the same test on `DMG_BLAST`, dealt ×`skill_demolitions_scale`
  (1.25) at the chokepoints and taken ×`skill_demolitions_resist_scale` (0.5) in `CBasePlayer::TakeDamage`
  before the armour split, own grenades included, which is how "Mastery makes your own explosives hurt you
  more" is answered: with a node worth buying for the resistance alone. The egon's splash carries the blast
  bit beside its energy and is scaled too; accepted rather than special-cased.
- **Headhunter** (id 38, 2026-09-14) multiplies the head hitgroup multiplier by `skill_headhunter_scale`
  (1.5) in `CBaseMonster::TraceAttack`, through `SkillHeadshotScale`, for player hits only. Decapitation,
  when built, keys on the same hitgroup; the Panthereye's head is hitgroup 2 and needs remapping first.
- **Swap Surge** (id 39, the Weapon Specialist's major, 2026-09-14). `DefaultDeploy` on the server, the
  one place every weapon comes up through, opens a window (`skill_swap_surge_window`, 2 s, counted from
  the swap so a faster draw is more of it spent firing) if the Skill is held and the cooldown
  (`skill_swap_surge_cooldown`, 6 s, also from the swap) has passed; while it is open everything the
  player deals is scaled at the chokepoints by `skill_swap_surge_scale` (1.5). A window rather than one
  empowered shot, so the egon and MP5 get their burst as much as the shotgun and python get a big first
  shot. Both times are the player's and saved. No readout yet: the player has to feel the window, which
  is a thing to judge before an icon is drawn for it.
- **Fast Reload** (id 3) scales the reload delay by `skill_reload_time_scale` (0.8) through
  `CBasePlayerWeapon::ReloadTimeScale`, applied in `DefaultReload` — the one place every clip-fed weapon
  funnels through, so the glock, MP5, python, crossbow and RPG all get it without a per-weapon list. The
  shotgun feeds shells one at a time and never calls `DefaultReload`, so since 2026-09-14 its own reload
  state machine calls the same scale on every timing of the sequence: the start, each shell, and the pump
  after the last one.
- **Quick Draw** (id 36, 2026-09-14) scales the draw delay by `skill_draw_time_scale` (0.6) through
  `CBasePlayerWeapon::DrawTimeScale`, in **both** copies of `DefaultDeploy` (`dlls/weapons.cpp` and
  `cl_dll/hl/hl_weapons.cpp`), since the delay it sets is predicted.

  Since 2026-09-15 the stock draw/reload animations play sped up to match, instead of running under a
  shortened timer at their normal rate. `SendWeaponAnim` took a third argument, `framerate` (default 1.0,
  `dlls/weapons.h`), and every call site that scales a timer by `DrawTimeScale()`/`ReloadTimeScale()`
  passes the reciprocal (`AnimSpeedupFor`, also `dlls/weapons.h`) — the shotgun's hand-rolled reload
  included. The client is the only side that acts on it: `HUD_SendWeaponAnim`
  (`cl_dll/com_weapons.cpp`) holds it, and `StudioDrawModel` applies it to the view model's
  `curstate.framerate` before the bones are set up. It is applied at draw time because writing it once
  did nothing: the engine rebuilds the view model's state every frame. The rate is tied to the
  sequence it was set for, and anything else on the view model plays at 1.0. That matters because fire
  and melee swing animations start from client events (`EV_WeaponAnimation`, `cl_dll/ev_hldm.cpp`), never
  pass through `HUD_SendWeaponAnim`, and would otherwise inherit the last draw's rate. Bespoke per-tier animations, rather than a sped-up
  stock one, remain unbuilt and would be [ART_DEBT.md](ART_DEBT.md) work if pursued.

  This is the first Skill that changes a **predicted** value, and it is the proof the prediction fix
  works. `m_flNextAttack` is owned by the client frame to frame, so the two sides shortening the reload
  differently would hitch at the end of every one. Both read the same cvar through
  `dlls/skill_tuning.h`, and both read the same `m_skills` — see pillar 4.
- **The Hive nodes** (ids 20, 21, 141, built 2026-09-16, untested in game) scale the hivehand rather than
  change it. **Hive Capacity**: `PlayerHornetMaxCarry` (`dlls/player_skills.cpp`) is `HORNET_MAX_CARRY` +
  `skill_hive_capacity_bonus` (4), read by `CHgun::Reload` and the multiplayer refill — though the
  `ItemInfo` registry cannot ask a player, so ammo boxes and `CanHaveAmmo` still cap a pickup at 8, a named
  gap. **Hive Replenish**: `PlayerHornetReplenishScale` divides the regrowth interval by
  `skill_hive_replenish_scale` (1.5), times one plus the held count of the Hornet Replenish Stat nodes
  (`skill_stat_hornet_replenish`, 0.05 each) — printed under `debug_damage`. **Hive Attack Speed**: both
  fire intervals × `skill_hive_attack_speed_scale` (0.75) through `skill_tuning.h`, both DLLs since the
  cadence is predicted; the fire animation itself is not sped up, a visible gap the way Quick Draw's used
  to be.

**The Backstab** — `CBaseMonster::FInRearArc` (`dlls/combat.cpp`) plus `CanBackstab()`, applied in
`CCrowbar::Swing`. A melee hit landed in a monster's rear arc deals `backstab_damage_scale`× (3) when the
angle beats `backstab_arc_dot` (-0.5, the rear 120°).

It is filed here rather than under stealth on purpose. It was designed as pillar 6's payoff and came out
**positional only** — whether the victim has noticed the player does not enter into it — which makes it a
melee mechanic that stealth happens to make easy to set up, rather than a stealth mechanic. The reasoning,
the rejected alternatives and the exclusion list are in
[ADR-0010](adr/0010-the-backstab-is-positional.md).

Three things worth knowing rather than rediscovering:

- **It stacks between Melee Force and the Follow-Up**, so each stage multiplies an already-stronger hit
  and the largest number a player can produce is every bonus at once. 3× is tuned to land just short of
  one-shotting a grunt at full melee investment (10 × 1.5 × 3 × 1.1 = 49.5 against 50 health), on the
  grounds that a reliable one-shot removes any reason to fight a grunt head-on.
- **`CanBackstab()` is a virtual, not a saved flag**, because `Spawn()` does not re-run on restore
  (`dlls/cbase.cpp:380-389`) and the exclusion list is per class rather than per instance. **Any future
  per-monster-type property should take the same shape** — the Perception Profile especially.
- **`FInRearArc` does not call `UTIL_MakeVectors`**, unlike `FInViewCone`. It is called partway through
  resolving a hit, and `gpGlobals->v_forward` still holds the player's aim vector needed by `TraceAttack`
  and the Follow-Up knockback.

~~Which leaves **Crowbar Speed** (id 11) as the only combat Skill that does nothing~~ — stale, corrected
2026-09-18: it was built as **Melee Speed** with the Melee Route on 2026-09-14, and the first-swing/follow-up
rule that blocked it was dropped the day before, so every swing is full damage.

**The melee alien grunt, v1** — `monster_alien_grunt_melee`, `CAGruntMelee` in `dlls/agrunt.cpp`, built
2026-09-18 overnight, verified in game the same day — "a very good base, it will be extended and refined
later" (Andrei). The alpha's chainsaw grunt, first stage: a bare-handed brawler on the
punches the stock grunt already has (`mattack2`/`mattack3`, `sk_agrunt_dmg_punch` as `DMG_CLUB`, the
250-unit shove). The arm bodypart's bare submodel, and no hornets (`CheckRangeAttack1` is false). **Every
melee grunt in a squad chases**: where the stock grunt asks for the one chase slot and, refused, stands off,
this one goes after the enemy regardless, and the hornet grunts keep the slot rules they have — so a mixed
squad has its hornet grunts holding back and shooting while the melee ones close. Alien military recruit
across classnames, so mixed squads form on their own. **No armour**: the stock grunt's plates take 20 off
every blow and ricochet it, which made it immune to the crowbar on most of its body; this one bleeds
wherever it is hit, and the Backstab and Follow-Up work on it. It looks armoured and is not — the model's
plates, and the bare arm standing in for the chainsaw, are in [ART_DEBT.md](ART_DEBT.md). With it, for
every alien grunt: a punch the Pulse deflects still shoves, but no longer plays the hit sound or draws blood.

**The maddened miner, v1** — `monster_maddened`, `CMaddened` in `dlls/maddened.cpp`, built 2026-09-19 and
**verified in game the same night** ("it works"). The mod's first human enemy: a miner who heard the
voice, on the player's own body from the SDK sources (`models/maddened.mdl`, `maddened_build.py`), with a
pick in his hand as a bodygroup and overalls or the suit as skins. The base AI's chase and melee, nothing
scripted: he walks unaware and runs when he chases (the grill said walk; the walking one read as no threat
in play and the decision reversed the same night), swings for `sk_maddened_dmg_swing`
(10/15/20, `DMG_CLUB`) at 64 units, has `sk_maddened_health` (50/60/70), mutters the alien slave's words
every 4–8 s as his tell in the dark, and is `CLASS_MADDENED`: everyone's enemy, allied with his own kind,
on a relationship table grown to 16×16. The default Perception Profile, backstabbable, no flinch. The first
stands at shaft1's vein, unsuited. [MADDENED.md](MADDENED.md) is the full record and the list for his grill.

**The Panthereye, v1** — `monster_panthereye`, `CPanthereye` in `dlls/panthereye.cpp`, built 2026-09-18,
**untested in game**. Half-Life's cut monster on HL: Extended's model and sounds (copied into `models/` and
`sound/panthereye/`), with its AI written new from the roadmap's settled behaviour; v1's scope was grilled
the same day. A lone `CBaseMonster`, `CLASS_ALIEN_PREDATOR`, always the blue body, backstabbable, on the new
**Predator** Perception Profile (fill ×2.0, drain ×0.25, hears Disturbances). **Two modes, one way.** It
**stalks** an enemy that has not seen it — a straight path to the player, running (200 u/s) beyond
`panther_stalk_crawl_dist` (512) while off screen, otherwise crawling on its belly (43 u/s) through
`crouch_to_crawl` and `walk_to_stand_1`, all by name over `ACT_RUN` — and growls every 3–5 s (`pa_idle3` at
`panther_growl_pitch` 75, `ATTN_IDLE`, no `CSoundEnt`). It turns to **combat for good** when the player sees
it (within `panther_spot_cone` 30° of the crosshair, a clear line from the eyes to its centre or head, held
`panther_spot_dwell` 0.25 s, no light term, no range limit), when the player hurts it, or when it starts a
slash; the alert plays then, not on acquisition. The claws work in both modes, every hit event
`sk_panthereye_dmg_claw` as `DMG_SLASH` (the double swipe hits twice). The pounce is combat only: the
headcrab's leap from `panther_leap_min`–`_max` (150–450), capped at `panther_leap_speed` (900), one hit on
touch in the air for `sk_panthereye_dmg_leap`, `panther_leap_cooldown` (3 s), the crouch before take-off
rescaled to `panther_leap_windup` (0.55 s, the model's own frame-11 event). Health 60/70/80, claw 13/15/20,
leap 20/25/35 — HL: Extended's numbers, not yet tuned. The model's head hitbox (group 2) counts as a head.
`panther_debug` centre-prints the spotted test. `minemap` has a den for it west of the hall, behind a door.

### What's missing

- ~~Attack-rate modifiers — see Crowbar Speed above.~~ Built as Melee Speed, 2026-09-14.
- A custom Shield sprite; `sprites/shockwave.spr` is standing in.
- **Tuning.** Every number is a first guess, and `pulse_ring_style` still has to be judged one way or the
  other so the winner can become the default.

### Design — the Pulse

Settled 2026-08-01. Vocabulary is in [CONTEXT.md](../CONTEXT.md); the two decisions with lasting
consequences are recorded in [adr/0005](adr/0005-the-shield-negates-a-curated-damage-list.md) and
[adr/0006](adr/0006-the-discharge-vents-at-the-crosshair.md).

**What it is.** The HEV suit emits a Pulse, raising a Shield for a brief Pulse Window during which
qualifying damage is negated, followed by a Recharge. Lore-wise the suit is mining equipment and the Shield
is what protects its wearer from falling debris.

**Availability.** ~~Suit hardware, not a Skill — `pev->weapons & (1 << WEAPON_SUIT)`. No Pulse before
Anomalous Materials. Skills evolve a verb the player already has rather than granting it, which lets level
design assume it.~~ **Superseded 2026-09-16** by [ADR-0013](adr/0013-the-pulse-is-a-found-module.md): the
Pulse is a found Module, `item_pulsemodule` opens its gate, and level design may assume it only after
that pickup. Still no Skill's to grant.

**Trigger.** `impulse 150`, handled in `CBasePlayer::ImpulseCommands()`. Deliberately *not* a button bit:
`usercmd_t.buttons` is an `unsigned short` (`common/usercmd.h:29`) and `common/in_buttons.h` already spends
all 16 usable bits. `usercmd_t.impulse` rides the same per-tick packet, so timing fidelity is identical to a
button, and it is self-clearing (`dlls/player.cpp:3615`) so the press is edge-triggered for free. Also
deliberately not a crowbar secondary attack — that would tie a suit ability to one weapon and drag it into
client prediction, where the player-owned Recharge state does not exist. **Since 2026-09-16 the key is
`+pulse`** (`cl_dll/input.cpp`), a press-and-release pair that sends `PULSE_IMPULSE` (150) on the press and
`PULSE_RELEASE_IMPULSE` (152) on the release, both in `game_shared/pulse_defs.h`, so the server can time a
*hold* for the Defense Matrix below; a release that lands in the same frame as its press waits one command
so the server always sees both. A bare `impulse 150` bind is still a tap and can never raise the Matrix.

**The window always runs its full duration.** Negating something does not close the Shield early, so
several attackers landing hits in one window are all negated. The Recharge length is decided when the
window ends: short if the Shield negated anything, long if it negated nothing. Good reads chain, whiffs
strand you.

**What is negated** — see [ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md). Verified
against the three intended test targets: headcrab leap (`dlls/headcrab.cpp:355`), zombie slashes
(`dlls/zombie.cpp:199`, `:223`, `:245`) and alien slave claws (`dlls/islave.cpp:316`, `:337`) are all
`DMG_SLASH`; the slave's zap (`dlls/islave.cpp:831`) is `DMG_SHOCK`. All four are on the list. The slave is
the best single test subject — it is the only one of the three with both a telegraphed melee and a hitscan
attack.

**The Discharge** is Skill-gated: on each negated hit it fires immediately from the gun position toward the
crosshair for `clamp(absorbed × scale, min, max)` damage, as `DMG_ENERGYBEAM`. One Discharge per negated
hit. A generic bolt ships first, with a per-attacker override table slotted in later without rework — see
[ADR-0006](adr/0006-the-discharge-vents-at-the-crosshair.md).

It fires on **melee** deflects too, not only ranged ones — deflecting a zombie swipe sends a beam wherever
you are aiming. That was never designed; it falls out of "one Discharge per negated hit", which does not
care what dealt the damage, and has been true since the Discharge was first written. It plays well, so it
is on by default and should not be "fixed" as an oversight — but it is the part of the Pulse most likely
to be judged wrong under more testing, so `pulse_discharge_melee 0` turns it off on its own.

`DMG_ENERGYBEAM` rather than `DMG_SHOCK` is also deliberate. The alien slave is the only thing in the game
immune to `DMG_SHOCK` (`dlls/islave.cpp:585`), and slaves are a headline target for the Pulse, so a
counter they alone shrugged off was the wrong counter. `DMG_ENERGYBEAM` has no immunity anywhere and is
what the Egon fires.

**Skills — three nodes.** `PulseWindow` → `PulseRecharge` → `PulseDischarge`. `ESkillId::CrowbarParry`
(id 12) is renamed to `PulseWindow` and **keeps id 12**: the id stays stable on the wire and in saves, and
only the meaning changes, which is safe because it currently has no meaning. Its existing description
already promises exactly this behaviour.

**Presentation.** Expanding ring via `TE_BEAMCYLINDER` + `sprites/shockwave.spr` — the pattern at
`dlls/houndeye.cpp:576-596` — with a custom sprite later. Sounds on Pulse, on each negated hit, and on
Recharge completing. Plus a HUD recharge indicator, which is the only part needing client DLL work: a new
user message and a `CHud` element.

**Tuning** through cvars, following the `inv_rows_*` precedent: `pulse_window`, `pulse_window_bonus`,
`pulse_recharge_hit`, `pulse_recharge_miss`, `pulse_recharge_scale`, `pulse_discharge_scale`,
`pulse_discharge_min`, `pulse_discharge_max`. Every number is a starting guess to be judged in play.

**State** lives in a `CPlayerPulse` held by value in `CBasePlayer`, mirroring `CPlayerSkills` —
`m_flShieldEndTime` and `m_flPulseReadyTime` as `FIELD_TIME` (which rebases on restore, so timers survive
save/load and level transitions) plus a `FIELD_BOOLEAN` recording whether the window absorbed anything.

**The Follow-Up** — `FollowUp` (id 18, `CrowbarFollowUp` until 2026-09-14) primes the next melee swing for `pulse_followup_time` seconds after a
deflect; the next swing that **connects** deals `pulse_followup_damage`× and is then spent. A whiff costs
nothing, so the timer rather than the swing is what stops it being banked.

Knockback is **headcrabs only**, alive or dead, and that restriction is the point. Half-Life does not knock
monsters back from damage at all — the one place it happens (`dlls/combat.cpp:891`) is gated on
`MOVETYPE_WALK`, which is the player; monsters are `MOVETYPE_STEP`. The corpse equivalent was written and
then disabled (`dlls/combat.cpp:986`, *"turn this back on when the bounding box issues are resolved"*), so
this is a road Valve started down and abandoned. Headcrabs are small and light enough for the result to
look right, and the restriction keeps it away from anything mid-script, boss-sized, or standing where it
must not be shoved. `pulse_followup_knockback` sizes it; `0` disables it.

Applied *after* the damage, deliberately, so a headcrab the hit killed is still thrown. If the hit gibs it
there is no body left and the gibs fly on their own.

### Known weaknesses of this design

Recorded now so they are not rediscovered as bugs:

- **Hitscan is a guess, not a read.** HECU grunts telegraph nothing, so Pulsing against gunfire is
  anticipation rather than reaction. The mechanic will feel best against melee and projectiles. The
  candidate answer is a fourth node, **`PulseSustain`**, where each absorbed hit *extends* the Pulse Window
  (~+0.3s per hit) — turning sustained fire from what punishes a blind Pulse into what rewards it. It needs
  a hard ceiling before it is built: four grunts land hits far faster than 0.3s apart, and uncapped the
  Shield would simply never drop. It also compounds with the asymmetric Recharge, since a long sustained
  Shield would still be followed by the *short* success Recharge.
- **`DMG_CRUSH` will occasionally feel arbitrary** — it is on the list for the fiction, but most crush
  damage in Half-Life comes from doors, trains and lifts.

### Next step

Tune. The mechanic is confirmed to feel rewarding in play against headcrabs, zombies and alien slaves;
every number behind it is still a first guess.

The pattern set by `PulseWindowFor` / `PulseRechargeFor` in `dlls/player_pulse.cpp` — a modifier read
from `m_skills` where the effect is computed — is now what every Skill in the tree does, and it held: no
Skill invented its own hook. Fast Reload is the first one to run through it on *both* sides, and it needed
no new pattern to do so: the same `HasSkill` call, in the same place, in a file that happens to compile
into the client too.

Weapon Mastery is the one that needed care in *where* the modifier is read, because "every weapon you
carry" is a claim about coverage rather than about one call site. The two-chokepoint answer and the two
consequences it produces are recorded under [pillar 4](#4-skill-trees), with the rest of the effects work.

### Acceptance criteria (draft)

- Every combat skill in the tree has a measurable in-game effect.
- Effects are computed server-side; the client never decides damage.
- No skill effect is applied twice (prediction and server both).

---

## 3. Custom items

**Status: Playable**

**Planned:** the rest of the [Modules](ROADMAP.md#pillar-3-modules) — the Pulse as a Module, and the Hook — plus the item
side of [Stations](ROADMAP.md#stations).

### The Dash — built 2026-09-15, untested in game

The first Module after the long jump it rides on. **Tap SHIFT** (`impulse 151`, `DASH_IMPULSE` in
`pm_shared/pm_shared.h`) for a burst along the direction the movement keys point — sideways and backwards
included, forward when no key is held — ~~from the ground only~~ **on the ground and in the air alike since
2026-09-18** (overnight, verified in game the same day): the flat Dash runs its time wherever it is, leaves gravity alone
in the air, and no longer ends at a ledge or on a jump, because a Dash glued to the ground cannot cross a
gap. Walk moved to **ALT**.

- **Found with the long jump.** `item_longjump` still gives the long jump and now also opens
  `EGate::DashModule` — the Shinobi region — with every charge ready. Having the Dash *is* that gate being
  open, so `skill_open_gates 2` grants it for testing, and a save whose player already had the long jump
  opens it on load.
- **Predicted.** The burst runs in `pm_shared.cpp` (`PM_CheckDash`, `PM_DashBurst`) off `pmove->fuser1`,
  the milliseconds left, carried in clientdata and through `HUD_TxferPredictionData`. Friction is skipped
  while it runs; when it ends, horizontal speed drops back to run speed. ~~Leaving the ground ended it too,
  so a Dash off a ledge or into a jump did not carry~~ — reversed 2026-09-18 with the Dash in the air.
- **Charges on the server.** `CBasePlayer::DashThink` refills one float of charges and writes physinfo
  keys (`dsc` ready, `dsn` ceiling, `dsv` speed, `dst` ms, `dsr` recharge); `DashAfterMove` spends a
  charge when fuser1 rose across the move. Not saved — a load comes back full.
- **Numbers**, all cvars and first guesses: `dash_speed` 800, `dash_time` 0.15 (about 120 units),
  `dash_recharge` 7 per charge.
- **Sound:** a placeholder whoosh on every Dash (`DASH_SOUND`, [ART_DEBT](ART_DEBT.md)). A ground Dash
  plays no footsteps while the burst runs; monsters still hear the speed, since body noise follows velocity
  (Phantom is the node that silences it).
- **The Shinobi Skills with an effect:** Dash Reach (`skill_dash_reach_scale` ×1.5 burst length), Dash
  Recovery (−`skill_dash_recovery` 0.25 of the recharge), each Dash Recovery Stat node
  (−`skill_stat_dash_recovery` 0.05), summed and floored at ×0.2; Second Wind (two charges); **Reprisal** —
  a melee hit that kills a monster at full health gives a charge back (`CCrowbar::ReprisalRefill`, at both
  hit sites, once per victim of a Cleave); **Air Dash** — below. Phase still has none.
- **The Air Dash.** With the major held (physinfo `dsa`), a Dash started in the air goes along the
  crosshair instead — up and down included — and the movement keys do nothing in the air. Since
  2026-09-18 it is the *directional* dash rather than the only one that leaves the ground, and its tooltip
  says so; its gate behind the double jump is settled and **not built**, because it strands the Dash
  Recovery Stat node at (1,0), an open placement question. Gravity is off
  while it runs, it ends when its time is up or it lands, and it **stops dead**: no speed carries on, so
  one reaches about 120 units. Any ready charge can be spent in the air, back to back. A dive into the
  floor lands as a fall, and takes fall damage. `fuser1` counts down negative for an Air Dash, so the two
  kinds end on their own rules with no second networked field.
- **Readout:** `CHudDash`, one bar per charge after the Concealment icon, read straight from physinfo.

The framework landed with inventory iteration 1, and the **Health Syringe** is the first item in the mod
that Half-Life does not have.

### The Night Vision Module — built 2026-09-16, untested in game

The fifth Module, settled 2026-09-15 and the Stealth region's reveal
gate ([SKILL_TREE.md](SKILL_TREE.md#the-night-vision-module)). `item_nightvision` (`dlls/items.cpp`) opens
`EGate::NightVision`; it needs the suit and turns a lit flashlight off first. With the gate open,
`impulse 100` (`FlashlightTurnOn` / `FlashlightTurnOff`) sets `EF_NIGHTVISION` instead of `EF_DIMLIGHT` —
the bit sat unused since the SDK and `pev->effects` is already saved, so night vision costs no new save
field and no new user message. **The flashlight stays until the Module is found**; the Module replaces it
outright, same battery drain and recharge, same key, same HUD icon. `gmsgFlashlight` grew to three bytes
(on, battery, mode) on all three senders. The client (`cl_dll/flashlight.cpp`, `DrawNightVision`) tiles
Opposing Force's `of_nv_b.spr` noise additively (copied from the gearbox install into the repo's `sprites/`
and `topmod/sprites/`) and allocates an eye-level client dlight keyed to the player's index; two cvars,
`nv_overlay` (160, the overlay's alpha, 0 off) and `nv_light_radius` (700, 0 off), both `FCVAR_ARCHIVE`.
Monsters are unaffected — no light touches the world — which is what closes the flashlight hole in the
light term recorded in [PERCEPTION.md](PERCEPTION.md#concealment--built-2026-09-01) (a flashlight never
touches the baked lightmap, so a lit corridor and a dark one conceal the same) by removing the flashlight
from the equation rather than fixing it. `models/w_silencer.mdl` stands in for the pickup and `!HEV_A1`
for its voice line, both [ART_DEBT.md](ART_DEBT.md) entries.

### The alien Module — built 2026-09-16, untested in game

The fourth Module, and the [Alien Route](ROADMAP.md#alien)'s first slice: Cores, a ghost, and the summon
weapon's left click. **Cores** are a real ammo type, `CORE_MAX_CARRY` (6); `item_core` gives one
(`w_gaussammo.mdl` stands in). `item_alienmodule` (`dlls/items.cpp`, a stand-in for the freed slave's
hand-over, `w_sqknest.mdl`) opens `EGate::AlienModule` and gives `weapon_summon` with `SUMMON_DEFAULT_GIVE`
(3) Cores.

**`weapon_summon`** (`dlls/summon.cpp`, `WEAPON_SUMMON` 17 in `dlls/cdll_dll.h`, bucket 4 position 4, the
hivehand's models, sprites and sounds standing in, `sprites/weapon_summon.txt` with the gauss ammo icon
standing in for a Core): left click spends one Core for one ghost, on a cooldown (`summon_cooldown` 3, ×
`skill_recall_scale` 0.5 with Recall) up to a cap (`summon_max_ghosts` 1, + `skill_pack_bonus` 1 with
Pack), spawned behind or beside the player — 64 then 96 units out — on a human-hull ground trace with a
clear line from the eye; a refusal costs nothing. Right click idles; the ultimate is the next slice.
Predicted only for the animation and the cooldown — the summon itself is server-only.

**`monster_ghost_slave`** (end of `dlls/islave.cpp`): `CISlave` gained two saved flags on the slave class
itself, so any future friendly slave can share them — `m_bAlly` (`Classify()` returns `CLASS_PLAYER_ALLY`)
and `m_bVanishOnDeath` (`Vanish()`: a `TE_TELEPORT`, `zap1.wav`, and `Remove()` — no corpse). The ghost adds
a summoner handle (`m_hSummoner`) and a lifetime (`m_flVanishTime`, `summon_ghost_lifetime` 30 ×
`skill_tether_scale` 1.5 with Tether), drawn translucent (render amount 160) and without
`FCAP_ACROSS_TRANSITION`. **Not built**: the ghost following the player, the ultimate, the slave's actual
hand-over, and the Alien Route's Major. Worth watching in play: a ghost's zap beam can hit the player
standing in its line, and a ghost behind the player is solid and can shove them.

### What exists

**The Item Type table** — `game_shared/inventory_defs.h`, compiled into both DLLs so the client and server
cannot disagree. Five entries: **Medkit**, **Antidote**, **Keycard**, **Battery** and **Health Syringe**.
Each row carries classname, display name, sprite, Cell width, Stack ceiling and whether the item is
usable. Ids are frozen once written to a save; adding is free, reordering corrupts.

The `usable` flag replaced a hardcoded `id == Medkit || id == Battery` test in the client's context menu,
which was found the only way it could be: the Syringe shipped with no **Use** button. The client now asks
the shared table, so a new item cannot repeat it.

**The Health Syringe** — Item Type id 5, `item_syringe`, three per Stack, one Cell. Its world model is
the mod's own `models/w_syringe.mdl` (since 2026-09-13; `w_adrenaline.mdl` from `valve/models` stood in
before it), and its Grid Icon is a render of that model. Placeable in a level editor via a `@PointClass`
line in `fgd/halflife.fgd`.

It is the first Item Type with **no legacy `m_rgItems[]` twin** — `MAX_ITEMS` is 5, so
`SyncLegacyItemCount` skips it and the Inventory is its only record. That path was already guarded, so
nothing had to change for it.

**The Infusion** — `dlls/player_infusion.cpp` / `.h`, a `CPlayerInfusion` held by value in `CBasePlayer`
alongside `m_pulse` and `m_skills`. Using a Syringe starts one: `infusion_rate` HP per second for
`infusion_duration` seconds, landing as **1 HP every 0.25s** through a fractional accumulator, so the
health readout climbs steadily rather than jumping. Hooks mirror the Pulse exactly — `Think()` from
`PreThink`, `Clear()` from `Spawn()`, `FIELD_TIME` fields through `InfusionSave`/`InfusionRestore`, so an
Infusion survives a save and a level transition with the right time left.

`m_bActive` is an explicit bool rather than an inferred zero timer, for the reason `CPlayerPulse`'s header
documents: `FIELD_TIME` rebases on restore, and a zero sentinel comes back as "just expired".

**Two rules, recorded in [ADR-0007](adr/0007-the-infusion-is-one-at-a-time.md)**: a Syringe may be used at
full health, and a second one is refused while an Infusion runs — refused, not queued, with the Syringe
unspent. Together they make the Syringe proactive where the medkit is reactive, which is the whole reason
it exists alongside one. Nothing interrupts a running Infusion.

**A status icon, and the first sender `gmsgStatusIcon` has ever had.** `CHudStatusIcons`
(`cl_dll/status_icons.cpp`) shipped complete with the SDK and was never wired up — nothing in `dlls/`
registered its message. The Infusion registers it and sends `(1, "cross", green)` on start and `(0,
"cross")` on end, so the icon cost **zero new client code**, and every future durable status is now one
`MESSAGE_BEGIN`.

Note this is a different system from the shock/fire/poison icons beside the health readout: those are
`CHudHealth`'s **damage tiles**, keyed to `DMG_*` bits and self-expiring after 2 seconds
(`DMG_IMAGE_LIFE`), which cannot express "on for exactly 10 seconds". `CHudStatusIcons` is the durable
one, and it draws up the left edge from mid-screen instead.

`ResetHUD` wipes the client's icon list, so `ForgetSentIcon()` is called at the `m_fInitHUD` site in
`UpdateClientData` — without it a save loaded mid-Infusion heals invisibly.

**Med Expert** (Skill id 19) — `+infusion_duration_bonus` seconds, additive. A root node: it was briefly
gated on Regeneration, which was cut on 2026-09-13, and it is the root of the Medical Route in
[SKILL_TREE.md](SKILL_TREE.md#medical).

**Three tuning cvars** in `dlls/game.cpp`: `infusion_rate` (4), `infusion_duration` (10),
`infusion_duration_bonus` (5). Named for the mechanic rather than the Syringe, so a later source of an
Infusion does not inherit syringe-flavoured names.

**The Healing Stat nodes** (ids 50–53, the Medical Route's roads, 2026-09-14) each add `skill_stat_healing`
(0.1) to one multiplier, `PlayerHealingScale`, on the Infusion's rate where the tick lands and on the
medkit's heal in both places a medkit heals (`PlayerMedkitHeal`, so the walk-over kit's "wastes nothing"
test and the Inventory's Use agree). The rate, not the duration: more per second, the same window. The
wall charger is untouched. **Leech** (id 48) is under [pillar 2](#2-enhanced-combat) with the melee Skills.

**Overheal** (id 47, 2026-09-14). With the Skill, the ticks `TakeHealth` would refuse on a full bar go
above the maximum instead, up to `skill_overheal_cap` (50) over it; once the Infusion ends, any health
above the maximum drains at `skill_overheal_decay` (2 per second) down to it and stops. The drain runs
in the Infusion's own Think, on any excess whatever put it there, so nothing else has to know. Read a
little wider than the shaped "a Syringe used at full health": an Infusion that *reaches* full keeps going
too, which is the same waste made a decision. A medkit is still refused above the maximum, so it is not
spent. The Juggernaut's decaying armour grant is the same shape, when it comes.

**Sounds** — `items/smallmedkit1.wav` on pickup, `items/medshot4.wav` on use, `items/medshotno1.wav` on a
refused press, and the `!HEV_HEAL7` suit line ("hiss, morphine_shot"), throttled `SUIT_NEXT_IN_30SEC`.
The *use* sound is deliberately not the medkit's — the two items must not sound alike in the moment they
do their work — which makes the shared pickup sound the weakest of the placeholders. See
[ART_DEBT.md](ART_DEBT.md).

No `gmsgItemPickup` is sent on pickup, unlike the medkit and battery: the pickup-history HUD resolves its
icon from the classname, and there is no `item_syringe` sprite in `hud.txt` for it to find.

### What's missing

- **No map places a Syringe.** Acquisition is `give item_syringe` with `sv_cheats 1` until there are
  custom maps — the same constraint the Row Grant pickup has (pillar 5). The FGD entry exists so the
  moment there is a map, it can be placed.
- **Antidote and Keycard still do nothing when used**, and render without icons. They are carried, not
  consumed, which is deliberate — but nothing in the world reads them either.
- **Numbers are first guesses.** 4 HP/s for 10s is 40 health from one Cell, more than two medkits; the
  duration is what pays for it, and whether that trade is right is a play question.
- The Infusion has no countdown, only an on/off icon. That is by design (ADR-0007) and is the first thing
  to revisit if refusal feels opaque in play.

### Next step

Tune, and then decide whether the second custom item follows the same shape. `InventoryUseEntry` is still
a `switch` over `EItemTypeId`; a function pointer in the table would be tidier but `inventory_defs.h`
compiles into the client, where server-only effect functions cannot go. Adding an item is currently: one
table row, one `EItemTypeId`, one `CItem` subclass, one FGD line, one `case`.

### Acceptance criteria (draft)

- ~~Adding an item means adding one table entry plus its effect, and nothing else.~~ **Met**, as literally
  as the two-DLL split allows — see "Next step".
- ~~Item ids are stable across saves and the network, like skill ids.~~ **Met** — `EItemTypeId` is frozen
  by the same rule as `ESkillId`.
- Every item in the inventory can be used or dropped, or is explicitly marked as neither. *Antidote and
  Keycard are droppable but inert on use; neither is marked as deliberately unusable.*

---

## 4. Skill trees

**Status: Playable**

**2026-09-15: the tree became the board, in one session.** [ADR-0012](adr/0012-the-skill-tree-has-one-start-and-open-roads.md)
is built: the Suit (id 65, `ENodeTier::Suit`, cost 0) is the one start, held from spawn and through a
Reset; **there are no prerequisites** — `SkillReachable` opens a node from any owned orthogonal
neighbour, `prereq`/`prereq2` are gone from `SkillDef`, and with them the connector edges, the
`Requires:` tooltip block and the `skilltree_debug_edges` overlay. Every node sits on the 15×15 board of
[SKILL_MAP.md](SKILL_MAP.md): 160 ids, all nine regions placed, the three Module-gated regions and the
Pulse nodes carrying an `EGate` and drawn as blank pads until the server's saved gate bitmask opens them
(a byte on `gmsgSkillTree`, now 35 bytes; the Pulse gate was open until the Pulse became a Module on 2026-09-16;
`skill_open_gates` / `skill_close_gates` are the cheats). Four compile-time checks hold the board: on the
board, one per cell, no islands, every node reachable from the Suit by flood fill. Built the same day:
Egon Focus, Overdraw, Last Stand and Glass Cannon; the hub's Max Health and Max Armour stats; and the
panel of [SKILL_PANEL.md](SKILL_PANEL.md) — 1:1 with drag to pan, the gauge strip, the hazard-striped
Reset switch, the suit-designation header, and the board drawn as the suit's circuit. **The bullets below
that describe prerequisites, connectors, the `Requires:` list, the column layout or the fit-to-area view
describe the tree as it was on 2026-09-14** and are struck where they contradict; none of it is code any
more.

**Planned:** the effects behind the hidden regions' nodes (Shinobi, Stealth, Alien, the Juggernaut's
Matrix), each with its Module; the art the circuit is drawn with (ART_DEBT). The **Melee Route was built
first, 2026-09-14**, whole; it is the worked example of a Route. Four ids are cut for good.

Every Skill in the tree changes how the game plays. The pillar's own acceptance criterion — "every unlocked
skill has an observable effect" — is met, which is what moved this off Scaffolded.

### What exists

**Definitions** — `game_shared/skill_defs.h`, compiled into both DLLs

- **52 nodes in the tree, every one costing one point** (a `static_assert` holds every row to it): the
  Melee Route in columns 0–3 — Reach at the root, two roads of Melee Damage Stat nodes down to Speed and
  Force, on to the Backstab node, meeting at Cleave — the Weapon Specialist Route in columns 9–11, built
  whole on 2026-09-14 (Marksman at the root in the middle of the region, Bullet Damage Stat nodes as its
  roads, Fast Reload and Weapon Mastery moved into it, Swap Surge at the bottom), the Medical Route
  building in columns 12–13 (Med Expert at the root, Healing Stat nodes as its roads, Leech and Overheal;
  Last Stand reserved), the Energy Route building in columns 14–15 (Energy Damage at the root, Energy
  Damage Stat nodes as its roads, Egon Efficiency and Insulation; Egon Focus, Quick Charge and the major
  reserved), and the pre-Routes columns between them (the Pulse 4–5, the suit 6, Survivability
  7–8, with Ricochet added under Armor Expert), each waiting for its Route to give it roads. Follow-Up sits at
  the seam between Melee and the Pulse because it is gated on one of each. The layout is in the comment
  above `k_SkillDefs`; the design is [SKILL_TREE.md](SKILL_TREE.md#the-matrix--settled-2026-09-14).
- **Stat nodes are rows like any other**, with `ENodeTier::Stat` and an `EStat` naming what they grant;
  `STAT_MELEE(id, col, row, prereq)` stamps one out. A Skill has `EStat::None`. Only stats a built Route
  uses are in the enum.
- **Eight reserved ids** with no row. Four are **cut for good** (2026-09-13): `HealthRegen` (10) and
  `BatteryRegen` (14) rewarded standing still, `HighJump` (5) and `SprintSpeed` (6) altered the normal
  movement rules. `HiveCapacity` (20) and `HiveRegrowth` (21) are held for the alien column below, 22 and
  23 for the stealth column. A reserved row is `SKILL_RESERVED(id)` — the id stays frozen and, for the ones
  that return, the Skill comes back unchanged, which is exactly what `FastReload` (id 3) and `MeleeSpeed`
  (id 11) did.
- **A second `static_assert` refuses two rows in one cell**, the mistake a hand-placed 180-row table will
  make, which would draw as one node hiding another.
- **The id space has a ceiling.** `k_SkillIdCeiling` (256 since 2026-09-14, 96 the day before) sizes the
  saved unlocked array and the sync mask; `ESkillId::_Count` bounds only the definition table, and a
  `static_assert` fires if it ever passes the ceiling. The raise was for the matrix tree, whose Stat nodes
  each take an id; the saved field was renamed again (`m_bUnlocked256`) so a 96-entry save resets rather
  than over-reads. Before this, both were sized by `_Count`, and `CRestore::ReadField` reads as many array
  entries as the code declares rather than as many as the save holds, so every Skill added over-read an
  older save's unlocked bits into the bytes of the field after them. The array is saved under its own field
  name (`m_bUnlockedCeiling`) so a save from before the ceiling is skipped rather than over-read: it loads
  with its tree reset and every point refunded, which is the same self-correction a cut Skill relies on.
- A `static_assert` enforces that the table is ordered by id. It is indexed positionally, so a row out of
  place would silently make a save's unlocked bits refer to different abilities — the grouping that reads
  most naturally to a human is exactly the mistake, so it is a compile error.
- Each `SkillDef` carries id, display name, description, icon, grid column/row, cost (always 1, the Suit
  0), a visual tier (`Stat` / `Minor` / `Medium` / `Major` / `Suit`), a stat and a gate. ~~**two**
  prerequisites … Both prerequisites are required, so a connector line always means "you need this".~~
  **Gone 2026-09-15**: there are no prerequisites and no connectors.
- ~~`SkillPrereqMet`~~ `SkillReachable` is the one implementation of the gating rule — any owned
  orthogonal neighbour — and server and client both call it.

**Server** — `dlls/player_skills.cpp` / `dlls/player_skills.h`

- Per-player state only: points earned, Reset Tokens banked, and the unlocked array.
- **Points are earned, not held.** `m_iPointsBase` (captured once from `skill_points_start`) plus
  `m_iPointsGranted` (found in the world) is what the player has *ever* had; what they have *left* is that
  minus the summed cost of everything unlocked. Nothing decrements. Following the `inv_rows` precedent of
  storing the inputs and deriving the answer buys three things: a reset is `memset` on the unlocked array
  with no refund code to get wrong, retuning a cost corrects existing saves instead of leaking points, and
  a Skill cut from the tree refunds itself on the next load.
- `TryUnlock()` validates prerequisites, cost, and duplicate unlock — server-authoritative.
- `TryReset()` spends one Reset Token and clears every unlocked Skill. It refuses on an empty tree, so a
  Token can never be burned for nothing.
- **Two pickups**, `item_skillpoint` and `item_resettoken` (`dlls/items.cpp`), both plain `CItem`s taken on
  contact. Neither is an Item Type: they occupy no Cells and a full Grid cannot refuse them, which matters
  because a progression reward left on the floor reads as a bug.
- State lives in `CBasePlayer::m_skills` (`dlls/player.h:362`) and saves/restores through a
  `TYPEDESCRIPTION` table (`dlls/player.cpp:3057`, `:3079`).

**Networking**

- `gmsgSkillTree`, **fixed** length: a bitmask of unlocked Skills, one bit per id up to the ceiling, then
  the player's unspent Skill Points, then their banked Reset Tokens. 34 bytes (32 of mask), and the length
  no longer changes when a Skill is added. Static Skill data is shared rather than sent — see
  [ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md).
- Client → server is the `skill_unlock <id>` console command (`dlls/client.cpp:647`); on success the server
  re-sends the state.
- Client handler: `CHudAmmo::MsgFunc_SkillTree` in `cl_dll/ammo.cpp`. It drops any message whose size
  disagrees with `k_SkillMaskBytes` rather than misreading it.

**Client** — `cl_dll/vgui_skilltree.cpp` / `.h`

- `CSkillTreeView`, a plain C++ helper owned by `CInventoryPanel` rather than a VGUI panel of its own.
- Tier-sized nodes, lazily loaded HUD sprite icons, edge-anchored connector lines that prefer vertical
  routing, hover tooltips, a skill-point counter and the Reset button.
- **The tree fits the panel, and icons fit their nodes.** Node size is the tier's designed size
  (`k_TierNodeW/H`: 60×44, 74×54, 88×64), the grid step is the Major node plus a gap, and one uniform
  scale fits the whole thing to the area — never magnifying past the designed size, and stopping at
  `k_MinScale` (0.55), which is now only the floor under the cost text. That replaces a hardcoded 100px
  step which needed a ~1600px-wide screen and was silently clipped below it, because `RebuildRects` clamps
  the centering offset at zero.

  **Since 2026-09-14 the icon is fitted into the node**, through the same scaled draw the Inventory Grid
  uses (`SPR_DrawFitted` in `cl_dll/spr_fit.h`, wrapping `SPR_DrawGeneric` with its frame-over-rect
  correction; the Grid's `DrawFootprintSprite` calls the same function). HUD sprites are
  **resolution-bucketed** — the same icon is 44px at 640 and 88px at 1280 — and until then the node was
  sized *from* the largest loaded sprite, so a 1280 screen got 112×128 Major nodes and the tree scaled
  down to fit them. Now the art has no say in the layout and seven columns fit a 1280 screen at full
  scale. **The fit shrinks but never magnifies**: the engine clips a sprite drawn larger than its frame
  (found the same day on the gauss and egon icons), so a sprite smaller than its node sits centred at 1:1.
  What that asks of the replacement icons is in [ART_DEBT.md](ART_DEBT.md).
- **Nothing is printed on a node.** The cost used to be, bottom-right; it pushed the icon out of the
  node's middle, was judged better hidden (2026-09-14), and then every node came to cost one with the
  matrix, so there is no price to print. The scale floor dropped from 0.55 to 0.3 with it, since the cost
  text was the only thing on a node that did not scale.
- **Three layout cvars** (client). `skilltree_preview_cols` and `skilltree_preview_rows` (default 0) force
  the grid to at least that many columns and rows and draw a Stat-sized ghost outline in every cell with no
  node, to judge the footprint of a matrix at a real resolution before the nodes are built; they can only
  widen the grid, never hide a column the table already uses. `skilltree_step` (default 0, meaning the
  designed 96) overrides the grid step for judging spacing by eye.
- **A layout check** (client), the second half of the pair SKILL_TREE.md asked for before the third Route.
  `SkillDefsOnePerCell` is a `static_assert` that no two rows of the table share a cell.
  ~~`skilltree_debug_edges` (default 0) redraws any prerequisite edge whose ends are not grid neighbours…~~
  **Deleted 2026-09-15** with the edges; the board's checks are now all compile-time (`SkillDefsOnBoard`,
  `SkillDefsNoIslands`, `SkillDefsAllReachableFromSuit` beside the one-per-cell assert).
- **No text labels on nodes, by design** — an icon and a cost, nothing else. Reading the tree means
  hovering, which is the same instinct behind the anonymization feature below. This makes icon
  distinctness *blocking* rather than cosmetic; see [ART_DEBT.md](ART_DEBT.md).
- ~~**Cost is drawn on each node**~~ Nothing is printed on a node since the matrix (every node costs one).
- **The hover bubble** carries name and description only ~~and a `Requires:` list naming unmet
  prerequisites~~ (gone 2026-09-15; the board has no prerequisites, and a hidden node's bubble reads
  `No signal`). Laid out from real font metrics; see [TECH_DEBT.md](TECH_DEBT.md).
- Labels, descriptions, icons, positions, costs, prerequisites and tiers all come from the shared
  `k_SkillDefs`. Adding a skill is one table row. The old client-local `k_SkillUiInfo` copy is gone.
- "Available" is derived client-side from the unlocked mask and the shared table. That is a display
  decision, not an authority change — `TryUnlock` re-validates everything server-side.

### What's missing

- **Nothing, for the effects.** All fourteen Skills in the tree do something. What is left is tuning:
  every number is a first guess, and none has been judged against a full playthrough.

  `MoreHealth` is the one that does not follow the read-it-where-it-is-computed pattern, and could not:
  max health is durable state rather than a value recomputed per hit, so `ApplySkillHealthBonus` is
  *re-applied* at spawn, on unlock and on reset instead of being consulted. It grants the health along
  with the cap — a cap raised alone does nothing until the next medkit, which reads as a broken Skill —
  and clamps back down on reset.
- **Anywhere to earn points.** The mechanism exists — `item_skillpoint` and `item_resettoken` are placeable
  entities — but no map places one, so in practice points still come from `skill_addpoints`. Blocked on
  custom maps, exactly as Row Grants are.
- **Node icons.** The tree is deliberately label-free, which makes icon distinctness *blocking* rather than
  cosmetic — and today five Skills share `suit_full`. See [ART_DEBT.md](ART_DEBT.md).

### ~~Two prerequisites per Skill — DONE~~ Superseded 2026-09-15

**No prerequisites remain** ([ADR-0012](adr/0012-the-skill-tree-has-one-start-and-open-roads.md)):
Follow-Up now sits in the hub's corner cell between Melee and Juggernaut and opens from either side; its
effect still needs a deflect. The section is kept as the record of the 2026-09 state.

A `SkillDef` held two prerequisites and required both. **Follow-Up** (id 18) was the Skill that
forced it — a crowbar payoff for a Pulse deflect, which used to hang off `CrowbarDamage` alone, so a
player could take it having never touched the Pulse tree for a Skill that does nothing without deflecting.
It is now gated on `CrowbarDamage` **and** `PulseRecharge`.

It cost nothing on the wire. [ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md) moved
static Skill data into `game_shared/skill_defs.h`, so prerequisites are no longer sent at all and the
message-size ceiling that used to bound this stopped applying. A third prerequisite would be equally free.

The connector loop builds one edge per prerequisite, so a two-gated node draws two lines. Both mean the
same thing — every line is a requirement — which is why only AND is supported. An OR gate would need a
second line style before it could be read.

### ~~Wanted: the alien column~~ Built 2026-09-16, untested in game

Kept as the record of the pre-matrix shape; ids 20 and 21 (Hive Capacity, Hive Replenish) and Hive Attack
Speed are built as part of the [Alien Route](ROADMAP.md#alien)'s first slice, at one point each under the
matrix rather than the Tier/Cost below — see [SKILL_TREE.md](SKILL_TREE.md#alien) and
[pillar 2](#2-enhanced-combat) for what exists today.

A branch of Skills for alien weapons, **hidden entirely until the player carries one**, so that reading the
tree does not spoil that the branch exists. The column is reserved now rather than built: ids 20 and 21 are
held for it, and it takes a region of its own when it opens (column 7 until the Weapon Specialist
Route rearranged the right of the tree on 2026-09-14).

Half-Life has exactly two alien weapons — the **Hivehand** and **Snarks**. The Gauss, Egon and Tau are all
HEV/human tech and belong in Armaments; the Displacer is not in HL1. So the column is naturally small, and
the Hivehand carries both knobs worth having, since its ammo already regenerates on a timer
(`m_flRechargeTime` in `hornetgun.cpp`):

| Reserved id | Skill | Effect | Tier | Cost |
| --- | --- | --- | --- | --- |
| 20 | Hive Capacity | Hivehand holds more hornets (raises `HORNET_MAX_CARRY`, currently 8) | Medium | 2 |
| 21 | Hive Regrowth | Hornets replenish faster | Major | 3 |

Both effects are server-side constants read at fire and recharge time — no prediction involved.

**Build this together with the anonymization feature below.** They are different rules — "hidden until you
hold an alien weapon" is a possession test, "anonymized past N hops" is a graph-distance test — but they
are the same rendering machinery, and a node that can be absent needs the layout to cope with a column
appearing mid-playthrough. Doing one without the other means writing that twice.

### Wanted: distant Skills are anonymized

Skills more than a few prerequisite hops from the player's unlocked set should render as `???` — no name,
no description, no icon — and reveal as the player approaches them. The far end of the tree becomes a
silhouette to be discovered rather than a shopping list read on hour one, which is the same instinct
behind the tree having no text labels on its nodes at all.

Cheap whenever it is wanted: the client holds the whole graph and the unlocked mask, so it is a
breadth-first walk from the unlocked set plus a branch in the node and tooltip render. Nothing needs
designing first, and no server or wire change is involved.

### Next step

The shared definition table ([ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md)), two
prerequisites, the points/Tokens economy, the curation pass and the UI pass are all in. What remains:

**Tuning.** Every effect exists; not one number has been judged against a full playthrough. ~~No map places
a Skill Point, so the economy is still theoretical.~~ The economy is a non-issue as of 2026-09-15, and
`topmap`, the default test map, places Skill Points. Nothing else here is blocked on code.

Notes from the effects work worth keeping:

**Weapon Mastery is applied at two chokepoints, not per weapon** — `ApplyMultiDamage`
(`dlls/weapons.cpp`) and the direct-`TakeDamage` branch of `RadiusDamage` (`dlls/combat.cpp`). Every
player weapon funnels through one or the other, so "every weapon you carry" is true by construction rather
than by a list somebody has to maintain. Scaling before the branch in `RadiusDamage` would apply it twice,
because the other branch already goes through `ApplyMultiDamage`.

Two consequences that fall out and are worth knowing rather than rediscovering. **The Pulse Discharge is
scaled by it**, because `FireDischarge` passes the player as attacker — defensible (it is the suit's
energy, fired by the player) but emergent rather than designed. And **a player's own explosives hurt them
10% more**, since `RadiusDamage` does not care that attacker and victim are the same entity.

**The predicted player now knows which Skills are held.** Weapon files compile into both DLLs, and the
client's `CBasePlayer` (`cl_dll/hl/hl_weapons.cpp`) always had a real `m_skills` — zero-filled, never
populated. `HUD_SetPredictedSkills` fills it from the same `gmsgSkillTree` message the Skill Tree panel
draws itself from, so `m_pPlayer->m_skills.HasSkill(...)` gives the same answer on both sides.

Two decisions inside that are worth knowing rather than rediscovering:

- **Fed from the message, not from `clientdata_t`.** A scratch field on `clientdata_t` was the obvious
  route and it does not survive inspection: `iuser3` is the engine's duck prevention and `iuser4` its fire
  prevention (`HUD_TxferPredictionData`), and what remains is `fuser`/`vuser`, whose floats would cap the
  id space at 24 bits where the message already carries 40. The mask changes only when the server says so,
  so there is nothing to reconcile per frame and nothing to add to the prediction snapshot.
- **Tuning cvars needed their own route.** They are registered in `game.cpp`, which is server-only, so
  predicted weapon code cannot name them. `dlls/skill_tuning.h` looks them up by name through
  `CVAR_GET_POINTER`, which works in both DLLs because `HUD_InitClientWeapons` points `g_engfuncs` at the
  engine's cvar functions and a listen server shares one registry. Only knobs a predicted value depends on
  belong there; a server-only effect keeps reading its `cvar_t` from `game.h`.

`SprintSpeed` and `HighJump` were never unblocked by this, since `pm_shared/` reaches nothing in
`m_skills`, and on 2026-09-13 they were cut outright: the normal movement rules stay, and reaching is a
Module's job.

**`CPlayerRegen` is gone.** It held `HealthRegen` and `BatteryRegen`, both cut on 2026-09-13 for rewarding
standing still; the file, its `REGEN` save block and its two rate cvars went with them. A save written
before that carries a `REGEN` block, which `CBasePlayer::Restore` consumes with no fields so the `PLAYER`
block after it still reads. Its one reusable idea, the fractional accumulator, lives on in the Infusion
where it came from.

**`PlayerMaxArmor`** is now the only correct answer to "how much armour can this player hold". Every place
that caps or fills armour must ask it rather than `MAX_NORMAL_BATTERY` — the battery item and the wall
charger — or `BatteryCapacity` silently does nothing through that route. It is also
the first Skill whose effect the **client** has to know: `gmsgBattery` grew a second short carrying the
player's maximum, because a HUD bar scaled against a fixed 100 shows full at 100 while the suit still has
50 to take. `m_iClientBatteryMax` is tracked separately from `m_iClientBattery` so unlocking the Skill
resends even when the armour value itself has not changed.

### The economy

**A non-issue, settled 2026-09-15.** The numbers below stand as set. They are not an open question, no
map is waited on to judge them, and no feature is held for them.

Both cvars default to **0**: every Skill Point and every Reset Token is found in the world. Neither is
capped — the ceiling on each is how many pickups a map places, and a cap would let a found pickup silently
do nothing.

| Cvar | Default | What it does |
| --- | --- | --- |
| `skill_points_start` | 0 | Skill Points a new game begins with |
| `skill_reset_tokens_start` | 0 | Reset Tokens a new game begins with |

Cheat-gated `skill_addpoints <n>` and `skill_addtokens <n>` mirror `inv_addrows`. For debugging a
Skill's effect rather than the economy, `skill_unlock_all 1` holds every Skill in the tree at once, cost
and prerequisites ignored: it writes into the same unlocked array a purchase does, so it saves, it reads
as zero points left, and setting it back to 0 takes nothing away — a Reset Token does (and is granted
back at once while the cvar stays set), or a new game.

~~The target is a tree **completable only by near-exhaustive exploration**: total findable points roughly
equal to the tree's total cost, so a player who sweeps every optional space affords essentially everything
by the end while a player on the critical path affords perhaps 60–70% and must genuinely choose.~~

**Reversed 2026-09-14.** Under [the matrix](SKILL_TREE.md#the-matrix--settled-2026-09-14) every node costs
one Skill Point, the roads of Stat nodes between Skills are the price, and the tree of 120–180 nodes is
**deliberately not completable**: ~~50–70 findable points buy a third to a half of it~~, and which part is
the build. A player who sweeps every optional space affords a deeper build, not the whole tree. Reset Tokens
matter more for it, since a road taken is a road paid for. Roughly 5–10 Tokens across the campaign, with
the first appearing around 20% in: enough that the tree is meant to be experimented with, not agonised
over.

**Renumbered 2026-09-15**, with [one start and open roads](SKILL_TREE.md#one-start-open-roads--settled-2026-09-15):
**100 findable points** — 40 on the critical path, 60 in optional spaces, the 2:3 split MAP_BRIEF already
asks of each map — against **about 140 nodes** on a 15×15 grid. A thorough player owns about 71% of the
tree and five or six of the seven Majors; a critical-path player about 29% and one. The 71% was raised as
a concern (most builds converge late) and accepted knowingly over a 200-node tree or 70 points. The tree
also changed shape the same day: the suit at the centre is the one start, held from the first moment,
Stat nodes open from any owned neighbour, and Skills keep their curated gates
([ADR-0012](adr/0012-the-skill-tree-has-one-start-and-open-roads.md)). None of it is built yet; the
code still has seven roots and curated edges everywhere.

None of this is placeable yet — see "What's missing".

Note that `CrowbarParry` (id 12) was renamed to `PulseWindow` rather than removed. Ids stay stable —
only the meaning changed, which was safe precisely because no skill had one at the time. It was the one
free opportunity to repurpose an id, and it is not a precedent now that skills do things.

Note that inventory capacity was considered as the first Skill effect and deliberately moved to exploration
instead (pillar 5). Nothing in the inventory work will give this pillar an effect, so it stays Scaffolded
until a combat skill is wired. The Row Grant counter is source-agnostic, so a capacity Skill can still
grant Rows later without rework.

### Acceptance criteria (draft)

- ~~Every unlocked skill has an observable effect.~~ **Met** — all 14 Skills in the tree do something.
- ~~Skill points are earned through play, not seeded.~~ **Met structurally** — `skill_points_start` is 0
  and points come only from `item_skillpoint`. Not yet met *in practice*: no map places one, so the only
  source today is the `skill_addpoints` cheat.
- ~~Skill ids stay stable; save games from before a skill was added still load.~~ **Met, and enforced** —
  a `static_assert` keeps `k_SkillDefs` in id order, and a Skill cut from the tree keeps its reserved id
  and refunds its cost on load rather than corrupting the save.

---

## 5. Inventory management

**Status: Playable**

**Planned:** a [Records](ROADMAP.md#pillar-1-records) list that is deliberately *not* an
Inventory change.

### What exists

**`CInventoryPanel`** (`cl_dll/vgui_inventory.cpp`) — a VGUI panel with three tabs, Inventory, Upgrades
and Status, each delegating to a plain helper view, and one nav table (`k_NavTabs`) for all three. Opened via the `+inventory` command bound in `cl_dll/input.cpp:990`, and only once the player has the suit
(`TeamFortressViewport::ShowInventory` checks `gHUD.HasSuit()`).

**`CInventoryGridView`** (`cl_dll/vgui_inventory_grid.cpp`)

- Renders weapons and inventory items in one grid with mixed cell widths (weapons occupy three cells).
  Ammo is not in the Grid (ADR-0001); it is listed in the left column.
- Drag and drop that asks the server to move an Entry and draws whatever the next sync says. Nothing is
  placed, clamped or re-packed client-side (ADR-0004).
- **The Grid is a lattice.** Lines sit on a fixed pitch; an Entry's Footprint is inset from its lines by
  the same amount on every side; the integer remainder of the pitch goes to the left, so the Grid's right
  edge meets the header's.
- **Icons are full-colour renders of the world models, laid straight on the lattice.** Decided
  2026-09-12, after System Shock 2: the suit colour stays on the chrome and the thing in the Grid keeps
  its own colour. One alphatest `.spr` per classname at `sprites/inv/`, found by classname with no table
  change, drawn untinted, alpha-blended and fitted to the Footprint through `SPR_DrawGeneric`
  (`DrawFootprintSprite`), so it looks the same at every resolution. No box under it at rest; the Footprint
  shows as a suit-coloured outline on hover and while dragging, and as a red one, at rest, for a weapon
  with no ammo — the one thing the untinted art can no longer say. An Entry with no Icon file (today only
  the Antidote, and the Syringe, which has no world model) keeps its HUD sprite, tinted and additive, in
  its dark box. Eighteen Icons exist: every stock weapon, the katana, medkit, battery and keycard. The
  pipeline is in [SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md).
- `inv_icon_pad` is the fitted margin; `inv_icon_fit 0` and `inv_icon_blend` are comparison switches for
  the HUD-sprite fallback; `inv_icon_debug 1` prints each Icon load and draw.
- Hit rectangles rebuilt every paint.

**The Status tab — built 2026-09-16** (`CStatusView`, `cl_dll/vgui_status.cpp`), designed the same day
in [STATUS_PANEL.md](STATUS_PANEL.md):

- **The doll.** Five fixed Slots drawn as rect frames in a body shape — head Night Vision, body Dash, left
  arm (screen left) Pulse, right arm the Alien Module, legs empty. A Slot fills when its Module's gate is
  open (`openGates` on `gmsgSkillTree`); filled, a stand-in HUD sprite tinted the suit colour, and a hover
  tooltip with name, the key from `Key_LookupBinding` on the Module's command, and a description. Empty,
  a dim frame with no tooltip. The Modules are one shared table, `k_ModuleDefs` in
  `game_shared/module_defs.h`.
- **The stats column.** HEALTH (Health, Healing), ARMOR (Armor, Armor efficiency, Explosive, Energy and
  Fall resistance), DAMAGE (Melee, Bullet, Energy, Explosive), and MODULES — Dash recharge once the Dash
  is found; Concealment (the share by which monsters learn slower) and Hornet replenish (its multiplier)
  went live the same way on 2026-09-16, when the Stealth region and the Hive nodes gave those two stats an
  effect to show (`k_ConcealmentBuilt`, `k_HornetReplenishBuilt`). Multipliers print as ASCII `x1.73`.
- **The numbers come from the server.** `SendSkillStatsToClient` (`dlls/player_skills.cpp`) runs at the end
  of every `SendSkillTreeToClient` and sends fourteen shorts (`k_SkillStatsBytes`) on `gmsgSkillStats`. Each is
  read from the function the effect itself uses: `PlayerStandingDamageScale` (Weapon Mastery and the typed
  damage; `SkillScaleWeaponDamage` is it times Swap Surge and Overdraw, which the page leaves out),
  `PlayerMeleeScale` (`CCrowbar::SwingDamage`), `PlayerArmorRatioScale`, `PlayerBlastTakenScale`,
  `PlayerEnergyTakenScale`, `PlayerFallTakenScale` (`CBasePlayer::TakeDamage` and the fall), plus
  `PlayerHealingScale`, `PlayerMaxArmor`, `max_health` and `DashRechargeTime`. `ARMOR_RATIO` moved to
  `dlls/player.h` for it. Current Health and Armor are the HUD's own.

**Context menu** — right-click gives Use and Drop. Weapons issue `use <classname>` and close the panel;
items issue `inv_use <classname>` and stay open.

**HUD integration** — while the panel is visible, `CHudAmmo::Draw` returns early (`cl_dll/ammo.cpp:923`) so
the default weapon sprites don't bleed through the panel.

### What's missing

Iterations 1 and 2 are **done**, and dropping is complete — the model is server-owned, saved and
capacity-limited, things enter the Inventory deliberately, and anything in it can be dropped back out.
Remaining:

- ~~**No Row Grant pickup.**~~ **Built 2026-08-31.** `item_rowgrant` is a `CItem` granting +1 Row on
  contact, alongside `item_skillpoint` and `item_resettoken` in `dlls/items.cpp`. It is the only
  progression pickup with a real ceiling — `inv_rows_max` — so it is also the only one that can arrive
  with nothing left to give, and it **refuses and stays standing** in that case rather than being
  consumed for nothing. A map that trips it has placed more Grants than the ceiling allows, and the
  author should see it still sitting there. Not yet placed by any map.
- **No Boxes**, so nothing in the world holds items — lootable caches are still future work.
- No custom Item Types yet — the table holds the four stock ones.
- The ammo readout overflows its panel when the player carries many ammo types; it needs a taller panel,
  a scroll, or two columns. Deferred to a UI pass.
- The Pickup Prompt is unstyled — it uses the engine console font pending a visual style for the mod.
- **The Prompt, since 2026-09-18, verified in game**: the Pickup Prompt widened to everything a use press
  acts on. A title and `[KEY] Action` with the key `+use` is really bound to — buttons, valves, use-only
  doors, the two chargers, scientists, guards, pushables, mounted guns, and a bare `Use` for anything else
  (`game_shared/prompt_defs.h`). Pickups that never reach the Grid (Skill Point, Reset Token, Row Grant,
  the Modules, the silencer, the Core) and ammunition are named and taken by a use press too, as
  `EEntryKind::Pickup`, which like `Suit` is never an Entry. Ammunition still reads a generic
  *Ammunition*. It is slice 1 of [Records](ROADMAP.md#pillar-1-records); no mapper override, suppress or
  state lines yet.
- **Untested:** dropping an exhaustible weapon (satchel, tripmine, snark, hand grenade). Their ammo is the
  item itself and lives in the pool rather than a clip, so the clip-transfer on drop does nothing for them.
  Nothing should be lost, but it is the one weapon class whose drop path has not been exercised.
- `inv_rows_max` has been eyeballed at 9 against vanilla content only. Re-judge it once there are more
  weapons and items than vanilla has.

### Deliberately deferred

Designed, agreed, and **not** in the first server-side version. Recorded so they aren't rediscovered as
bugs:

- **Boxes, and everything that needs them.** Dropping a Stack was going to need a `CWeaponBox` extended
  with `(itemTypeId, count)` arrays. It does not: "Drop all" spawns one world entity per item with a
  small scatter, and a Stack is at most a handful. Boxes are now purely a future feature — lootable
  caches and the two-panel loot window that goes with them.
- **Container UI.** Taking things out of a box would be "take whatever fits, leave the rest", driven by
  the same look-and-use prompt as any other pickup, with the loot window as a later presentation change
  over the same `TryAdd` logic. Nothing depends on it today.
- **Splitting and merging Stacks by hand.** Pickups merge automatically; there is no manual split. A
  player wanting to drop one medkit out of five must drop all five. Purely additive to fix.
- **Auto-sort / re-pack button.** Deliberately absent. Nothing may re-arrange the Grid behind the
  player's back — that was the whole point of making placement explicit.

Two hazards in `CWeaponBox` that must be fixed *before* anything puts items in a box, or the player's
belongings get deleted:

- `IsEmpty()` (`dlls/weapons.cpp:1357`) only checks weapons and ammo. A box holding only items would
  report empty and be removed.
- `Touch` clears each ammo slot unconditionally after `GiveAmmo` (`dlls/weapons.cpp:1184`), destroying
  ammo that didn't fit in the player's pool.
- Boxes created by the multiplayer death-drop path get `SetThink(&CWeaponBox::Kill)` (`dlls/player.cpp:698`)
  and self-destruct on a timer. A box holding deliberately dropped belongings must never do that.

### Agreed design

Settled 2026-08-01. Vocabulary is in [CONTEXT.md](../CONTEXT.md); the decisions with lasting consequences
have their own records in [adr/](adr/).

**Shape.** The Inventory is a container that owns its contents, not a view over `m_rgItems[]`. An Entry is
`(what, count, col, row)` and carries no state beyond that — there is no half-used medkit. Item Types
declare a maximum Stack size; unique things are simply those with a maximum of one, and picking up past the
maximum creates a second Entry.

**What competes for space.** Weapons (3 Cells) and items (1 Cell). Ammo does not — see
[ADR-0001](adr/0001-ammo-is-not-in-the-inventory.md). Ammo moves to the Inventory Panel's left column.

**Grid.** 12 Cells wide, fixed; **5 Rows to start and 4 more to earn, 9 maximum** (60 to 108 Cells), all
drawn at all times with un-granted Rows greyed out — see
[ADR-0003](adr/0003-fixed-grid-width-rows-only-growth.md). Rows are granted by things found in the world,
not by Skills, which makes Inventory growth a reward for exploration. Because Rows are only ever granted,
the Grid never shrinks and no eviction rules are needed.

The base Grid is deliberately generous: this mod adds more weapons and items than vanilla Half-Life, whose
full 15-weapon arsenal would occupy 45 of the 60 starting Cells on its own.

`inv_rows_start` and `inv_rows_max` are tuning cvars; granted Rows are saved player state, clamped only at
grant time and never re-clamped on load, so lowering a cvar can never shrink a Grid under a player.
`inv_addrows <n>` (cheat-gated) exists to tune the maximum by eye.

**Ownership.** The server owns contents *and* placement; the client renders and requests — see
[ADR-0004](adr/0004-the-server-owns-the-inventory.md). Placement on pickup is first-fit, scanning
left-to-right, top-to-bottom.

**Item Types** are defined once, in a table under `game_shared/` compiled into both DLLs. Weapons keep
Half-Life's `WeaponId` — see [ADR-0002](adr/0002-two-identity-spaces-for-weapons-and-items.md).

**Verbs.** Move, Use, Drop. Nothing else. Dropping a single item spawns its world entity; dropping a
Stack offers "Drop 1" or "Drop all", the latter spawning one entity per item with a small scatter.
Weapons drop through Half-Life's own `DropPlayerItem`, which spawns the real weapon model.

A dropped weapon keeps its **loaded clip** but grants no reserve ammo, so dropping and retaking is exactly
lossless and never profitable. **Reserve ammo stays with the player** — it is a pool, not a property of the
weapon (ADR-0001), so you keep shells for a shotgun you no longer own. Vanilla differs here: `CWeaponBox`
packs half your reserve into the box. Ours is the deliberate choice, on the grounds that reserve ammo costs
no Cells and so is harmless to carry.

**Acquisition.** Everything is walk-over, as in Half-Life — settled 2026-09-13 after a playtest, see
[ADR-0011](adr/0011-pickups-are-walk-over.md). A weapon walked over is taken if there is room. A medkit or
battery walked over is consumed on the spot when doing so wastes none of it — the test is inclusive,
`health + heal <= maxHealth`, so a perfect fit is consumed rather than carried — and is carried
otherwise. Antidote, keycard and syringe are carried on contact. A full Grid leaves an item standing,
silently, exactly as a full ammo pool leaves an ammo box; the Pickup Prompt still names it, and a use
press on it prints "No room in inventory". Auto-Consume runs before the Grid is consulted, so a wounded
player with a full Grid is still healed by a medkit on the floor.

The Pickup Prompt stays for everything takeable, as it always did for weapons. Its two remaining jobs are
lifting a thing off a shelf the player cannot step onto, and carrying the full-Grid explanation.

**A dropped item cannot walk straight back in.** It spawns inside the dropper's own box, so
`CItem::DisarmUntilClear` switches its touch off and `ArmWhenClear` turns it back on once the item has
landed and no player is standing in it. A dropped weapon gets the same for free: `CBasePlayerItem::FallInit`
leaves it a point until it lands. A use press on a dropped item takes it at once, since that path never
goes through touch. `item_debug 1` prints what every touch decided and when a dropped item re-arms.

A pickup that went into the Grid is announced on the **pickup history** — the column of icons at the
bottom right — with the `inv_carried` arrow badge drawn just left of its icon, centred on its height
(`HistoryResource::DrawAmmoHistory`, `cl_dll/ammohistory.cpp`). Beside rather than over: the first cut
sat on the icon and the item art fills its corners, so it read as intrusive in play. A pickup used on the spot flashes the plain icon, as vanilla. The flag rides
on `gmsgItemPickup` as one byte after the classname, sent only through `CItem::AnnouncePickup`. That
arrow is what replaced the old rule's promise that "the player is never surprised by what they are
carrying".

**Persistence.** Save games and level transitions, via `CBasePlayer`'s save table. Death is a full reload
from the last save, so there is no respawn-inventory case to design. Things left behind stay in the map
they were left in; maps are designed with backtracking in mind, so this is a consequence to build levels
around rather than a problem to solve.

### Planned iterations

1. ~~**The model, server-side.**~~ **Done 2026-08-01.** `game_shared/inventory_defs.h` holds the Item
   Type table; `CPlayerInventory` (`dlls/player_inventory.cpp`) holds Entries in the player's save table;
   placement is first-fit with Stack top-up; `gmsgInventory` syncs in chunks; the client renders and
   requests. `NormalizeGridLayout`, the offset vectors, and `gWR.riGridCell` are gone.
2. ~~**The pickup interaction.**~~ **Done 2026-08-01.** `FindLookedAtPickup` answers "what would a use
   press take?" once, and both the Pickup Prompt and the take use that same answer, so they cannot
   disagree. It deliberately also weighs ordinary usable entities, so pressing use at a button never
   grabs a medkit behind it. Items were made use-only here, with the medkit Auto-Consuming on contact
   when nothing is wasted and weapons keeping walk-over pickup. **Reversed 2026-09-13**: a playtest
   found use-only unintuitive for Half-Life players, and every pickup is walk-over again — see
   *Acquisition* under "Agreed design" and [ADR-0011](adr/0011-pickups-are-walk-over.md). Ammo readout
   moved to the left column in iteration 1.
   *Done when a map plays start-to-finish without opening the panel and it feels normal.*
3. ~~**Containers and the world.**~~ **Postponed 2026-08-01.** Dropping was completed without needing
   containers: weapons became droppable, and a Stack offers "Drop 1" and "Drop all", spawning one world
   entity per item. The Row Grant pickup that was also filed here is **built** — `item_rowgrant`, see
   "What's missing" above. What remains under this heading is genuinely future work: lootable Boxes and
   the loot window.

Iteration 1 is the only one that is hard to reverse.

### Acceptance criteria

- No overlap or overflow across all Entry widths.
- Repeated drag/drop sequences produce stable, predictable placements, and nothing moves that the player
  did not move.
- Inventory contents *and* layout survive save/load and level transitions.
- Lowering an `inv_rows_*` cvar never destroys or displaces anything a player is carrying.

---

## 6. Stealth

**Status: Partial** — everything up to the moment a monster notices the player is built, and the player is
told about it. Everything after that moment is the base game.

Added as a pillar 2026-08-02. It is not filed under enhanced combat because it is the *alternative* to
combat — it moves enemy perception, player movement, weapon choice and level layout at once, and pillar 2
would have to mean "everything you do to things that are alive" to contain it.

**Amended 2026-09-17: stealth is swallowed by combat.** That paragraph is about where the work files, and
the section stays. In the ranking stealth is not beside combat but inside it — one way to solve an
encounter, optional with declared exceptions, in a game where monsters charging the player is still the
core loop. Investing in it pays off for most of the game, not all of it.

### What exists

**Concealment and Suspicion, since 2026-09-01.** A monster no longer acquires the player the frame it sees
them. It fills a per-monster meter at a rate set by four multiplied terms — how central the player is in
that monster's own cone, how far away as a fraction of that monster's own sight range, whether the player is
crouched or moving slowly, and how brightly lit they are — and only becomes hostile when the meter fills.
Every number is a cvar, `debug_schedule 1` shows the meter of the monster under the crosshair, and `suspicion_enable 0` restores vanilla
acquisition exactly so the two can be compared in play.

Seven things about it are worth knowing rather than rediscovering:

- **It gates three condition bits and nothing else.** `SEE_HATE`, `SEE_DISLIKE` and `SEE_NEMESIS`, for the
  player only. Everything downstream of acquisition is untouched, which is the whole reason the vanilla
  campaign still plays the way it did. The four rejected placements are in
  [adr/0009](adr/0009-suspicion-gates-the-relationship-bits.md).
- **Shooting something acquires it immediately.** Not from the settled design — `TakeDamage` turns a monster
  toward the attack but never sets `m_hEnemy`, so gating `Look` alone would have left a monster the player
  shot standing there for seconds. Being shot fills the meter outright.
- **Monster aim never came from monster facing, and stealth is what exposed it.** `ShootAtEnemy` aims at
  `m_vecEnemyLKP`; `pev->angles` is not consulted and `SetBlending` blends pitch only, so a grunt shoots
  accurately through its own back whenever anything refreshes its LKP. Vanilla, and **still true** — the gate
  only made it a normal thing to witness. Diagnosis and the proposed one-function fix are in
  [the post-aggro step](ROADMAP.md#the-post-aggro-step).
- **Nothing after acquisition changed, and that is enforced in one line.** `UpdateSuspicion` pins the meter
  full the moment the player becomes a monster's enemy, so combat is byte-for-byte the base game. Every
  combat complaint therefore has exactly two possible causes — vanilla, or the fact that this mod lets you
  reach vanilla states Half-Life never expected — and never a third.
- **So once acquired, a monster keeps the player forever.** `GetIdealState`'s only exit from COMBAT is a
  null enemy and nothing sets one. De-escalation was attempted and reverted on 2026-09-02; it and the
  aim-versus-facing problem are [the post-aggro step](ROADMAP.md#the-post-aggro-step).
- **The per-monster FOV table carries through for free.** Angle and distance are scaled to each monster's
  own `m_flFieldOfView` and `m_flDistLook`, so the sharpest existing difference between monsters becomes a
  difference in how hard each is to sneak past, with no second table to maintain.
- **Light barely matters yet.** It is the mildest of the four terms deliberately: vanilla maps are lit for
  readability, not for hiding. It becomes a real lever only with custom maps.
- **Quiet movement is now a decision, not a side effect.** `UpdatePlayerSound` made a crouched player
  quieter only because they were slower, which left an audible radius of ~107 units against a crowbar that
  reaches ~32 — so the Backstab's own approach was impossible. `noise_stance_duck` / `noise_stance_walk`
  scale the **body** volume only; firing is exactly as loud crouched as standing, because a quiet weapon is
  the silencer and that is deferred.

The Perception Profile that scales all this is a **virtual**, not a member set in `Spawn` — `Spawn()` does
not re-run on restore, so anything set there and not saved comes back default after every load. `CanBackstab`
already took that shape and anything else per-class must too.

Underneath it, an unusual amount of the base game's own machinery was already in place and unused, which is
why this was a pillar rather than a wish. The full reference is
[PERCEPTION.md](PERCEPTION.md) — written 2026-08-31, and the first place to look before touching any of
this. The short version:

- **The noise model is complete and running.** `CBasePlayer::UpdatePlayerSound()` derives a per-frame noise
  volume from the player's velocity, whether they are airborne, whether they jumped, and how loud their
  weapon just was, and posts it as `bits_SOUND_PLAYER` (`dlls/player.cpp:2586-2666`). Crouching and walking
  made the player quieter as a side effect of being slower; the stance multipliers above now do it on
  purpose.
- **Monsters listen to it** — it is in the default sound mask (`dlls/monsters.cpp:402`) and in most
  individual ones.
- **`CBasePlayer::Illumination()`** (`dlls/player.cpp:4567`) returns the engine's light level at the player
  plus a decaying virtual muzzle flash that every gun sets. It had no callers at all until Concealment; the
  muzzle-flash half of the light term arrived free because of it.
- **`m_fNoPlayerSound`** (`dlls/player.h:175`) is a working silent-movement switch, labelled in its own
  comment as a debugging feature.
- **The glock's silencer is one commented-out line** — `dlls/glock.cpp:77`. The submodel, the reduced
  `QUIET_GUN_VOLUME`, the dimmed flash and the `GLOCK_ADD_SILENCER` animation all exist.

**Two corrections to what this file used to say**, both found while writing PERCEPTION.md:

- It claimed grunts investigate player noise when they cannot see their enemy, citing `dlls/hgrunt.cpp:1984`.
  **That code is commented out** (`:1983-1988`) and has never run. What hearing the player actually causes
  is a `MakeIdealYaw` toward the noise, and nothing else.
- It did not record that **deaths and corpses are entirely imperceptible**. `CBaseMonster::Killed` notifies
  nobody but `pev->owner`, no sound enters `CSoundEnt` on death, and `Look` skips anything with
  `health <= 0` (`dlls/monsters.cpp:324`). A body in a lit corridor is never noticed by anyone. That is
  load-bearing for a stealth pillar and was missing from both documents.

### The Stealth region and the Night Vision Module — built 2026-09-16, untested in game

Every node the Stealth region did not wait on [the post-aggro step](ROADMAP.md#the-post-aggro-step) has an
effect now, gated behind the Module that also reveals the region — [SKILL_TREE.md](SKILL_TREE.md#stealth)
has the table with every id and cvar. None of it has been played yet.

- **Ten Concealment Stat nodes** (ids 131–140), the region's roads: `PlayerConcealmentScale`
  (`dlls/perception.cpp`) adds `skill_stat_concealment` (0.05) per node to the multiplier
  `UpdateSuspicion` applies to the fill rate, player only.
- **Soft Step** (126) multiplies the crouch and walk body-noise scales again by `skill_soft_step_scale`
  (0.5) in `UpdatePlayerSound`; running is untouched.
- **Nightfall** (127) scales `conceal_light_dark`, the light term's worst end, by `skill_nightfall_scale`
  (0.5) in `ConcealmentOf`, for a player holding it.
- **Slip Away** (128) adds a saved `m_bSuspicionHadTarget` to `CBaseMonster`, remembering whether the last
  `Look` had a target. On the seen→unseen edge, with the meter between `suspicion_notice` and
  `suspicion_acquire`, it is multiplied by 1 − `skill_slip_away_fraction` (0.33), once — logged under
  `debug_schedule`. The single-player assumption is `UTIL_PlayerByIndex(1)`.
- **Ambush** (22) reads the *victim's* own meter at both damage chokepoints (`ApplyMultiDamage` in
  `weapons.cpp`, the direct branch of `RadiusDamage` in `combat.cpp`), through `PlayerAmbushScale`
  (`dlls/player_skills.cpp`) beside `SkillScaleWeaponDamage`, before `TakeDamage` fills the meter:
  ×`skill_ambush_noticed_scale` (1.5) below `suspicion_notice`, else ×`skill_ambush_spotted_scale` (1.25)
  below `suspicion_acquire`. Never on the always-aware profiles, `SF_MONSTER_IGNORE_CONCEALMENT`, or a
  non-hostile; shown as `xAMBUSH` in the `debug_damage` line through the new `DebugDamageAppend`, which
  adds to the melee breakdown rather than replacing it.
- **Phantom** (23): both Backstab branches of the crowbar call `CBasePlayer::PhantomStart()` when a
  Backstab on a victim below Noticed kills it (the hit alone was the first shape; Andrei set the kill on
  2026-09-16), saving `m_flPhantomUntil`. The silence is `UpdatePlayerSound` zeroing body
  noise; the speed rides a physinfo key (`"phs"`, percent) written by `PhantomSync` (`PreThink`) and read
  in `PM_CheckParamters`, multiplying `pmove->maxspeed` before the wish-speed clamp, since
  `pfnSetClientMaxspeed` can only lower it. `skill_phantom_duration` (4), `skill_phantom_speed_scale`
  (1.5); cues are `buttons/blip2.wav` at pitch 150 on start and 80 on end, a placeholder shared with
  Cleave's ready blip ([ART_DEBT.md](ART_DEBT.md)).
- **Silent Kill** (130), the Major, remains unbuilt — the post-aggro step. **Cut the Head** (129) was
  dropped on 2026-09-17 and **Shroud** (160, ×0.8 on the fill) takes its cell; neither is built.

**The Night Vision Module** — `item_nightvision` (`dlls/items.cpp`) opens `EGate::NightVision`; it needs
the suit and turns off a lit flashlight first. With the gate open, `impulse 100` sets `EF_NIGHTVISION`
instead of `EF_DIMLIGHT` on the player — the bit sat unused since the SDK and `pev->effects` is already
saved, so there is no new save field and no new user message beyond `gmsgFlashlight` growing to three
bytes (on, battery, mode) on all three senders. The client (`cl_dll/flashlight.cpp`, `DrawNightVision`)
tiles Opposing Force's `of_nv_b.spr` noise additively and adds an eye-level client dlight keyed to the
player's index; `nv_overlay` (160, 0 off) and `nv_light_radius` (700, 0 off), both `FCVAR_ARCHIVE`.
Monsters are unaffected, so this is also what closes the flashlight hole in the light term above (a
flashlight never touches the baked lightmap) — by removing the flashlight from the equation, not by
patching it. `models/w_silencer.mdl` and the sentence `!HEV_A1` stand in ([ART_DEBT.md](ART_DEBT.md)); see
[pillar 3](#3-custom-items) for the item itself.

### What's missing

**A readout the player has not yet judged.** `CHudConceal` is an icon in the suit cluster after the Pulse:
dim yellow when hidden, amber when noticed, red and blinking when spotted. Its sprite is a placeholder — the
flashlight's own icon, which is a genuine confusion risk and is recorded in
[ART_DEBT.md](ART_DEBT.md#the-concealment-readout--icon). Whether three states is the right granularity, and
whether a corner icon is read in time mid-approach, are questions only play answers — `hud_conceal 0` turns
it off for comparison.

**Nothing gives up.** Once a monster acquires the player it keeps them forever — no give-up, and no
captain's notice propagation. Settled and unbuilt; see the next step.

### The cost of a kill — built 2026-09-17, tested 2026-09-18

Step 5f of the post-aggro step, built the day it was settled and run through
[STEALTH_CHECKLIST.md](STEALTH_CHECKLIST.md) the next day; the Search was fixed on the way (below):

- **Witnesses.** Every hostile with a meter that had a line to the victim as it died jumps to 0.75, turns
  to the body and speaks; from there it drains to a permanent 0.3 floor, cleared only by a level change.
  Player-dealt kills only. `PerceptionOnKilled` in `dlls/perception.cpp`, from `CBaseMonster::Killed`.
- **The Disturbance.** The death spot enters the sound list as a new sound bit for 20 seconds at 512
  units, heard only by the Trained profiles through a flag on the profile struct.
- **The Search.** The SDK's own investigate schedule: walk to the body, idle ten seconds, walk back. A
  squad's leader sends its nearest free member and pushes the schedule onto it; a loner goes itself, so a
  leaderless group arrives as a mob ([ADR-0014](adr/0014-a-body-draws-one-squad-member-or-every-loner.md)).
- **Silent Kill** is now real: on a victim below Spotted no Disturbance is inserted. Witnesses in sight
  still react.
- **The grunt speaks** three new lines composed from vanilla words in `sound/sentences.txt`, copied to the
  install by hand: on witnessing, on sending, on turning for home.
- **With it:** the silencer as `item_silencer`, found and permanent, the first Evolution; Headhunter
  ignoring the grunt's helmet; Shroud (id 160) in Cut the Head's cell; Ambush ×1.5 / ×2;
  `suspicion_fill` 2.0.

The checklist found the Search dead: no searcher ever walked, because the route to a body that is still
mid-death-animation ends inside a solid hull. The searcher now stops 48 units short of the body. With that
fix, squads send one grunt and leaderless survivors all go, as designed. What the checklist left open is
the captain's channel (5e), not built: the leader's meter only rises from his own sight, and a member that
did not see a kill stays at 0.

### Next step

**The design is settled** as of 2026-08-31, and **everything after acquisition was settled on 2026-09-17**
under one frame: stealth is *predator first* — the loop is unseen, kill, unseen again, and crossing a room
unseen is a way to reach the first kill rather than the measure. The order is the cost of a kill first
(witnesses at a 0.75 jump, a Disturbance sound that draws one squad member or every loner, the Search on
the SDK's own investigate schedule), then the give-up into the same Search, then the captain's notice
propagation. Aim-versus-facing is dropped. With it come the silenced pistol as a found item and a helmet
skip on Headhunter, because the intended play — stab, swap, silenced headshot on the witness — did not add
up without them. It is written up in [PERCEPTION.md part 2](PERCEPTION.md#part-2--the-model-this-mod-adds),
and [ROADMAP.md](ROADMAP.md#the-post-aggro-step) holds the order, the numbers and the sentences.

**Finish, then freeze — 2026-09-17.** [STEALTH_CHECKLIST.md](STEALTH_CHECKLIST.md) first; then 5b, 5e and
5g, which stop the loop being sticky; then no new stealth features until real maps exist. What stealth
needs after that is content — ritual rooms, the military wing, soft gates a Stealth build can ghost — not
code.

**The Backstab is built and is filed under [pillar 2](#2-enhanced-combat)**, because it came out positional
— awareness does not gate it — which makes it a melee mechanic rather than a stealth one. It was this
pillar's first commit and is the only piece judgeable in vanilla maps, but it is not stealth and this
pillar should not take credit for it.

### Acceptance criteria (draft, rewritten 2026-09-17)

- A player can clear an occupied room one monster at a time, through choices they made — light, speed,
  weapon, who to take first — and each kill leaves them unseen again.
- A kill that was seen costs them the room: a fight, or a Search that walks to where they stand.
- No stealth state is decided client-side.

---

## Cross-cutting cleanup

Not pillars, but they affect all of them:

- Debug `ALERT(at_console, ...)` calls left in `dlls/items.cpp:229`, `dlls/healthkit.cpp:78`, and
  `dlls/UserMessages.cpp:27` log to console on every pickup and on message registration.
- Custom systems have no tests and no debug visualization; both TECH_DEBT entries ask for a debug overlay.
