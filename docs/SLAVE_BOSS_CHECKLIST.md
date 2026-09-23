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
