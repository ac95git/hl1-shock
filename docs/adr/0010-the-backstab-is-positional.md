# The Backstab is positional only

A Backstab is a melee hit landed in a monster's rear arc. Whether the monster has noticed the player does
not enter into it, and there is one tier — a single multiplier, applied whenever the geometry is right.

So it lands mid-fight on anything you can circle behind, and a stealth approach and a flank produce
identical damage.

This is deliberate and it contradicts both the genre and this project's own earlier proposal, which is why
it is written down. ROADMAP.md previously defined a Backstab as *"a melee hit on a monster that has not
acquired the player, from behind"* — awareness was removed from the definition when the design was settled
on 2026-08-31.

## Considered options

**Unaware and from behind**, the original proposal. It keeps the Backstab firmly a stealth payoff and gives
an alerted monster a way to defend itself: turn around. Rejected because it makes the mechanic invisible
outside stealth, and because it welds a cheap, fun melee verb to an unbuilt perception system — under this
rule nothing about the Backstab could be judged in play until Suspicion existed.

**Unaware only, at any angle.** Rejected because the word stops describing the act, and because walking up
to a grunt's face to execute him is not what anyone means by a backstab.

**Two tiers — positional always, larger when unaware.** The strongest option on paper: it preserves the
positional rule while giving stealth a payoff open combat cannot reach. Rejected for this version to keep
one number to tune rather than two, and because the gap between the tiers is exactly the kind of value that
cannot be guessed without play data. It remains the obvious lever if stealth proves unrewarding, and it
costs one extra branch on a test that is already being made — see ROADMAP.md pillar 6, *Deliberately
deferred*.

## The exclusion list is curated, not derived

Some monsters cannot be backstabbed at all: headcrab (and babycrab, by inheritance), snark, roach, rat,
leech, hornet, flyer, barnacle, tentacle, tentacle maw, controller, the turret family, apache, osprey,
nihilanth and big momma.

A rule was considered and rejected on the same grounds [ADR-0005](0005-the-shield-negates-a-curated-damage-list.md)
rejected one: every clever rule admits something wrong. The obvious rule here — exclude `CLASS_MACHINE` and
anything with `m_flFieldOfView <= 0` — catches the Gargantua (`-0.2`) and misses the headcrab (`0.5`), which
is precisely backwards.

Three reasons appear in the list, and they are not interchangeable:

- **No coherent facing** — too small, too fast, or spinning. Headcrab, snark, roach, rat, leech, hornet,
  flyer.
- **No back** — ceiling-mounted, rooted, or scenery. Barnacle, tentacle, tentacle maw.
- **"Behind" names nothing** — `VIEW_FIELD_FULL` or wider. Controller, turrets, nihilanth, apache, osprey.

**Big momma is excluded for a fourth reason, and it is a fact rather than a judgement.** `CBigMomma::TakeDamage`
clamps `pev->health = flDamage + 1` until her node path finishes (`dlls/bigmomma.cpp:588-596`), so she is
unkillable by construction and any multiplier applied to a hit on her is eaten before it means anything.
Excluding her prevents a mechanic that silently does nothing, not a boss fight that would be too easy.

**"Boss" is explicitly not a reason, and the Gargantua proves it.** Powerscaling is an intended property of
this mod: with the right build and the right weapon, a Gargantua should fall to one backstab in the endgame.
It is not excluded.

That said, a crowbar Backstab on a Gargantua deals **exactly zero** today, and `dlls/gargantua.cpp` is
deliberately not modified. `GARG_DAMAGE` is `DMG_ENERGYBEAM | DMG_CRUSH | DMG_MORTAR | DMG_BLAST`
(`dlls/gargantua.cpp:47`); the crowbar is `DMG_CLUB`, so `CGargantua::TraceAttack` zeroes the damage and
plays a ricochet (`:830-851`) and `TakeDamage` would multiply by 0.01 on top (`:858-871`). The Gauss Katana,
already proposed as `DMG_ENERGYBEAM` in ROADMAP.md, passes that filter. **The endgame melee weapon is what
makes a Gargantua stabbable, not a special case in the Backstab.**

## Why a virtual and not a flag

`CanBackstab()` is a virtual on `CBaseMonster` returning `true`, overridden to `false` in the excluded
classes. A `bool` member set in `Spawn()` was the obvious shape and is wrong twice over.

It would not survive a save. On restore, `Spawn()` runs only for entities with `FCAP_MUST_SPAWN`; everything
else gets `Restore()` and `Precache()` and nothing more (`dlls/cbase.cpp:380-389`). A flag set in `Spawn`
would come back default-constructed after every load, and fixing that means adding it to the save table —
paying save-game bytes, forever, for a value that is a property of the monster's *type* and can never
differ between two instances of it. This is exactly why Valve saves `m_flFieldOfView` despite every monster
setting it in `Spawn`.

And the exclusion list is per class, so a virtual puts the answer where the fact lives. `CBabyCrab` inherits
the headcrab's, and `CTurret`, `CMiniTurret` and `CSentry` all inherit `CBaseTurret`'s, at no cost.

**The Perception Profile should take the same shape** when it is built, for the same reason.

## Consequences

**Circle-strafing into the rear arc will trivially Backstab slow enemies** — zombies and headcrabs turn
slowly, and they are exactly the monsters the Follow-Up is already tuned against. If melee becomes a matter
of walking in circles, the rear arc narrows (`backstab_arc_dot`) before the multiplier drops.

**Pillar 6 loses its damage-model reward.** Nothing in this rule distinguishes a stealth kill from a flank,
so a stealth player's advantage has to be carried entirely by not fighting and by silent leader kills
dissolving squads. PILLARS.md records that as a known weakness of the pillar rather than of this decision.

**A new monster is backstabbable by default.** The exclusion list is opt-out, so a monster added without
thought gets it, which is the right default for anything humanoid and the wrong one for anything that is
scenery.

**Melee weapons inherit it without a list.** The test lives where crowbar damage is computed, so the Carbon
Pickaxe and the Gauss Katana get it by being melee weapons rather than by being added anywhere.

`FInRearArc` deliberately does not call `UTIL_MakeVectors`, unlike its sibling `FInViewCone`. It is called
partway through `CCrowbar::Swing`, which still needs `gpGlobals->v_forward` afterwards for `TraceAttack` and
the Follow-Up knockback. Computing forward from yaw directly avoids a very quiet bug, and monsters are
upright so the two agree in every case that occurs.
