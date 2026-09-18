# Overnight brief — mining, Stations, and two leftovers

Settled in a grilling session at 04:20 on 2026-09-18, to be built in the hours after it while Andrei
sleeps. He checks briefly around 10:00 and attends fully from 11:00. This file is the record of what was
decided, so the work survives a compaction, a crash, or a fresh session; [MORNING_CHECKLIST.md](MORNING_CHECKLIST.md)
is the record of what was *built* and is written as each slice lands, not at the end.

## How the work lands

- **A commit per slice on `hl-shock`**, each message ending **"Not verified in game"**. The standing rule
  is that Andrei tests before a commit is blessed; a commit that says it is untested is not blessed, and
  a bisectable history is worth more overnight than a single pile in the working tree.
- **Docs move with the slice that earns them**, per the project's own rule: PILLARS gains what is built,
  the ROADMAP entry shrinks as its content graduates, CONTEXT.md takes each settled term, ART_DEBT takes
  each placeholder.
- **Sequential, one slice at a time.** No parallel agents: nearly every slice touches `inventory_defs.h`,
  the weapon registration and the FGD, and the 2026-09-15 wave showed shared-file contention makes
  per-feature commits impossible. The dependency chain (tool → material → deposit → Station → map) means
  parallelism would buy little anyway.
- **Undecided questions are decided, not deferred** — the option that makes the most sense, written down
  in the commit message and in the checklist's *Decisions I made* section with the alternative rejected,
  so overturning one is a one-line instruction in the morning.

### Where the work stops instead of guessing

1. Anything the memory marks as not to be guessed: the endings, wing identities, how many Pieces, where
   the Dash and the double jump are found, the names.
2. Anything under **pillar 6** — [STEALTH_CHECKLIST.md](STEALTH_CHECKLIST.md) blocks it until Andrei
   reports rows.
3. **Retuning a system already verified in game**: the Defense Matrix's timing, the katana's numbers, the
   suit, Records.
4. Anything needing a decompile — ask Andrei by name, never work around it.

Also: **`topmap` is not touched.** It is out of the repo, J.A.C.K. owns it, and editing it from outside
the editor while it is open silently overwrites.

## The design, as settled

