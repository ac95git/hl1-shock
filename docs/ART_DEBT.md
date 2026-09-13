# Art Debt Register

Placeholder assets currently standing in for art this mod does not have yet. Everything here **works** —
it is borrowed stock Half-Life content chosen because it was available, not because it is right. Each
entry names what is wrong with the stand-in, so the replacement is judged against something.

Distinct from [TECH_DEBT.md](TECH_DEBT.md): that register is about code that needs fixing. This one is
about assets that need making.

## The Pulse — Shield sprite

### Scope
`dlls/player_pulse.cpp`, `DrawShieldRing` / `DrawShieldEffect`.

### Current stand-in
`sprites/shockwave.spr`, drawn as two nested rings plus a `TE_DLIGHT` flash. The sprite is the
**houndeye's sonic blast** texture, borrowed from `dlls/houndeye.cpp:576-616` because it was the only
ring-shaped thing already in the game.

### What's wrong with it
- It reads as a houndeye attack, because it *is* one. A player who has fought houndeyes will recognise it.
- It is a soft, diffuse shockwave. The Shield is hard protective equipment — it wants a crisp edge, an
  energy-field look, something that suggests a surface rather than a pressure wave.
- The colour is applied as a tint over a greyscale sprite, so the cyan is muddy rather than luminous.

### What to look for
A ring/energy-field texture that suits a **suit-projected barrier**: crisp bright edge, ideally with some
internal structure (hex, scanline, interference pattern) so it doesn't read as a plain circle. It is drawn
by `TE_BEAMTORUS` or `TE_BEAMCYLINDER`, so it is used as a *beam texture* tiled along the ring — a tall
thin sprite, not a circular one.

Candidates already in `valve/sprites` worth trying before authoring anything: `plasma.spr`,
`xenobeam.spr`, `zbeam1-6.spr`, and `gwave1.spr` (a large wave sprite the SDK never references).

`pulse_ring_style` and `pulse_ring_scale` exist to judge geometry by eye; swapping the sprite still needs
a code change, so consider a cvar for the sprite name if this turns into much iteration.

### Done when
The effect is recognisably the mod's own, and nobody mistakes it for a houndeye.

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
| Recharge complete | `items/suitchargeok1.wav` | wall charger |
| Pressed while Recharging | `items/suitchargeno1.wav` | wall charger, denied |

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

## Progression pickups — world models and pickup sound

### Scope
`dlls/items.cpp`, `CItemSkillPoint`, `CItemResetToken` and `CItemRowGrant`, plus the shared
`SetProgressionLook` helper above them.

### Current stand-ins

| Pickup | Model | Scale | Glow shell |
| --- | --- | --- | --- |
| Skill Point | `models/crystal.mdl` — a Xen crystal formation | 0.25 | cyan |
| Reset Token | `models/sphere.mdl` — a small unused orb | 1.5 | gold |
| Row Grant | `models/w_isotopebox.mdl` — a shipping case with a handle | 1.0 | green |

All three also use `items/gunpickup2.wav` and a centre-print line.

All three models live in `valve/models` and reach the mod by game-directory fallback, the same route
`w_adrenaline.mdl` takes for the Syringe. None is referenced anywhere else in this codebase, which is
why they were free to take.

**Revised 2026-08-31.** The previous stand-ins — `w_longjump.mdl` for the Skill Point and
`w_security.mdl` for the Reset Token — were *actively misleading* rather than merely unevocative, and
that is now fixed: the longjump module is a real pickup the player can also find (and Modules will add
more), and the keycard is a door key. Neither collision remains.

### What's wrong with them

- **The glow shell is carrying the identification, not the models.** `kRenderFxGlowShell` is what says
  "progression, not equipment" and what ranks the three against each other by colour. That was a
  deliberate stopgap: it works at a distance and in the dark, and it makes the family legible before the
  art exists. It is not a substitute for three distinct silhouettes.
- **`crystal.mdl` is Xen's.** A Skill Point found in a Black Mesa office is not a Xen crystal, and the
  model will read as scenery once there are actual Xen levels using it as scenery.
