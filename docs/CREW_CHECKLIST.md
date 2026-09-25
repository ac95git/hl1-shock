# The Crew, step 1 — test rows

Built 2026-09-24, straight after the Crew's grill: the four classes on one base, the lines file, the Prompt,
the FGD. **Not verified in game.** The design and the calls made while building are in
[ROADMAP.md, "The Crew"](ROADMAP.md#the-crew--grilled-2026-09-24). Write the verdict beside each row.

**Before testing:** copy `crew_lines.txt` from the repo root to the mod directory (done once by the build
session, 2026-09-24; again after every edit). The FGD is synced. Place one of each kind in `topmap` or
`mines1` by hand; the flee rows want `info_node`s, which `topmap` does not have.

## The Post and the line

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 1.1 | J.A.C.K.'s entity list | `monster_miner`, `monster_security`, `monster_construction`, `monster_technician`, each with a model preview (Ivan, Barney, Ivan, the scientist in Ivan's suit) | |
| 1.2 | Walk up to each | He stands where he was placed; the Prompt reads *Miner* / *Security* / *Construction worker* / *Technician*, *[E] Talk* | |
| 1.3 | Press use on a miner, several times | One line prints under the Prompt, white, a new one on each press, never the same one twice running; he turns his whole body to face you | |
| 1.4 | Press use on security and the technician | Their own pools' lines; they turn to you (and their heads follow you when you walk round them) | |
| 1.5 | Set `line` to `security_drift_door` on one security | He says that two-line text on every press, and nothing from the pool | |
| 1.6 | Set `line` to a made-up id | One red console line at map load naming it; he falls back to his pool | |
| 1.7 | A miner with `pick 1`, one without | The pick is in the first one's hand only | |
| 1.8 | Several miners left on *Hair colour: Random* | Mixed hair colours, as the maddened have | |
| 1.8a | A miner and a construction worker set to *Ginger* (any one colour) | Both have that colour every time the map loads | |
| 1.9 | Walk into one | He does not step aside; he turns to look at you | |
| 1.10 | Stand near them a minute | Silence: no hello, no chatter, nothing printed unprompted (there are no recordings yet) | |
| 1.11 | A technician, walked round and made to flee | `scientist_suit.mdl` (2026-09-25): the nerd head in Ivan's suit, collar still round the neck, head turning inside it; runs with the scientist's run, no tearing past what HLMV showed | Animation checked in game on the vanilla scientist (`_sv_override_scientist_mdl`), 2026-09-25: "doesn't read too bad". The technician itself untested |

## Harm

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 2.1 | Shoot a miner once | He runs for cover; the others nearby stay at their Posts | |
| 2.2 | Walk up to him after | He runs from you; use gives no line | |
| 2.3 | Kill one with others in sight | The others turn against you, as scientists do: they run from you | |
| 2.4 | Throw a grenade near a group | They scatter from it | |
| 2.5 | A maddened reaches a Crew member | The maddened attacks him; he runs | |
| 2.6 | A security with Trigger Condition *Death* and TriggerTarget at a `player_loadsaved` or `game_end` | Killing him fires it | |
| 2.7 | Crew health | A miner dies to about five 9 mm rounds to the body (25 health) | |

## The lines file

| # | Test | Expect | Verdict |
| --- | --- | --- | --- |
| 3.1 | Edit a line's text in the mod directory's `crew_lines.txt`, then `restart` | The new text prints | |
| 3.2 | Add `wav` naming a file that does not exist | A console line at map load says it is not there; the line still prints | |
| 3.3 | Add `wav` naming any real wav under `sound/` to a use line | It plays from him as the text prints; on security (Barney) the jaw moves | |
| 3.4 | Add a line with `pool hello` and a real wav to the miner kind | Miners greet you once, audibly, when they first see you; nothing prints | |
| 3.5 | Save next to a Crew member, load | He is still there, same line override, same pick | |
