# Pickups are walk-over

Every pickup is taken by walking over it, as in Half-Life. A medkit or battery is used on the spot when
that wastes none of it and carried otherwise; antidote, keycard and syringe are carried on contact. The
one exception is a suit offered to a player already wearing one, which waits for a use press.

This reverses a deliberate rule from 2026-08-01, which made every Inventory item use-only: walk over a
battery and nothing happened, and the Pickup Prompt under the crosshair told you a use press would take
it. The rule was written down with its reason — "so the player is never surprised by what they are
carrying" — and it was reversed on 2026-09-13 after a playtest, which is why this record exists.

## What the playtest found

Two things, and they compound. Half-Life players did not know a use press was expected: walking over a
battery and leaving it on the floor read as a bug, not as a rule. And the ones who did know found
stopping to look down and press use on every consumable to be friction that broke the game's pace.

Better communication — a styled prompt, a tutorial line — would mitigate the first and not the second.
The deciding principle was that this is a Half-Life mod that leans on Half-Life's conventions, and
"touching a thing takes it" is one a player has fifteen hours of training in before they reach anything
this mod adds. A rule that only an unstyled line of console text teaches is a rule that will not be
learned.

## Considered options

**Vanilla outright: consumed on contact, always.** Battery and medkit stop being Item Types. Rejected
because it throws away the Inventory's main tenants — carrying a consumable for later is most of what the
Grid is for.

**Always carried, never consumed on contact.** Rejected because a wounded player at 10 health would have to
open a panel to heal, which is worse than vanilla.

**Keep the medkit's rule as it was: Auto-Consume when nothing is wasted, otherwise leave it standing with
the prompt.** This was the design that failed the playtest.

**Chosen: Auto-Consume when nothing is wasted, otherwise carried.** Touching a thing always does something.
At low armour a battery behaves exactly as vanilla, so the trained reflex is rewarded; at full armour the
player keeps it instead of wasting it. The test is inclusive — a charge that lands exactly on the ceiling
is a perfect fit — and it runs before the Grid is consulted, so a wounded player with a full Grid is still
healed.

## The details that follow

**A full Grid leaves the item standing, silently.** Vanilla precedent: an ammo box at a full pool is left
in place with no message, and players already read "I touched it and it stayed" as "I'm full". A message
on every touch would fire on every brush past a crowded shelf; a use press on the item prints "No room in
inventory" for anyone who wants the explanation. Consuming it anyway was rejected because it destroys
something the player might have wanted to make room for, which the Inventory refuses to do anywhere else.

**The suit is vanilla wherever vanilla has an answer.** A suitless player walks into the locker at
Anomalous Materials. Vanilla has no behaviour for a second suit because it refuses one, so switching
variant is the mod's own invention, and it is the one pickup where a brush past does irreversible harm:
the old variant does not drop. So switching stays use-only, and the prompt names the variant first.
`CItemSuit::AutoPickupOnTouch` returns true only while the player has no suit.

**The Pickup Prompt stays for everything takeable.** It always showed for walk-over weapons, which proves
the pattern is harmless. Its two remaining jobs are lifting a thing off a shelf the player cannot step
onto, and carrying the full-Grid explanation. Showing it only where a use press is the sole route was
rejected because the prompt would then appear and vanish by state the player cannot see — the kind of
rule the playtest just said they will not learn.

**A carried pickup is announced.** The old rule's promise — never surprised by what you carry — is kept
by the pickup history instead: an item that went into the Grid flashes its icon at the bottom right with
the `inv_carried` arrow badge beside it, and one used on the spot flashes the plain icon, as vanilla.
(The badge began on top of the icon, at 58% of its square; it was intrusive in play, and moved to the
icon's left at 40%.) That
column is where a Half-Life player already looks to learn what they just picked up, and the only new
information is "this one was kept". A pop near the crosshair was rejected because consumables are most
often grabbed mid-fight, and that is where the player is aiming. The history draws the suit-tinted HUD
sprite rather than the Grid's full-colour Icon, because the history is an additive tinted column and the
arrow already carries the one bit that matters. The keycard got a HUD sprite for this, since it had none
and would otherwise have arrived silently; the antidote still has none and does, which is acceptable for
a placeholder and is recorded in ART_DEBT.md.

## Consequences

- `AutoPickupOnTouch` takes the player and defaults to true. The impulse 101 exemption remains, because
  the cheat hands its suit through the same touch and a suited player would otherwise find it at their
  feet.
- `gmsgItemPickup` carries a byte after the classname, 1 for carried. `CItem::AnnouncePickup` is the only
  sender, so the client reads the byte unconditionally.
- The battery's charging moved into `ApplyBatteryCharge` / `BatteryChargeRoom` in
  `dlls/player_inventory.cpp`, shared by the Grid's Use verb and the pickup's Auto-Consume, so the two
  cannot drift.
- `instructions/04-CUSTOM-FEATURES.md` no longer tells a new Item Type to override `AutoPickupOnTouch`.
- **Dropping needed a guard.** The first build took a dropped item straight back on the next frame,
  because the drop spawns it inside the dropper's box and there was no longer a use press between the
  two. `CItem::DisarmUntilClear` / `ArmWhenClear` now hold the touch off until the item has landed and
  nobody is standing in it, which is what `CBasePlayerItem::FallInit` already did for weapons.
- Backtracking is slightly cheaper to design for: things left behind are now only things the Grid
  refused, or things the player dropped.
