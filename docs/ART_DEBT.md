# Art Debt Register

Placeholder assets currently standing in for art this mod does not have yet. Everything here **works** —
it is borrowed stock Half-Life content chosen because it was available, not because it is right. Each
entry names what is wrong with the stand-in, so the replacement is judged against something.

Also here: assets **imported from other mods**, which are not wrong but are not ours. Each of those
entries names the source and the two ways out — credit it and keep it, or replace it — so that neither
is forgotten at release. The first is *The Cleave swing — imported from Half-Life: Extended*; the
second, *The longsword moveset — imported from Age of Chivalry*, is a probe that ships nowhere yet; the
third, *Night vision — imported from Opposing Force*, shipped 2026-09-16.

Distinct from [TECH_DEBT.md](TECH_DEBT.md): that register is about code that needs fixing. This one is
about assets that need making.

## ~~The Pulse — Shield sprite~~ — CLOSED 2026-09-20, by deletion

**Resolved by removing the thing that needed the art.** Kept here rather than deleted because the way it
closed is the useful part.

The debt was `sprites/shockwave.spr` — the **houndeye's sonic blast** texture (`dlls/houndeye.cpp:576-616`)
— drawn as two nested rings around the player, borrowed because it was the only ring-shaped thing already
in the game. The complaint on record was that it read as a houndeye attack because it *was* one, that a
soft diffuse shockwave is the wrong substance for hard protective equipment, and that tinting a greyscale
sprite gave a muddy colour rather than a luminous one. The brief was a crisp-edged energy-field texture
with some internal structure.

