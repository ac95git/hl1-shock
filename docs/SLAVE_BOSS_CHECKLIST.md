# The slave boss build — test rows

Built 2026-09-23 in one session, straight after the boss's grill, one commit per slice, **none verified in
game**. Andrei asked for it that way: *"do it all in separate commits, I'll test everything and report
back."* Each slice has its rows here; write the verdict beside each. The design is
[ROADMAP.md, "The alien slave boss"](ROADMAP.md#the-alien-slave-boss) and
[ADR-0016](adr/0016-the-discharge-is-innate-and-answers-only-slave-beams.md).

Commits are in build order, so a slice that fails can be reverted without the ones before it.

## Slice 1 — the Pulse at one second

`pulse_window` 0.25 → 1.0; the tail and `pulse_tail_scale` removed; Pulse Window (12) inert and
`pulse_window_bonus` removed.

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 1.1 | Tap the Pulse, watch the bar | The Shield state lasts about a second, then the Recharge | |
| 1.2 | Tap into a zombie's swipe, early in the second and late in it | Both negated | |
| 1.3 | Tap with nothing coming | Long Recharge (3 s) straight after the second; no braced clang on a hit taken afterwards | |
| 1.4 | Tap in front of a grunt's burst | Most or all of the burst negated | |
| 1.5 | Hold the key (with Defense Matrix) | The Matrix comes up as the Shield ends, no gap | |
| 1.6 | The Skill Tree, Pulse Window | Reads "No effect yet"; buying it changes nothing | |
| 1.7 | The Shield's look | `pulse_shield_sweep` is still 0.5 in your `config.cfg`, which is half the window, so there is still no hold. Try `pulse_shield_sweep 0.25` for a half-second stand, and keep what looks right | |
| 1.8 | An old save taken mid-tail | Loads; the Pulse is simply Ready or recharging | |

## Slice 2 — the innate Discharge

The Discharge fires for every player with the Pulse, only off a slave's beam, with a green screen flash.
A slave it hits is Staggered. `PulseDischarge` (16) retired; `pulse_discharge_melee` removed.

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 2.1 | Without ever buying a Pulse Skill, Pulse a slave's zap while aiming at him | Two Discharge beams (one per bolt) hit him; the screen flashes green | |
| 2.2 | The same slave, right after | He flinches, his zap is broken off, and he waits at least a second before the next one | |
| 2.3 | Two slaves; deflect one's zap while aiming at the other | The other one takes it and flinches | |
| 2.4 | Pulse a zombie's swipe, a headcrab's leap, a grunt's burst | Negated, and **no** Discharge beam, no green flash | |
| 2.5 | A save where you held Pulse Discharge | The node is gone from the tree and the point is back | |
| 2.6 | Pulse a slave's claw | Negated, no Discharge (a claw is not a beam) | |

## Slice 3 — the Barrier

`func_barrier` (`dlls/bmodels.cpp`), in the FGD and synced to `top_mod.fgd`. **No map places one**, so the
test needs one built: a brush tied to `func_barrier` in `topmap` or a practice map, a slave on the far
side, and a button targeting the Barrier's name. Any stock texture will do for now.

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 3.1 | Look at it | Drawn additive (glowing see-through) without touching Render Mode | |
| 3.2 | Walk into it; shoot through it; throw a grenade at it | Blocks you and the bullets; the grenade bounces; nothing breaks it | |
| 3.3 | Stand behind it while the slave zaps you | His bolt stops at the Barrier, a green beam goes back into him, and he flinches. You take nothing | |
| 3.4 | Press the button, then again | Power off: gone and passable. Power on: back and solid | |
| 3.5 | "Starts Off" flag | Absent until the button is pressed | |
| 3.6 | A grunt shooting at you through it | The bullets stop; nothing comes back | |
| 3.7 | Save and load with it on, and with it off | Loads in the same state | |
