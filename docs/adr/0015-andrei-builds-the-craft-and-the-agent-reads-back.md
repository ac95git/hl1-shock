# 15. Andrei builds the craft, and the agent reads back

Date: 2026-09-20

## Status

Accepted, in use from the same day. The running record is [CRAFT_LOG.md](../CRAFT_LOG.md); the map
side of the split is in [MAP_WORKFLOW.md](../MAP_WORKFLOW.md#where-the-agent-stands), the model side in
[MODEL_WORKFLOW.md](../MODEL_WORKFLOW.md).

## Context

The session of 2026-09-20 opened with the agent offering a menu of roadmap entries to pick from. Andrei
declined it and asked instead how to improve his own agency in the project. The diagnosis he accepted:

- 126 commits in ten days, nearly every line of code, layout, doc and commit message written by the
  agent. His part had shrunk to the two ends of the work, deciding in grills and judging in game, with
  nothing in the middle.
- The symptoms were already in the notes: a 3,400-line roadmap he does not read, a plan that waited
  unread, the session of 2026-09-19 opening on "a bit blocked", and the diagnosis that day of a
  design-complete, play-zero project.
- The cause is rate. Past a certain volume of agent output the only role left for the person is to
  approve it.

What he said he wants, in his words: mapping is "the single most crucial point" (the systems are in a
good place, his mapping is basic, and he wants skills *and* autonomy); modelling is a close second (the
three weapons — pickaxe, the alien grunt's melee weapon, the energy rifle — and improving existing
models; a monster from scratch is "unrealistic" and not planned); he would "definitely take writing a
feature every now and then"; and one-hour or overnight sessions delegated to the agent are still
welcome sometimes, "but that can wait".

## Decision

The split is reset so that the middle of the work is his.

1. **A session opens with Andrei's sentence, not a menu.** He says what he wants to make or see; the
   agent breaks it into steps and does the parts he hands it. The agent does not offer roadmap entries
   to pick from.
2. **He writes the intent files**: the rooms spec for a map, the "what I want" half of a model brief,
   the request lines in ART_DEBT, the questions a grill should answer. The agent writes *from* them,
   never *into* them.
3. **He tunes in the console.** Every number he might judge is a cvar; he changes it live, reports what
   stuck, and the agent commits the values. A cheat sheet of the cvars is owed.
4. **Craft is built by him in the editor.** The agent writes the steps (tool, dialog, number, from
   J.A.C.K.'s own manual), reads the saved file back, compiles and installs. It does not build the
   thing. The next map after `shaft1` is blocked out by him with no generator.
5. **Less unasked agent output.** No roadmap entries from the agent's own ideas, no overnight sessions
   that build new systems, shorter docs. Nothing he has not asked for in his own words gets built.
6. **A code feature now and then is his**, with a written brief and the agent as reviewer only. The
   friendly alien slave is the first candidate.

Two skill ladders carry this, one rung per technique on one room or one model, each checked by a
read-back and a compile. They are at the head of CRAFT_LOG.md.

## Considered

**Keep the agent building and let him approve.** The state before. Rejected: it is what produced the
block, and it makes the mod less his with every commit.

**Overnight and one-hour delegated sessions as the main mode.** Deferred by Andrei: still welcome
sometimes, not now.

**The agent editing the `.map` text for craft** (shapes, textures, lights). Rejected: the agent cannot
see the result, and the point is his hands on the editor. The agent edits map text for entity work only.

**Tutoring by screenshot alone.** Rejected in favour of reading the saved file: a `.map` is text and a
Blender scene is data over the MCP, and both can be checked exactly, which a screenshot cannot.

## Consequences

- **Slower, and his.** The four pillars took an evening with two round trips for one mistake; the agent
  would have written them in a minute. That is the trade, chosen on purpose.
- **Read-back tooling is the agent's job.** `utils/maptool/mapsemdiff.py` and `brushes_near.py` came out
  of the first session because J.A.C.K. rewrites the whole file on save and `git diff` is useless on it.
  Every later rung will want its own check, and those are agent work.
- **The agent's editor knowledge comes from the manual now**, `D:\Apps\J.A.C.K\VDKManual.pdf`, not from
  memory of Hammer. Steps are given as tool, hotkey, dialog field and value.
- **The desktop agent cannot compile or read the file back** when its shell is unavailable; on that
  surface a craft session is steps and screenshots only, and the CLI agent closes the loop after.
- **The roadmap stops growing on its own.** Entries are added when Andrei asks for them in his words.
- The first two rungs (the chamber's pillars and the crystal deposit unit) were built and committed the
  same day, which is the evidence the split works.
