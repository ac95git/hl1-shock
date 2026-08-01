# Weapons keep Half-Life's identity; items get their own

An Entry in the Inventory names either a Half-Life weapon (by its existing `WeaponId`) or an Item Type (by
an id from our own table). We deliberately did *not* fold weapons into the new Item Type table, even though
one identity space would be tidier. Half-Life's weapons are entities with `ItemInfoArray`, `WeaponId`, the
`m_rgpPlayerItems` linked lists, and client-side `WEAPON` structs all keyed together; re-identifying them
touches far more of the SDK than this project wants to disturb.

## Consequences

Anything walking the Inventory must discriminate on whether an Entry is a weapon or an Item Type. That
discrimination is the permanent cost of the decision and should be made explicit in the Entry type rather
than inferred.

Item Types cover non-weapon things only. A "weapon" that behaves like an item (a thrown rock, a tool) has
to pick a side.
