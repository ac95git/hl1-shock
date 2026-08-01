# Technical Debt Register

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

## Inventory Grid Placement Consistency

### Scope
- Inventory grid layout and drag/drop behavior in [cl_dll/vgui_inventory_grid.cpp](../cl_dll/vgui_inventory_grid.cpp)
- Context: mixed-width slots (weapons/items/ammo), offset persistence, and normalization after drag/pickup

### Current State (as of 2026-05-10)
- Overlap/overflow prevention was added through layout normalization and drag clamping.
- Core behavior is improved and generally stable.

### Known Issues
1. Layout normalization can still produce occasional visual inconsistencies after complex manual drags.
2. Neighbor slot movement during or after drag may feel non-deterministic in some sequences.
3. Mixed-width slot repacking priority is heuristic, so final placement may be surprising to players in edge cases.

### Why This Is Debt
- The placement solver currently prioritizes safety (no overlap/overflow) over strict positional predictability.
- Resolution rules are implicit and not yet codified as user-facing behavior.

### Recommended Next Steps
1. Define explicit placement invariants (for example: preserve untouched slot order, only move colliding slots).
2. Split "live drag clamping" from "post-drop normalization" so behavior is easier to reason about.
3. Add deterministic tie-break rules for mixed-width placement and document them in code comments.
4. Add a debug overlay mode to visualize slot occupancy and solver decisions.

### Acceptance Criteria For Closure
- No overlap/overflow across all slot types and widths.
- Repeated drag/drop sequences produce stable, predictable placements.
- Non-involved neighboring slots do not move unless required by explicit collision resolution rules.
