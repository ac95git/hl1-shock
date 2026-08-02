# 8. Skill definitions are shared, not networked

Date: 2026-08-02

## Status

Accepted.

## Context

A `SkillDef` holds a Skill's id, name, description, icon, grid position, cost, prerequisites and
visual tier. Every one of those is the same for every player and never changes at runtime.

The table nonetheless lived in `dlls/player_skills.cpp` as `extern const`, and the server sent
`gridCol`, `gridRow`, `cost`, `prereq` and `tier` to the client on every sync — six bytes per node,
116 bytes of a 192-byte user message at nineteen Skills. Names and descriptions were too large to
send, so the client kept `k_SkillUiInfo` in `cl_dll/vgui_skilltree.cpp`: a **second, hand-maintained
copy** of every name and description. Adding a Skill meant editing both tables, and nothing detected
it when only one was edited.

Two consequences were already being felt. Adding a second prerequisite was scoped in
[PILLARS.md](../PILLARS.md) as "6 bytes per node becomes 7 … leaving room for roughly eight more
Skills before the message has to be chunked" — a design constraint arising purely from sending
constants. And the client and server could disagree about how many Skills exist, silently.

## Decision

Static Skill data moves to `game_shared/skill_defs.h` and is compiled into both DLLs, exactly as
`inventory_defs.h` already does for Item Types (see
[ADR-0002](0002-two-identity-spaces-for-weapons-and-items.md)). `k_SkillUiInfo` is deleted.

`gmsgSkillTree` carries **state only**: a bitmask of unlocked Skills, one bit per id, followed by
unspent Skill Points. It is registered at a fixed length derived from `k_MaxSkills`, so a
client/server disagreement is an engine-level error rather than a misread.

"Available" — prerequisites met, affordable, not already held — is derived **client-side** from the
mask and the shared table. The server does not send it.

## Consequences

**Adding or changing a Skill is one edit.** Name, description, icon, position, cost, prerequisites
and tier are one table row. The drift hazard is deleted rather than managed.

**Static data has no wire cost.** The second prerequisite this change enables costs zero bytes, as
would a third, or per-Skill icons, or anything else static. The message went from 116 bytes to 4.

**Client and server cannot disagree about the Skill set.** Both derive `k_MaxSkills` and
`k_SkillMaskBytes` from the same constant.

**The gating rule has one implementation.** `SkillPrereqMet` in the shared header is called by the
server against its own unlocked array and by the client against the mask it was sent.

**Ids are load-bearing in a new way.** An id is now a bit position in the sync message as well as an
index into the save array, so the existing freeze on ids is unchanged but now has two reasons behind
it. A Skill removed from the tree keeps its id reserved and simply stops appearing.

**The client computing "available" is not an authority change.** It decides what to draw and what to
let you click. `CPlayerSkills::TryUnlock` re-validates prerequisites, cost and duplicate unlock
independently, so a client that lies about availability still cannot unlock anything.
