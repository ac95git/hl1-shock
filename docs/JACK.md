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