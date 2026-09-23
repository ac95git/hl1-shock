# 16. The Discharge is innate, and answers only slave beams

Date: 2026-09-23

## Status

Accepted in the grill of the alien slave boss (ROADMAP.md, "The alien slave boss"). **Built the same
day, and verified in game that evening through the boss** (`TryNegate` in `dlls/player_pulse.cpp`; `SlaveBeamFrom`, `SlaveStagger` and
`CISlave::Stagger` in `dlls/islave.cpp`). Amends
[ADR-0006](0006-the-discharge-vents-at-the-crosshair.md): the Discharge still vents at the crosshair and is
still `DMG_ENERGYBEAM`; what changes is who has it and what sets it off.

## Context

The alien slave boss is the fight that tests the Pulse, and his signature attack, the Overcharge, is answered
only by sending its energy back into him. That needs every player to have the Discharge by wing one's end.
It was a Skill, `PulseDischarge` (id 16), on the Juggernaut Route, and by then most players will not have
spent a point on it. Andrei's call: *"make discharge functionality innate to the pulse, disable reflect for
any other attack but the beams. The pulse working on the slave beams innately will be part of the fiction."*

## Decision

- **Innate.** Every player with the Pulse Module Discharges; no Skill grants it.
- **Slave beams only.** A Discharge fires when the Shield negates an alien slave's zap (the stock zap, and
  the boss's volley and Overcharge). Every other hit the Shield negates — melee, falling debris, the rest of
  [ADR-0005](0005-the-shield-negates-a-curated-damage-list.md)'s list — is only negated.
- **Still at the crosshair**, as ADR-0006 decided: a deflect, then aim. A beam can be put into a different
  slave than the one that fired it.
- **A green screen flash** on a Discharge, in place of the suit colour's tint for that event.
- **The Skill is retired.** Id 16 is never reused (the id rule in CLAUDE.md). Its node on the board needs a
  new occupant, open in ROADMAP.md.
- **In the fiction**, the suit's mining shield happens to answer vortigaunt energy.

## Considered

**Back at the slave who fired it, automatically.** Cannot be missed, and the deflect alone is the whole
skill. Rejected for the Pulse: aiming gives the Discharge a use beyond the shooter, and wasting an Overcharge
by looking away is a fair cost in a fight whose loop comes round again. It **is** what a Barrier does (the
world entity, ROADMAP.md), because a Barrier has no crosshair.

**Keep the Skill as an upgrade of an innate return.** Innate: a zap goes back to its shooter; with the Skill:
aim it, and on every negated hit. Rejected for one rule instead of two.

**Retire the Discharge altogether.** Leaves the boss with no answer to the Overcharge but a new one.

## Consequences

- **The melee Discharge is gone**, and with it `pulse_discharge_melee`. PILLARS.md calls the melee vent a
  thing that plays well; it is given up for a Discharge that means one thing. The shelled headcrab's worry
  that a Discharge kills a flipped crab before the swing (ROADMAP.md) is answered by it.
- **The cap stays load-bearing.** `clamp(absorbed × scale, min, max)` against the Overcharge, the biggest hit
  a slave deals. Against the boss it does not matter: a Discharge off an Overcharge breaks a Binding and deals
  no damage to his health.
- **Energy Damage and Weapon Mastery still scale it** (SKILL_TREE.md's cross-Route links); they now scale an
  innate verb rather than a Skill.
- **Save compatibility**: a save holding id 16 keeps the bit, which then does nothing. The node's
  replacement decides whether the point is refunded.