None of it was ever drawn. On 2026-09-20 the Shield got a **first-person** treatment instead — a surface
the player is drawn inside, `cl_dll/pulse_shield.cpp`, see
[ROADMAP.md](ROADMAP.md#the-shield-in-first-person--settled-2026-09-20-built-and-seen) — and once that could be
seen beside the rings, Andrei cut them: *"disable or remove the torus/rings all together, they are ugly
compared to what we have on our hand."* The cylinder had already gone earlier the same day. The cvars
(`pulse_ring_style`, `pulse_ring_scale`), the precache and the sprite reference are all gone with them.
The Pulse now draws **no sprite in the world at all** — a `TE_DLIGHT` and nothing else, kept because
lighting real walls is the one thing a first-person overlay cannot do.

**The transferable lesson:** this entry sat open for weeks describing art to replace a placeholder, and
the answer turned out to be that the placeholder's *whole feature* was in the wrong place. Worth asking of
any long-lived entry here — is this a stand-in that wants better art, or a stand-in for something that
should not exist? The successor debt is
[The Shield in first person — surface texture](#the-shield-in-first-person--surface-texture) below, which
is a genuinely different brief: a tiling facet pattern seen from inside, not a beam texture tiled along a
ring.

## The Shield in first person — surface texture

### Scope
The first-person Shield's drawn surface, in `HUD_DrawTransparentTriangles` (`cl_dll/tri.cpp`). Designed
2026-09-20, [ROADMAP.md](ROADMAP.md#the-shield-in-first-person--settled-2026-09-20-built-and-seen); not built
at the time of writing, so this entry describes a stand-in that does not exist yet.

### Current stand-in
**No texture at all.** v1 is pure vertex colour. The surface is read entirely from two procedural cues:
the bright leading edge sweeping out from the crosshair and back, and the obliquity falloff that makes
the Shield near-invisible where you are looking and dense toward the screen edge.

Deliberate, not an oversight. The decision was to find out whether those two cues sell a *surface* on
their own before spending drawing time on one — a question answerable in an evening, where the sprite is
a session of hand work.

### What's wrong with it
Possibly nothing, and that is the point of shipping it this way. The failure mode to watch for is that
without internal structure the Shield reads as **a nicer gradient rather than an object** — which would
be the same criticism that killed the thing it replaced, the flat `hud_pulse_tint` rectangle. If a player
in a real firefight cannot tell they are inside a surface, the procedural cues were not enough.

### What to look for
A **tiling facet pattern** in spherical coordinates — hexes being the obvious first try, panels the
second. It must survive three things the world rings never had to:

- **Every Suit Variant.** The Shield is tinted from `RGB_SUIT` (`cl_dll/hud.h:36`, `gHUD.SuitColour()`),
  which follows the Variant, so the texture is luminance only: greyscale on black, per
  [SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md), with no hue of its own.
- **Additive blending over a bright periphery.** Most of the visible surface is the dense oblique band at
  the screen edge. A pattern that only reads at low brightness will be washed out exactly where it is
  needed.
- **The white-hot deflect flare drawn on top of it.** The flare is a broad quadrant gradient; the facets
  have to stay legible under it rather than disappearing into it.

Pole distortion is a real constraint: spherical UVs pinch at the zenith and nadir, and unlike most
spherical mappings, **both poles are reachable here** — the player can look straight up or straight down.
Either the pattern tolerates the pinch or the mapping is not spherical.

### Done when
A player who has never been told there is a Shield can say, from one press, that something curved is
around them — and can still read the deflect flare through it.

## The Backstab — hit cue

### Scope
`dlls/crowbar.cpp`, in the body-hit branch of `CCrowbar::Swing`.

### Current stand-in
`debris/bustflesh1.wav` / `bustflesh2.wav`, randomised, pitched down to 85–95, played on `CHAN_VOICE`.
Borrowed from `func_breakable`'s flesh material (`dlls/func_break.cpp:669-672`).

### What's wrong with it
- It is the sound of a **breakable crate made of meat**, and a player who has smashed one will know it.
- It is a wet burst. A Backstab is a heavy, precise, committed hit — it wants weight and impact, not gore.
  The gore reading also collides with [Decapitation](ROADMAP.md#pillar-2-decapitation), which is a
  genuinely gory event and should own that timbre.
- Two samples is thin for something the player will hear constantly once melee is invested in.

### What to look for
A dry, heavy, low-frequency impact — the *thump* of a hit landing where it should. It must not share a
timbre with `weapons/cbar_hitbod1-3.wav`, which plays in the **same instant** on `CHAN_ITEM`.

That constraint is the transferable lesson from the Pulse entry above: the Pulse's first sound set failed
not because the samples were bad but because the Pulse and the deflect shared a timbre and landed a
fraction of a second apart, so the deflect was simply not heard. This is the same failure mode with an even
smaller gap — zero.

Being on a separate channel is what makes a distinct sample audible at all; it does not make a
similar-sounding one distinguishable.

### Done when
A player can tell a Backstab landed with their eyes shut, and does not think something broke.

## The Concealment readout — icon

### Scope
`cl_dll/hud_conceal.cpp`, `k_ConcealSprite`.

### Current stand-in
`flash_full` from `sprites/hud.txt` — **the flashlight readout's own icon**. Drawn additively in the
bottom-left suit cluster after the Pulse, tinted by state: the cluster's own `RGB_SUIT` at `MIN_ALPHA`
when Unseen, amber when Noticed, red when Spotted. The two warnings are fixed colours on purpose — they
are the one thing on the HUD the Suit Variant must not recolour.

Chosen because it is the one stock HUD icon that is about light and being seen, and because it exists in
all four resolution blocks of `hud.txt`. That second property is not optional: `autoaim_c`, the other
plausible candidate, has no 320 entry, and `GetSpriteIndex` returns -1 for a missing name, which
`GetSpriteRect` does not check.

### What's wrong with it
- **It is the flashlight.** `CHudFlashlight` draws this same icon top-right and turns it **red when the
  battery is low** (`cl_dll/flashlight.cpp:113-116`). A red flashlight icon in the corner will read as a low
  battery before it reads as *you have been spotted* — the worst possible confusion for the one state that
  most needs to be read instantly.
- It says *light*. Light is one of four Concealment terms and deliberately the weakest; the readout is about
  being **noticed**, not about being lit.
- It is a different size from the suit icons it sits beside (32×32 against 40×40 at 640), so it is centred
  on them vertically rather than sharing their top edge, and still looks borrowed.

### What to look for
An eye or sight-line silhouette in the visual language of the suit readouts beside it, reading at the suit
icon's size at every resolution — 20×20 at 320 through 120×120 at 2560. **Greyscale**, because the tint is
applied additively and a coloured source comes out wrong.

The mod has no `sprites/hud.txt` of its own; it falls back to Half-Life's. Adding any new HUD icon therefore
means shipping a mod-side `hud.txt` containing **all four** resolution blocks, not just the new line.

If the flashlight is replaced by night vision, the night-vision readout must not reuse this icon either —
the collision above just moves.

### Done when
Nobody reads it as the flashlight, and a glance at the corner mid-fight tells Noticed from Spotted without a
second look.

## The Health Syringe — sounds

### Scope
`dlls/items.cpp` (`CItemSyringe`) and the `k_Infusion*` sound constants in `dlls/player_infusion.cpp`.

**World model resolved 2026-09-13.** `models/w_syringe.mdl`, the mod's own and the first model in the
pipeline authored from nothing: one bone, 284 triangles, one painted texture, about 27 units long with a
red liquid filling the front two thirds of the barrel, a dark stopper behind it, graduations, and the
needle along the entity's forward axis. Made by `E:\CustomAssets\scripts\syringe_world.py`
([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md)); the Grid Icon `sprites/inv/item_syringe.spr` is a render of it,
so the Grid now shows the same object the floor does. Size is a first guess and a `--scale` argument;
judge it on a floor. The HUD status icon below is unchanged and still right for the Infusion readout.

**Icon resolved 2026-09-12.** `item_syringe` — a shaded greyscale syringe in the `item_*` family's style
and sizes (20/44/88/132), generated by `utils/sprtool/icons/syringe.py` into `sprites/top/` and declared
in the mod's own `sprites/hud.txt` (built from valve's by `utils/sprtool/make_hud_txt.py`). Used in both
the Grid and the Infusion status icon, as the old `cross` was. The repo's `sprites/` is the source of truth
and is copied to `topmod/sprites/` by hand, the same arrangement as the FGD.

~~**Reopened for the Grid, 2026-09-12.** The Grid's Icons are now full-colour renders of world models
(`docs/SPRITE_WORKFLOW.md`, *Inventory Icons*), and the Syringe has no world model of its own, so it is
one of two Entries still drawn as a tinted HUD glyph in a box among untinted renders.~~ Closed with the
world model above; the Antidote, which has no model in Half-Life at all, is now the only such Entry.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| Picked up | `items/smallmedkit1.wav` | medkit |
| Infusion starts | `items/medshot4.wav` | health charger |
| Use refused while infusing | `items/medshotno1.wav` | health charger, denied |
| Voice line | `!HEV_HEAL7` — "hiss, morphine_shot" | stock HEV sentence |

The use sounds and the voice line are close enough to right that they may simply stay. The **pickup**
sound is the medkit's outright, which is the odd one out: the Syringe deliberately does not sound like a
medkit when it is *used*, so sounding exactly like one when it is *taken* undoes half of that.

### Done when
Taking a Syringe does not sound like taking a medkit.

## The Antidote — no HUD sprite, so no pickup flash

### Scope
`CItemAntidote` in `dlls/items.cpp`, and the `spriteName` column of `k_ItemTypes` in
`game_shared/inventory_defs.h`, which is `nullptr` for it.

### Current stand-in
Nothing. Since every pickup became walk-over (2026-09-13, [ADR-0011](adr/0011-pickups-are-walk-over.md))
a carried item is announced by the pickup history flashing its HUD sprite with the `inv_carried` arrow
over it. The Antidote has no HUD sprite, so `AnnouncePickup` is sent and the history draws nothing: it is
the one Item Type that goes into the Grid with no signal beyond the HEV line.

Accepted knowingly: the Antidote is a placeholder Item Type that does nothing yet. The keycard was in
the same position and got its sprite in the same change — `item_security`, drawn by
`utils/sprtool/icons/security.py` in the `item_*` family's style and sizes — because a door key arriving
silently would have been a real gap.

### Done when
The Antidote has a purpose, and with it a HUD sprite at all four buckets so its pickup flashes like the
others. If it is cut instead, this entry goes with it.

## The Pulse — sounds

### Scope
`dlls/player_pulse.cpp`, the `k_PulseSound*` constants.

### Current stand-ins

| Event | Sound | Borrowed from |
| --- | --- | --- |
| Pulse — Shield goes up | `weapons/cbar_miss1.wav` | crowbar swing |
| Deflect | `weapons/cbar_hit1/2.wav`, randomised + pitched | crowbar hitting a wall |
| Recharge complete | `player/recharged.wav` | **imported from Team Fortress 2** (2026-09-15), its `sound/player/recharged.wav` downmixed to mono; ships in the repo's `sound/`. Replaced the wall charger's `items/suitchargeok1.wav` |
| Pressed while Recharging | `items/suitchargeno1.wav` | wall charger, denied |
| Defense Matrix comes up (`k_MatrixSoundRaise`, 2026-09-16) | `weapons/electro4.wav` | the alien slave's zap — the electrical timbre the tap could not use, because the Matrix has no deflect landing a tenth of a second after it |
| Defense Matrix drops (`k_MatrixSoundDrop`) | `weapons/electro5.wav` | the same zap set |
| Matrix on Kill (`k_MatrixSoundKill`) | `items/suitchargeok1.wav`, quiet | wall charger, "ok" — armour coming back |
| Matrix ready again | `player/recharged.wav` at pitch 80 | the Pulse's own ready chime, pitched down so the two readiness cues are told apart by ear |
| Matrix raise, the light | a `TE_DLIGHT` in the suit's colour, held a second | the Shield's own light, longer; deliberately not its rings, so the two verbs on the key do not look alike |
| A hit braced in the tail (`CPlayerPulse::TailScale`, 2026-09-18) | `weapons/cbar_hit1/2.wav` at pitch 62–68, volume 0.7 | the deflect's own impact, pitched well down — the same family so it reads as the Pulse, duller so it never reads as a parry. Never in the same instant as a deflect: one hit is one or the other |
| The tail begins, the ring | one ring at 0.6 of the Shield's scale, in the suit's colour at a third of its brightness | the Shield's own ring, dimmed. **Not accepted in play (2026-09-18)**: the tail is not clear or rewarding enough to read — see ROADMAP, "The Pulse's tail" |

The Matrix wants a sound that says *stance*: something coming up and staying up, not a discharge, and a
drop that is unmistakably the same thing powering down. It should share nothing with the Pulse's set,
since a Shield fires at the front of every hold and the Matrix follows it a second later.

These were arrived at by elimination rather than design. The first attempt used `weapons/electro4.wav` and
`weapons/electro6.wav`; the deflect landed a fraction of a second after the Pulse in the same timbre and
was **completely inaudible**. The crowbar set was chosen because a swing and an impact are unmistakably
different from each other.

### What's wrong with them
- They are crowbar sounds. They work because of their *shape* — a whoosh followed by a metallic clang —
  but nothing about them says energy field or powered armour.
- Only two deflect samples, leaning on pitch variation to disguise repetition. Under sustained fire the
  repetition is still audible.
- The charger sounds carry the HEV suit's voice-line association, which is close but suggests *refilling*
  rather than *rearming*.

### What to look for
The set has to preserve one property above all: **the Pulse and the deflect must not share a timbre.**
They can land 0.1s apart and the deflect is the one carrying the information — that you read the attack
correctly. If a candidate set fails this, it does not matter how good it sounds in isolation.

- **Pulse**: a short rising energy discharge, not a whoosh. Should feel like something being *deployed*.
- **Deflect**: a hard, bright impact with a metallic or crystalline edge. At least three variants.
- **Recharge complete**: quiet and unobtrusive — it fires constantly and must not become annoying. This is
  the player's only readiness cue when they are not looking at the HUD.
- **Denied**: distinct from the deflect, so a mistimed press is never mistaken for a successful one.

### Done when
Nothing in the Pulse's audio comes from another weapon, and a deflect is unmistakable over the Pulse that
preceded it.

## The Dash — sound

### Scope
`DASH_SOUND` in `pm_shared/pm_shared.h`, played by `PM_CheckDash` in `pm_shared/pm_shared.cpp` on every
Dash, ground and air, and precached in `ClientPrecache`.

### Current stand-in
`zombie/claw_miss2.wav` at pitch 115 — the zombie's swipe missing. Chosen for its shape, a short whoosh,
and because the crowbar's miss is already the Pulse's.

### What's wrong with it
- It is a monster's attack sound. A player who has fought zombies hears a swipe beside them, not their own
  suit moving.
- One sample, no variation: a Route built on dashing will play it constantly.

### What to look for
- A short burst of air and servo — the suit throwing the player, not a blade.
- Distinct from the Pulse's Shield sound, since both are pressed mid-fight and both are whooshes today.
- Two or three variants, or one that survives repetition.

### Done when
The Dash has a sound of its own that no weapon or monster also makes.

## Progression pickups — pickup sound

### Scope
`dlls/items.cpp`, `CItemSkillPoint`, `CItemResetToken` and `CItemRowGrant`, plus the shared
`SetProgressionLook` helper above them.

**World models resolved 2026-09-13.** All three are the mod's own, one-bone props authored from numbers
by `E:\CustomAssets\scripts\progression_world.py` on `smdprims.py` ([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md)),
each in its own shape class so the three are told apart in silhouette:

| Pickup | Model | What it is | Size |
| --- | --- | --- | --- |
| Skill Point | `models/w_skillpoint.mdl` | An upright machined hex shard, point-down in a dark ring base. Faceted like the Xen crystal it replaces, but cut rather than grown. **The shard is the light** | 9 across, 18 tall |
| Reset Token | `models/w_resettoken.mdl` | A thick gold medallion: raised rim, a recessed ring of twelve ticks, a centre boss with a stamp. The word is Token; a coin is what it means. **The ticks and the stamp are the light** | 14 across, 5.5 tall |
| Row Grant | `models/w_rowgrant.mdl` | A rack of four open compartments with a carry handle — the Grid row it grants, made into a thing. Storage without being a Box. **The cell grid on the divider and a strip along the top bar are the light** | 23 long, 4 deep, 9.4 tall |

**They emit light, and the glow shell is gone — settled 2026-09-13, finished 2026-09-14.** Two parts.
Each model carries a second material, `<name>_glow.bmp`, patched `STUDIO_NF_ADDITIVE` in the compiled
`.mdl` by `smdprims.set_flags`: the shard whole, the Token's marks and the Row Grant's grid on thin
overlays a fraction above solid faces, so only the painted marks read as lit. That is the *look*; it
turned out not to glow in the dark on its own, because the renderer still multiplies it by the room's
light. The glow itself is a **dynamic light per visible pickup on the client**
(`cl_dll/entity.cpp`, `ProgressionLight`; radius `progression_light`, 100, the Token a quarter larger),
in the family's colour, which lights the model and the floor around it and costs nothing on the
network. The `kRenderFxGlowShell` and its brief `progression_glow` cvar were removed. (The stock
battery, the reference asked for, emits nothing: its panel is a bright painted texture with no flags,
and it dims with the room.) Colour is baked into each texture (cyan, gold, green), so the models rank on
their own. Sizes and the light radius are first guesses to be judged on a floor. None needs a Grid Icon:
they are banked counters, never Entries.

The stand-ins they replace, for the record: `crystal.mdl` (Xen scenery at quarter scale), `sphere.mdl`
(a featureless ball) and `w_isotopebox.mdl` (a radioactive-materials case), all borrowed from
`valve/models`. Before those, `w_longjump.mdl` and `w_security.mdl`, which were actively misleading.

### What remains

- **The pickup sound is now the whole of the debt.** One generic weapon-pickup click,
  `items/gunpickup2.wav`, for all three, plus a centre-print line. This is the moment exploration pays
  out, and it sounds like picking up ammo. The three should not share a sound at all — a Reset Token is
  rare and should announce itself.

### Done when
The Reset Token does not sound like ammo.

## The Gauss Katana — third-person model, sounds, HUD icon

### Scope
`dlls/katana.cpp`, and `sprites/weapon_katana.txt`.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| Third-person (`p_`) model | `models/p_crowbar.mdl` | the crowbar |
| Swing, hit, body-hit sounds | `weapons/cbar_*.wav` | the crowbar |
| HUD selection icon | `weapon_crowbar`'s sprites, via a copied `.txt` | the crowbar |

The viewmodel and world model are the mod's own (`models/v_katana.mdl`, `models/w_katana.mdl`).

### What's wrong with them
- Other players and the player's shadow hold a crowbar. Single-player barely shows the `p_` model, so
  this is the least urgent of the three.
- The sounds are a crowbar's. A heavy blade wants a longer, lower swing and a cut rather than a clang;
  the crowbar's clang on a wall hit is exactly wrong for an edge.
- The HUD icon **is the crowbar's**, in the same slot, one position over. Two identical icons in the
  melee bucket is the one of these a player will hit immediately.

### Done when
The selection bucket shows a katana, and a swing sounds like a blade.

## The Carbon Pickaxe — a black crowbar, and the crowbar's sounds and HUD icon

### Scope
`dlls/pickaxe.cpp`, `models/{v,w}_pickaxe.mdl`, `sprites/weapon_pickaxe.txt`,
`sprites/inv/weapon_pickaxe.spr`. Built 2026-09-18.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| Viewmodel | `models/v_pickaxe.mdl` — the mod's crowbar in black metal, gloves and Cleave swing kept | the crowbar, recoloured by `E:\CustomAssets\scripts\pickaxe_black.py` |
| World model | `models/w_pickaxe.mdl` — the vanilla crowbar in black metal | the same script |
| Inventory Icon | `sprites/inv/weapon_pickaxe.spr`, rendered from `w_pickaxe` (`render_icon.py --view top --roll 180`) | — |
| Third-person (`p_`) model | `models/p_crowbar.mdl` | the crowbar; no decompile |
| Swing, hit, body-hit sounds | `weapons/cbar_*.wav` | the crowbar |
| HUD selection icon | `weapon_crowbar`'s sprites, via a copied `.txt` | the crowbar |

The black metal (grill, 2026-09-18) is a palette remap of the two metal textures, which are chrome
environment maps: luminance onto a dark curve with a faint cold cast, so the shaft reads as blued steel
with its highlight and the red paint becomes a near-black grip. Only palettes change, so the UVs and the
chrome flags are the crowbar's.

### What's wrong with them
- **It is still a crowbar**, only black. Told apart from the crowbar at a glance now, but nothing about the
  silhouette says *pick*.
- The HUD selection icon is the crowbar's, in the same bucket two places over.
- The sounds are a crowbar's. A pick wants a heavier, duller strike, and a ring on crystal.
- The black icon may be hard to read on the Grid's dark lattice; the Grid judges that, not the sheet.

### What to look for
A real pick head on the crowbar's rig, so the animations carry over. A HUD icon of it. Sounds with weight.

### Done when
A pickaxe in hand, on the floor, in the bucket and in the Grid reads as a pick, not as a crowbar.

## shaft1 — the cold open's stand-ins

### Scope
`maps/shaft1.rooms.txt`, the campaign's first map, grilled 2026-09-19 (ROADMAP.md, *The cold open*). A
greybox by design: everything below is a stand-in on purpose until the plan is walked and J.A.C.K. opens.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| The Heart, on the seat and in the Grid | `item_heart` is not written; when it is, a stock model and a rendered Icon | — |
| ~~The maddened miner~~ | ~~`monster_zombie` at the vein~~ **`monster_maddened` since the evening of 2026-09-19**, its own entry below | — |
| The crew and security | `monster_scientist`, `monster_barney`, no lines | the stock cast |
| The hoist | `func_station` with the charger-panel texture | the Station's own stand-in |
| Rock, gravel, the vein | `-0TNNL_RCK1`, `-0OUT_GRVL1`, `-0OUT_RK3`, `CRYS_1A` | valve's WADs, minemap's set |
| Steel sets, timber sets, lockers, chainlink, water, the ladder | `METAL_BORD07`, `OUT_WD`, `METAL_WALL05B`, `{CHAINLINK`, `{GATE`, `!WATER1`, `{LADDER1` | valve's WADs |
| The shrine | a rock slab, five loose Shards, two candle lights | — |
| **The incantations** heard down the old workings (third cut) | `ambience/alienvoices1.wav`, then `aslave/slv_word5.wav`, then `slv_word2.wav`, each once, louder | Xen ambience and the alien slave's words: thematically the voice, but a vortigaunt's, not a man's |
| The crew's and the guard's lines | sentence groups `SC_PQUEST`, `BA_OK`, `BA_WAIT`, `BA_POK` | vanilla's scientist and Barney vocabulary; none says what the beat means |
| Mine ambience | `ambience/industrial2.wav`, `pumper.wav`, `rotormachine.wav`, `drips.wav`, `wind2.wav`, `alien_hollow.wav`, `the_horror2.wav` | vanilla; the pump and the fan are close to right |

### What's wrong with them
- The Heart is the most important object in the game and has no shape. Amber, arranged, wrong for a mine.
- **The missing man's voice is the road's whole point** and it is a vortigaunt's. The three whispers want a
  human voice half-saying words, the same voice the maddened miner then uses when he turns; the crew's two
  lines and the guard's two want to say what they mean ("he's been gone since second shift", "hand it in").
- The mine has no textures of its own: rock, timber or steel sets, mesh, the rail, the lamps, and the
  amber crystal vein, which is the first texture the WAD was created for.

### Done when
The Heart has a model the Icon is rendered from, and the mine is textured from the mod's own WAD.

## The maddened miner — Ivan in blue, a black crowbar, no flinch

### Scope
`monster_maddened` in `dlls/maddened.cpp`; `models/maddened.mdl`, built by `E:\CustomAssets\scripts\maddened_build.py`.
Grilled and built 2026-09-19 (ROADMAP.md, "The cult and the maddened"). The body changed twice: the
first build was the player's own body with Gordon's face, verified in game; two SDK deathmatch bodies
stood in for an hour; on 2026-09-20 Andrei chose Ivan, Half-Life's original protagonist from the 25th
anniversary update, decompiled by him into `E:\CustomAssets\models\decompiled\npcs\ivan\`, as the one
and only body. Not yet verified in game.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| The body | Ivan, as decompiled, with his three suit textures washed to a work blue by the script; face, hair, gloves, collar and boots as they are | the 25th anniversary's `models/player/ivan/ivan.mdl` |
| The pick | `reference_crowbar`, the crowbar mesh skinned to the hand, in the pickaxe's black | the crowbar; the real head is the hand-made `v_pickaxe` project, and goes into this model the same way when it exists |
| The swing | the crowbar swing's two pitch files as one blended sequence at 12 fps, held level | the player's upper-body crowbar swing: his legs stand planted while he hacks |
| A flinch | none | the player never flinches, so neither does he: hits land without a reaction until one kills him |
| Idle, walk, run, deaths | Ivan's decompiled set, which is the player's | — |
| The fall | `falling.smd` fitted to Ivan's bones | the SDK's player sources |
| The mutter | `aslave/slv_word1/2/4/5/7.wav` at half volume, pitched down, every 4–8 s | the alien slave's words: the voice, but a vortigaunt's |
| Swing miss and hit | `weapons/cbar_miss1.wav`, `weapons/cbar_hitbod1-3.wav` at pitch 80–88 | the crowbar's |
| Pain and death | `barney/ba_pain1-3.wav`, `barney/ba_die1-3.wav` | Barney's |

### What's wrong with them
- **Every maddened is the same man**, Ivan's one face and one suit. A second face is a texture on the
  same mesh; a variety of bodies is the SDK's deathmatch roster on this rig (Barney's, the scientist's,
  the masked grunt's), which the script's fitter already takes.
- **His suit is Ivan's, not the mod's.** A maddened who turned in the mod's suit has no body yet.
- **The pick is a crowbar painted black**, the same stand-in the player's own pickaxe wears.
- **He cannot flinch**, so the pickaxe fight has no feedback on him beyond blood and the health he loses.
  A short flinch animation on this rig would fix it, and the player model would gain one too.
- **The swing is planted**: a full-body swing with a step would read as a man, not a turret.
- **His voice is a vortigaunt's.** The mutter is his tell in the dark and it should be a man half-saying
  words — the same voice the road's whispers use, so the road and the man are one sound.

### Done when
More than one face, a real pick, a flinch, a swing with a step, and a human mutter. Until then he is a
bearded man in a blue suit in the dark with a crowbar, which already reads as a man who broke.

## The melee alien grunt — armour it does not have, and no chainsaw

### Scope
`CAGruntMelee` in `dlls/agrunt.cpp`, `monster_alien_grunt_melee`. Built 2026-09-18.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| Model | `models/agrunt.mdl` with the arm bodypart on `Bare_arm` | the stock alien grunt |
| Weapon | nothing — the bare arm stands in for the alien chainsaw | — |
| Sounds | the stock alien grunt's | — |

### What's wrong with them
- **It wears plates it does not have.** The melee variant takes full damage everywhere; the stock model
  still shows the armour, so a player who has learned that alien grunts shrug off the crowbar will not try.
  Half-Life: Extended's `agrunt_noarmor.mdl` shows the idea exists; its origin is unknown.
- It is told from the hornet grunt only by its empty arm.
- No charge: no alien grunt model on this machine has a charge animation (see ROADMAP.md).

### What to look for
An unarmoured body on the stock rig, so every animation carries over. Later, the chainsaw and a charge.

### Done when
The melee grunt is told from the hornet grunt at a distance, and nothing about it suggests armour.

## The Panthereye — sounds that are nothing like it, and above all the growl

### Scope
`CPanthereye` in `dlls/panthereye.cpp`, `monster_panthereye`. Built 2026-09-18. The model is not debt: it is
Valve's cut Panthereye as HL: Extended ships it, and it is what the monster is.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| **The growl** | `panthereye/pa_idle3.wav` played at pitch 75 (`panther_growl_pitch`) | HL: Extended's Panthereye idle |
| Idle, alert, pain, death, the leap's cry | the other nine `panthereye/pa_*.wav` | HL: Extended |
| Claw hit and miss | `zombie/claw_strike*.wav`, `zombie/claw_miss*.wav`, pitched to 90 | the stock zombie |

### What's wrong with them
- **They are not at all what the Panthereye was meant to sound like** (Andrei, 2026-09-18). All ten.
- **The growl matters most.** It is the player's only warning that they are being stalked, so it is a
  gameplay cue before it is flavour: it has to read as *something low and close is watching me* at low
  volume and short range, and be told from every other monster's idle at once. An idle pitched down is the
  nearest thing in the set, and it is still an idle.
- The claws sound like a zombie's.

### What to look for
A low, continuous, throaty growl that loops or varies well at 3–5 s intervals; a cat-like snarl for the
alert, which is the moment it is spotted and stops hiding; heavier claw impacts than the zombie's.

### Done when
A player hears the growl in a dark room and knows what it means before they see anything, and nothing the
Panthereye makes is mistaken for another monster.

## The Crystal Shard — a glass gib in a glow shell, a glass clink, no Grid Icon

### Scope
`CItemShard` in `dlls/items.cpp`. Built 2026-09-18. The HUD icon (`item_shard`, `utils/sprtool/icons/shard.py`)
is the mod's own and is **not** debt.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| World model | `models/glassgibs.mdl`, body 0, with an amber `kRenderFxGlowShell` (amount 8) | `func_breakable`'s glass |
| Pickup sound | `debris/glass2.wav` at pitch 140–160 | glass breaking |
| Inventory Icon | none — the Grid falls back to the HUD icon, tinted and additive | — |

### What's wrong with them
- A glass gib is flat and shapeless; the shell is what makes it read as anything. On a bright floor the
  shell may be the only thing seen, and the amber was chosen by elimination (the Records own pale cold,
  the progression pickups cyan and gold), not against a crystal.
- Glass breaking, pitched up, is a clink at best and a breakage at worst.
- The Grid Icons are world-model renders; a Shard has no model to render.

### What to look for
A small crystal point, one to three facets deep, in whatever hue crystal settles on for the whole game —
the deposit's texture, the Shard, the unstable vein's arcs and eventually the Heart should agree. A pickup
sound that rings.

### Done when
A Shard on the floor is a crystal at a glance, and its Grid Icon is rendered from that model.

## The HEV suit pickup — three recoloured stock suits

### Scope
`models/w_suit.mdl` and `models/w_suitT.mdl`, compiled from
`E:\CustomAssets\models\src\w_suit` by `E:\CustomAssets\scripts\suit_world.py`. Placed by `item_suit`
(`dlls/items.cpp`), whose `variant` keyvalue sets the entity's skin on spawn.

### Current stand-in
The stock `w_suit` recompiled with three skin families, one per Suit Variant. The recolour is a colour
wash: every pixel of the orange shell keeps its luminance and takes the variant's hue, and the grey and
white armour panels take 22% of it. Cyan is skin 0, under the original texture names, so the model shows
Agility with no code at all.

Deliberately **not** the glove treatment. The gloves keep their plates grey and put the colour only in the
seams, which is right on something held a foot from the eye and useless on something lying on a floor
twenty feet away. Being tellable apart on a floor is this model's entire job — a suit is a pillar 1 find,
and a find the player cannot identify before walking to it is not one.

### What's wrong with it
- **It is Gordon's suit in a different colour.** Nothing about the silhouette says this mod. Goal 4 of
  [the roadmap entry](ROADMAP.md#viewmodel-hands-and-the-custom-hev-suit) is a suit of the mod's own, and
  this exists to be replaced by it — and, meanwhile, to give that design three references to be judged
  against.
- **It covers the pickup and nothing else.** The **wall chargers** still show the stock orange suit, and
  so does the **player model** in a mirror or in third person. A player who sees themselves wearing the
  red suit will see an orange one. The player model is the sharper of the two, because `pev->skin` on the
  player is already the variant — a three-family player model would need no code at all.
- The wash is uniform. The stock texture's coloured details — the blue boot lights, the chest readout —
  go through it with everything else, so they stop reading as separate parts.

### What to look for
A suit that reads as three variants of one design rather than one design in three paints: something that
differs in shape or panel layout, not only hue, so the three are still tellable apart in silhouette and in
the dark. It has to survive being seen at pickup distance on a floor **and** on a standing player.

Whatever replaces it ships as `w_suit.mdl` **and** `w_suitT.mdl` — the QC keeps the stock model's
`$externaltextures`, so the skin families live in the T file, and shipping only one of the two makes the
engine fall back to valve's textures and the suit comes out orange again.

### Done when
Three suits nobody mistakes for Half-Life's, tellable apart across a room, and the same suit on the floor,
on the charger and on the player.

## The Pulse — readout icon

### Scope
`cl_dll/hud_pulse.cpp`, the `suit_full` sprite in `Draw`.

### Current stand-in
`suit_full` from `sprites/hud.txt` — **the armour readout's own icon**, drawn immediately to its right in
the same shape and, since the HUD started following the Suit Variant, in the same colour.

### What's wrong with it
It used to be fine, and the thing that made it fine is gone. The Pulse had a private cyan for exactly one
reason: two identical icons side by side, told apart by colour. A cyan-suited player would have had a cyan
HUD and erased that difference anyway, so the readout gave the colour up rather than keep a cyan the suit
could collide with. What tells them apart now is the Pulse's vertical charge bar, which the armour has no
equivalent of — real, but it is a *neighbouring* element doing the identifying rather than the icon.

### What to look for
An icon that says **energy field, ready** rather than armour: something with an edge or a boundary in it,
readable at the suit icon's size in every resolution block of `hud.txt` (20/40/80/120), greyscale because
the tint is applied additively and a coloured source comes out wrong.

The same `hud.txt` constraint as the Concealment readout applies: the mod ships its own four-block
`hud.txt` built by `utils/sprtool/make_hud_txt.py`, and a new icon must exist in **all four** blocks.

### Done when
The Pulse readout is not the armour icon, and a glance at the cluster tells which is which without reading
the bar.

## The crossbow viewmodel — hand clips through the stock

### Scope
`models/v_crossbow.mdl`, compiled from `E:\CustomAssets\models\src\v_crossbow` by `gloves_rollout.py`.

### What's wrong
Seen in game on 2026-09-12 after the glove skins went in: the right hand intersects the crossbow's stock
slightly. Not a texture problem. Either the stock model always did this and the darker gloves make it
visible, or the decompile/recompile round trip moved something; check by compiling the untouched
decompile and comparing before touching the mesh.

### Done when
The hand sits on the stock through the whole draw and fire cycle.

## The Skill Tree — node icons are load-bearing

### Scope
The `spriteName` column of `k_SkillDefs` in `game_shared/skill_defs.h`.

### Current stand-in
Stock HUD sprites. A Stat node's icon is per stat **on purpose**, so a road reads as what it is made of,
and on 2026-09-16 the roads and the Skills that had an obvious vanilla match were re-picked for meaning
rather than mere distinctness:

- **The hub** wears the HUD's own readouts: Max Health is `cross`, the health number's icon, and Fortitude
  (+25 max HP) the same; Max Armour is `suit_full`, the armour number's icon, and Armor Expert and the
  Juggernaut's continuing armour road the same. Battery Capacity keeps `item_battery`, the pickup that
  does what it does. The Suit has no icon by decision: the processor frame with its die is the mark, and
  the suit icon belongs to the armour roads around it.
- **Medical**: the Healing road is `item_healthkit`, since Healing scales what a medkit heals; Med Expert is
  `item_syringe`, the Infusion it lengthens; Leech `dmg_bio`. Overheal is `dmg_chem` and Last Stand
  `dmg_heat`, placeholders.
- **Weapon Specialist**: the Bullet Damage road is `d_9mmAR`; Marksman is `autoaim_c`, the crosshair that
  is the Route's settled motif; Headhunter a skull (the head), Quick Draw a revolver, Demolitions a rocket.
  Fast Reload (a pistol), Mastery (a shotgun) and Swap Surge (`d_bolt`, freed by Marksman) mean nothing;
  Swap Surge was a hornet until 2026-09-16, which made the Specialist's major read as an Alien node beside
  the Alien region.
- **Melee**: the road is `d_crowbar`, and still wants art of its own, small and plainly "melee". Reach is a
  tripmine, Speed a revolver (Quick Draw's too, in another region), Force a skull, the Backstab node a
  crossbow, Cleave `d_grenade` — placeholders. Cleave's name was `d_handgrenade`, not a sprite in
  `hud.txt`, until 2026-09-16, so the node drew with no icon at all.
- **Energy**: the road is `dmg_shock`; Energy Damage is the egon, Egon Focus `flash_beam` (the one narrow
  beam in the set), Insulation the radiation tile (a resistance, near enough); Energy Efficiency a satchel
  and Overdraw a battery, placeholders.
- **Juggernaut's Pulse and Matrix**, and the three hidden regions, are untouched placeholders: the Pulse's
  four (crosshair, empty flashlight, egon, beam), the Matrix's three (suit, battery, radiation), the
  Shinobi's train arrows and long jump, the Stealth's flashlights and skulls, the Alien's hornet on four
  nodes at once. Nothing vanilla says a Dash, an eye or a hive.

Cleave's **ready icon** at the screen edge is `d_crowbar` too, for want of anything that says "melee"
better; when the Stat node gets its icon, the ready icon should be the same image so the two read as one
thing.

### What's wrong with it
This is the one entry here that is **blocking rather than cosmetic**. The tree is deliberately going
label-free — no node names, no column headers — so that reading it means hovering and discovering
(see PILLARS pillar 4, and the deferred anonymization feature). That design assumes each node's icon
identifies it. It does not: today the icon narrows a node down to "something to do with the suit".

Until the icons are distinct, either the tree keeps text labels or it is unreadable. The design decision
and the art are the same decision.

### What to look for
One distinct icon per Skill, grouped so a branch reads as a branch — a shared motif or palette per column,
with the individual Skill distinguishable inside it. **Settled 2026-09-15** ([SKILL_PANEL.md](SKILL_PANEL.md)):
**monochrome glyphs in the HUD sprite style**, white-on-black additive, tinted by state together with the
node's frame, one motif per Route — blade, crosshair, cross, shield, bolt, hive, wind, eye — with the
Skill's own detail inside it; a Stat node's glyph is its stat's, shared along the road. Full-colour icons
were rejected: on a colour wash they are noise, and the tint has to carry the state.

### The size constraint, which is no longer one
**Since 2026-09-14 the tree fits each icon into its node** through `SPR_DrawFitted` (`cl_dll/spr_fit.h`,
the scaled draw the Inventory Grid uses; see `docs/SPRITE_WORKFLOW.md` for the trap it wraps). The node is
sized by the layout and the icon is scaled to the room above the cost, keeping its aspect. Before that the
tree drew icons at native size and sized the node *from* the largest one, so the art dictated the layout.

HUD sprites are still **resolution-bucketed**: `hud.txt` defines each sprite at 320/640/1280/2560 and the
engine picks the bucket for the current screen, so `item_healthkit` is 44px at 640 and 88px at 1280. The
fit absorbs that — the same icon now lands the same size in the node at every bucket. It also means the
`dmg_*` family (**128×128** at 1280) is usable again for what it fits semantically (`dmg_shock` for the
Discharge, `dmg_chem` for Med Expert); it was excluded only because it was over twice a node.

What that leaves for the replacement art:

- **Make it at least as large as the box, at every bucket.** The engine shrinks but does not magnify
  (`docs/SPRITE_WORKFLOW.md`), so an icon smaller than its node sits at native size with room around it,
  and one larger is shrunk to fit. A Major node offers about 80×56 at full scale with the cost hidden,
  80×44 with it shown; one size a little over that, defined at all four buckets so `GetSpriteIndex` never
  misses, fits every node at every resolution. Oversize is safe; undersize is what shows.
- **Aspect matters more than pixels.** The fit keeps the sprite's aspect, so a tall icon in a wide node
  is small. Icons wider than tall use the node best.
- Below the `k_MinScale` floor the tree clips rather than shrinks, for the cost text's sake, not the icons'.

### Done when
Every Skill in the tree has its own icon, and a player can tell two Skills apart without hovering either.

## Cleave and the Follow-Up — the two empowered swings have borrowed everything

### Scope
`EV_Cleave` in `cl_dll/ev_hldm.cpp` (the sweep and the swing sound), `CBasePlayer::CleaveThink` in
`dlls/player.cpp` (the ready icon and cue), `CPlayerPulse::SyncFollowUpIcon` in `dlls/player_pulse.cpp`
(the primed icon), `CCrowbar::FollowUpSound()` and the two sequence hooks in `dlls/weapons.h`.

### Current stand-in
| Cue | Stand-in | Why it is wrong |
| --- | --- | --- |
| The Cleave air shock | Twelve `shockwave.spr` beam segments as the region's front edge, born at the weapon and travelling to the radius in a quarter second, widening and fading; white for the crowbar, gauss-orange for the katana (`CleaveSweepStyle`) | The right idea in the wrong material: it traces the region honestly and moves like displaced air, but it is still a beam texture, and the katana's orange is the crescent's colour rather than a look of its own. Wants a real displacement — heat-haze, dust, a ring sprite — per weapon |
| The Cleave swing sound | `weapons/cbar_miss1.wav` at pitch 70 | The ordinary miss, lower. Reads as heavier, not as a different act |
| The Cleave wall impact | `weapons/cbar_hit1.wav` at pitch 60, once per wave where the bow first meets a wall | The crowbar's own wall hit, lower. Wants the sound of displaced air stopping, per weapon: a dull thump for the crowbar, a crackle for the katana. (The wave passes through bodies and makes no sound on them; each victim's own hit feedback is the cue) |
| The Cleave ready icon | `d_crowbar`, gold, at the screen edge | The death-notice crowbar; also the Melee Damage Stat node's icon, on purpose for now |
| The Cleave ready cue | `buttons/blip2.wav` at pitch 130, quiet | A button. **Since 2026-09-16 Phantom's start and end cues are the same file** at pitch 150 and 80 ([SKILL_TREE.md](SKILL_TREE.md#stealth)) — three different meanings on one sample, told apart only by pitch |
| The Follow-Up primed icon | `d_gauss`, gold | The Follow-Up's own tree placeholder, itself a gauss gun |
| The Follow-Up attack sound | `zombie/claw_strike1.wav` at pitch 90 | A zombie's swipe, on top of the crowbar's own body-hit sound |
| The Cleave swing animation | Half-Life: Extended's `attack_swing_miss3`, retargeted onto Valve's bones, at `CROWBAR_CLEAVE` on both viewmodels since 2026-09-15 | Not a stand-in — it reads right and Andrei accepted it in game — but imported, not made. Its own entry is below: *The Cleave swing — imported from Half-Life: Extended* |
| The Follow-Up swing animation | None: the stock swing plays | The hook exists (`FollowUpSequence`, −1 for stock) and nothing overrides it; the primed state does not reach the client either |

### What to look for
- **One visual language for "the big swing"**, shared by the sweep and the two icons, so the region on
  the ground, the mark at the screen edge and the node in the tree read as one thing. The sweep should
  say *weight* — a crowbar going through the air — rather than light.
- **A Follow-Up swing animation** on each viewmodel; the Follow-Up's wins when a swing is both. Before it
  can play, Follow-Up-primed has to reach the client the way Cleave-ready now does (clientdata `fuser4`,
  `dlls/client.cpp` → `cl_dll/hl/hl_weapons.cpp`), because the swing animation is predicted; that is
  code, not art. The Cleave's swipe is done for the crowbar hands; a roster weapon on other hands will
  want its own and overrides `CleaveSequence` with it.
- **Two sounds that are not the crowbar's own**: the Cleave's swing, and the Follow-Up's hit, which today
  lands in the same instant as the body-hit sound and has to be told from it — the Backstab cue's lesson.
- The ready cue should sit with the Pulse's Recharge cue, which is the precedent: quiet, short, and not a
  button.

### Done when
A player who has never read a tooltip can tell a Cleave swing from a plain one with the sound off, can
tell it is ready without looking at the screen edge, and sees a Follow-Up coming.

## The Cleave swing — imported from Half-Life: Extended

### Scope
`cleave.smd` in `E:\CustomAssets\models\src\v_crowbar\v_crowbar_anims\` and
`...\v_katana\v_crowbar_anims\` (one file, copied), compiled as sequence 11 (`CROWBAR_CLEAVE`) into
`models/v_crowbar.mdl` and `models/v_katana.mdl`; `cleave_swing_time` in `dlls/game.cpp` is set to its
length.

### What it is
`attack_swing_miss3` from **Half-Life: Extended**'s crowbar viewmodel (the mod installed on this
machine as `Half-Life/hl_extended`; the decompile is at
`E:\CustomAssets\models\decompiled\crowbar_hl_exetended\v_crowbar\`), one of the three extra swing
animations that mod adds after Valve's eleven. Its 45-bone rig carries Valve's eleven bones first,
under other names and with identical rest values, so `utils/mdltool/smd_retarget.py` moved it onto
our hands as a rename and a prune of the fingers and the left arm — the numbers are the original
animator's, untouched. 36 frames at 30 fps: a low horizontal swipe right to left over the first ten
frames with the bar flat across the view and the fork leading, then a long return where the bar comes
up, stands and settles. Installed 2026-09-15 after five cuts of the mod's own script-authored swipe
(`E:\CustomAssets\scripts\crowbar_cleave.py`, kept for its measurements) did not get the bar's attitude
right; Andrei accepted it in game the same day.

### Why it is here
It is another mod's work. Its licence has not been checked and its author is not named anywhere in
this repo. Nothing about it is wrong as art — it is the first Cleave swing that read as a swipe — so
this entry is not a call to replace it; it is a call to settle its status before anything ships.

### The two ways out
1. **Keep it and credit it.** Find Half-Life: Extended's licence and author credit (its `liblist.gam`
   and readme in `Half-Life/hl_extended`; its ModDB page), confirm reuse is allowed, and add an
   attribution line to README.md's contributors and licensing section naming the mod, the animation and
   the author. Then delete this entry. The same check covers anything else taken from that install —
   ROADMAP.md's NPC roster looks at its alien grunt and headcrab models, so one credit line may end up
   carrying several assets.
2. **Replace it with the mod's own.** The slot, the rig contract and the drop-in steps are in
   MODEL_WORKFLOW.md (*Dropping in a hand-made or borrowed animation*), and *What the Cleave swipe
   taught* is the brief for whoever animates it: a horizontal swipe with the fork leading, the bar
   broadside to the view, joints inside Valve's 81° except where the weight of the swing wants more.
   This animation is the reference to match; `bar_lean.py` and the strain table are how to compare.

### Done when
Either README.md credits Half-Life: Extended for the animation under a licence that allows it, or
`CROWBAR_CLEAVE` plays an animation made for this mod. Until one of those, this asset does not ship.

## The longsword moveset — imported from Age of Chivalry

### Scope
`E:\CustomAssets\models\src\v_katana_aoc\` — the probe compiled by `E:\CustomAssets\scripts\katana_aoc.py`
on 2026-09-16 from Andrei's Crowbar decompile of Age of Chivalry's `v_longsword.mdl` at
`E:\CustomAssets\models\decompiled\aoc\v_longsword\`. Nothing in the repo's `models/`; in the install
only as a session's drop-in over `v_katana.mdl` (`--install`), for judging in HLMV and in game
(ROADMAP.md, *The longsword moveset*).

### What it is
Three things from **Age of Chivalry**, the Source mod that became Chivalry: Medieval Warfare: the
44-bone ValveBiped viewmodel rig; its eight longsword animations, verbatim (idle_01, draw, holster,
swing1, swing2, stab, block, deflect — a two-handed moveset, both arms and all fingers animated); and
the `fpv_good_footman` hand mesh, used because it is the one AoC hand mesh under GoldSrc's vertex limit,
with its upper arms cut off and a texture of the mod's own painted onto it (an HEV glove in the three
suit variants, by texture bands), since the decompile brought no textures and the mod is not installed
here. The decompiled longsword mesh rides along as the second submodel of the `blade` bodygroup, for
comparison only. The katana blade on it is ours.

### Why it is here
If the moveset is kept, the animations ship on the katana in some form — as decompiled or retargeted
onto Valve's hands, timed to the weapon or the weapon to them — and either way they are another team's
work. The footman hands would ship only on the painted-HEV-glove route, and the longsword mesh never.
Licence unchecked; author not named anywhere in this repo.

### The two ways out
1. **Keep and credit.** Find Age of Chivalry's licence and the animator's credit (its Steam store page,
   ModDB page and readme), confirm reuse is allowed, and add an attribution line to README.md naming
   the mod, the animations and, if it ships, the hand mesh. Then delete this entry.
2. **Replace.** The animations were the whole point; replacing them means the mod's own two-handed
   swings, which *What the Cleave swipe taught* in MODEL_WORKFLOW.md is the brief for. The hands have a
   replacement already: Valve's gauss hands, on the retarget route in ROADMAP.md.

### Done when
Either README.md credits Age of Chivalry under a licence that allows it, or nothing from the decompile
is in a shipped model. Until one of those, the probe stays a probe. If the moveset is rejected in HLMV,
delete this entry with the probe.

## The Skill Tree — the circuit: substrate, traces and frames

### Scope
The drawing of the Skill Tree panel in `cl_dll/vgui_skilltree.cpp`: the background behind the nodes, the
connectors between them, and the shape of each node. Settled as a direction on 2026-09-15
([SKILL_TREE.md, the presentation](SKILL_TREE.md#the-presentation)); nothing here exists yet, so this
entry is a brief rather than a complaint.

### Current stand-in
A translucent dark rectangle with a purple outline for the background; connectors as one-pixel
`drawFilledRect` doglegs, grey when locked and orange when the prerequisite is held; nodes as filled
rectangles of four sizes (32, 44, 54, 64 on a 96 step) with a top stripe whose height is the tier and a
double outline for a Major. Every node is the same shape, so a Stat node and a Major differ only in size.

### What it wants
The tree is the suit's circuit, and the assets are the metaphor made literal:

- **The substrate.** A tileable circuit-board texture, 256×256 or smaller, low contrast so the nodes
  and traces stay legible over it, loaded as a TGA through VGUI1's `BitmapTGA` (the class-menu loader)
  and tiled under the tree area. One colour wash per Route's region goes over it, faint, so a region
  reads without a label. **The palette, settled 2026-09-15**: Melee rust orange, Weapon Specialist steel
  blue-grey, Medical white-green, Juggernaut HEV amber, Energy electric cyan, Alien Xen green, Shinobi
  violet, Stealth deep indigo, the hub bare copper — each grounded in an object of the game's, and faint
  enough that the suit's cyan / red / purple chrome never fights them.
- **The gauge strip and the Reset switch** ([SKILL_PANEL.md](SKILL_PANEL.md)): a boxed two-digit segment
  display for the Skill Points in the HUD's digit style, token pips, and a red-and-black hazard-striped
  frame for the Reset. Drawable with rects and the HUD's number sprites; a digit sprite of their own if
  the HUD's do not fit the box.
- **Traces.** Connectors become copper traces, one between every pair of orthogonally adjacent nodes and
  nowhere else (there are no gates to draw since 2026-09-15). A trace is dim copper unlit, lit copper
  when either end is held, and glows when it leads from a held node to one that can be bought. Drawable
  with filled rects; the colours and the glow are the art decision.
- **Frames.** One frame sprite per tier, tinted by state exactly as the icons are today (white held, gold
  available, grey locked): a square pad for a Stat node, a small chip for Minor, a larger chip for
  Medium, a large chip with pins for a Major, and the processor for the suit at the centre. The keystone
  is a Major-sized frame in red. Defined at all four resolution buckets like every HUD sprite
  (`docs/SPRITE_WORKFLOW.md`), sized at or above the node at each bucket, since the engine shrinks but
  will not magnify.
- **Gated regions** draw as blank pads on the substrate: a hidden Route shows a footprint, not a hole.

### Done when
A player who has never opened the tree can tell a Stat node from a Skill from a Major by shape, can see
which region is which by colour, and can follow a lit trace from the suit to what they own.

## The Status page — suit silhouette and Module glyphs

### Scope
The doll on the Inventory Panel's Status tab ([STATUS_PANEL.md](STATUS_PANEL.md)): the figure the five
Slots sit on, and the icon in each filled Slot. Settled and built 2026-09-16 with stand-ins.

### Current stand-in
Five boxed rect frames arranged as a body with nothing behind them (`CStatusView::Paint`), and in each
filled Slot an existing HUD sprite named in `k_ModuleDefs` (`game_shared/module_defs.h`), borrowed from the
tree's own nodes: `dmg_gas` Night Vision, `item_longjump` Dash, `autoaim_c` Pulse, `d_hornet` Alien.

### What's wrong with it
Five boxes in a cross read as a diagram, not a suit, and the borrowed HUD sprites share nothing: the
long jump's pack icon says "pickup", not "a burst of speed built into the suit's torso".

### What to look for
- **The silhouette.** An HEV suit outline, front-on, drawn in the suit colour at low contrast behind the
  frames, so the Slots sit on the head, torso, forearms and legs they name. Monochrome and tinted like the
  rest of the chrome, so one file serves all three Suit Variants.
- **Four glyphs**, monochrome, white on black, additive and tinted — the HUD sprite style of the Skill
  Tree's icons. Each is its Module's Route motif from [SKILL_PANEL.md](SKILL_PANEL.md): **eye** Night
  Vision, **wind** Dash, **shield** Pulse, **hive** the Alien Module. Distinct from each other at a
  glance, and related to the tree glyph of the region the Module opens.
- Made with the sprtool scripts, at all four resolution buckets ([SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md)).

### Done when
A player opening the tab reads a suit with parts on it before reading boxes, and can say which region of
the Skill Tree each filled Slot's Module opens from its glyph alone.

## Night vision — imported from Opposing Force

### Scope
`cl_dll/flashlight.cpp` (`DrawNightVision`), `sprites/of_nv_b.spr`.

### What it is
The noise overlay Opposing Force's night vision tiles over the screen, copied from the gearbox install
into this repo's `sprites/` and the mod dir's `topmod/sprites/` and adapted 2026-09-16
([SKILL_TREE.md](SKILL_TREE.md#the-night-vision-module)) alongside `E:\Projects\halflife-op4-updated`,
where the updated source lives. It is Gearbox's asset, not this mod's; the eye-level client dlight beside
it is new code with no art of its own.

### Why it is here
It is another studio's work, shipped without a credit line. Licence unchecked; nothing in this repo names
Gearbox or Opposing Force.

### The two ways out
1. **Keep it and credit it.** Confirm Opposing Force's assets may be reused, and add an attribution line
   to README.md's contributors and licensing section naming Opposing Force and the sprite. Then delete
   this entry.
2. **Replace it.** A noise overlay of the mod's own, in the same visual language as the rest of the
   suit's HUD — the Concealment readout and the Pulse's rings are the nearest references.

### Done when
Either README.md credits Opposing Force for `of_nv_b.spr` under terms that allow it, or the night vision
overlay is the mod's own.

## The Night Vision Module — pickup model and voice line

### Scope
`dlls/items.cpp` (`CItemNightVision`), `models/w_silencer.mdl`, the `!HEV_A1` sentence.

### Current stand-ins
| Use | Asset | Borrowed from |
| --- | --- | --- |
| World model | `models/w_silencer.mdl` | the glock's silencer attachment prop |
| Voice line | `!HEV_A1` | a stock HEV sentence, picked for want of a better one |
| HUD icon | the flashlight's own `flash_empty` / `flash_beam` (`cl_dll/flashlight.cpp`) | unchanged: the icon cannot tell which device is on, only the screen effect can. Whatever it becomes must not be `flash_full`, which the Concealment readout already borrows |

### What's wrong with them
- A silencer is a weapon attachment. Nothing about it says *seeing in the dark*, and a player who has
  found the real silencer prop elsewhere will not read this as night vision at all.
- `!HEV_A1`'s line was picked without checking what it says; it is a placeholder in the same sense the
  wrong model is.

### What to look for
A pickup that reads as an optical device — goggles, a headset, a scope — matching the shape the other
four Modules already have a look for. The voice line wants whatever `!HEV_A1` actually plays checked, and
likely replaced with something that names night vision the way the Infusion's `!HEV_HEAL7` names healing.

### Done when
The pickup on the floor says *this lets me see in the dark* before the player reads a tooltip, and the
voice line says the same thing out loud.

## The Pulse Module — pickup model and voice line

### Scope
`dlls/items.cpp` (`CItemPulseModule`), `models/w_adrenaline.mdl`, the `!HEV_A1` sentence.

### Current stand-ins
| Use | Asset | Borrowed from |
| --- | --- | --- |
| World model | `models/w_adrenaline.mdl` | a stock Half-Life pickup model the game never places |
| Voice line | `!HEV_A1` | the same stock sentence the other Module pickups use |

### What's wrong with them
- A syringe-shaped canister says *medicine*, and the Infusion already owns that shape in this mod; nothing
  about it says *the suit's discharge*.
- `!HEV_A1` is the line every Module pickup plays; none of them names its Module.

### What to look for
A pickup in the family the other Modules will share — the Pulse's own motif is the shield glyph the Status
page and the tree already use — and a voice line that names it, as the Infusion's `!HEV_HEAL7` names
healing.

### Done when
The pickup on the floor says *this is the Pulse* before the tooltip does.

## The alien Module — stand-in models and the summon weapon's borrowed everything

### Scope
`dlls/items.cpp` (`CItemAlienModule`), `dlls/summon.cpp` (`CSummon`, `CCoreAmmo`), `sprites/weapon_summon.txt`.

### Current stand-ins
| Use | Asset | Borrowed from |
| --- | --- | --- |
| `item_alienmodule` world model | `models/w_sqknest.mdl` | the squeak grenade's nest prop |
| ~~`item_core` world model~~ | ~~`models/w_gaussammo.mdl`~~ **replaced 2026-09-19 by `models/w_core.mdl`, below** | ~~the gauss gun's ammo box~~ |
| `weapon_summon` viewmodel, world model, sounds | the hivehand's | the hivehand |
| `weapon_summon`'s HUD/ammo icon | the gauss ammo icon, in `sprites/weapon_summon.txt` | the gauss, standing in for a Core |

### What's wrong with them
- Nothing here says *alien* or *summon* except the effect. A squeak nest is the wrong prop for a hand-over
  from a freed slave, which the Module does not yet dramatise at all — [ROADMAP.md](ROADMAP.md#alien)
  records that the hand-over itself is a stand-in pickup, not the boss fight it should follow.
- The summon weapon plays and sounds exactly like the hivehand, so drawing it reads as drawing the
  hivehand rather than a new weapon with a new verb.
- A Core on the HUD is a gauss ammo icon — borrowed because it exists, not because it says the right
  thing, the same accident the Pulse's first readout icon was.

### The Core's replacement, decided 2026-09-18, built 2026-09-19
Andrei: **a Core is a green battery.** `w_battery` is reskinned from its blue to the vortigaunt beam's
green, and both the battery and the Core give off light in their own colour. The light is a row each in
`ProgressionLight`'s table (`cl_dll/entity.cpp`), which is keyed by model name. So the Core wants its own
`.mdl`, not a skin of `w_battery`, and a green that can be told apart from the Row Grant's (80, 255, 80).
The battery's source is in the SDK at `Weapon Models/world_models/wrld_battery` ([HL_SDK.md](HL_SDK.md)).
The HUD icon follows the model.

**Built:** `models/w_core.mdl` by `E:\CustomAssets\scripts\core_world.py`. The one coloured thing on the
stock battery is the teal charge-window cloud on `batside.bmp`; it takes the islave zap beam's hue and
saturation (180, 255, 96, `dlls/islave.cpp`) at its own luminance, so the cloud's texture survives, and
nothing else changes — the hazard stripes, the orange readout and the grey body say *battery*, the
window says *green*. The SMD and QC are the SDK's, the textures the shipped T file's (the SDK's
`batside.BMP` is 100 wide against the shipped 116). `$externaltextures` dropped, so one file ships.
`item_core` uses it and is precached in `W_Precache`; the light rows are the Core in the beam's
yellow-green and the battery in its window's teal (50, 220, 185), both 8 units up. The sheet in
`E:\CustomAssets\render\w_core\` shows a variant with the body washed 0.15 toward the green, not shipped.
**Not yet verified in game.** The HUD icon is still the gauss's.

### Done when
`item_alienmodule`, `item_core` and `weapon_summon` each have a look of their own, and a Core reads on the
HUD as something other than gauss ammo.

## The Search and the silencer — voices, a shared pickup model, a pickup sound

### Scope
`dlls/hgrunt.cpp` (the three `OnSearch*` lines), `dlls/perception.cpp` and `dlls/squadmonster.cpp` (the
witness and dispatch hooks the other Trained monsters leave silent), `dlls/items.cpp` `CItemSilencer`,
`sound/sentences.txt`.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| The grunt's witness, send and no-sign lines | `HG_WITNESS`, `HG_SEND`, `HG_NOSIGN`, composed from vanilla's `hgrunt/` word files | the vocoder vocabulary; no new audio |
| The assassin, alien grunt and alien slave witnessing or searching | nothing — they go in silence | — |
| The silencer pickup's world model | `models/w_silencer.mdl` | the real thing — but `item_nightvision` stands in on the same model |
| The silencer pickup's sound | `!HEV_A1` | the Night Vision Module's placeholder |

### What's wrong with them
- Two different pickups on one model. The silencer *is* `w_silencer.mdl`, so it is the Night Vision
  Module's stand-in that has to move, not this one.
- Three of the four soldiers that search do it mute, which makes a dispatched alien grunt look like a
  wander rather than a Search. The grunt has a vocabulary; the others have grunts and hisses that could
  carry the same three moments.
- The composed lines are limited to words Valve recorded. "Man down" does not exist; "we got casualties"
  is the nearest.

### Done when
The Night Vision Module has a model of its own, the silencer pickup has a sound of its own, and a
searching alien makes a noise when it is sent and when it gives up.

## Records — the document model, the unread glow, and no sound at all

### Scope
`dlls/record.cpp`, `game_shared/record_defs.h` (`RECORD_PLACEHOLDER_MODEL`, `RECORD_GLOW_SPRITE`), the
`record` entity's `model` keyvalue default in `fgd/halflife.fgd`.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| A loose document's world model | `models/w_security.mdl` | the security card pickup — the flattest thing vanilla ships |
| The unread glow, loose form | `kRenderFxGlowShell`, pale blue, `record_glow_shell` 25 | the buster egon's shell (`dlls/multiplay_gamerules.cpp`) |
| The unread glow, brush form | `sprites/glow01.spr` at `kRenderGlow`, `record_glow_halo` 0.35 | vanilla's general-purpose glow sprite |
| Reading a Record | nothing — it is silent | — |
| A `record_lock` refusing a press | `buttons/button11.wav` | vanilla's access-denied beep |

### What's wrong with them
- **A security card is not a document.** It is a flat rectangle, which is the only property being
  borrowed, and it already means *this opens a door* in Half-Life — the exact wrong reading for a thing
  whose whole job is to be read. It is meant to look wrong.
- **The glow is one colour for every Record**, so a Transmission and a Medical file are indistinguishable
  until they are read. Whether category should carry colour is an open question, not a decision.
- **Nothing makes a sound.** Opening the reader, registering a Record and closing it are all silent, so
  the one moment worth confirming — *this is now in the suit's memory* — is carried by a line of text at
  the foot of the page and nothing else.
- **A refused lock borrows a button sound.** `button11.wav` is the right *idea* — it is what Half-Life
  means by access denied — but it is a button's voice, not the suit's, and the lock is a thing the suit
  is reading rather than a thing the player is pressing. The accepting case has no sound at all, so the
  only audible feedback the mechanic has is its failure.

### What to look for
A document prop that reads as paper or as a slate from across a room: a clipboard, a folder, a loose
sheaf, a handheld terminal. It has to be legible at a glance lying flat on a desk, which is where most of
them will be placed, and it has to look different from every pickup the player can take — a Record is
never taken. The fixed form needs no model, but a terminal's *screen* is brushwork the mapper builds, so
a matching texture set would pay for itself.

### Done when
A Record on a desk reads as something to read before the Prompt says so, and registering one makes a
sound in the suit's voice.
