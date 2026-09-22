---
name: map-check
description: Read back a map Andrei has exported from J.A.C.K. — what changed since the last commit, brush by brush — then install it, compile it with the four VHLT tools and summarise the log. Use whenever he says a map is saved, exported or done, or asks to check, compile or install a map.
---

The read-back for a hand-edited map. Andrei builds in J.A.C.K. and compiles there with Run Map as
often as he likes; this is the check before a commit, when he says "done". Never edit the `.map`
while J.A.C.K. is open, never run `greybox.py` on a map J.A.C.K. has saved, and never write brushwork
into a map's text — entities only, and only when he asks, because each text edit costs him a reopen of
the `.map` to overwrite the `.jmf` (docs/MAP_WORKFLOW.md, "The division of the work"). A map that is
not in the repo can still have its region listed and its log read from `topmod/maps/`; only the diff
needs a committed baseline.

## Run

```
python utils/maptool/mapcheck.py maps/<name>.map [--region x0 y0 z0 x1 y1 z1] [--layout-only]
```

- Always compare against the last commit (`--ref HEAD` is the default). J.A.C.K. rewrites the whole
  file on export, so `git diff` is useless; the script compares by computed brush vertices, textures
  and entity keyvalues and prints only what was removed, added or moved between entities.
- Add `--region` for the box he was working in (from the exercise's coordinates): it lists every brush
  there with its owner entity and brush number, extents, which axis a prism or point runs along, and
  off-grid vertices. That is how a wrong-way cylinder or a floating floor is diagnosed.
- `--layout-only` (CSG and BSP, fullbright) while a layout is moving; the default runs all four, which
  a lit chamber or texture lights need. A full `shaft1` compile is about fifteen seconds.

## Read the result

1. **The timestamp warning comes first.** The `.jmf` in `maps/jmf/` newer than the `.map` means he
   saved and did not export (Ctrl+S writes the `.jmf`; the `.map` changes only on File → Export or Run
   Map): say so and stop, the diff is against a stale map. The `.map` newer than the `.jmf` means an
   agent text edit he has not reopened yet: tell him to open the `.map` in J.A.C.K. and overwrite the
   `.jmf` before he edits anything. An empty diff with no warning means he changed nothing.
2. **Check the diff against what he said he built.** Expected: the old brushes gone, the new ones
   added, in the entity they should be in (world for structure, `func_detail` for shapes that are not
   boxes, the game entity for things in play). J.A.C.K. writes every FGD default into an entity once
   its properties were opened (`health`, `yield`, render keys); those match the code's defaults and are
   noise, but a missing spawnflag (Unstable, Start Off) is not.
3. **The log.** A leak fails BSP: tell him to load the pointfile (Map → Load Pointfile) and follow the
   line. Errors stop the compile. Warnings by kind: *Ambiguous leafnode content* in an entity's hulls
   means brushes of that entity pass through each other and the widened collision copies disagree;
   the compiler resolves each one, and it is tested by walking against the thing. Texture lights show
   in the direct-light count (57 for `shaft1` before the crystal glow, 1748 after).
4. **The `.bsp` line must say "just written".** Stale means a tool failed before it.

## Report to Andrei

What changed, in his terms (the pillar, the unit, the flag), not the script's; anything missing from
what he described; what the compile said; and what to test in game, as two or three rows. Then wait
for his verdict before committing — [verify in game before committing](../../../docs/MAP_WORKFLOW.md).
After his yes: commit the exported `.map` (never the `.bsp`), with what was built in the commit
message, and add learned-only bullets to `docs/CRAFT_LOG.md` if the session taught something.

Paths and flags live at the top of `mapcheck.py`: the compilers in `D:\Apps\J.A.C.K\halflife`, the
install in `topmod\maps`, and the compile runs with the current directory on `D:` because
`worldspawn`'s wad paths are drive-relative.
