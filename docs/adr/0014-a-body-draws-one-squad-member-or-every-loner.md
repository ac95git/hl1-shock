# 14. A body draws one squad member, or every loner

Date: 2026-09-17

## Status

Accepted, not built. Part of [the post-aggro step](../ROADMAP.md#the-post-aggro-step); the model is in
[PERCEPTION.md](../PERCEPTION.md#the-search-is-the-sdks-own--settled-2026-09-17-not-built).

## Context

A player-dealt death inserts a **Disturbance**, a real sound in the sound list, and monsters with a Trained
Perception Profile hear it and can run the SDK's own investigate schedule to it: walk to the body, idle,
walk back. Stealth is predator first — *unseen, kill, unseen again* — so what a body draws decides what
the player's next target looks like.

A squad with a leader sends **one** member, the nearest; the rest hold. That was chosen so a clean kill
rewards the predator with the next isolated target. The question was what a *leaderless* group does. The
SDK dissolves a squad outright when its leader dies — `SquadRemove` nulls every handle, there is no
promotion — so every survivor is a loner in every check, and there is nobody to pick one.

## Decision

**Loners all go.** Every leaderless monster that hears a Disturbance walks to it, and a leaderless group
arrives at the body as a mob. Squads isolate; rabble converge.

## Considered

**One answers, by a claim on the sound.** The Disturbance entry carries a claimant; a leader claims it for
its nearest member, a loner claims it for itself if unclaimed, and later loners turn and hold. One walks
over in every case, and the rule fits in a sentence: a body draws one monster. **Rejected** by Andrei: the
mob has an answer — kill a grunt, make a noise, drop a satchel, leave — and it gives the loop two flavours,
a knife against a squad and a grenade against a mob.

**Loners do not go, they only turn and hold.** Cheapest. Rejected as cutting the Disturbance reaction
entirely, which Andrei was not in favour of.

## Consequences

- **The captain-first opening trades isolation for containment.** Killing the leader turns the one-at-a-time
  Search a squad gives for free into a crowd at the corpse. What it buys instead is that a slip alerts the
  one who saw it and not four (no `SquadCopyEnemyInfo`), everyone fires and throws at once through each
  other (`OccupySlot` and `NoFriendlyFire` both return true for a loner), and nobody propagates a notice.
  The captain is worth killing first for what he does *alive*, not for a node — which is why Cut the Head
  was dropped the same day.
- **Cut the Head (id 129) is retired** and Shroud takes its cell.
- The satchel play is now load-bearing. If explosives are ever rebalanced, the mob is what they are
  balanced against.
- A mob is many monsters on one schedule toward one point; the `SquadMemberInRange` spacing rule does not
  apply to loners, so they will stack at the body. Accepted as the look of rabble.
