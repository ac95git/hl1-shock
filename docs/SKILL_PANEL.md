# The Skill panel — the suit's circuit on the suit's screen

The look and behaviour of the Skill Tree's page in the Inventory Panel, settled in a grill on 2026-09-15
after [SKILL_TREE.md](SKILL_TREE.md) settled the tree's shape. This is the presentation only; the rules
are in SKILL_TREE.md, the art is briefed in [ART_DEBT.md](ART_DEBT.md#the-skill-tree--the-circuit-substrate-traces-and-frames),
and the code is `cl_dll/vgui_skilltree.cpp` and `cl_dll/vgui_inventory.cpp`. **Built 2026-09-15, the
same day it was settled**, with rects and the HUD's own sprites standing in for every asset the art
brief names: the substrate is a drawn grid until a tile exists, the frames are rect chips with drawn
pins until frame sprites exist, the icons are the stock HUD placeholders. Each row below says what stands
in. The Skill Points counter shows two digits, 0–99.

## The theme in one paragraph

The Inventory Panel is the suit's own screen and tints with the suit, as it already does. The Skill page
shows **the suit's circuit**: a board of copper under the nodes, every Skill a chip on it, every road a run
of pads, every connector a trace, and the suit itself the processor at the centre. The chrome — header,
tabs, the gauge strip, the Reset switch — follows the suit colour like the rest of the panel; the board is
the one place the suit colour steps back, because the copper and the regions have colours of their own.
Nothing is printed on a node.

## The panel

| Element | Settled |
| --- | --- |
| **Header** | A suit designation, suit-coloured, the same on both tabs: `HEV MK IV  //  STRENGTH`, the codename following the Suit Variant worn (Agility, Strength, Intelligence — their first job) |
| **Tabs** | `Inventory` and `Upgrades`, unchanged. "Upgrades" stays by decision, although the glossary avoids the word elsewhere |
| **Gauge strip** | Across the top of the field, fixed while the board pans under it: `SKILL POINTS` in the small font, the count in a boxed suit-coloured two-digit segment display in the HUD's own digit style, then `RESET TOKENS` with one lit pip per token and dim pips for none. The tokens never need a number |
| **Reset switch** | Top-right of the field, fixed: a boxed control with a red-and-black hazard-striped frame, label `RESET TREE`. First click arms it (the frame blinks, the label becomes `CONFIRM  -1 TOKEN`), second click fires, any other click disarms. Dimmed and unstriped with no token. The rising and falling sounds stay |
| **Close** | Unchanged |

## The board

| Element | Settled |
| --- | --- |
| **View** | **1:1 always, drag to pan.** The board is drawn at its designed size on every screen, never fitted (1680 px square at the 112 step, so every screen pans). It opens centred on the suit; a drag anywhere on the field pans it; a press that moves under 4 px is a click; the view clamps to the board's edges plus a margin; the last position is kept while the panel is closed and reopened on the same map. `skilltree_preview_cols/rows` keep drawing ghost cells for layout work |
| **Substrate** | A tiled circuit-board texture under the whole field, low contrast, a TGA loaded through VGUI1 |
| **Regions** | A faint colour wash per region, grounded in the game's own objects: Melee **rust orange** (the crowbar), Weapon Specialist **steel blue-grey**, Medical **white-green** (the Syringe), Juggernaut **HEV amber**, Energy **electric cyan** (the gauss and egon), Alien **Xen green**, Shinobi **violet**, Stealth **deep indigo**, the hub **bare copper**. Faint, so the suit's cyan / red / purple chrome never fights them. No region labels anywhere |
| **Traces** | Copper, one between every pair of orthogonally adjacent nodes and nowhere else. Dim unlit; lit when the node at either end is held; glowing when it leads from a held node to one that can be bought |
| **Frames** | One frame sprite per tier, tinted by state (white held, gold available, grey locked): a square **pad** for a Stat node, a small **chip** for Minor, a larger for Medium, a large chip with pins for a Major, the **processor** for the suit. The keystone is a Major-sized frame in **red**. Major-sized: the eight regional Majors, and Weapon Mastery, Pulse Discharge, Pulse Rebound and Follow-Up, kept large by decision |
| **Sizes** | A **112 px step**; nodes **36 / 50 / 62 / 74**, the processor **92**. The board is 1680 px square ([SKILL_MAP.md](SKILL_MAP.md#sizes)) |
| **Icons** | **Monochrome glyphs in the HUD sprite style**, white-on-black additive, tinted by state with the frame. One motif per Route — blade, crosshair, cross, shield, bolt, hive, wind, eye — with the Skill's own detail inside it; a Stat node's glyph is its stat's, shared along the road. Made with the sprtool scripts |
| **Hidden pads** | A gated region draws as blank pads on the substrate: a footprint, not a hole. A hidden pad cannot be bought, and hovering it shows a blank tooltip reading **`No signal`** |
| **Tooltip** | **Name and effect only.** No Route line, no totals, no requirements (there are none), no cost. The region is read from its colour and its road's glyph |
| **Sound** | **One unlock sound for every node**, whatever its tier; hover and pan silent. `common/wpn_select.wav` stands in |

## Rejected in the grill

Renaming the tab to *Skills* or *Suit*; a header without the codename; a monochrome board with no washes;
a suit-tinted board; full-colour icons; the readout in the header or on the processor; the Reset behind
the suit node; hold-to-confirm; fitting the whole board to the field; mouse-wheel zoom; a Route line,
stat totals or a points-away line in the tooltip; naming the Module on a hidden pad; per-tier unlock
sounds; a hover tick.

## What it costs to build

- The pan: a drag state on the field with the 4 px click threshold, a clamped offset, kept per map; the
  gauge strip and the switch drawn after the board, in field coordinates.
- The header string from the Suit Variant, refreshed each paint as the label colour already is.
- The gauge strip's segment digits: the HUD's number sprites drawn through the fitted path, or a small
  digit sprite of their own.
- The substrate loader (`BitmapTGA`), the region washes as filled rects behind the traces, the traces as
  filled rects between neighbours (the dogleg connector code goes with the gates).
- Frame sprites and glyph icons: art, in ART_DEBT.
