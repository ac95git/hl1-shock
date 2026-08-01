# Technical Debt Register

## VGUI Draw Order: All Sprites, Then All Text

### Scope
Every VGUI panel that draws both sprites and text in one pass — currently
[cl_dll/vgui_inventory.cpp](../cl_dll/vgui_inventory.cpp),
[cl_dll/vgui_inventory_grid.cpp](../cl_dll/vgui_inventory_grid.cpp),
[cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp).

### The rule
**Draw every sprite first, then every piece of text.** Text drawn before a later
`SPR_DrawAdditive` gets overwritten — the sprite renders in place of the glyphs.

Interleaving per row or per item looks natural and is wrong:

```cpp
for (auto& row : rows) { DrawSprite(row); DrawText(row); }   // text is eaten
```

Collect the text into a small local list during the sprite pass and flush it at the end
of `paintBackground`. `DeferredCountLabel` in the grid view and `DeferredText` in the
inventory panel are the existing examples.

This applies across the *whole* paint, not just within one helper: a panel that draws
text and then calls a sub-view which draws sprites has the same bug. That is why the
ammo readout collects its counts in the left column and draws them after the Grid.

### Why This Is Debt
The underlying cause is not understood — it is presumed to be text cursor / render state
left behind by the VGUI1 text path, which the sprite path then trips over. Every call
site works around the symptom rather than fixing the cause, and the workaround is easy to
forget because interleaved drawing is the obvious way to write it.

### Recommended Next Steps
1. Find the actual state leak between `drawPrintText`/`drawSetTextPos` and `SPR_DrawAdditive`.
2. If it can be reset explicitly, wrap it in one helper and delete the deferral lists.
3. Failing that, give panels a tiny shared "deferred text" collector so the workaround is
   one type rather than re-invented per file.

### Acceptance Criteria For Closure
- A panel can draw sprites and text in natural order without text loss.
- The deferral lists are gone.

## Skill Tree Tooltip Layout Reliability

### Scope
- Client skill tree UI in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp)
- Specifically the hover tooltip bubble shown for skill descriptions

### Current State (as of 2026-05-09)
The skill tree has received multiple layout and UX improvements:
- Nodes are centered in the panel.
- Node labels were removed; tooltip carries the detail.
- Tooltip width/height is content-driven with wrapping and clamping.
- Connector lines now anchor to node edges and prefer vertical routing when vertically aligned.

However, tooltip sizing/wrapping is still not fully robust in all cases.

### Known Issues
1. Wrapping and width fitting can still produce edge cases where text appears cramped or wraps sub-optimally for some strings and panel widths.
2. Font metrics are approximated with fixed per-character widths, which is inherently imprecise across different VGUI fonts and resolutions.
3. Title width and wrapped body width are estimated separately and can still mismatch slightly in extreme cases.
4. Tooltip geometry is computed in a single render pass and is sensitive to heuristics (char width, line height, padding), making behavior difficult to guarantee.

### Why This Is Debt
- Logic relies on heuristic text measurement rather than actual font metrics.
- Layout behavior is not deterministic across all content/viewport combinations.
- Repeated tweaks increase complexity without establishing a canonical measurement model.

### Recommended Next Steps
1. Introduce a single tooltip layout function that:
   - measures title/body with one shared width model,
   - wraps body to the same final width,
   - returns final rect + line positions in one structure.
2. Replace fixed char-width constants with proper text measurement if available in VGUI APIs; if unavailable, centralize width constants per font and calibrate once.
3. Add guardrails for pathological inputs:
   - very long unbroken words,
   - explicit newlines,
   - narrow panel widths.
4. Add a small debug mode (temporary cvar or compile flag) to draw tooltip bounds and line boxes for visual verification.
5. Validate at minimum resolutions and common HUD scaling settings.

### Acceptance Criteria For Closure
- No title overflow at supported resolutions.
- Description wraps predictably with consistent left/right padding.
- Tooltip height always fits content with stable top/bottom spacing.
- Tooltip remains clamped to skill-tree panel bounds.

### Notes
This debt is non-blocking for gameplay and can be addressed in a dedicated UI refinement pass.

## Inventory Grid Placement Consistency — RESOLVED 2026-08-01

Closed by deletion rather than by fixing. The client-side placement solver
(`NormalizeGridLayout`) no longer exists: the server owns each Entry's position, the client
renders what it is told, and nothing re-packs the Grid behind the player's back. See
[adr/0004-the-server-owns-the-inventory.md](adr/0004-the-server-owns-the-inventory.md).

All three acceptance criteria are met structurally, not behaviourally — an Entry can only sit
where the server put it, and the server refuses any placement that overlaps or overflows:

- No overlap/overflow across all Entry widths — enforced by `CPlayerInventory::CanPlaceAt`.
- Repeated drag/drop sequences produce stable placements — a drag is a request for one specific
  Cell, accepted or rejected; there is no solver to produce a different answer the second time.
- Non-involved Entries never move — nothing but an explicit `inv_move` changes a position.

Kept as a record because the mixed-width fragmentation that caused much of this is also gone:
the Grid width is now a multiple of the weapon width, so no Cell is stranded at a row end.
