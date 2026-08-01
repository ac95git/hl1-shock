# The Grid has a fixed width and grows only in Rows

The Grid's width is a constant (12 Cells) and an Inventory grows by gaining Rows. Growing in both
dimensions was considered and rejected: VGUI1 layout is awkward enough that a Grid changing shape in two
directions is a meaningful rendering burden for a reward the player can get from Rows alone.

Two properties fall out of the fixed width and are worth keeping deliberately:

- Width is a multiple of a weapon's 3 Cells, so no Cell is ever permanently stranded at the end of a Row.
  The previous 11-wide Grid stranded 2 Cells per Row and was the source of much of its placement weirdness.
- A Cell's linear index (`row * width + col`) keeps its meaning forever. Had width been variable, every
  stored Cell index would silently change meaning when it changed, scrambling saved layouts with no error.

## Consequences

The width constant is effectively frozen once saves exist. The save block records the width it was written
with, so a future change can be detected and re-packed rather than silently misread.

All Rows up to the maximum are drawn at all times, with un-granted ones greyed out, so Cell size never
changes and the player can see the space they have yet to earn.
