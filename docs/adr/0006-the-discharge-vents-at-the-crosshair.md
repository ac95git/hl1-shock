# The Discharge vents at the crosshair, and is not a reflection

A Discharge traces from the player's gun position along their view vector — not back toward whatever dealt
the damage. "Reflect" is on the _Avoid_ list in [CONTEXT.md](../../CONTEXT.md) for this reason, because the
word describes something the mechanic cannot do.

**Half-Life has nothing to reflect.** `FireBullets` / `FireBulletsPlayer` (`dlls/combat.cpp:1397`, `:1532`)
resolve a `UTIL_TraceLine` instantly, so a fired bullet never exists as an entity. Both enemies this
mechanic was designed against are hitscan: the HECU grunt's MP5 and shotgun (`dlls/hgrunt.cpp:799`, `:826`),
and the alien slave's zap, which draws a `sprites/lgtning.spr` beam along a traceline *after* the damage has
already been applied (`dlls/islave.cpp:801-834`). The slave's attack looks like a projectile and is not one.

Real projectiles do exist — `CGrenade`, `CRpgRocket`, `CHornet`, `CSquidSpit` — and could be physically
turned around. But that needs a per-frame entity sweep during the Pulse Window, and it would only ever fire
against the minority of attacks that use them, which excludes both enemies above.

So a Discharge has to be a *new* attack rather than a redirected one. Given that, it is aimed at the
crosshair so the player chooses the target: absorbing a grunt's burst and putting the energy into a
different grunt is a decision, where automatic retaliation is not.

## Considered options

**Back at the attacker** — the most literal reading of "reflect". Rejected as the base behaviour because it
needs two guards that the crosshair version needs neither of: a validity check for attackers that are not
creatures (a falling crate, the world itself), and a line-of-sight check, without which blast damage from a
grenade thrown from cover would be answered through the wall.

**Mirrored along the incoming vector** — physically honest, but `DMG_BLAST` has no meaningful direction, and
a shot taken at an angle would send the Discharge into a wall.

## Consequences

Because the hit and the Discharge resolve in the same frame, the player is usually already looking at the
threat — so in the common case this behaves like retaliation anyway. The difference only shows when they are
mid-turn, or deliberately pointing elsewhere.

A Discharge hits whatever its trace reaches, friendly NPCs included, exactly like any weapon.

Damage is `clamp(absorbed × scale, min, max)`. The cap is load-bearing: without it, timing a Pulse against
the hardest-hitting attacks in the game yields the strongest counter, which inverts the difficulty curve
precisely where it should be steepest.

A Discharge deals `DMG_ENERGYBEAM`, not `DMG_SHOCK`. It was `DMG_SHOCK` first, which read well against a
lightning beam until it turned out that the alien slave is the *only* entity in Half-Life immune to that
type (`dlls/islave.cpp:585`) — and slaves are among the enemies the Pulse is aimed at, since they carry
both a telegraphed melee and a hitscan attack. A counter that one headline target alone ignores is the
wrong counter. `DMG_ENERGYBEAM` has no immunity anywhere, is what the Egon fires (`dlls/egon.cpp:271`),
and describes vented energy at least as well.

The Discharge also fires on melee deflects, not only ranged ones, which falls out of "one per negated hit"
rather than having been designed. It is kept deliberately.
