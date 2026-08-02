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

## The Health Syringe — icon, world model and sounds

### Scope
`game_shared/inventory_defs.h` (the `k_ItemTypes` row), `dlls/items.cpp` (`CItemSyringe`), and the
`k_Infusion*` constants in `dlls/player_infusion.cpp`.

### Current stand-ins

| Use | Asset | Borrowed from |
| --- | --- | --- |
| Grid icon **and** Infusion status icon | `cross` | the health readout's cross |
| World model | `models/w_adrenaline.mdl` | `valve/models`, by game-directory fallback |
| Picked up | `items/smallmedkit1.wav` | medkit |
| Infusion starts | `items/medshot4.wav` | health charger |
| Use refused while infusing | `items/medshotno1.wav` | health charger, denied |
| Voice line | `!HEV_HEAL7` — "hiss, morphine_shot" | stock HEV sentence |

The use sounds and the voice line are close enough to right that they may simply stay. The **pickup**
sound is the medkit's outright, which is the odd one out: the Syringe deliberately does not sound like a
medkit when it is *used*, so sounding exactly like one when it is *taken* undoes half of that. The sprite
and the model are the rest of the debt.

### What's wrong with them
- **`cross` is the health readout's own icon.** Used in the Grid it says "health", not "syringe", and it
  will read as a second medkit to a player scanning the Grid quickly. It is deliberately the same sprite
  in both places so the item and its effect are visibly linked — a replacement should keep that property
  and supply *one* new sprite used twice, not two.
- The status icon is drawn additively and tinted green, so a greyscale source is required; a coloured
  sprite will come out wrong.
- **`w_adrenaline.mdl` is not used anywhere in Half-Life** — it ships in `valve/models` unreferenced,
  which is why it was free to take. It is small, so it may sit oddly inside the standard
  `size(-16 -16 0, 16 16 36)` pickup bounds, and nothing about it says *this heals you over time*.

### What to look for
A syringe silhouette that reads at 20×20 (the 320-res sprite size) as well as at 132×132, distinct at a
glance from the medkit's box. The world model wants to be recognisable on a floor from standing height.

### Done when
Nobody confuses a Health Syringe with a Medkit in the Grid, and the status icon is legibly a syringe
rather than a cross.

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
