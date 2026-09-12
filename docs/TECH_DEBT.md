# Technical Debt Register

## A Leaked Move-Wait Freezes A Monster For Up To 99 Seconds

**Base-game bug, present in unmodified Half-Life.** Diagnosed 2026-09-12 from in-game capture; deliberately
not fixed yet, because it is a movement/schedule fault and does not belong inside stealth work.

### Scope
`dlls/schedule.cpp` (`ChangeSchedule`, the `TASK_FIND_COVER_*` family, `TASK_CLEAR_MOVE_WAIT`),
`dlls/monsters.cpp` (`CBaseMonster::Move`), `dlls/hgrunt.cpp` (`tlGruntGrenadeCover1`).

### The bug

`m_flMoveWaitFinished` is a monster-wide "do not move until this time" stamp. Nothing scopes it to the
schedule that set it, and `ChangeSchedule` (`dlls/schedule.cpp:75-83`) resets the schedule, task index, task
status, conditions and fail-schedule — but **not this field**. So one schedule can strand another.

The grunt is where that becomes visible, via `tlGruntGrenadeCover1` (`dlls/hgrunt.cpp:1505-1515`):

| # | Task | Effect |
| --- | --- | --- |
| 1 | `TASK_FIND_COVER_FROM_ENEMY, 99` | sets `m_flMoveWaitFinished = time + 99` (`schedule.cpp:753`) |
| 2 | `TASK_FIND_FAR_NODE_COVER_FROM_ENEMY, 384` | **`TaskFail()` when no node cover exists** (`schedule.cpp:712`) |
| 3 | `TASK_PLAY_SEQUENCE, ACT_SPECIAL_ATTACK1` | the grenade drop |
| 4 | `TASK_CLEAR_MOVE_WAIT` | releases the freeze (`schedule.cpp:917-919`) |

That `99` is not a wait — it is a **freeze-in-place idiom**: hold him still through the animation, then
release. It is the only non-zero data value on any cover task anywhere in the codebase (every other one is
`(float)0`), and `TASK_CLEAR_MOVE_WAIT` is used exactly once in the codebase — at task 4 of this schedule.

A task failure ends a schedule just as an interrupt does. When task 2 fails — routine on maps with sparse
or missing node graphs — the schedule dies **two tasks before its release**, and the 99 seconds leaks into
whatever schedule comes next.

What the frozen monster then does, all of it following from `Move` returning early at `monsters.cpp:1856`:

- **It never turns.** The early return skips `MakeIdealYaw`/`ChangeYaw` at `monsters.cpp:1883-1884`. If it is
  parked in `TASK_WAIT_FOR_MOVEMENT` (`schedule.cpp:439`), that task has no facing logic either, so nothing
  in the frame commands a turn at all.
- **It keeps its old animation.** `MoveExecute` (`monsters.cpp:2020`) is the only place `m_IdealActivity`
  becomes the movement activity, and it is never reached. `MaintainSchedule` re-applies `m_IdealActivity`
  every think (`schedule.cpp:274-277`), so a monster frozen out of a firing burst keeps playing
  `ACT_RANGE_ATTACK1` — **and keeps emitting its shot animation events**.
- **It still shoots accurately.** Aim comes from `m_vecEnemyLKP` via `ShootAtEnemy` (`monsters.cpp:3234`) and
  never from `pev->angles`, and `CheckEnemy`'s "behind or beside" clause (`monsters.cpp:1165`) refreshes that
  position to the player's exact origin whenever the player is within 256 units, in the clear and out of the
  cone. Net effect: **it fires through its own back, dead on target, for up to 99 seconds.**
- **It fails schedules and throws debug sparks.** Movement never completes, so cover and pathing tasks cycle
  and fail; `dlls/schedule.cpp:189` draws sparks on each one (`#ifdef DEBUG` only, so Release never shows it).

A lone grunt picks this schedule on a coin flip (`dlls/hgrunt.cpp:2207`), which is why the symptom appears
only sometimes. `m_flMoveWaitFinished` is also in the save table as `FIELD_TIME` (`monsters.cpp:56`), so a
leaked freeze survives a save/load.

### Evidence

`debug_monster_aim` (throwaway diagnostic, `dlls/combat.cpp`), six consecutive seconds of capture:

