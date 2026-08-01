# The Shield negates a curated list of damage types

A Shield negates `DMG_BULLET`, `DMG_SLASH`, `DMG_CLUB`, `DMG_BLAST`, `DMG_SHOCK`, `DMG_SONIC`,
`DMG_ENERGYBEAM` and `DMG_CRUSH`, and nothing else. The rule the list encodes is that a Shield repels what
is coming at the player — it does not cushion the player's own body, and it does not filter what they are
standing in.

Two exclusions are deliberate and will read as omissions.

**`DMG_FALL` is not negated.** The Skill Tree already sells `FallResistance` (−50% fall damage) for one
Skill Point. A Pulse that cancelled a fall outright would make that node worthless for the price of a key
press, and would turn a quarter-second window into a free descent from any height.

**Everything under `DMG_TIMEBASED` is not negated** — drown, poison, radiation, nerve gas. It is applied on
a repeating tick rather than as an event, so "negated during the window" only means eating the same tick a
fraction of a second later. It would read as the Pulse being broken rather than as a boundary.

## Considered options

"Negate anything that has an attacker" was considered as a rule with no list to maintain, and rejected:
Half-Life gives `func_train`, `func_door` and most environmental hazards a perfectly good attacker, so a
Shield would stop a moving train while still needing exceptions elsewhere.

"Negate literally everything" was considered and rejected for the `FallResistance` collision above.

## Consequences

`DMG_CRUSH` is in the list for the fiction — the suit is mining equipment and falling debris is what it
exists for — but most crush damage in Half-Life actually comes from doors, trains and lifts. It is the one
entry most likely to produce a "why did that work?" moment in play.

Adding a damage type to the game means deciding which side of this list it falls on. There is no default,
and an unlisted type is silently not negated.
