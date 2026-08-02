# The Infusion is one at a time, and a Syringe may be used at full health

Two rules govern when a Health Syringe can be used, and they pull in opposite directions on purpose:

- It **may** be used at full health, wasting whatever ticks land on a full health bar.
- It **may not** be used while an Infusion is already running. The attempt is refused, a denied sound
  plays, and the Syringe is not spent.

Together they make the Syringe **proactive** where the medkit is reactive, which is the only reason for it
to exist alongside one. A medkit answers damage you have already taken; a Syringe is spent *before* the
damage, betting that the next ten seconds will hurt.

## Why full-health use is allowed

`UseMedkit` refuses at full health (`dlls/player_inventory.cpp`), and mirroring that here was the obvious
move. It is wrong for this item.

Using a medkit at full health wastes **all** of it, so refusing is pure protection with no cost. Using a
Syringe at full health wastes only the ticks that land while the bar is topped up — take a hit at second
three and the remaining seven seconds all land. The protection is therefore worth much less, and it buys
that little by removing a real tactical decision: dosing before a fight rather than during it.

Refusing would also produce an absurd cliff — denied at 100 health, allowed at 99 — for an item whose whole
premise is that it pays out later.

**Auto-Consume can never apply to a Syringe** as a direct consequence. That rule consumes a pickup on
contact when doing so wastes nothing ([CONTEXT.md](../../CONTEXT.md)), and "wastes nothing" is not
computable for an effect that pays out over time. Syringes are always taken, never consumed off the floor.

## Why a second one is refused

The alternatives were **extend** (`endTime += duration`) and **refresh** (reset to full, discard the
remainder).

**Extend** wastes nothing, but makes emptying the whole Stack in one press the dominant play. Stacks are
three deep and nothing caps the total carried, so that is potentially minutes of banked healing — and it
destroys the timing decision that the full-health rule above exists to protect.

**Refresh** silently throws away the remaining time. The Infusion's only readout is an on/off status icon
with **no countdown**, so the player cannot know whether refreshing costs them one second or fourteen.
Punishing a decision the player has no information to make is worse than either alternative.

**Refusing** wastes nothing, banks nothing, and needs no information the player does not already have: the
icon on screen *is* the answer to "why was that refused?". That is what lets the icon stay a simple
on/off state instead of growing a timer. It also matches `CPlayerPulse::TryPulse`, which refuses and plays
a denied sound when the player is not Ready — the pattern and the feel already exist in the mod.

## Consequences

The last second or two of an Infusion is dead time: you cannot pre-load the next one. If that grates in
play, the fix is a threshold — permit re-use once the remaining time drops below some value — which is a
tuning change inside `TryStart` and structural nowhere else.

Because refusal returns `false` all the way up through `UseSyringe`, the Stack count is untouched on a
denied press. That is the behaviour to test for; a version that decrements first and starts second would
look identical until the player pressed use twice.

Nothing interrupts a running Infusion — not damage, not fire, not drowning. Half-Life has no notion of
"in combat", and an item that promises 40 health over 10 seconds is only worth using proactively if the
promise holds. It follows that a player can heal and burn simultaneously and net out negative, which is
their read to make.
