# Selecting faces instead of the whole brush

Faces are only selectable in Texture Application / Surface Properties mode — Shift+A. With the dialog open, left-click a face in the 3D view to select it; hold Ctrl while clicking to add more faces. You can also select whole brushes first, then open the tool, and all their faces come in.

Trap the manual calls out (p. 22): outside that mode, with the ordinary Apply Current Texture tool (Shift+T), a brush with one face selected gets the texture applied to every face. Face selection doesn't exist in the Selection tool — only inside Surface Properties.

# Adding vertices

Shift+V is Vertex Manipulation mode. Press it repeatedly (or Shift+V again) to cycle three sub-modes: vertices+edges (default), vertices only, edges only. White dots are vertices, yellow dots are edges. Hold Alt while clicking if you don't want to deselect the brush by accident.

But VM only moves what's already there. To create new vertices you use Face Splitting: in VM mode select two opposing edges and press Ctrl+F (or right-click → Split Face). The two edges become vertices and a new edge is placed between them — that's how you add corners to work with.

# Two things worth knowing before you start dragging:

- There is no "vertex inside a brush". A GoldSrc brush is a convex solid defined by its planes; every vertex is a corner of the hull. Concave shapes are invalid. If you want detail inside a volume, that's a second brush, not an extra vertex.
- VM makes invalid solids very easy to create. The manual recommends enabling solid validity restrictions mode if you're unsure — it blocks invalid drags, at the cost of vertices being harder to place. And the non-planar faces VM produces are fixed with the Triangulate command in VM's right-click menu, run once when you're done.
- Undo doesn't work inside VM — exit the mode first, then undo.

# Putting a door (or any opening) in a wall that already exists

Don't delete the wall and rebuild it, and don't Carve. Carve re-cuts the whole brush into wedges around
the cutter and is how a clean wall turns into a fan of slivers — PROVING_MAP.md's rules already say never
Carve. **Split the wall with the Clipping tool and delete the piece where the hole goes.** A clip only
ever cuts along the one plane you drew, so everything else about the brush survives untouched.

A 64 × 96 door (the standard from PROVING_MAP.md) in a wall running east–west. Work in the 2D view that
shows the wall **face-on**, so the clip line is a vertical plane cutting through the wall's depth:

1. Grid to 16 (`[` / `]`). Every doorway edge lands on the grid.
2. Select the wall. `Shift+X` for the Clipping tool, then `Shift+X` again to cycle the mode until it is
   **keep both sides** (the third of the three; the 2D view shows which part survives as you cycle).
3. Vertical line at the doorway's **left** edge → `Enter`. Two brushes now, geometry unchanged.
4. Select only the right-hand brush, vertical line at the **right** edge → `Enter`. Three: left jamb,
   middle column, right jamb.
5. Select the middle column, **horizontal** line 96 above the floor → `Enter`. Above is the lintel, below
   is a door-shaped slab.
6. Delete the slab. `Shift+S` back to the Selection tool.

For a bare axis-aligned slab there's a lazier version that's just as correct: resize the wall with its 2D
handles until it *is* the left jamb, then Shift-drag it twice to clone the right jamb and the lintel.
Clip when the wall already carries detail; clone when it doesn't — cloning can't leave an off-grid vertex
behind.

Then the door itself: a new brush filling the opening, same depth as the wall or a touch less so it
doesn't z-fight the jambs, `Ctrl+T` → `func_door`. In `Alt+Enter`: **angle** is the direction it moves,
**lip** how much stays poking out at the end (8–16, so it doesn't vanish), speed ~100, delay before close
4, `wait -1` to leave it open for good. A sliding door needs somewhere to slide into — either a recess in
the wall or the acceptance that it buries itself in the jamb. A swinging `func_door_rotating` needs its
origin brush on the hinge edge.

- The faces the clip creates (jamb sides, lintel underside) inherit the wall's texture and land
  misaligned. Fit or align them in Surface Properties afterwards.
- Any face left permanently buried after the cut gets `NULL`.
- Same method for a window, a vent or a hatch — it is only the number of clips that changes.