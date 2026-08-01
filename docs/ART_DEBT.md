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
