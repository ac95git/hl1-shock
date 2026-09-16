# 13. The Pulse is a found Module

Date: 2026-09-16

## Status

Accepted, and **built the same day** (`item_pulsemodule` in `dlls/items.cpp`, the gate check in
`CPlayerPulse::TryPulse` and `MatrixThink`, `PULSE_NONE` on `gmsgPulse`). Untested in game. Reverses the
reasoning recorded in PILLARS.md pillar 2 on 2026-08-02, which this file replaces.

## Context

The Pulse came with the suit. PILLARS pillar 2 gave the reason: *"Skills evolve a verb the player already
has rather than granting it, which lets level design assume it."* The Skill Tree's Pulse nodes were
therefore never hidden, and `EGate::PulseModule` was opened on every spawn as a formality.

On 2026-09-12 the Modules were settled (ROADMAP.md, "Pillar 3: Modules") with the Pulse as the first of
them: *"found in the world, early. It is the player's first Module, and the stretch of game that cannot
assume it is short."* By 2026-09-16 the other three Modules had pickups that open their gates — the Dash
on `item_longjump`, Night Vision and the alien Module on stand-in pickups — and the Pulse was the one
Module still granted for free. Andrei asked for a pickup "for consistency".

## Decision

The Pulse is found. `item_pulsemodule` opens `EGate::PulseModule`; nothing else does, and nothing opens
it at spawn. Without the gate: the Pulse key does nothing and makes no sound, the Defense Matrix cannot
be raised, the Pulse bar is absent (the server sends `PULSE_NONE`, its own state, because the suit no
longer implies the Pulse), and every Pulse node, the Matrix trio included, is a blank pad.

The pickup is a stand-in like the alien Module's: the design says only "found in the world, early", and a
walk-over item is all of that until a place in a map decides where.

## Considered

**Leave the gate open and add the entity anyway.** An entity that gives what the player already holds is
not a pickup. Rejected.

**Open the gate at spawn only in vanilla maps.** Would keep the vanilla campaign playable with the Pulse,
but it makes "found" mean "found, except where it isn't", which is the kind of rule nobody can learn.
Rejected; `give item_pulsemodule` and `skill_open_gates 1` cover the vanilla case for testing, and
`topmap` places pickups.

## Consequences

- Level design may assume the Pulse only after its pickup, which is the rule every other Module already
  lives under. The Backstab, Concealment and the crowbar are what the player has before it.
- A save from before the gate mask existed (2026-09-15) had its Pulse gate opened on load by the formality
  this removes; such a save now loads without the Pulse until `skill_open_gates 1`. Saves written since
  carry the bit and keep it.
- `CItemPulseModule` calls `ForgetSentState` so the bar appears on the next sync rather than on the next
  state change.
- PILLARS pillar 2's "suit hardware" reasoning is superseded here and marked so there.