The source entries are ROADMAP.md's [Mining and crystal shards](ROADMAP.md#mining-and-crystal-shards) and
[Stations](ROADMAP.md#stations), both shaped 2026-09-17. What this grill added to them:

### The mining tool

`weapon_pickaxe`, a `CCrowbar` subclass on the katana's pattern — the whole weapon is different numbers
and different models. **25 damage, ~0.75s swing**, difficulty-scaled through `sk_plr_pickaxe1-3`: 2.5×
the crowbar's hit at 1.5× the time, so it beats the crowbar on both burst and DPS, which it must, because
the crowbar is free and the pickaxe costs Cells. Still far under the katana, which costs three Cells and
uranium. Backstab, Melee Force, Melee Reach, Melee Speed and the Follow-Up all come free with the
subclass.

Models are the crowbar's, as placeholders, with an ART_DEBT entry. The stretch goal — last, after
everything else works — is a **black metal** retexture compiled from the two decompiles that exist:
`E:\CustomAssets\models\decompiled\topmod\v_crowbar` (the mod's, three glove families, metal on
`chrome.bmp`) and `E:\CustomAssets\models\decompiled\crowbar_vanilla\w_crowbar`. `p_crowbar` is not
decompiled and stays the crowbar's.

**Not tonight:** the pickaxe as the starting tool in the cold open, and the crowbar becoming a find. Both
are campaign and map decisions.

### The Shard

An ordinary Item Type: **1 Cell, stack of 10**. The stack size is chosen so that a Fuel Processor's price
— one full stack — is legible off the Grid without arithmetic. With a few dozen veins at 3 Shards each,
the game holds roughly 100 Shards, so a Skill Point is about three veins found. Icon is a placeholder
with an ART_DEBT entry; the Grid's Icons are world-model renders and a Shard has no model yet.

### Deposits

`func_deposit`, a **brush entity** on `CBreakable`. Brush rather than a point entity with a model because
no crystal model exists, and because mapper-carved shapes are what make a vein read as hand-placed — "a
secret that happens to be made of crystal". The point-and-model version is deferred until there is a
model; it is not stubbed.

- **Only mining damage breaks one.** Everything else sparks off, and the vein says why: the Prompt names
  it and the **state line** built for record locks reads *Requires a mining tool*. The rule teaches itself
  the first time someone swings a crowbar at one. The katana does not mine — the pickaxe's Cells are paid
  for by exactly this. Explosives are not an alternative route: a deposit is optional content, not a gate,
  and a grenade-priced bypass would make explosives the mining meta.
- **Yield is a keyvalue, default 3**, scattered on break as loose `item_shard` pickups, one entity per
  Shard, tossed from the break point. Walk-over, per ADR-0011; a full Grid refuses politely and the Shards
  stay on the floor, which is the existing rule everywhere. Shards never go straight into the Inventory —
  that is the "destroyed the player's belongings" bug the Stations entry warns about.

### Unstable deposits

A spawnflag on `func_deposit`, and **the form is arcs of electricity** (Andrei, in the grill).

- **Free-running cycle**, from map load until the vein is broken: small arcs crawling over the vein as the
  telegraph, a pause, then the discharge. Free-running rather than proximity- or hit-triggered because the
  rhythm has to be learnable *before* the player commits to it — this is the parry's first tutor, a fully
  readable telegraph with no enemy attached — and because a cycle that runs unobserved is what makes
  luring a zombie past a vein possible at all.
- **One arc per victim.** Every damageable thing in radius that passes a **line-of-sight** trace gets a
  beam drawn to it and takes the damage; cover works. With nothing in range, two or three arcs snap to
  nearby world surfaces so the vein still performs. What is seen is exactly what hurts. Beams originate
  from the vein's surface nearest the victim, not its centre.
- Damage is `DMG_ENERGYBEAM` (the katana's type; nothing in the game is immune to it, unlike `DMG_SHOCK`
  and the alien slave), and it hurts **everything** — player and monsters alike, a hazard on nobody's side.
- Vanilla art only: `sprites/lgtning.spr`, `debris/zap*.wav`.
- **To check, not to decide:** the discharge must be on the Shield's curated damage list
  ([ADR-0005](adr/0005-the-shield-negates-a-curated-damage-list.md)) or the parry cannot tutor anything.
  If `DMG_ENERGYBEAM` is not on it, say so in the checklist rather than quietly widening the list.

### Stations

`func_station`, a brush, on `func_recharge`'s shape: `+use`, a finite budget, a sound. **One press is one
trade.** The Prompt names the Station and states the trade, which answers the roadmap's open "are recipes
known, found or discovered" question by consequence: **known**, and stated. No VGUI panel — that is a
night's work by itself and would force the recipe question open again.

- Identity is a **Station Type** keyvalue from a shared table (`game_shared/station_defs.h`, the Module
  and Item Type pattern), not free-form input/output keys on the entity. Free-form recipes would be a
  crafting system, which nobody asked for.
- **Two types tonight**, because the choice the entry names — *ammunition now, or power for good* —
  needs exactly two to be true:
  - **Fuel Processor**: one full stack (10 Shards) → one Skill Point, **once**, saved on the entity.
  - **Ammunition Station**: 3 Shards → 20 uranium, or → 2 Cores, with which ammunition a keyvalue.
  - Budget keyvalue, default 3 uses, `-1` for unlimited. All prices are first guesses, in the table so
    they are one edit; Andrei tunes them.
- **Room is checked before anything is consumed.** A Station that takes the input and cannot deliver the
  output has destroyed the player's belongings. `TryAdd` already refuses when there is no room.
- Recycling (item → Shards) is **not built tonight** — it is undecided in the entry and is third on the
  reserve list.

### The test bed

`minemap`, generated and compiled here, not built in J.A.C.K.: a small sealed greybox with a few stable
veins, an unstable one in a room with something to lure past it, both Station types, a pickaxe on the
floor, and a case that fills the Grid so the polite refusal can be seen. Installed to `topmod/maps/`, so
the morning is `map minemap` with no editor work. The `.map` source lives in `maps/` in the repo, like
`proving.map` did.

The FGD gains `func_deposit`, `func_station`, `item_shard` and `weapon_pickaxe` in **both** copies (repo
`fgd/halflife.fgd` and `topmod/top_mod.fgd`) regardless — that is the FGD sync rule, not a choice — so
placing any of it in `topmap` later is drag-and-drop.

## The slices, in order

| # | Slice | Contents |
| --- | --- | --- |
| 1 | The mining tool | `weapon_pickaxe`, skill cvars, FGD, ART_DEBT |
| 2 | The Shard | Item Type, `item_shard`, placeholder icon, CONTEXT terms |
| 3 | Deposits | `func_deposit`, mining-only damage, the state line, yield, the scatter |
| 4 | Unstable deposits | The spawnflag, the arc cycle, LOS victims, cvars, the Shield-list check |
| 5 | Stations | `station_defs.h`, `func_station`, the two types, room before consumption |
| 6 | `minemap` | Generated, compiled, installed; FGD finalised |
| 7 | The Pulse's tail | Two cvars, the dimmer ring, a pitched cue, up to the Matrix's raise |
| 8 | The base Dash in the air | Ground check off the base Dash only; the Air Dash gate untouched |
| 9 | *(stretch)* Black metal | `v_pickaxe` / `w_pickaxe` retextured and compiled |

Slices 1–6 are the target for 10:00; 7–9 are bonus and 9 is the first to fall.

**Reserve, in order, if the list finishes early:** ① the crowbar first-swing/follow-up timer split (the
roadmap calls it small, and it is the latent bug that makes Melee Speed silently halve every swing —
now the pickaxe's problem too); ② the melee alien grunt v1, shaped 2026-09-13 and explicitly needing no
new art; ③ Station recycling.

## Caveats agreed in advance

- The Pulse's tail runs **exactly** up to the moment a held key raises the Defense Matrix, which is tuned
  and verified code. If building the tail looks like it would disturb the Matrix, stop and leave it.
- Andrei's standing permission: *if decisions become hard and implementation becomes a slog, stop at any
  time.* Stopping early with a true checklist beats finishing with a doubtful one.