- **`sphere.mdl` is a featureless ball.** It says "special" and nothing further. It ranks correctly
  against the crystal and communicates nothing on its own.
- **`w_isotopebox.mdl` suggests hazard, not capacity.** It is a radioactive-materials case. The handle
  and the box shape are the right idea; the contents label is wrong.
- **The pickup sound is unchanged and is now the weakest part of this entry.** One generic
  weapon-pickup click for all three. This is the moment exploration pays out, and it sounds like picking
  up ammo. The three should not share a sound at all — a Reset Token is rare and should announce itself.

### What to look for

Three clearly *different* small pickups that read as progression rather than equipment, and that rank
against each other at a glance — the Token should look rarer than the Point, and the Row Grant should
read as *storage*. None should resemble anything in the HEV/keycard vocabulary.

Two constraints the current set established and a replacement should respect:

- **Distinct shape classes, not just distinct colours.** Faceted shard / smooth orb / handled box is the
  right kind of separation, because it survives being seen in silhouette or in the dark.
- **The Row Grant must not look like a Box.** Lootable Boxes are future work and will use
  `w_weaponbox.mdl`; two things that grow what the player can carry must not look identical. This is why
  the better metaphor was deliberately passed over.

Whether the glow shell stays once the models are distinct is an open decision — as a permanent family
marker it is defensible, but it should then be a choice rather than a leftover.

### Done when
A player who has never read a manual can tell all three apart on sight, none is mistaken for equipment,
and the Reset Token does not sound like ammo.

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
Stock HUD sprites, heavily duplicated: `suit_full` on five Skills, `d_crowbar` on three, `cross` on
three. Six Skills are currently drawn with the same image as at least two others.

### What's wrong with it
This is the one entry here that is **blocking rather than cosmetic**. The tree is deliberately going
label-free — no node names, no column headers — so that reading it means hovering and discovering
(see PILLARS pillar 4, and the deferred anonymization feature). That design assumes each node's icon
identifies it. It does not: today the icon narrows a node down to "something to do with the suit".

Until the icons are distinct, either the tree keeps text labels or it is unreadable. The design decision
and the art are the same decision.

### What to look for
One distinct icon per Skill, grouped so a branch reads as a branch — a shared motif or palette per column,
with the individual Skill distinguishable inside it.

### The size constraint, which is not obvious
**`SPR_DrawAdditive` draws at native size, and that is what the tree uses.** So today a node cannot shrink
an icon to fit; the node is sized *from* the icon (`RebuildNodeMetrics`), and the whole tree is then scaled
to the panel. *(This entry used to say there was no scaled sprite draw in the HUD API at all. There is —
`SPR_DrawGeneric`, and the Inventory Grid fits its tiles through it since 2026-09-12; see
`docs/SPRITE_WORKFLOW.md` for its two traps. The tree has not been moved onto it. Until it is, everything
below still holds; once it is, the size points below become preferences rather than limits.)*

HUD sprites make this worse by being **resolution-bucketed**: `hud.txt` defines each sprite at 320/640/
1280/2560, and the engine picks the bucket for the current screen. `item_healthkit` is 44px at 640 and
88px at 1280. The `dmg_*` family is **128×128** at 1280 — over twice a node — which is why none of them
are used despite being the best semantic fits (`dmg_shock` for the Discharge, `dmg_chem` for Med Expert).

Consequences for the replacement art:

- **One fixed size, not a bucketed set.** A single size at every resolution makes node geometry stable.
- **Small.** Seven columns must fit `panelW - 264`. Anything over ~64px forces nodes so large the tree
  stops fitting on a 1280-wide screen.
- Below 1024×768 the icons are dropped rather than spilled — seven columns of icons cannot fit the
  ~340px tree area at 640×480 at any node size. That is a limit of the layout, not of the art.

### Done when
Every Skill in the tree has its own icon, a player can tell two Skills apart without hovering either, and
the icons are one fixed size small enough that the tree fits a 1280-wide screen without scaling.
