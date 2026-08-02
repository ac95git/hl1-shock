# Technical Debt Register

## The Pulse Discharge Bypasses Every `TraceAttack` Damage Rule — RESOLVED 2026-08-02

Fixed as recommended below, by option 1. `FireDischarge` now copies `gMultiDamage` to a local, runs
`ClearMultiDamage` → `pHit->TraceAttack(...)` → `ApplyMultiDamage(...)` on a clean accumulator, and
restores the original. All twelve rule sets in the table below apply to a Discharge again, including the
alien slave's `DMG_SHOCK` immunity that surfaced the bug.

The `TraceResult` from the Discharge's own traceline is passed through, so hitgroups resolve normally —
head shots, helmets and armour plates all behave as they do for any other attack.

Kept as a record because the underlying hazard has not gone away: **`gMultiDamage` is a single global and
`ApplyMultiDamage` leaves it live**, so any future code that deals damage from inside a `TakeDamage`
handler faces the same trap and needs the same save/restore.

### Scope
`dlls/player_pulse.cpp`, `FireDischarge` (the `pHit->TakeDamage(...)` call). Affects every entity that
implements damage rules by overriding `TraceAttack`.

### The bug
`FireDischarge` traces a line and then applies damage with a direct `pHit->TakeDamage(...)` rather than
`ClearMultiDamage` / `TraceAttack` / `ApplyMultiDamage`. That was deliberate — it runs from inside
`CBasePlayer::TakeDamage`, which is itself normally called from `ApplyMultiDamage`
(`dlls/weapons.cpp:97`), and that function leaves `gMultiDamage` live, so clearing it there would corrupt
the sequence in flight.

The consequence was under-thought. **Half-Life puts a great deal of per-entity damage logic in
`TraceAttack`, and a direct `TakeDamage` skips all of it.** Twelve types are affected:

| Entity | Rule in `TraceAttack` | What the Discharge does instead |
| --- | --- | --- |
| `CISlave` (`islave.cpp:585`) | **immune to `DMG_SHOCK`** | damages it, and provokes it |
| `CBigMomma` (`bigmomma.cpp:559`) | only hitgroup 1 is vulnerable | damages from any angle |
| `CApache` (`apache.cpp:969`) | blades ignore beam/bullet/club | damages the blades |
| `COsprey` (`osprey.cpp:807`) | per-engine damage caps | ignores the caps |
| `CHGrunt` (`hgrunt.cpp:605`) | helmet absorbs 20 | ignores the helmet |
| `CAGrunt` (`agrunt.cpp:221`) | armour plate ricochets | ignores the plate |
| `CBarney` (`barney.cpp:570`) | chest/stomach halve damage | full damage |
| `CBaseTurret` (`turret.cpp:993`) | armour hitgroup | ignores it |
| `CNihilanth` (`nihilanth.cpp:1262`) | irritation state machine | never advances it |
| `CGargantua`, `CGMan`, `CBreakable` | ricochets, sparks, death handling | none of it fires |
| `CBaseMonster` (`combat.cpp:1327`) | hitgroup multipliers | flat damage |

The alien slave was the visible one because the Discharge was `DMG_SHOCK` at the time — the exact type
slaves are built to be immune to — so Discharging into one produced a monster taking damage it should
shrug off, which read in-game as the slave "bugging out".

Note that fixing this then made slaves *correctly* immune, which played badly for a different reason: a
counter that one headline target ignores is the wrong counter. The Discharge now deals `DMG_ENERGYBEAM`,
which nothing is immune to. That is a design decision recorded in
[adr/0006](adr/0006-the-discharge-vents-at-the-crosshair.md), not part of this fix — the bug below was
real regardless of damage type, and eleven other entity types were affected by it.

### Why This Is Debt
The comment at the call site explains why `TraceAttack` is avoided but not what is lost by avoiding it, so
the trade-off looks settled when it is not. It will also silently mis-handle any future entity whose
damage rules live in `TraceAttack`, with no compile error and no obvious symptom.