```
human_grunt#111 GruntEstablish[4]
yaw 80>80 off+0 lkp0u
plr-0.50! shot-0.50 ->plr1.00 AT-PLR
mw75.10 → 73.90 → 72.70 → 72.10 → 70.90 → 69.70   st1 mg2 ri0
```

`mw` is the leaked freeze counting down in real time, sampled ~24 seconds in. `off+0` proves no turn is ever
commanded — the yaw controller is healthy and simply never asked. `lkp0u` is the free position, and
`->plr1.00` is the shot landing on a player standing behind him.

### Why This Is Debt

It is reachable in unmodified Half-Life by aggroing a grunt and hiding, but nothing in the base game gives a
player a reason to do that, so it was never noticed. This mod's stealth work makes breaking line of sight
and repositioning **the point**, so it went from a freak state to a routine one — and it reads as three
separate bugs (a monster shooting backwards, a monster frozen, a monster throwing sparks) that are in fact
one.

It also blocks judgement of [pillar 6's post-aggro step](ROADMAP.md#the-post-aggro-step): any stealth
behaviour after acquisition is measured against a monster that may be frozen for a minute and a half.

### Recommended Next Steps

1. **Clear `m_flMoveWaitFinished` in `ChangeSchedule`.** The freeze means "held until *this schedule*
   releases me", so it is scoped to that schedule by definition, and a schedule change is exactly when it
   must end. One line, fixes every monster and any future schedule with the same shape.
2. **Known risk of (1):** the same field carries the door wait set in `AdvanceRoute`
   (`monsters.cpp:1528`, via `OpenDoorAndWait`). Clearing on a schedule change means a monster could step
   toward a door that has not finished opening. Minor and self-correcting — blocked movement already has
   handling — but it is a real behaviour change and should be watched for.
3. **Consider also** making the grunt's schedule tolerate the failure directly, by ordering the freeze after
   the fallible node-cover task. Narrower, but leaves the general leak in place for the next schedule that
   uses the idiom.
4. Verify with `debug_monster_aim 1`: the diagnostic exists for this and should be kept until closure.
5. **Unverified lead, noted only so it is not re-investigated from scratch:** `m_flLastYawTime`
   (`dlls/basemonster.h:112`) is used by `ChangeYaw` to scale turn speed by elapsed time, and it is **not in
   the save table** (`dlls/monsters.cpp:47-104`). It was investigated as a candidate cause here and ruled
   out — the freeze explains everything without it — but a stale stamp across a load could in principle make
   a turn go the wrong way or not at all. Worth a look only if yaw misbehaves specifically after a save/load.

### Acceptance Criteria For Closure

1. A grunt whose grenade-cover schedule fails at the node-cover task resumes moving on the next think —
   `debug_monster_aim` shows `mw0.00` rather than a large countdown.
2. A grunt that loses the player turns to face where it is shooting; `off` does not sit at `+0` while `plr`
   is negative.
3. No monster stays frozen in place for more than its intended wait, on a map with no node graph.
4. The debug sparks stop accompanying a lost player in Debug builds.
5. `OpenDoorAndWait`'s behaviour is unchanged for a door that is genuinely opening.

## Skill State Never Reaches Client-Side Weapon Code — RESOLVED 2026-08-31

Fixed. `HUD_SetPredictedSkills` (`cl_dll/hl/hl_weapons.cpp`) fills the client player's `m_skills` from the
unlocked mask, and `CPlayerSkills::ApplyUnlockedMask` — the inverse of `BuildUnlockedMask`, and beside it
in `dlls/player_skills.h` — does the unpacking. Both go through the `SkillMaskGet`/`SkillMaskSet` pair in
`skill_defs.h`, so the packing is still defined once.

All four acceptance criteria below are met. `dlls/crowbar.cpp` has no `#ifndef CLIENT_DLL` around Crowbar
Reach, and `FastReload` is unreserved, wired through `CBasePlayerWeapon::DefaultReload`, and in the tree at
column 4 behind Weapon Mastery.

**Two things the recommendation below got wrong, kept because the reasoning is worth having:**

1. **The mask is fed from the message, not from `clientdata_t`.** Step 1 named `cd->iuser3` and `cd->iuser4`
   as unwritten and available. They are unwritten by `dlls/client.cpp` but not unclaimed: `HUD_TxferPredictionData`
   (`cl_dll/entity.cpp:198,210`) labels them *duck prevention* and *fire prevention*, and `pm_shared.cpp:2078`
   reads `pmove->iuser3`. That leaves the `fuser`/`vuser` floats, and a float caps the id space at 24 bits
   where `gmsgSkillTree` already carries 40 — a worse ceiling than the one the step warned about.

   `gmsgSkillTree` is the better carrier anyway. It is already sent, already arrives on spawn and on every
   change, and the mask is not frame-varying state: nothing predicts a Skill unlock, so there is nothing to
   reconcile per frame and nothing to add to the prediction snapshot. `CHudAmmo::MsgFunc_SkillTree` now has
   two independent consumers — the panel and the predicted player — and prediction deliberately does not
   depend on the VGUI panel existing.

2. **Population was not the only thing missing.** The entry says *"The blocker is one word: population.
   Nothing else is missing."* The tuning cvars were also missing. They are defined and registered in
   `dlls/game.cpp`, which is not in the client project, so predicted weapon code cannot name
   `skill_crowbar_range_scale` at all — the `#ifndef CLIENT_DLL` in `crowbar.cpp` was guarding two things,
   not one, and removing it produced an undefined symbol rather than a working Skill.

   `dlls/skill_tuning.h` is the answer: a `CSkillTuning` looks the cvar up **by name** through
   `CVAR_GET_POINTER`, which resolves in both DLLs because `HUD_InitClientWeapons`
   (`hl_weapons.cpp:445-447`) points `g_engfuncs` at the engine's own cvar functions, and a listen server
   shares one registry between the game DLL and the client. The fallback is the *neutral* value — the Skill
   reads as not held — rather than a copy of the cvar's default, so there is no second set of defaults to
   keep in step with `game.cpp`. Only knobs a predicted value depends on belong there; a server-only effect
   keeps reading its `cvar_t` from `game.h`.

**What this does not unblock:** `SprintSpeed` (6) and `HighJump` (5) stay reserved. They change movement,
which `pm_shared/` owns, and `pm_shared` runs from `playermove_t` — it cannot reach `m_skills` through any
of this. `CrowbarSpeed` (11) also stays reserved, but for a new and smaller reason: `CCrowbar::Swing` reads
`m_flNextPrimaryAttack` to tell a first swing (full damage) from a follow-up (half), so shortening the
cadence silently makes every swing a follow-up. That is a damage-rule problem, not a prediction one.

The ceiling the entry noted is unchanged in kind and further away in practice: ids ≥ 40 would need a longer
message. Ids are frozen and only ever grow, so the day that matters is real but distant.

Everything below is the original diagnosis, as written before the fix. Kept because it is what made the
work small, and because the two places it was wrong are worth more than the entry would be if trimmed.

### Scope
`cl_dll/hl/hl_weapons.cpp` (`HUD_WeaponsPostThink`), `dlls/client.cpp` (`UpdateClientData`), and every
weapon file that compiles into both DLLs. Visible at the time at `dlls/crowbar.cpp:169-177`.

### The bug
Weapon code compiles into **both** DLLs so the client can predict it. `CPlayerSkills m_skills` is a member
of `CBasePlayer` (`dlls/player.h:370`), and the client has a real `CBasePlayer` — `static CBasePlayer
player` at `cl_dll/hl/hl_weapons.cpp:40`. **It is never populated.** `HUD_WeaponsPostThink`
(`hl_weapons.cpp:512`) copies about twenty-five fields from `from->client` into that object every frame —
ammo, buttons, origin, velocity, FOV, `m_flNextAttack` — and the skill state is not among them.

So any Skill that changes a value the client predicts makes the two copies disagree, and the player sees
the disagreement. The one shipped case is guarded and documented in place:

```cpp
float flRange = 32.0f;
#ifndef CLIENT_DLL
    if (m_pPlayer->m_skills.HasSkill(ESkillId::CrowbarRange))
        flRange *= std::max(1.0f, skill_crowbar_range_scale.value);
#endif
```

That is tolerable because damage is decided server-side either way and the client's trace only picks which
swing animation plays — at the far edge of the extended reach an unlocked player can see a miss animation
for a hit that landed. It is **not** tolerable for reload time, attack rate or movement speed, which the
client owns frame to frame. Four Skill ids are `SKILL_RESERVED` and cut from the tree for exactly this
reason: `FastReload` (3), `HighJump` (5), `SprintSpeed` (6), `CrowbarSpeed` (11). *(The entry originally
listed these as 4, 7 and 8, which are Weapon Mastery, Sure Footing and Fortitude. Corrected here rather
than left, because a wrong frozen id is the one kind of error in this file that could be copied into
code.)*

### What is *not* the problem

Three plausible causes are all already handled, which is why this is much smaller than it looks:

- **Not linkage.** `CPlayerSkills::HasSkill` is inline in `dlls/player_skills.h:51-56` and reads only
  `m_bUnlocked` and `k_MaxSkills`. The header includes nothing but `game_shared/skill_defs.h`, which
  compiles into both DLLs already ([ADR-0008](adr/0008-skill-definitions-are-shared-not-networked.md)).
  It links in `cl_dll` today.
- **Not storage.** The field exists in the client's player object because `hl_weapons.cpp:20` includes
  `player.h`. It is zero-filled, not absent.
- **Not the wire.** The client is *already sent* the unlocked mask — `gmsgSkillTree`, fixed 5 bytes,
  received by `CHudAmmo::MsgFunc_SkillTree` in `cl_dll/ammo.cpp` so the Skill Tree can draw itself. The
  data is on the client; it is sitting in the HUD, not in the predicted player.

The blocker is one word: **population**. Nothing else is missing.

### Why This Is Debt
The comment at `dlls/crowbar.cpp:170` currently reads *"m_skills does not exist client-side"*, which is
the wrong diagnosis and makes the fix look structural. Anyone costing `FastReload` from that comment will
conclude the client cannot have skill state at all, when in fact it has the storage, the linkage and the
data, and lacks only the copy.

It is also a silent failure mode. A new Skill that touches a predicted value compiles, links and runs; it
just desyncs, and only under latency, and only sometimes.

### Recommended Next Steps
1. **Carry the unlocked mask in a scratch field on `clientdata_t`.** `UpdateClientData`
   (`dlls/client.cpp:2014-2062`) already pushes ammo and weapon state through `iuser*`, `fuser*` and
   `vuser*`; `HUD_WeaponsPostThink` reads them straight back. One `int` field carries 32 skill ids, and the
   tree's highest id today is 21.

   By inspection of `dlls/client.cpp`, `cd->iuser3`, `cd->iuser4`, `cd->fuser1`, `cd->fuser4`, `cd->vuser3.x`
   and `cd->vuser3.y` are unwritten and available. Confirm none is claimed by the engine before taking one,
   and prefer an `iuser` field: `vuser` components are `vec_t` (float) and would silently lose bits above
   2²⁴.
2. **Rebuild `m_bUnlocked` from the mask** in `HUD_WeaponsPostThink`, next to the existing ammo copies.
   `BuildUnlockedMask` (`dlls/player_skills.h:91`) already produces the packed form; the inverse belongs
   beside it so the two cannot drift.
3. **Then delete the `#ifndef CLIENT_DLL` in `crowbar.cpp`** and confirm the animation desync is gone. That
   is the cheapest possible proof the mechanism works, because the bug it fixes is already known and
   reproducible.
4. **Only then unreserve a Skill.** `FastReload` is the smallest.

Do **not** solve this per feature. Seven features want it, and a mask on the predicted player serves all of
them through the same `HasSkill` call the server uses — which keeps the
`PulseWindowFor` / `PulseRechargeFor` pattern (read the modifier where the value is computed) intact on
both sides instead of forking it.

Note the ceiling this introduces: skill ids ≥ 32 would need a second field. Ids are frozen and only ever
grow, so the day that matters is real but distant.

### Acceptance Criteria For Closure
- `m_pPlayer->m_skills.HasSkill(...)` returns the same answer in `cl_dll` and `dlls` for the local player.
- `dlls/crowbar.cpp` has no `#ifndef CLIENT_DLL` around Crowbar Reach, and no swing at the extended reach
  plays a miss animation for a hit that landed.
- One reserved prediction-blocked Skill is unreserved, wired, and shows no visible hitch under artificial
  latency.
- No Skill effect is applied twice — the client predicts it, the server decides it, and they agree.

## The Pulse Discharge Bypasses Every `TraceAttack` Damage Rule — RESOLVED 2026-08-02

Fixed as recommended below, by option 1. `FireDischarge` now copies `gMultiDamage` to a local, runs
`ClearMultiDamage` → `pHit->TraceAttack(...)` → `ApplyMultiDamage(...)` on a clean accumulator, and
restores the original. All twelve rule sets in the table below apply to a Discharge again, including the
alien slave's `DMG_SHOCK` immunity that surfaced the bug.

The `TraceResult` from the Discharge's own traceline is passed through, so hitgroups resolve normally —
head shots, helmets and armour plates all behave as they do for any other attack.

Kept as a record because the underlying hazard has not gone away: **`gMultiDamage` is a single global and
`ApplyMultiDamage` leaves it live**, so any future code that deals damage from inside a `TakeDamage`
handler faces the same trap and needs the same save/restore.

### Scope
`dlls/player_pulse.cpp`, `FireDischarge` (the `pHit->TakeDamage(...)` call). Affects every entity that
implements damage rules by overriding `TraceAttack`.

### The bug
`FireDischarge` traces a line and then applies damage with a direct `pHit->TakeDamage(...)` rather than
`ClearMultiDamage` / `TraceAttack` / `ApplyMultiDamage`. That was deliberate — it runs from inside
`CBasePlayer::TakeDamage`, which is itself normally called from `ApplyMultiDamage`
(`dlls/weapons.cpp:97`), and that function leaves `gMultiDamage` live, so clearing it there would corrupt
the sequence in flight.

The consequence was under-thought. **Half-Life puts a great deal of per-entity damage logic in
`TraceAttack`, and a direct `TakeDamage` skips all of it.** Twelve types are affected:

| Entity | Rule in `TraceAttack` | What the Discharge does instead |
| --- | --- | --- |
| `CISlave` (`islave.cpp:585`) | **immune to `DMG_SHOCK`** | damages it, and provokes it |
| `CBigMomma` (`bigmomma.cpp:559`) | only hitgroup 1 is vulnerable | damages from any angle |
| `CApache` (`apache.cpp:969`) | blades ignore beam/bullet/club | damages the blades |
| `COsprey` (`osprey.cpp:807`) | per-engine damage caps | ignores the caps |
| `CHGrunt` (`hgrunt.cpp:605`) | helmet absorbs 20 | ignores the helmet |
| `CAGrunt` (`agrunt.cpp:221`) | armour plate ricochets | ignores the plate |
| `CBarney` (`barney.cpp:570`) | chest/stomach halve damage | full damage |
| `CBaseTurret` (`turret.cpp:993`) | armour hitgroup | ignores it |
| `CNihilanth` (`nihilanth.cpp:1262`) | irritation state machine | never advances it |
| `CGargantua`, `CGMan`, `CBreakable` | ricochets, sparks, death handling | none of it fires |
| `CBaseMonster` (`combat.cpp:1327`) | hitgroup multipliers | flat damage |

The alien slave was the visible one because the Discharge was `DMG_SHOCK` at the time — the exact type
slaves are built to be immune to — so Discharging into one produced a monster taking damage it should
shrug off, which read in-game as the slave "bugging out".

Note that fixing this then made slaves *correctly* immune, which played badly for a different reason: a
counter that one headline target ignores is the wrong counter. The Discharge now deals `DMG_ENERGYBEAM`,
which nothing is immune to. That is a design decision recorded in
[adr/0006](adr/0006-the-discharge-vents-at-the-crosshair.md), not part of this fix — the bug below was
real regardless of damage type, and eleven other entity types were affected by it.

### Why This Is Debt
The comment at the call site explains why `TraceAttack` is avoided but not what is lost by avoiding it, so
the trade-off looks settled when it is not. It will also silently mis-handle any future entity whose
damage rules live in `TraceAttack`, with no compile error and no obvious symptom.

### Recommended Next Steps
1. **Preferred:** save and restore `gMultiDamage` around a proper nested sequence. It is a plain POD
   (`MULTIDAMAGE` in `dlls/weapons.h:405-412`), so copying it to a local, running
   `ClearMultiDamage` → `pHit->TraceAttack(...)` → `ApplyMultiDamage(...)`, then copying it back, makes
   the nesting safe and restores every rule above. A few lines, surgical.
2. Alternative: defer the Discharge to a pending list drained in `PostThink`, outside the damage
   sequence entirely. Costs the "fires in the same frame the hit lands" property that
   [adr/0006](adr/0006-the-discharge-vents-at-the-crosshair.md) describes.
3. Do **not** special-case the slave. The slave is a symptom.

### Acceptance Criteria For Closure
- Discharging into an alien slave does nothing, because slaves are immune to `DMG_SHOCK`.
- A Discharge into an HGrunt's helmet or an AGrunt's armour plate behaves like any other hit there.
- Hitgroup multipliers apply, or their absence is a documented decision rather than a side effect.

## A Deflected Melee Attack Still Reports As A Hit To The Attacker — MITIGATED 2026-08-02

The view kick is now **scaled rather than suppressed**, which turned out to be the better outcome: a
deflect that produces no reaction at all reads as the blow having missed, where a small nudge reads as it
glancing off. `pulse_deflect_punch` (default `0.25`) controls how much survives; `0` removes it entirely,
`1` restores vanilla.

`CPlayerPulse::DampenDeflectPunch` is called from `UpdateClientData` (`dlls/client.cpp:1932`) rather than
from the player's think, because the attacker writes `punchangle` *after* the player has thought — the
player is entity 1 and runs first. Scaling it a frame later would show the full kick for one frame and
then snap. `UpdateClientData` runs once every entity has thought, so the value is final and the client
never sees the undamped one. Only the delta the attacker added is scaled, so a kick already being carried
from something else is untouched.

**The root cause is untouched** and the entry stays open for it: `CheckTraceHullAttack` still reports a
hit that was refused, so hit-flesh *sounds* still play on a deflect, and any future reaction driven off
that return value will leak the same way. What follows still applies to that.

### Scope
`dlls/combat.cpp:1129` `CBaseMonster::CheckTraceHullAttack`, and every monster that acts on its return
value.

### The bug
`CheckTraceHullAttack` calls `pEntity->TakeDamage(...)` and then returns `pEntity` **whether or not the
damage was actually taken**. Monsters treat a non-null return as "I hit them" and act on it.

So a Pulse that deflects an alien slave's claw still gets the player's view kicked, because
`islave.cpp:317-323` sets `punchangle` on the returned entity. The same pattern appears in the zombie,
bullsquid, agrunt and others — hit sounds and reactions all fire on a deflected blow.

Damage is correctly refused. Only the *reaction* leaks through.

### Why This Is Debt
It undermines the Pulse's core feedback promise: a successful deflect should feel like nothing touched
you. A view kick with no damage reads as the mechanic half-working rather than as a deliberate choice.

The blood half of this same problem was fixed for `TraceAttack` (`dlls/player.cpp:342`), so the
inconsistency is now visible: bullets deflect cleanly, claws do not.

### Recommended Next Steps
1. Have `CheckTraceHullAttack` return `NULL` when `TakeDamage` refused the damage — the signature already
   carries the information, since `TakeDamage` returns a bool that is currently discarded at
   `combat.cpp:1150`.
2. Audit callers first. Some may rely on a non-null return for reasons unrelated to damage landing (for
   example hitting a breakable or a non-damageable entity), so a blanket change could suppress reactions
   that should still fire.

### Acceptance Criteria For Closure
- A deflected melee attack produces no view punch and no hit-flesh sound.
- Monsters still react normally when they strike something they genuinely damaged.

## `IN_INVENTORY` Is Silently Truncated And Never Reaches The Server

### Scope
[common/in_buttons.h](../common/in_buttons.h), [cl_dll/input.cpp](../cl_dll/input.cpp).

### The bug
`IN_INVENTORY` is defined as `(1 << 16)`, but `usercmd_t.buttons` is an `unsigned short`
([common/usercmd.h](../common/usercmd.h) line 29). `cmd->buttons = CL_ButtonBits(true)`
(`cl_dll/input.cpp:745`) assigns an `int` into that 16-bit field, so **every bit above 15 is
discarded**. The bit is set at `cl_dll/input.cpp:876` and arrives at the server as zero.

It is harmless *today* only by accident: `IN_InventoryDown` / `IN_InventoryUp`
(`cl_dll/input.cpp:526-541`) open and close the panel locally on the client, and nothing in
`dlls/` reads `IN_INVENTORY`. The bit is doing no work at all.

### Why This Is Debt
It is a trap rather than a malfunction. The definition looks exactly like every other button
bit, so the first person to write `pev->button & IN_INVENTORY` server-side will find it never
fires, with nothing in the code to explain why.

Note also that bits 0–15 are now **fully allocated**, so there is no room for another button.
`IN_ALT1` (bit 14) and `IN_CANCEL` (bit 6) are set by the client and read by nothing in `dlls/`,
making them the only reclaimable bits. This is why the Pulse uses `impulse 150` rather than a
button bit — see [PILLARS.md](PILLARS.md) pillar 2.

### Recommended Next Steps
1. Either reclaim a dead bit for `IN_INVENTORY` so it genuinely transmits, or drop it to a plain
   `kbutton_t` with no `IN_` constant, making its client-local nature explicit.
2. Comment the 16-bit ceiling in `common/in_buttons.h` so the next addition doesn't repeat it.

### Acceptance Criteria For Closure
- No `IN_` constant exists above bit 15.
- `common/in_buttons.h` states the `unsigned short` limit.

## VGUI Draw Order: All Sprites, Then All Text

### Scope
Every VGUI panel that draws both sprites and text in one pass — currently
[cl_dll/vgui_inventory.cpp](../cl_dll/vgui_inventory.cpp),
[cl_dll/vgui_inventory_grid.cpp](../cl_dll/vgui_inventory_grid.cpp),
[cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp).

### The rule
**Draw every sprite first, then every piece of text.** Text drawn before a later
`SPR_DrawAdditive` gets overwritten — the sprite renders in place of the glyphs.

Interleaving per row or per item looks natural and is wrong:

```cpp
for (auto& row : rows) { DrawSprite(row); DrawText(row); }   // text is eaten
```

Collect the text into a small local list during the sprite pass and flush it at the end
of `paintBackground`. `DeferredCountLabel` in the grid view and `DeferredText` in the
inventory panel are the existing examples.

This applies across the *whole* paint, not just within one helper: a panel that draws
text and then calls a sub-view which draws sprites has the same bug. That is why the
ammo readout collects its counts in the left column and draws them after the Grid.

### Why This Is Debt
The underlying cause is not understood — it is presumed to be text cursor / render state
left behind by the VGUI1 text path, which the sprite path then trips over. Every call
site works around the symptom rather than fixing the cause, and the workaround is easy to
forget because interleaved drawing is the obvious way to write it.

### Recommended Next Steps
1. Find the actual state leak between `drawPrintText`/`drawSetTextPos` and `SPR_DrawAdditive`.
2. If it can be reset explicitly, wrap it in one helper and delete the deferral lists.
3. Failing that, give panels a tiny shared "deferred text" collector so the workaround is
   one type rather than re-invented per file.

### Acceptance Criteria For Closure
- A panel can draw sprites and text in natural order without text loss.
- The deferral lists are gone.

## Skill Tree Tooltip Layout Reliability — RESOLVED 2026-08-02

Closed by doing what step 2 below only offered conditionally. It hedged: *"Replace fixed char-width
constants with proper text measurement **if available in VGUI APIs**; if unavailable, centralize width
constants per font and calibrate once."* It **is** available —
[`utils/vgui/include/VGUI_Font.h`](../utils/vgui/include/VGUI_Font.h) exposes
`getTextSize(text, wide, tall)` and `getTall()`. The heuristics were never necessary, and every one of
them is gone: `kTitleCharW = 11`, `kDescCharW = 7`, the hardcoded 12px line height, and the three-pass
width-fitting loop that existed only because the estimate and the wrap disagreed.

`CSkillTreeView::BuildTooltip` is now the single layout function step 1 asked for. It measures the title
and body with one model, wraps the body to a **pixel** width, and returns a `TooltipLayout` carrying the
final rect, the line height and the laid-out lines together — so the box cannot disagree with what is
drawn into it. It is one pass, not iterative.

Wrapping is exact rather than iterative because of the order: wrap at the maximum allowed width, then
shrink the box to the widest line actually produced. Shrinking to a line's own width can never make that
line overflow, so no second pass is needed.

Both pathological inputs from step 3 are handled: explicit `\n` breaks a line, and a word wider than the
box is broken by character instead of overhanging. The bubble is clamped to the tree area on all four
sides, flipping to the other side of the node before clamping.

All four acceptance criteria are met. Steps 4 and 5 (a debug bounds-drawing mode, and validation at
minimum resolutions) were not done and are no longer needed for this entry — geometry is now derived from
measurement rather than tuned by eye, so there is nothing to eyeball. Low-resolution *layout* is a
separate matter and is handled by the fit-to-area scaling described in PILLARS pillar 4.

### Scope
- Client skill tree UI in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp)
- Specifically the hover tooltip bubble shown for skill descriptions

