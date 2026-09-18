# Sound Workflow

How the mod's sounds are made, converted, reviewed and shipped. Set up 2026-09-18 in the grill on the art
workflow; the first sound through it has not been made yet. Companion to
[SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md) and [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md).

The loop is: **Andrei makes the sound in FL Studio and exports it, in any format, to the hand-made tier →
`convert.py` turns it into what the engine plays and writes the repo's `sound/` → copied to the install →
judged in game, in the sequence it plays in.** The FL Studio project stays his; the export is the source
the pipeline reads; the repo holds the converted file.

## Where things are

| | Path |
| --- | --- |
| Hand-made tier (his exports; **nothing writes here but Andrei**) | `E:\CustomAssets\sounds\src\<engine path>` |
| Shipped sounds, source of truth for the install | repo `sound/<engine path>.wav` |
| Install copy | `D:\GameLibrary\steam\steamapps\common\Half-Life\topmod\sound\` |
| The stock set, for neighbours and references | `...\Half-Life\valve\sound\` |
| The converter | `utils/sndtool/convert.py` |
| ffmpeg (not on PATH) | `D:\Utils\ffmpeg-master-latest-win64-gpl-shared\bin\` |

The path under the tier **is** the engine path: an export saved as `sounds/src/pulse/brace.wav` ships as
`sound/pulse/brace.wav` and is played as `"pulse/brace.wav"`. The extension in the tier can be anything
FL Studio writes (`.wav`, `.flac`, `.mp3`, `.ogg`); the shipped file is always `.wav`. The
`sounds/sound/` folder beside the tier is an old Team Fortress 2 dump and is not part of this.

## Facts that bind the work

- **The engine plays loose `.wav` files**, mono, 8- or 16-bit PCM, found by path under `sound/`. No
  `.pak`, no other format for effects. The stock set is **11025 Hz 8-bit** for the oldest sounds and
  **22050 Hz** for the newer ones (every `weapons/` sound). A sound exported at 44100 Hz stereo is not
  refused; it plays **louder and brighter than everything beside it**, which is why the converter's
  default is 22050 mono 16-bit and the peak is normalised to one level for the whole set.
- **A sound is judged in the sequence it plays in, never alone.** The Pulse's first sound set failed with
  good samples: the Pulse and the deflect shared a timbre and landed a tenth of a second apart, and the
  deflect — the cue that carried the information — was inaudible ([ART_DEBT.md](ART_DEBT.md), *The Pulse —
  sounds*). Every request therefore names what plays within half a second of the new sound, and the
  converter's `--check` compares the two before the game is launched.
- **Looping is a cue point in the file.** A stock looping sound (`ambience/wind2.wav`,
  `weapons/egon_run3.wav`, checked 2026-09-18) carries a `cue ` chunk with one point whose sample offset is
  where the loop restarts, and a `LIST adtl ltxt` chunk of purpose `mark` giving the loop's length; a
  one-shot stock sound carries neither. That is the Quake `GetWavinfo` form GoldSrc inherited. FL Studio
  does not write it; `--loop` does, and `convert.read_loop` reads it back.
- **The attack lands on the first frame.** Leading silence in an export is latency in game: a deflect cue
  50 ms late is a deflect that feels late. The converter trims everything below −50 dBFS at the start,
  keeping eight samples of run-up so a soft attack does not click.
- **Monster voices are sentences, not files.** A line a grunt speaks is composed in `sound/sentences.txt`
  from word samples, and the mod's own `sentences.txt` already exists for the Search's lines. A new
  voice is samples through this pipeline plus a sentence entry; `!NAME` in code plays the sentence.
- **Pitch is applied in code, not baked.** Half the mod's cues are one stock sample at two pitches
  (`player/recharged.wav` at 100 and 80 for the two readiness chimes). A new sound is made at pitch 100
  and the code keeps its pitch offsets; two meanings on one sample told apart only by pitch is an
  ART_DEBT entry, not a target.
- **Every borrowed sound says where it came from** in [ART_DEBT.md](ART_DEBT.md). A sound made here says
  it was made here.

## Tools

`utils/sndtool/convert.py` (Python 3, standard library plus ffmpeg):

| Invocation | Does |
| --- | --- |
| `convert.py pulse/brace [more paths]` | One export from the tier → `sound/pulse/brace.wav`: mono, 16-bit, 22050 Hz, leading silence trimmed, peak at −1 dBFS. Prints what it did and an ART_DEBT line to fill in. |
| `convert.py --all` | Every file under the tier. |
| `--install` | Also copies the result to the install's `topmod/sound/`. |
| `--loop [SECONDS]` | A looping sound: the cue point at `SECONDS` from the trimmed start (default 0, the whole file loops). |
| `--check NEIGHBOUR` | Compares the result with the sound named (a repo `sound/` path, a `valve/sound/` path, or a file): length, peak, RMS, spectral centroid and zero-crossing rate over the first 0.3 s, a low/mid/high band, and a **clash flag** when both are short and in the same band, or a softer one when the bands match. |
| `--peak DB`, `--no-normalize`, `--trim-db DB\|none`, `--rate 11025\|22050\|44100`, `--out DIR` | The knobs. `--out` is for tests; it refuses any directory under the tier. |

The converter **never writes under `sounds/src/`**, and refuses to.

## Requesting a sound

A sound is asked for as an [ART_DEBT.md](ART_DEBT.md) entry, or a row in one, and the row says three
things the converter and the review need:

| | |
| --- | --- |
| **When it fires** | The event, and where in code (`k_PulseSoundDeflect`, `DASH_SOUND`) |
| **What plays beside it** | Every sound that can land within half a second of it, by path — the deflect beside the Pulse, the Follow-Up's hit beside the body hit, the tail's brace beside nothing |
| **What it must say** | One line: *deployed*, *parried*, *stance held*, *readiness*, *refusal*. The Pulse entry's *What to look for* is the model |

Whether it loops, and any pitch offsets the code will apply, go in the same row.

## Review

Two steps, and only the second closes an entry:

1. **The converter's check**, run with `--check` against each neighbour the request named. A clash flag
   is a reason to change the sound before launching the game; a note about peaks is a reason to look at
   the level. The numbers are rough — a centroid from a 1024-point window, pure Python — and they exist to
   catch the Pulse's failure early, not to grade a sound.
2. **Andrei, in play, in the sequence.** The same action that fires the neighbour, several times, with
   the HUD not looked at. The question is the ART_DEBT entry's own: can the new cue be told from the one
   beside it, by ear, at the moment it matters. That judgement is written into the entry and closes it.

## Adding a sound

1. Read the ART_DEBT entry, or write the row: the event, the neighbours, what it must say.
2. Make it in FL Studio. Export to `E:\CustomAssets\sounds\src\<engine path>` in any format.
3. `python utils\sndtool\convert.py <engine path> --check <neighbour> [--loop] [--install]`.
4. Point the code at the path if it is a new sound; rebuild both DLLs (the build installs them). A
   replacement for a stock path needs no code change.
5. Copy to the install if `--install` was not used. Verify in game, in sequence, before committing.
6. Commit `sound/<path>.wav` with the ART_DEBT entry updated in the same commit.

## Not yet covered

- **Nothing has been through it.** The first sound the register asks for is the Panthereye's growl, then
  the Pulse set; the converter was tested on a synthetic burst only.
- **Trailing silence** is kept, since a decay is intentional and a trailing trim would need a judgement
  the script cannot make.
- **Music.** The engine streams `media/*.mp3` by track number through a different path; not this pipeline.
- **Sentences for a new voice** (the friendly slave's English) need the sample cut and the
  `sentences.txt` entry written; the converter handles only the samples.
