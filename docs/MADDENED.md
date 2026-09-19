# The maddened miner

`monster_maddened`, built 2026-09-19 and verified in game the same night. This is the record of what he
is today, in enough depth that the grill that follows can start from the built thing rather than from the
brief. The decisions behind him, with their rejected alternatives, are in
[ROADMAP.md](ROADMAP.md#the-cult-and-the-maddened); what stands in for art is in
[ART_DEBT.md](ART_DEBT.md#the-maddened-miner--the-players-body-gordons-face-a-black-crowbar-no-flinch).

## Who he is

A miner who spent years at the deepest faces, heard the voice, and did something with it. Not controlled:
the Nihilanth plants ideas and nothing else, so he is a man who broke, not a puppet. The maddened are the
fair melee enemy of the first hour and the stealth tutorial before the soldiers arrive; the cult is their
organised form and will share his class and most of his code. The first one the player meets is the
missing man of shaft1's crew, at the vein in the old workings, heard three times down the road before he
is seen.

## The seven decisions, as built

| # | Decision | In the code |
| --- | --- | --- |
| 1 | The player's body, from the SDK sources | `models/maddened.mdl`, built by `E:\CustomAssets\scripts\maddened_build.py` |
| 2 | ~~He walks, always~~ **He runs when he chases** — reversed on play the same night: the walking miner "looks like no threat" (Andrei), the running one had already worked | `run` is the player's `new_run` at 40 fps under `ACT_RUN`; the base AI's chase uses it. Unaware, he stands and walks |
| 3 | Health 60, swing 15, reach 64, about a second a swing | `sk_maddened_health` 50/60/70, `sk_maddened_dmg_swing` 10/15/20; the base AI's 64-unit melee test; the swing at 12 fps over 13 frames |
| 4 | Everyone's enemy, allied with his kind | `CLASS_MADDENED` (15), a new row and column in the relationship table |
| 5 | The default Perception Profile, backstabbable | Nothing overridden: `GetPerceptionProfile` returns the default and `CanBackstab` is true on every monster |
| 6 | Stand-in sounds: the slave's words as his mutter, the crowbar's, Barney's | `pMutterSounds` on a 4–8 s clock in `PrescheduleThink`; hit and miss on the swing event; pain and death |
| 7 | The one in shaft1 is unsuited | `"suited" "0"` on the entity at the vein, in `maps/shaft1.map` and the spec |

## The code

`dlls/maddened.cpp`, `CMaddened : CBaseMonster`, about 250 lines. Nothing in it is a schedule: the base
AI already knows how to notice, chase and swing, and gives a monster the melee capability by itself when
its model has a sequence tagged `ACT_MELEE_ATTACK1` (`CBaseMonster::MonsterInit`, `dlls/monsters.cpp`).
What the class supplies:

- **Spawn.** The human hull, `MOVETYPE_STEP`, red blood, the skill health, `VEC_VIEW`, a 0.5 field of
  view (the zombie's), `bits_CAP_DOORS_GROUP`. The `suited` keyvalue sets `pev->body` and `pev->skin`
  together: body is head + 2 × weapon in the QC's bodygroup order, so bare head with the pick is 2 and
  helmet with the pick is 3; skin 0 is the suit, 1 the overalls.
- **The swing.** The QC fires event 1 on frame 5 of either swing. `HandleAnimEvent` runs
  `CheckTraceHullAttack(70, sk_maddened_dmg_swing, DMG_CLUB)`, the zombie's trace: a hit punches the
  victim's view and shoves it 60 forward, and plays a body-hit sound; a miss plays the miss. The base AI
  decides when to swing: within 64 units, facing within a 0.7 dot, the enemy on the ground.
- **The mutter.** `PrescheduleThink` runs its own clock rather than the AI's idle-sound roll (which fires
  about every ten seconds and only in IDLE): every 4–8 s, alive and not mid-swing, one of five
  `aslave/slv_word*.wav` at half volume and pitch 85–95. He mutters while he walks at you. It is his tell
  in the dark, the way the Panthereye's growl is.
- **Pain and death.** Barney's, a third of the time on pain, always on death.
- **Classify.** `CLASS_MADDENED`. The table in `IRelationship` grew from 14×14 to 16×16: row and column
  14 is `CLASS_VEHICLE`, which Valve defined and never gave a row (all `R_NO`), and 15 is the maddened.
  His row: dislikes machines, hates the player, passive humans, human military, alien military, alien
  monsters and player allies, dislikes prey, predators and both bioweapons, ignores insects and
  vehicles, and is allied with his own class. Everyone else's column: the player and both bioweapons
  dislike him, humans and alien military hate him, alien monsters, prey and predators dislike him,
  insects fear him, passives and vehicles ignore him. `IRelationship` now range-checks both classes,
  since `CLASS_BARNACLE` is 99 and never indexed the table on purpose.
- **Save.** `m_bSuited` and the mutter clock.
- **Skill cvars** in `dlls/game.cpp`, read in `dlls/gamerules.cpp` into `gSkillData.maddenedHealth` and
  `maddenedDmgSwing`, declared in `dlls/skill.h`.

What he does not have, on purpose or not yet: no flinch (the rig has no flinch
animation, so a hit is blood and a number until one kills him), no Disturbance hearing (only Trained
profiles listen), no ranged anything, no squad, no ritual state (the cult's, and it has to be his own
spawn state rather than a script, because a scripted monster's Suspicion is frozen), no schedule of his
own.

## The model

`models/maddened.mdl`, 177 KB, 45 bones, 42 textures, three bodyparts, twelve sequences. Built by
`maddened_build.py` from `Player Models/player/` in the SDK ([HL_SDK.md](HL_SDK.md)); the pattern for
any monster on the player's rig.

- **Meshes.** The body is cut by the script from `player_template_biped1.smd`, the whole-body reference,
  by dropping every triangle that touches `Bip01 Head` or the five face bones: 486 triangles kept, 153
  dropped, 255 vertices, the shipped player's own count. The SDK's split `(No_Head)1` file is not used:
  it is a broken export with its vertices on the ends of bone chains, and it is why the first miner in
  game folded at the shoulders ([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md), *Facts that bind the work*).
  The heads are the SDK's `(Gordon_Head)1` and `(Helmet)1`. The pick is `reference_crowbar`, the crowbar
  mesh skinned to a `Box01` bone under the right hand, wearing the black crowbar texture from
  `models/src/w_pickaxe/`.
- **Bodygroups.** `body` (one), `head` (bare, helmet), `weapon` (blank, pick).
- **Skins.** Two families: the suit as shipped, and overalls, which are the suit's thirteen shell textures
  (back, bicep, calf, chest, chrome, cuff, deltoid, forearm, knee, leg back, legs, shoulder joint,
  shoulders) rewritten to a work blue at their own luminance, a little darker. Face, teeth, mouth, helmet,
  gloves and boots are the same on both. The three suit colours are not on him yet; when they are, they
  are three more families on the same textures, the way the suit pickup has them.
- **Sequences and activities.** `idle` and `idle2` (`new_idle`, `new_idle2`, ACT_IDLE), `walk` (`new_walk`,
  ACT_WALK, linear movement extracted), `run` (`new_run`, ACT_RUN, 40 fps), `swing_down` and
  `swing_up` (`ref_swingdown_crowbar`, `ref_swingup_crowbar`, ACT_MELEE_ATTACK1, event 1 at frame 5, 12
  fps), `die_simple`, `die_backward`, `die_forward` (`player_die1`), `die_headshot`, `die_gutshot` (the
  player's, with their body-drop events), `falling` (ACT_FALL).
- **The QC otherwise** is the player's: the Character Studio bone renames, the three hand attachments,
  the four spine controllers, the hitboxes. The origin is the feet (no `$origin`), where the player's is
  36 up at the hull's centre.
- **Textures** stay internal (no `$externaltextures`), one file ships.

## Where he is placed

shaft1's vein, in the old workings' third leg, unsuited, facing the vein; the road's three whispers are
`ambient_generic`s that play his sound before he is seen. Placing another is one entity: `monster_maddened`
with `suited` 0 or 1, and `info_node`s, since he walks the node graph like any monster.

## Numbers to judge against

| | |
| --- | --- |
| Health | 60 on medium: three pick swings (25) from the front, two with a Melee Damage node, one Backstab (3×) |
| Swing | 15 on medium, `DMG_CLUB`; seven hits on an unsuited player with 100 health |
| Reach | 64 to decide to swing, a 70-unit trace when it lands |
| Cadence | about 1.1 s a swing, then the AI re-evaluates |
| Walk, run | `new_walk` at 26 fps unaware; `new_run` at 40 fps in the chase; each its own extracted speed |
| Mutter | every 4–8 s, half volume, pitch 85–95 |
| Notice | the default profile: fill 1.0, drain 1.0; no Disturbances |

## Open for the grill

Not decided, or decided for now and worth revisiting once he has been fought a few times:

- **The run, now that he has it.** He runs in the chase since the first fight. What the cult then has that
  he does not, and whether the cult is a spawnflag on him or a class of its own. Whether the first fight in
  the game, a man sprinting at the player in the dark with a pick, wants a beat of warning before it —
  the mutter already is one.
- **The flinch.** He takes hits without reacting. A short flinch on this rig would also give the player
  model one. Or a rule that a Backstab-arc hit staggers him and a frontal one does not.
- **The profile.** A slower one of his own (fill 0.75, drain 1.5) was proposed and deferred; the cult and
  the leader may want faster ones.
- **The voice.** His mutter is the road's whisper is a vortigaunt's. What he says, in what language, and
  whether the maddened ever speak a word the player understands.
- **The bodies.** Which maddened are suited, and in which colour; whether a suited one drops or hands over
  anything; whether the face is one man's or several.
- **Security.** The roadmap has a maddened security member as a lone grunt; whether that stays a grunt or
  becomes a variant of him with a pistol.
- **The ritual state.** The cult's passive-until-noticed spawn state, which the feeding Panthereye also
  needs: a keyvalue, a looping schedule, woken by `Use`.
- **The leader.** A boss who fights pick against pick: what he has that a maddened does not.
- **The pick.** When the hand-made head exists, it goes into this model as the weapon bodygroup; whether
  the maddened's pick is the Carbon Pickaxe or an older, plainer tool.