### Recommended Next Steps
1. **Preferred:** save and restore `gMultiDamage` around a proper nested sequence. It is a plain POD
   (`MULTIDAMAGE` in `dlls/weapons.h:405-412`), so copying it to a local, running
   `ClearMultiDamage` → `pHit->TraceAttack(...)` → `ApplyMultiDamage(...)`, then copying it back, makes
   the nesting safe and restores every rule above. A few lines, surgical.
2. Alternative: defer the Discharge to a pending list drained in `PostThink`, outside the damage
   sequence entirely. Costs the "fires in the same frame the hit lands" property that
   [adr/0006](adr/0006-the-discharge-vents-at-the-crosshair.md) describes.
3. Do **not** special-case the slave. The slave is a symptom.

### Acceptance Criteria For Closure
- Discharging into an alien slave does nothing, because slaves are immune to `DMG_SHOCK`.
- A Discharge into an HGrunt's helmet or an AGrunt's armour plate behaves like any other hit there.
- Hitgroup multipliers apply, or their absence is a documented decision rather than a side effect.

## A Deflected Melee Attack Still Reports As A Hit To The Attacker — MITIGATED 2026-08-02

The view kick is now **scaled rather than suppressed**, which turned out to be the better outcome: a
deflect that produces no reaction at all reads as the blow having missed, where a small nudge reads as it
glancing off. `pulse_deflect_punch` (default `0.25`) controls how much survives; `0` removes it entirely,
`1` restores vanilla.

`CPlayerPulse::DampenDeflectPunch` is called from `UpdateClientData` (`dlls/client.cpp:1932`) rather than
from the player's think, because the attacker writes `punchangle` *after* the player has thought — the
player is entity 1 and runs first. Scaling it a frame later would show the full kick for one frame and
then snap. `UpdateClientData` runs once every entity has thought, so the value is final and the client
never sees the undamped one. Only the delta the attacker added is scaled, so a kick already being carried
from something else is untouched.

**The root cause is untouched** and the entry stays open for it: `CheckTraceHullAttack` still reports a
hit that was refused, so hit-flesh *sounds* still play on a deflect, and any future reaction driven off
that return value will leak the same way. What follows still applies to that.

### Scope
`dlls/combat.cpp:1129` `CBaseMonster::CheckTraceHullAttack`, and every monster that acts on its return
value.

### The bug
`CheckTraceHullAttack` calls `pEntity->TakeDamage(...)` and then returns `pEntity` **whether or not the
damage was actually taken**. Monsters treat a non-null return as "I hit them" and act on it.

So a Pulse that deflects an alien slave's claw still gets the player's view kicked, because
`islave.cpp:317-323` sets `punchangle` on the returned entity. The same pattern appears in the zombie,
bullsquid, agrunt and others — hit sounds and reactions all fire on a deflected blow.

Damage is correctly refused. Only the *reaction* leaks through.

### Why This Is Debt
It undermines the Pulse's core feedback promise: a successful deflect should feel like nothing touched
you. A view kick with no damage reads as the mechanic half-working rather than as a deliberate choice.

The blood half of this same problem was fixed for `TraceAttack` (`dlls/player.cpp:342`), so the
inconsistency is now visible: bullets deflect cleanly, claws do not.

### Recommended Next Steps
1. Have `CheckTraceHullAttack` return `NULL` when `TakeDamage` refused the damage — the signature already
   carries the information, since `TakeDamage` returns a bool that is currently discarded at
   `combat.cpp:1150`.
2. Audit callers first. Some may rely on a non-null return for reasons unrelated to damage landing (for
   example hitting a breakable or a non-damageable entity), so a blanket change could suppress reactions
   that should still fire.

### Acceptance Criteria For Closure
- A deflected melee attack produces no view punch and no hit-flesh sound.
- Monsters still react normally when they strike something they genuinely damaged.

## `IN_INVENTORY` Is Silently Truncated And Never Reaches The Server

