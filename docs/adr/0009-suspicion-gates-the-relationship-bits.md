# Suspicion gates the relationship bits, and nothing else

The Suspicion meter is enforced in exactly one place: `CBaseMonster::Look` (`dlls/monsters.cpp`), where it
withholds `bits_COND_SEE_HATE`, `bits_COND_SEE_DISLIKE` and `bits_COND_SEE_NEMESIS` for the player until the
meter reaches the acquisition threshold.

Nothing else in the perception chain is touched. `GetEnemy`, `CheckEnemy`, `BestVisibleEnemy`,
`GetIdealState`, the schedules and the squad code are all unmodified, and every hostile that is not the
player is still acquired the instant it is seen.

## Why those three bits

They are the whole of the acquisition path. `GetEnemy` (`dlls/monsters.cpp:3334`) runs only when
`bits_COND_SEE_HATE | SEE_DISLIKE | SEE_NEMESIS` is set, and it is the only thing that writes `m_hEnemy` from
sight. Withhold those three and the monster does not acquire; grant them and it acquires in one frame,
exactly as the base game does. Between those two states there is nothing to get wrong, because there is no
new code on the path.

Everything else `Look` produces is left alone, and each exclusion is load-bearing rather than incidental:

| Left untouched | What would break if it were gated |
| --- | --- |
| `bits_COND_SEE_CLIENT` | scripted AI trigger conditions, and Barney and the scientists noticing the player at all |
| the `m_pLink` list | `BestVisibleEnemy` for every *other* target the monster is choosing between |
| `SF_MONSTER_WAIT_TILL_SEEN` | monsters authored to activate on being seen would never activate |
| `bits_COND_SEE_ENEMY` | an already-alerted monster would lose track of the player mid-fight |
| `bits_COND_SEE_FEAR` | fleeing behaviour, which is not an acquisition and has no business waiting on a meter |

That last row is why the gate is written as three guarded cases inside the existing `switch` rather than as
an early `continue`: the fear case has to keep running.

## Considered options

**Gate `GetEnemy` instead.** Superficially tidier — one branch at the top of one function. Rejected because
`GetEnemy` is also the path by which a monster acquires a *different* hostile, and by which `PopEnemy`
restores an older one. Gating there would make Suspicion govern monster-versus-monster acquisition as a side
effect, which is precisely the generalisation
[PERCEPTION.md deliberately does not make](../PERCEPTION.md#deliberately-not-generalised) — and would make it
without the muzzle-flash asymmetry being resolved first.

**Gate `FInViewCone` or `FVisible`.** The most tempting, because "he can't see you" is what the mechanic
*means*. Rejected because both are called from far more than perception — `FVisible` alone is used by cover
selection, by grenade throwing, by `NoFriendlyFire`, and by the squad code. A monster that could not trace to
the player would also stop being able to aim at one it had already acquired.

**A new `MONSTERSTATE`.** Rejected outright. `m_MonsterState` is saved, is read by dozens of schedules, and
`GetIdealState` is overridden by dozens of monsters. Adding a state means auditing all of them, for a concept
that is a float rather than a mode.

**Suppress the monster from `UTIL_EntitiesInBox` results.** Would have removed the player from the monster's
world entirely, breaking `SEE_CLIENT` and the link list along with acquisition. Rejected for the same reason
as the cone.

## Consequences

**The blast radius is one function.** Every behaviour downstream of `m_hEnemy` is unchanged, which is what
makes it credible that the vanilla campaign still plays the way it did. `suspicion_enable 0` restores the
base game exactly, and is there so that claim can be checked in play rather than asserted.

**Gating `Look` alone is not sufficient, and this is where that showed up.** `TakeDamage` never sets
`m_hEnemy` — it sets the last known position and turns the monster toward the attack, and leaves acquisition
to `Look`. Gate `Look` and a monster the player has just shot waits out its own meter before fighting back.
`SuspicionFromDamage` fills the meter outright for player-dealt damage. It is the only other write to the
meter, and it is still an *acquisition* write: it decides that a fight starts, which is this decision's
remit. A write that decided a fight should *end* would not be.

**Combat is a one-way door, still.** `GetIdealState`'s only exit from `MONSTERSTATE_COMBAT` is
`m_hEnemy == NULL` (`dlls/monsterstate.cpp:201-206`), and this change does not add one. So the gate decides
whether a fight *starts* and has no say in whether it ends.

That boundary is enforced by one line: `UpdateSuspicion` pins the meter to 1.0 the moment `m_hEnemy` is the
player and returns. **Past acquisition the meter has no effect on anything**, which is what makes the
one-function claim above literally true rather than approximately true.

De-escalation was attempted on 2026-09-02 and reverted the same day, for that reason: giving the meter a
second job — deciding when a fight *ends* — cost the property that makes this design defensible, inside a
commit that was supposed to be about detection. It is deferred to
[the post-aggro step](../ROADMAP.md#the-post-aggro-step) with its diagnosis intact. When it is built, what
holds an enemy must be **contact** (last sight, or last damage) and not the meter, or a monster under fire
from an unseen attacker will quietly time out mid-firefight.

**A monster outside the player's PVS is frozen, not draining.** `RunAI` does not call `Look` at all in that
case (`dlls/monsterstate.cpp:82`), so a monster the player has walked away from keeps whatever Suspicion it
had. The per-think delta is clamped to 0.5s so that its first think on the player's return is a normal step
rather than a several-minute one. This is the right behaviour and not merely a tolerable one: forgetting
should cost the player time *in the room*, not time spent elsewhere.

**The meter is advanced from inside `Look`, which means it is advanced once per think and not once per
frame.** Think intervals vary by monster and by state, so it accumulates against `gpGlobals->time` rather
than per call. The consequence is that a monster in a state that thinks rarely fills its meter at the same
rate as one that thinks often, which is the intent.
