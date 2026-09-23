# The maddened miner

`monster_maddened`, built 2026-09-19 and verified in game the same night. This is the record of what he
is today, in enough depth that the grill that follows can start from the built thing rather than from the
brief. The decisions behind him, with their rejected alternatives, are in
[ROADMAP.md](ROADMAP.md#the-cult-and-the-maddened); what stands in for art is in
[ART_DEBT.md](ART_DEBT.md#the-maddened-miner--ivan-in-blue-a-black-crowbar-no-flinch).

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
| 1 | The player's rig, from the SDK sources. The body changed twice on the evening of 2026-09-19 and settled on **Ivan**, Half-Life's original protagonist as the 25th anniversary shipped him, in his own bulkier suit washed to work blue. One body, no variants | `models/maddened.mdl`, built by `E:\CustomAssets\scripts\maddened_build.py` from Andrei's Crowbar decompile |
| 2 | ~~He walks, always~~ **He runs when he chases** — reversed on play the same night: the walking miner "looks like no threat" (Andrei), the running one had already worked | `run` is the player's `new_run` at 40 fps under `ACT_RUN`; the base AI's chase uses it. Unaware, he stands and walks |
| 3 | Health 60, swing 15, reach 64, about a second a swing | `sk_maddened_health` 50/60/70, `sk_maddened_dmg_swing` 10/15/20; the base AI's 64-unit melee test; the swing at 12 fps over 13 frames |
| 4 | Everyone's enemy, allied with his kind | `CLASS_MADDENED` (15), a new row and column in the relationship table |
| 5 | The default Perception Profile, backstabbable | Nothing overridden: `GetPerceptionProfile` returns the default and `CanBackstab` is true on every monster |
| 6 | Stand-in sounds: the slave's words as his mutter, the crowbar's, Barney's | `pMutterSounds` on a 4–8 s clock in `PrescheduleThink`; hit and miss on the swing event; pain and death |
| 7 | ~~The one in shaft1 is unsuited~~ There is one body now, so nothing to choose | The `suited` keyvalue is gone from the class, the FGD, `maps/shaft1.map` and the spec (2026-09-20) |

## The code

`dlls/maddened.cpp`, `CMaddened : CBaseMonster`, about 250 lines. Nothing in it is a schedule: the base
AI already knows how to notice, chase and swing, and gives a monster the melee capability by itself when
its model has a sequence tagged `ACT_MELEE_ATTACK1` (`CBaseMonster::MonsterInit`, `dlls/monsters.cpp`).
What the class supplies:

- **Spawn.** The human hull, `MOVETYPE_STEP`, red blood, the skill health, `VEC_VIEW`, a 0.5 field of
  view (the zombie's), `bits_CAP_DOORS_GROUP`. `pev->body` is 1, the pick, since the body group has
  one entry; `pev->skin` is 0; `pev->blending[0]` is 127, the centre of the swing's pitch blend, so he
  swings level (the first build had the blend's two endpoint files as two sequences and swung at the
  floor or the ceiling at random). Nothing else touches the blend byte.
- **The swing.** The QC fires event 1 on frame 5 of the swing. `HandleAnimEvent` runs
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
- **Save.** The mutter clock.
- **Skill cvars** in `dlls/game.cpp`, read in `dlls/gamerules.cpp` into `gSkillData.maddenedHealth` and
  `maddenedDmgSwing`, declared in `dlls/skill.h`.

What he does not have, on purpose or not yet: no flinch (the rig has no flinch
animation, so a hit is blood and a number until one kills him), no Disturbance hearing (only Trained
profiles listen), no ranged anything, no squad, no ritual state (the cult's, and it has to be his own
spawn state rather than a script, because a scripted monster's Suspicion is frozen), no schedule of his
own.

## The model

`models/maddened.mdl`, 114 KB, 23 bones, 11 textures, two bodyparts, thirteen sequences. Built by
`maddened_build.py` from Andrei's Crowbar decompile of **Ivan** at
`E:\CustomAssets\models\decompiled\npcs\ivan\`, plus two files from the SDK's `Player Models/player/`
([HL_SDK.md](HL_SDK.md)). Ivan is the protagonist of Half-Life's early builds, a bearded man with a
madman's stare in a bulkier suit that is not the mod's, shipped by the 25th anniversary update as a
multiplayer model; Andrei chose him on 2026-09-20 over the two SDK deathmatch bodies that stood in for an
hour (the sealed-helmet suit and the scientist in a blue coat) because he already looks the part. Not
verified in game yet. The SDK's deathmatch roster (Gordon, the helmet, Barney, the scientist, Gman, Gina,
a grunt, a recon grunt, a zombie) stays the pool of bodies on this rig that take the whole animation set
without hand modelling; Barney's is the maddened security member's when the roadmap reaches him.

- **The rig.** Ivan is the player's skeleton with the finger and face bones dropped, 22 bones, and his
  hand's rest pose matches the SDK's to the fourth decimal. The script fits every SMD to one skeleton,
  those 22 plus `Box01` under the right hand: the SDK's `reference_crowbar` has its 69 bones cut to them
  (all its vertices are on `Box01`), and each animation gains `Box01` at its rest pose from the crowbar
  file, since studiomdl wants every bone in every animation. The fitter refuses a kept bone whose parent
  differs between files and a vertex on a dropped bone. His mesh has vertices on the thighs, calves and
  upper arms, counted before use, unlike the SDK's split player file that folded the first miner at the
  shoulders ([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md), *Facts that bind the work*).
- **Bodygroups.** `body` (Ivan), `weapon` (blank, pick). The pick is the crowbar mesh in the black
  crowbar texture from `models/src/w_pickaxe/`.
- **Textures.** Ivan's ten, as decompiled, with his three suit textures renamed from their multiplayer
  colour-remap names (`suit_front`, `suit_back`, `suit_boot`; the face too, to `face`) and washed to the
  work blue of the first miner's overalls, every pixel to hue 150 at its own luminance, a little darker.
  Face, hair, gloves, the grey collar and the boot soles stay. `$cliptotextures`, as his QC has it. No skin
  families; the suit colours, if the maddened ever wear the mod's suit, are not on him.
- **Sequences and activities.** `idle` and `look_idle` (ACT_IDLE), `walk` (`walk2handed`, ACT_WALK, 26
  fps, linear movement extracted), `run` (`run2`, ACT_RUN, 40 fps), `swing` (ACT_MELEE_ATTACK1: the
  crowbar swing's aiming-down and aiming-up files as one sequence blended on pitch, `blend XR -45 45`, 12
  fps over 13 frames, event 1 at frame 5), seven deaths with the player's activity weights and body-drop
  events (`die_simple`, `die_backwards1`, `die_backwards`, `die_forwards`, `headshot`, `die_spin`,
  `gutshot`), `falling` (the SDK's, fitted; ACT_FALL). The attachments, spine controllers and hitboxes
  are Ivan's QC's. The origin is the feet: the decompile's reference has them there, but its animations
  carry the shipped player's 36-unit origin baked in (idle's root at z 2.9 against the reference's 39.5),
  so compiled as they came he stood in the floor to his waist (Andrei, 2026-09-20). The script shifts the
  root of each of Ivan's animations up by 36 and refuses a decompile whose gap is not that; the SDK's
  fall is authored at the reference height and is not shifted. Checked on the compiled model: the idle's
  box runs 0 to 73, as the first miner's did.
- **Textures** stay internal (no `$externaltextures`), one file ships.

## Where he is placed

shaft1's vein, in the old workings' third leg, facing the vein; the road's three whispers are
`ambient_generic`s that play his sound before he is seen. Placing another is one entity: `monster_maddened`,
no keyvalues of its own, and `info_node`s, since he walks the node graph like any monster.

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
  he does not. (Whether the cult is a spawnflag or a class of its own: one class, a keyvalue setting skin
  and bodygroup — ROADMAP, "How a cultist looks", 2026-09-23.) Whether the first fight in
  the game, a man sprinting at the player in the dark with a pick, wants a beat of warning before it —
  the mutter already is one.
- **The flinch.** He takes hits without reacting. A short flinch on this rig would also give the player
  model one. Or a rule that a Backstab-arc hit staggers him and a frontal one does not.
- **The profile.** A slower one of his own (fill 0.75, drain 1.5) was proposed and deferred; the cult and
  the leader may want faster ones.
- **The voice.** His mutter is the road's whisper is a vortigaunt's. What he says, in what language, and
  whether the maddened ever speak a word the player understands.
- **The bodies.** Every maddened is Ivan now, one face and one blue suit. **Face variants started
  2026-09-23**: hair, beard and skin colours as skin families, made by `maddened_build.py` from hair masks
  Andrei corrects in Paint.NET (drafts from `utils/mdltool/hair_mask_draft.py`); beard shapes are
  modelling, since Ivan's beard is geometry (a wedge down the neck), and become a head bodygroup. Whether the crew that wears the
  mod's suit (Andrei: not all of the crew wears suits, but the ones on special operations always do)
  ever turns maddened in it, and what that body is; whether any maddened drops or hands over anything.
- **Security.** The roadmap has a maddened security member as a lone grunt; whether that stays a grunt or
  becomes a variant of him with a pistol.
- **The ritual state.** The cult's passive-until-noticed spawn state, which the feeding Panthereye also
  needs: a keyvalue, a looping schedule, woken by `Use`.
- **The leader.** A boss who fights pick against pick: what he has that a maddened does not.
- **The pick.** When the hand-made head exists, it goes into this model as the weapon bodygroup; whether
  the maddened's pick is the Carbon Pickaxe or an older, plainer tool.