### Scope
[common/in_buttons.h](../common/in_buttons.h), [cl_dll/input.cpp](../cl_dll/input.cpp).

### The bug
`IN_INVENTORY` is defined as `(1 << 16)`, but `usercmd_t.buttons` is an `unsigned short`
([common/usercmd.h](../common/usercmd.h) line 29). `cmd->buttons = CL_ButtonBits(true)`
(`cl_dll/input.cpp:745`) assigns an `int` into that 16-bit field, so **every bit above 15 is
discarded**. The bit is set at `cl_dll/input.cpp:876` and arrives at the server as zero.

It is harmless *today* only by accident: `IN_InventoryDown` / `IN_InventoryUp`
(`cl_dll/input.cpp:526-541`) open and close the panel locally on the client, and nothing in
`dlls/` reads `IN_INVENTORY`. The bit is doing no work at all.

### Why This Is Debt
It is a trap rather than a malfunction. The definition looks exactly like every other button
bit, so the first person to write `pev->button & IN_INVENTORY` server-side will find it never
fires, with nothing in the code to explain why.

Note also that bits 0–15 are now **fully allocated**, so there is no room for another button.
`IN_ALT1` (bit 14) and `IN_CANCEL` (bit 6) are set by the client and read by nothing in `dlls/`,
making them the only reclaimable bits. This is why the Pulse uses `impulse 150` rather than a
button bit — see [PILLARS.md](PILLARS.md) pillar 2.

### Recommended Next Steps
1. Either reclaim a dead bit for `IN_INVENTORY` so it genuinely transmits, or drop it to a plain
   `kbutton_t` with no `IN_` constant, making its client-local nature explicit.
2. Comment the 16-bit ceiling in `common/in_buttons.h` so the next addition doesn't repeat it.

### Acceptance Criteria For Closure
- No `IN_` constant exists above bit 15.
- `common/in_buttons.h` states the `unsigned short` limit.

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

## Skill Tree Tooltip Layout Reliability — RESOLVED 2026-08-02

Closed by doing what step 2 below only offered conditionally. It hedged: *"Replace fixed char-width
constants with proper text measurement **if available in VGUI APIs**; if unavailable, centralize width
constants per font and calibrate once."* It **is** available —
[`utils/vgui/include/VGUI_Font.h`](../utils/vgui/include/VGUI_Font.h) exposes
`getTextSize(text, wide, tall)` and `getTall()`. The heuristics were never necessary, and every one of
them is gone: `kTitleCharW = 11`, `kDescCharW = 7`, the hardcoded 12px line height, and the three-pass
width-fitting loop that existed only because the estimate and the wrap disagreed.

`CSkillTreeView::BuildTooltip` is now the single layout function step 1 asked for. It measures the title
and body with one model, wraps the body to a **pixel** width, and returns a `TooltipLayout` carrying the
final rect, the line height and the laid-out lines together — so the box cannot disagree with what is
drawn into it. It is one pass, not iterative.

Wrapping is exact rather than iterative because of the order: wrap at the maximum allowed width, then
shrink the box to the widest line actually produced. Shrinking to a line's own width can never make that
line overflow, so no second pass is needed.

Both pathological inputs from step 3 are handled: explicit `\n` breaks a line, and a word wider than the
box is broken by character instead of overhanging. The bubble is clamped to the tree area on all four
sides, flipping to the other side of the node before clamping.

All four acceptance criteria are met. Steps 4 and 5 (a debug bounds-drawing mode, and validation at
minimum resolutions) were not done and are no longer needed for this entry — geometry is now derived from
measurement rather than tuned by eye, so there is nothing to eyeball. Low-resolution *layout* is a
separate matter and is handled by the fit-to-area scaling described in PILLARS pillar 4.

### Scope
- Client skill tree UI in [cl_dll/vgui_skilltree.cpp](../cl_dll/vgui_skilltree.cpp)
- Specifically the hover tooltip bubble shown for skill descriptions

### Current State (as of 2026-05-09, superseded)
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
