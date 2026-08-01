# Ammo is not in the Inventory

Ammunition keeps Half-Life's own model — a pool per type with a fixed cap — and occupies no Cells, even
though the Inventory Panel still displays it. Putting Ammo in the Grid was tried in the first client-side
prototype and rejected: in Half-Life you collect ammo constantly and mid-firefight by walking over it, so
every pickup would become a placement decision and a single crate could spawn several new Stacks. That
converts combat into inventory Tetris for no design gain, since Ammo already has a working scarcity model
in its per-type cap.

## Consequences

The Inventory Panel needs somewhere to show Ammo that is not the Grid — currently the panel's left column.

Weapons are the only thing competing for Grid space against items, which is what makes "which guns do I
carry?" the interesting question rather than "where do I put these 40 rounds?".