### Current State (as of 2026-05-09, superseded)
The skill tree has received multiple layout and UX improvements:
- Nodes are centered in the panel.
- Node labels were removed; tooltip carries the detail.
- Tooltip width/height is content-driven with wrapping and clamping.
- Connector lines now anchor to node edges and prefer vertical routing when vertically aligned.

However, tooltip sizing/wrapping is still not fully robust in all cases.

### Known Issues
1. Wrapping and width fitting can still produce edge cases where text appears cramped or wraps sub-optimally for some strings and panel widths.
2. Font metrics are approximated with fixed per-character widths, which is inherently imprecise across different VGUI fonts and resolutions.
3. Title width and wrapped body width are estimated separately and can still mismatch slightly in extreme cases.
4. Tooltip geometry is computed in a single render pass and is sensitive to heuristics (char width, line height, padding), making behavior difficult to guarantee.

### Why This Is Debt
- Logic relies on heuristic text measurement rather than actual font metrics.
- Layout behavior is not deterministic across all content/viewport combinations.
- Repeated tweaks increase complexity without establishing a canonical measurement model.

### Recommended Next Steps
1. Introduce a single tooltip layout function that:
   - measures title/body with one shared width model,
   - wraps body to the same final width,
   - returns final rect + line positions in one structure.
