# Gameplay Pillars — Progress

The five things this mod is actually about. Everything inherited from the base SDK is scaffolding; this
document tracks the custom gameplay on top of it.

This is a living document. When a pillar's state changes, update its section and the summary table in the
same commit as the code change.

**Last updated:** 2026-08-02 (branch `hl-shock`, at `d709eef`)

## Status legend

| Label | Meaning |
| --- | --- |
| **Not started** | No code exists. |
| **Scaffolded** | Data structures, networking, or UI exist, but the player cannot feel it in-game. |
| **Playable** | The player can use it in a normal play session, rough edges accepted. |
| **Done** | Behaviour is final and covered by the acceptance criteria in this document. |

## Summary

| # | Pillar | Status | One-line state |
| --- | --- | --- | --- |
| 1 | [Exploration](#1-exploration) | **Not started** | Its rewards exist — Row Grants, Skill Points, Reset Tokens are all findable entities — but no map places one, so nothing is explored *for* yet. |
| 2 | [Enhanced combat](#2-enhanced-combat) | **Playable** | The Pulse is complete and plays well — Shield, Recharge, Discharge, three Skills, readiness bar. Numbers untuned. Melee skills still do nothing. |
| 3 | [Custom items](#3-custom-items) | **Playable** | The Health Syringe works end to end — Item Type, world entity, the Infusion, a status icon and a Skill. No map places one yet. |
| 4 | [Skill trees](#4-skill-trees) | **Scaffolded** | Curated to 15 Skills; unlocks, saves and renders; points and Reset Tokens are earned and spent. Six have effects, nine are inert — the only thing keeping this off Playable. |
| 5 | [Inventory management](#5-inventory-management) | **Playable** | Grid, drag-drop, and context actions work. Client-side model only — the server-owned rebuild is designed and scheduled. |

---

## 1. Exploration

**Status: Not started**

### What exists

Nothing. No custom code touches level traversal, discovery, secrets, map flow, or navigation aids.

### What's missing

The pillar has no definition yet, let alone an implementation. Open questions that need answering before any
code is worth writing:

- Is exploration rewarded through **items** (pillar 3), **skill points** (pillar 4), or both?
- Does it need new entities (discoverable caches, lore pickups, optional-area triggers), new map work, or
  only tuning of existing HL maps?
- Is there any navigation/HUD affordance (compass, objective marker, map notes), or is it purely level design?

### Next step

The reward loop has two concrete answers now, and the question at the top of this section is settled: it is
**both** items and Skill Points.

**Row Grants.** Inventory capacity grows from things found in the world rather than from Skills (see
pillar 5), so a hidden cache off the critical path permanently increases what the player can carry.

**Skill Points and Reset Tokens.** Both are found in the world and nowhere else — `skill_points_start`
defaults to 0, so a player who explores nothing unlocks nothing (see pillar 4). `item_skillpoint` and
`item_resettoken` are placeable entities today. The tree is deliberately sized so it is completable only by
near-exhaustive exploration, which makes reach — not just speed — the thing exploration buys.

What is still missing is the same thing in both cases: **maps**. Every mechanism exists and nothing places
one, because vanilla Half-Life maps cannot be edited to hold them. Until there are custom maps, both loops
are reachable only through `inv_addrows` and `skill_addpoints`.

### Acceptance criteria (draft)

- A player who explores off the critical path is measurably better off than one who does not.
- The reward is visible in the UI at the moment it is earned.

---

## 2. Enhanced combat

**Status: Playable**

The Pulse is the first custom mechanic in the mod that changes how the game plays, and the first Skill
effect of any kind.

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

**Ten tuning cvars**, registered in `dlls/game.cpp` — eight for behaviour, two for the ring's look.

**Feedback** — nested rings plus a `TE_DLIGHT` flash, and sounds on Pulse, on each deflect, on a denied
press, and on Recharge completing. All stock placeholder assets. Deflects use randomised
`weapons/ric1-5.wav`: a ricochet rather than another electrical noise, because the electrical one landed a
fraction of a second after the Pulse's own in the same timbre and was simply not heard.

The ring geometry is cvar-driven — `pulse_ring_style` picks `TE_BEAMCYLINDER` (a ring expanding along the
ground, borrowed from the houndeye) or `TE_BEAMTORUS` (screen-aligned and centred on the player, so it
reads as a bubble), and `pulse_ring_scale` sizes it. Nothing else in the SDK uses the torus, so there was
no existing call site to take a correct scale from; these exist to be dialled in by eye.

**The readiness readout** — `CHudPulse` (`cl_dll/hud_pulse.cpp`), sitting immediately right of the armour
readout on the same baseline. The `suit_full` sprite in cyan — the armour's is yellow, which is what tells
two identical icons apart — with a vertical charge bar beside it filling bottom-up. Geometry mirrors
`CHudBattery` so the two stay aligned at any resolution.

While a Shield stands the screen is tinted cyan, alpha via `hud_pulse_tint` (0 disables).
**`m_Pulse.Init()` is registered first in `CHud::Init` on purpose:** `AddHudElem` appends, so Init order is
draw order, and the tint has to go under every other readout rather than over it.

`gmsgPulse` carries `(state, duration in tenths)` and is sent **only on a state change** — the client runs
the fill off its own clock from the duration it was given. A whole Pulse costs three 2-byte messages
instead of one per frame. `CPlayerPulse::ForgetSentState()`, called where `m_fInitHUD` is handled in
`UpdateClientData`, forces a resend after the client's HUD is reset so the bar cannot go stale.

Everything else in this pillar is still only *descriptions*: Crowbar Reach, Crowbar Force, Fast Reload,
Weapon Mastery and Crowbar Speed change nothing.

### What's missing

- Melee reach/damage/speed multipliers in `dlls/weapons.cpp` and the crowbar implementation.
- Reload-time and global damage modifiers.
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

**Availability.** Suit hardware, not a Skill — `pev->weapons & (1 << WEAPON_SUIT)`. No Pulse before
Anomalous Materials. Skills evolve a verb the player already has rather than granting it, which lets level
design assume it.

**Trigger.** `impulse 150`, handled in `CBasePlayer::ImpulseCommands()`. Deliberately *not* a button bit:
`usercmd_t.buttons` is an `unsigned short` (`common/usercmd.h:29`) and `common/in_buttons.h` already spends
all 16 usable bits. `usercmd_t.impulse` rides the same per-tick packet, so timing fidelity is identical to a
button, and it is self-clearing (`dlls/player.cpp:3615`) so the press is edge-triggered for free. Also
deliberately not a crowbar secondary attack — that would tie a suit ability to one weapon and drag it into
client prediction, where the player-owned Recharge state does not exist.

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

**The Follow-Up** — `CrowbarFollowUp` (id 18) primes the crowbar for `pulse_followup_time` seconds after a
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

The pattern for every Skill effect after this one is set by `PulseWindowFor` / `PulseRechargeFor` in
`dlls/player_pulse.cpp`: a modifier is read server-side from `m_skills`, applied where the effect is
computed, and never touched in prediction. The nine remaining Skills should follow it rather than each
inventing their own hook. Reload and attack-rate Skills are deliberately *not* among them — those are
client-predicted and were cut from the tree until that work is done.

### Acceptance criteria (draft)

- Every combat skill in the tree has a measurable in-game effect.
- Effects are computed server-side; the client never decides damage.
- No skill effect is applied twice (prediction and server both).

---

## 3. Custom items

**Status: Playable**

The framework landed with inventory iteration 1, and the **Health Syringe** is the first item in the mod
that Half-Life does not have.

### What exists

**The Item Type table** — `game_shared/inventory_defs.h`, compiled into both DLLs so the client and server
cannot disagree. Five entries: **Medkit**, **Antidote**, **Keycard**, **Battery** and **Health Syringe**.
Each row carries classname, display name, sprite, Cell width, Stack ceiling and whether the item is
usable. Ids are frozen once written to a save; adding is free, reordering corrupts.

The `usable` flag replaced a hardcoded `id == Medkit || id == Battery` test in the client's context menu,
which was found the only way it could be: the Syringe shipped with no **Use** button. The client now asks
the shared table, so a new item cannot repeat it.

**The Health Syringe** — Item Type id 5, `item_syringe`, three per Stack, one Cell. Uses
`models/w_adrenaline.mdl`, which ships unreferenced in `valve/models` and reaches the mod by
game-directory fallback. Placeable in a level editor via a `@PointClass` line in `fgd/halflife.fgd`.

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

**Med Expert** (Skill id 19) — `+infusion_duration_bonus` seconds, additive. A root node, deliberately:
every skill in the survivability column is still inert, so gating it behind one would charge points for
nothing to reach something.

**Three tuning cvars** in `dlls/game.cpp`: `infusion_rate` (4), `infusion_duration` (10),
`infusion_duration_bonus` (5). Named for the mechanic rather than the Syringe, so a later source of an
Infusion does not inherit syringe-flavoured names.

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

**Status: Scaffolded**

The most complete system by line count, and the one furthest from affecting play.

### What exists

**Definitions** — `game_shared/skill_defs.h`, compiled into both DLLs

- **15 Skills in the tree**, across seven columns: Melee (0), the Pulse (1–2), the suit (3), Armaments (4),
  Survivability (5–6). Total cost **33 points**, which is the target for how many Skill Points a campaign
  places.
- **Six reserved ids** with no row: `FastReload`, `HighJump`, `SprintSpeed` and `CrowbarSpeed` are cut
  pending prediction work; `HiveCapacity` and `HiveRegrowth` are held for the alien column below. A
  reserved row is `SKILL_RESERVED(id)` — the id stays frozen and the Skill returns unchanged later.
- A `static_assert` enforces that the table is ordered by id. It is indexed positionally, so a row out of
  place would silently make a save's unlocked bits refer to different abilities — the grouping that reads
  most naturally to a human is exactly the mistake, so it is a compile error.
- Each `SkillDef` carries id, display name, description, icon, grid column/row, cost, **two**
  prerequisites, and a visual tier (`Minor` / `Medium` / `Major`). Both prerequisites are required, so a
  connector line always means "you need this".
- `SkillPrereqMet` is the one implementation of the gating rule; server and client both call it.

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

- `gmsgSkillTree`, **fixed** length: a bitmask of unlocked Skills, one bit per id, then the player's unspent
  Skill Points, then their banked Reset Tokens. 5 bytes at nineteen Skills, down from 116. Static Skill data
  is shared rather than sent — see [ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md).
- Client → server is the `skill_unlock <id>` console command (`dlls/client.cpp:647`); on success the server
  re-sends the state.
- Client handler: `CHudAmmo::MsgFunc_SkillTree` in `cl_dll/ammo.cpp`. It drops any message whose size
  disagrees with `k_SkillMaskBytes` rather than misreading it.

**Client** — `cl_dll/vgui_skilltree.cpp` / `.h`

- `CSkillTreeView`, a plain C++ helper owned by `CInventoryPanel` rather than a VGUI panel of its own.
- Tier-sized nodes, lazily loaded HUD sprite icons, edge-anchored connector lines that prefer vertical
  routing, hover tooltips, and a skill-point counter.
- Labels, descriptions, icons, positions, costs, prerequisites and tiers all come from the shared
  `k_SkillDefs`. Adding a skill is one table row. The old client-local `k_SkillUiInfo` copy is gone.
- "Available" is derived client-side from the unlocked mask and the shared table. That is a display
  decision, not an authority change — `TryUnlock` re-validates everything server-side.

### What's missing

- **Nine of the fifteen effects.** Six work — `PulseWindow`, `PulseRecharge`, `PulseDischarge`,
  `PulseRebound` and `CrowbarFollowUp`, all read by `dlls/player_pulse.cpp`, plus `MedExpert` (id 19), read
  by `dlls/player_infusion.cpp`. The other nine unlock, persist and render without changing anything.
  Every one of them is server-side and cheap; this is the last thing standing between the pillar and
  Playable.
- **Anywhere to earn points.** The mechanism exists — `item_skillpoint` and `item_resettoken` are placeable
  entities — but no map places one, so in practice points still come from `skill_addpoints`. Blocked on
  custom maps, exactly as Row Grants are.
- **Node icons.** The tree is deliberately label-free, which makes icon distinctness *blocking* rather than
  cosmetic — and today five Skills share `suit_full`. See [ART_DEBT.md](ART_DEBT.md).
- Tooltip layout debt — heuristic text measurement rather than font metrics. See
  [TECH_DEBT.md](TECH_DEBT.md).

### Two prerequisites per Skill — DONE

A `SkillDef` now holds two prerequisites and requires both. **Follow-Up** (id 18) was the Skill that
forced it — a crowbar payoff for a Pulse deflect, which used to hang off `CrowbarDamage` alone, so a
player could take it having never touched the Pulse tree for a Skill that does nothing without deflecting.
It is now gated on `CrowbarDamage` **and** `PulseRecharge`.

It cost nothing on the wire. [ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md) moved
static Skill data into `game_shared/skill_defs.h`, so prerequisites are no longer sent at all and the
message-size ceiling that used to bound this stopped applying. A third prerequisite would be equally free.

The connector loop builds one edge per prerequisite, so a two-gated node draws two lines. Both mean the
same thing — every line is a requirement — which is why only AND is supported. An OR gate would need a
second line style before it could be read.

### Wanted: the alien column

A branch of Skills for alien weapons, **hidden entirely until the player carries one**, so that reading the
tree does not spoil that the branch exists. The column is reserved now rather than built: ids 20 and 21 are
held for it, and it takes column 7 when it opens.

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
prerequisites, the points/Tokens economy and the curation pass are all in. What remains:

1. **The UI pass.** Seven columns fitted to the panel rather than hardcoded steps, cost drawn on each node,
   and the hover bubble rebuilt on real font metrics (see [TECH_DEBT.md](TECH_DEBT.md)). Note the tree
   currently needs a wide screen: seven columns at `k_ColStep` 100 is 700px, and the tree area is
   `panelW - 264`.
2. **The nine remaining effects**, each following `PulseWindowFor` / `PulseRechargeFor`.

### The economy

Both cvars default to **0**: every Skill Point and every Reset Token is found in the world. Neither is
capped — the ceiling on each is how many pickups a map places, and a cap would let a found pickup silently
do nothing.

| Cvar | Default | What it does |
| --- | --- | --- |
| `skill_points_start` | 0 | Skill Points a new game begins with |
| `skill_reset_tokens_start` | 0 | Reset Tokens a new game begins with |

Cheat-gated `skill_addpoints <n>` and `skill_addtokens <n>` mirror `inv_addrows`.

The target is a tree **completable only by near-exhaustive exploration**: total findable points roughly
equal to the tree's total cost, so a player who sweeps every optional space affords essentially everything
by the end while a player on the critical path affords perhaps 60–70% and must genuinely choose. That is
what makes Reset Tokens matter for the majority case without denying completionists the top of the tree.
Roughly 5–10 Tokens across the campaign, with the first appearing around 20% in: enough that the tree is
meant to be experimented with, not agonised over.

None of this is placeable yet — see "What's missing".

Note that `CrowbarParry` (id 12) was renamed to `PulseWindow` rather than removed. Ids stay stable —
only the meaning changed, which was safe precisely because no skill had one at the time. It was the one
free opportunity to repurpose an id, and it is not a precedent now that skills do things.

Note that inventory capacity was considered as the first Skill effect and deliberately moved to exploration
instead (pillar 5). Nothing in the inventory work will give this pillar an effect, so it stays Scaffolded
until a combat skill is wired. The Row Grant counter is source-agnostic, so a capacity Skill can still
grant Rows later without rework.

### Acceptance criteria (draft)

- Every unlocked skill has an observable effect.
- Skill points are earned through play, not seeded.
- Skill ids stay stable; save games from before a skill was added still load.

---

## 5. Inventory management

**Status: Playable**

### What exists

**`CInventoryPanel`** (`cl_dll/vgui_inventory.cpp`) — a VGUI panel with two tabs, Inventory and Upgrades,
each delegating to a plain helper view. Opened via the `+inventory` command bound in `cl_dll/input.cpp:990`.

**`CInventoryGridView`** (`cl_dll/vgui_inventory_grid.cpp`)

- Renders weapons, inventory items, and ammo in one grid with mixed cell widths (weapons occupy three cells).
- Drag and drop with live clamping during the drag and normalization after the drop, so slots cannot overlap
  or overflow the grid.
- Per-slot pixel offsets persist across opens, owned by `CInventoryPanel` so context-menu actions can read
  them.
- Hit rectangles rebuilt every paint.

**Context menu** — right-click gives Use and Drop. Weapons issue `use <classname>` and close the panel;
items issue `inv_use <classname>` and stay open.

**HUD integration** — while the panel is visible, `CHudAmmo::Draw` returns early (`cl_dll/ammo.cpp:923`) so
the default weapon sprites don't bleed through the panel.

### What's missing

Iterations 1 and 2 are **done**, and dropping is complete — the model is server-owned, saved and
capacity-limited, things enter the Inventory deliberately, and anything in it can be dropped back out.
Remaining:

- **No Row Grant pickup**, so Rows can only be earned via `inv_addrows`. Needs custom maps before it can
  be placed at all.
- **No Boxes**, so nothing in the world holds items — lootable caches are still future work.
- No custom Item Types yet — the table holds the four stock ones.
- The ammo readout overflows its panel when the player carries many ammo types; it needs a taller panel,
  a scroll, or two columns. Deferred to a UI pass.
- The Pickup Prompt is unstyled — it uses the engine console font pending a visual style for the mod.
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

**Acquisition.** A weapon walked over is taken automatically if there is room, and otherwise left where it
is. A medkit walked over is consumed on the spot when doing so wastes none of its healing — the test is
`health + heal <= maxHealth`, so a perfect fit is consumed rather than left. Everything else is taken by
looking at it and pressing use.

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
   grabs a medkit behind it. Items are use-only now; medkits Auto-Consume on contact when nothing is
   wasted; weapons keep walk-over pickup. Ammo readout moved to the left column in iteration 1.
   *Done when a map plays start-to-finish without opening the panel and it feels normal.*
3. ~~**Containers and the world.**~~ **Postponed 2026-08-01.** Dropping was completed without needing
   containers: weapons became droppable, and a Stack offers "Drop 1" and "Drop all", spawning one world
   entity per item. What remains under this heading is genuinely future work — lootable Boxes, the loot
   window, and the `item_inventory_upgrade` pickup that grants Rows. Until that entity exists Rows come
   only from `inv_addrows`, and it cannot be exercised in-game before there are custom maps to place one
   in.

Iteration 1 is the only one that is hard to reverse.

### Acceptance criteria

- No overlap or overflow across all Entry widths.
- Repeated drag/drop sequences produce stable, predictable placements, and nothing moves that the player
  did not move.
- Inventory contents *and* layout survive save/load and level transitions.
- Lowering an `inv_rows_*` cvar never destroys or displaces anything a player is carrying.

---

## Cross-cutting cleanup

Not pillars, but they affect all of them:

- Debug `ALERT(at_console, ...)` calls left in `dlls/items.cpp:229`, `dlls/healthkit.cpp:78`, and
  `dlls/UserMessages.cpp:27` log to console on every pickup and on message registration.
- Custom systems have no tests and no debug visualization; both TECH_DEBT entries ask for a debug overlay.
