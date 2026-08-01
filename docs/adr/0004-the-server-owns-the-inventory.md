# The server owns the Inventory, including where things sit

The server holds both the contents of an Inventory and each Entry's Cell position; the client renders what
it is told and sends requests. This is not a preference — the GoldSrc client DLL has no save/restore hook
at all, and `cl_dll/hud.h:77` documents `InitHUDData` as running "every time a server is connected to",
which includes loading a save. Any layout the client owns is therefore wiped on load and cannot persist.

Having the server merely *store* an opaque client-owned layout was considered and rejected. Weapons are
picked up by touching them, on the server, with no client in the loop — so the server must already own the
occupancy model, the placement solver, and the Grid dimensions in order to answer "is there room?". Once it
has those, a client-side solver is redundant and gives two sources of truth that can disagree.

## Consequences

Moving an Entry costs a round trip: it does not move until the server confirms. Imperceptible in
single-player. If it ever matters, the client can move optimistically and snap back on rejection.

Client-side layout normalisation — the heuristic that re-packed the whole Grid on every paint and produced
non-deterministic neighbour movement — stops existing rather than being fixed. Nothing re-arranges the Grid
behind the player's back.