2. Replace fixed char-width constants with proper text measurement if available in VGUI APIs; if unavailable, centralize width constants per font and calibrate once.
3. Add guardrails for pathological inputs:
   - very long unbroken words,
   - explicit newlines,
   - narrow panel widths.
4. Add a small debug mode (temporary cvar or compile flag) to draw tooltip bounds and line boxes for visual verification.
5. Validate at minimum resolutions and common HUD scaling settings.

### Acceptance Criteria For Closure
- No title overflow at supported resolutions.
- Description wraps predictably with consistent left/right padding.
- Tooltip height always fits content with stable top/bottom spacing.
- Tooltip remains clamped to skill-tree panel bounds.

### Notes
This debt is non-blocking for gameplay and can be addressed in a dedicated UI refinement pass.

## Inventory Grid Placement Consistency — RESOLVED 2026-08-01

Closed by deletion rather than by fixing. The client-side placement solver
(`NormalizeGridLayout`) no longer exists: the server owns each Entry's position, the client
renders what it is told, and nothing re-packs the Grid behind the player's back. See
[adr/0004-the-server-owns-the-inventory.md](adr/0004-the-server-owns-the-inventory.md).

All three acceptance criteria are met structurally, not behaviourally — an Entry can only sit
where the server put it, and the server refuses any placement that overlaps or overflows:

- No overlap/overflow across all Entry widths — enforced by `CPlayerInventory::CanPlaceAt`.
- Repeated drag/drop sequences produce stable placements — a drag is a request for one specific
  Cell, accepted or rejected; there is no solver to produce a different answer the second time.
- Non-involved Entries never move — nothing but an explicit `inv_move` changes a position.

Kept as a record because the mixed-width fragmentation that caused much of this is also gone:
the Grid width is now a multiple of the weapon width, so no Cell is stranded at a row end.
