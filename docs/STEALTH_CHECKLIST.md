# The Stealth checklist — 5f, untested

**This list blocks all stealth work.** Nothing in pillar 6 — not 5b, not 5e, not 5g, not a tuning pass, not
a new node — is touched until every row below has a result from Andrei. Written 2026-09-17, the night 5f
was built; the design it tests is in [PERCEPTION.md part 2](PERCEPTION.md#part-2--the-model-this-mod-adds)
and [ROADMAP.md](ROADMAP.md#the-post-aggro-step). Report results row by row; a row's letter and number is
its name. "Pass", "fail" and what was seen instead is enough; the readouts below give the numbers.

## Setup

`topmap`, Debug build. A grunt squad with a leader (the beret) and at least three members in sight of each
other at spawn, and one lone grunt out of their line of sight. Console:

```
sv_cheats 1
debug_schedule 1        // the monster under the crosshair, and the last kill / search lines
give item_nightvision   // opens the Stealth region
give item_silencer      // E rows
give weapon_9mmhandgun
skill_unlock_all 1      // or buy the nodes a row names; note which
```

`debug_invisible 1` is allowed wherever a row says *from cover* and holding cover is impractical — it makes
you unseen and unheard, and being shot still counts. Say when it was used. `debug_damage 1` for the F rows;
run only one `debug_*` readout at a time, they share the screen centre. The known Debug-build stack check
on any save ([TECH_DEBT.md](TECH_DEBT.md), the save writer) is not a stealth result; note it if it fires
and carry on.

## A. Load and baseline

| # | Do | Expect |
| --- | --- | --- |
| A1 | Load `topmap` | No freeze, no stack check at load. The 2026-09-17 audible-list loop is fixed; this is the regression row |
| A2 | Aim `debug_schedule` at each grunt | Leader reads `leader/N` with the right N; others `member`; the lone one `loner`. Nobody reads `leader` twice unless two squads formed (see the spawn rule in the recap: line of sight at spawn) |
| A3 | Aim at a zombie or headcrab | Readout works; role reads `-` |
| A4 | Stand in front of a grunt, lit, and let it see you | It acquires; vanilla combat follows. Nothing after acquisition should feel different from before |

## B. Witnesses

| # | Do | Expect |
| --- | --- | --- |
| B1 | From cover, Backstab a member while another member can see the victim but not you | Kill line `kill: hgrunt, 1 wit, Disturbance`. The witness turns to the body, speaks one of the witness lines, its `susp` reads 0.75, the Concealment icon goes amber. It does not acquire you |
| B2 | Same, but stand in the open next to the body | The witness is Spotted within a second; the icon goes red and blinks; a fight |
| B3 | Kill a member no other monster can see | `0 wit, Disturbance`. Nobody turns. The icon stays dim |
| B4 | After B1, break contact and watch the witness | Its `susp` drains from 0.75 and **stops at 0.30**; `floor 0.30`. The icon goes back to dim once it drops below 0.35 |
| B5 | Kill a headcrab or zombie in view of a grunt | The grunt is a witness (0.75). Say whether this feels right or wrong — the victim can be any monster today |
| B6 | Kill a member in view of two others | `2 wit`; both jump; only one speaks (the shared speaking gate) |

## C. The Disturbance and the Search

| # | Do | Expect |
| --- | --- | --- |
| C1 | After B3 (a body nobody saw), stay hidden and watch the squad | Within a second or two: `search: hgrunt -> hgrunt`, a **different** grunt than the victim. The leader says a send line if it sent someone else. The searcher walks **to the body**, not past it and not toward you |
| C2 | Watch the others during C1 | They stay where they were and turn toward the body. Only one walks |
| C3 | Aim at the searcher | `InvestigateSound`, task index advancing. At the body it idles about ten seconds, says a no-sign line, walks back to where it stood |
| C4 | Kill a second member while the searcher is still out | No second search line until the first is back. When it is back, a fresh body gets a fresh dispatch |
| C5 | Wait more than 20 s after a kill, then let a grunt arrive in earshot | It does not go: the Disturbance has expired |
| C6 | Kill a grunt in earshot of only zombies | Nobody comes. Zombies do not hear bodies |
| C7 | Kill the leader first, then kill a member out of everyone's sight | Every surviving grunt walks to the body — the mob. Say whether it reads as rabble or as a bug |
| C8 | The lone grunt: kill something in its earshot | It goes itself: `search: hgrunt -> hgrunt (self)` |
| C9 | Throw a grenade near a searcher mid-walk | It breaks off for cover (danger sounds still interrupt); footsteps and a second body do not |

## D. Silent Kill

Hold Silent Kill (id 130) for these.

| # | Do | Expect |
| --- | --- | --- |
| D1 | Kill an Unseen or Noticed member with nobody in sight of it | `0 wit, silent`. Nobody searches, ever |
| D2 | Kill a below-Spotted member in sight of another | `1 wit, silent`. The witness still jumps to 0.75 and speaks; nobody is *sent*. Then the play: silenced headshot on the witness before it fills |
| D3 | Get Spotted by a member, then kill it | `Disturbance` despite the Skill — the victim was at 1.0 |

## E. The silencer

| # | Do | Expect |
| --- | --- | --- |
| E1 | Pick up `item_silencer` with the pistol **in hand** | The attach animation plays; the pistol is locked for about three seconds; the silencer shows afterwards |
| E2 | Pick it up with the pistol holstered, then draw | Draws silenced, no animation |
| E3 | Pick it up **before** having a pistol, then find one | The pistol comes silenced |
| E4 | Idle, reload, draw, fire | The silencer renders in **every** animation, idle included (fixed 2026-09-17; this is the regression row) |
| E5 | Fire | The quiet report, no muzzle flash. Compare with `sv_cheats` off and the flag not found: the loud one, a flash |
| E6 | Fire silenced about 300 units from an unaware grunt facing away | It does **not** turn. Fire unsilenced from the same spot: it turns (quiet is 200 units, normal 600) |
| E7 | Quicksave, quickload | Still silenced. Drop the pistol, pick it up again: still silenced |

## F. The numbers

`debug_damage 1`. Difficulty as noted.

| # | Do | Expect |
| --- | --- | --- |
| F1 | With Headhunter, a silenced round to the **top of the head** of a helmeted member at Noticed (0.75) | About 54 on medium; the grunt dies (50). `xAMBUSH x1.50` in the line |
| F2 | Without Headhunter, same shot | A ricochet, 0.01. The helmet |
| F3 | On hard (80): stab one, swap to the pistol, headshot the witness within two seconds | Swap Surge's x1.5: about 81, dead. Outside the two seconds: 54, alive |
| F4 | With Headhunter and Ambush, a silenced headshot on an **Unseen** member | `xAMBUSH x2.00`, about 72 |
| F5 | Shoot the leader (beret, no helmet) in the head without Headhunter | 24 plus Ambush; no ricochet. He never had a helmet |

## G. Shroud and fill

| # | Do | Expect |
| --- | --- | --- |
| G1 | Open the Skill Tree | Shroud sits at Cut the Head's old cell (13,2), hidden until Night Vision, buyable for one |
| G2 | Buy Shroud, open the Status page | The Concealment line moves by 20% |
| G3 | No Stealth nodes: run at a lit grunt looking at you from close | Spotted in well under half a second |
| G4 | No nodes: crouch across the front of a lit grunt at mid range | Spotted in about 1.5 s |
| G5 | All ten roads plus Shroud: G4 again | About 3.5 s — cover to cover, and not more |
| G6 | Same, in the darkest spot topmap has, Nightfall held | Noticeably longer than G5. Say roughly how long; the light term is the least tuned |

## H. Save and level

| # | Do | Expect |
| --- | --- | --- |
| H1 | After B4, quicksave and quickload | The witness keeps its 0.30 floor and its meter |
| H2 | Cross a level transition with a primed grunt in the transition volume, if topmap has one | Its floor and meter clear (it is *not* pushed to Idle yet — that is 5g). Skip if no transition |

## I. Voices

| # | Do | Expect |
| --- | --- | --- |
| I1 | B1 | A witness line: "we got hostiles", "we got casualties" or "god damn" |
| I2 | C1 with the leader sending someone else | A send line: "sweep that sector", "check that zone", "go recon" |
| I3 | C3 at the turn for home | A no-sign line |
| I4 | Any of the above with an assassin, alien grunt or alien slave, if placed | Silence. Expected; it is in ART_DEBT |

## What a result changes

- A failure in A or C1–C3 stops everything until fixed.
- B5, C7 and G6 are questions, not pass/fail: the answers decide whether victims are filtered, whether the
  mob stays, and how the light term is tuned.
- F rows that miss by a point or two are tuning; F rows that miss by a helmet are bugs.
