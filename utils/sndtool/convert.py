#!/usr/bin/env python3
"""Turn a hand-made sound into one the engine plays, and say what it will sound like beside its neighbour.

    convert.py PATH [PATH ...] [options]
    convert.py --all [options]

PATH is the engine path of a file under the hand-made tier, `E:\\CustomAssets\\sounds\\src\\`
(`pulse/brace.wav`, with or without the extension; any format FL Studio exports), or an absolute
file. The result is written to the repo's `sound/<path>.wav` (or `--out DIR/<path>.wav`):
mono, 16-bit PCM, 22050 Hz, leading silence trimmed so the attack lands on the first frame, peak
normalised to the set's level. Nothing is ever written under `sounds/src/`.

Options:
    --all               every file under sounds/src/
    --out DIR           write under DIR instead of the repo's sound/ (tests, scratch)
    --install           also copy to the mod directory's sound/
    --no-normalize      keep the source's level
    --peak DB           peak target in dBFS (default -1.0)
    --trim-db DB        leading silence threshold in dBFS (default -50.0); --trim-db none keeps it
    --loop [SECONDS]    a looping sound: cue point at SECONDS (default 0.0) from the trimmed start
    --check NEIGHBOUR   compare the result with a sound that plays beside it (repo `sound/` path,
                        valve's `sound/` path, or an absolute file) and flag a likely timbre clash
    --rate HZ           22050 by default; 11025 to match the oldest stock set

The loop cue is what the stock looping sounds carry (checked 2026-09-18 on wind2, steamjet1,
alien_beacon, labdrone1, egon_run3): a `cue ` chunk with one point whose sample offset is where the
loop restarts, and a `LIST adtl ltxt` chunk of purpose `mark` giving the loop's length. That is the
Quake `GetWavinfo` form GoldSrc inherited; a non-looping stock sound carries neither.
"""

import argparse
import array
import math
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import wave

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
REPO_SOUND = os.path.join(REPO, "sound")
SRC = r"E:\CustomAssets\sounds\src"
INSTALL_SOUND = r"D:\GameLibrary\steam\steamapps\common\Half-Life\topmod\sound"
VALVE_SOUND = r"D:\GameLibrary\steam\steamapps\common\Half-Life\valve\sound"
FFMPEG = r"D:\Utils\ffmpeg-master-latest-win64-gpl-shared\bin\ffmpeg.exe"

AUDIO_EXT = (".wav", ".mp3", ".flac", ".ogg", ".aif", ".aiff", ".m4a", ".wma")


def die(msg):
    print("convert.py: " + msg, file=sys.stderr)
    sys.exit(1)


def under(path, root):
    path = os.path.normcase(os.path.abspath(path))
    root = os.path.normcase(os.path.abspath(root))
    return path == root or path.startswith(root + os.sep)


# ---------------------------------------------------------------- reading

def read_pcm(path):
    """Any audio -> (rate, list of float samples in -1..1), mono, via ffmpeg. Also reads the stock
    8-bit files."""
    tmp = tempfile.NamedTemporaryFile(suffix=".wav", delete=False)
    tmp.close()
    try:
        run_ffmpeg(path, tmp.name, None)
        with wave.open(tmp.name, "rb") as w:
            rate = w.getframerate()
            raw = w.readframes(w.getnframes())
        samples = array.array("h")
        samples.frombytes(raw)
        return rate, [s / 32768.0 for s in samples]
    finally:
        os.unlink(tmp.name)


def run_ffmpeg(src, dst, rate):
    if not os.path.exists(FFMPEG):
        die("ffmpeg not found at " + FFMPEG)
    cmd = [FFMPEG, "-v", "error", "-y", "-i", src, "-ac", "1", "-c:a", "pcm_s16le"]
    if rate:
        cmd += ["-ar", str(rate)]
    cmd += ["-f", "wav", dst]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        die("ffmpeg failed on %s:\n%s" % (src, r.stderr.strip()))


def read_int16(path):
    with wave.open(path, "rb") as w:
        assert w.getsampwidth() == 2 and w.getnchannels() == 1
        rate = w.getframerate()
        samples = array.array("h")
        samples.frombytes(w.readframes(w.getnframes()))
    return rate, samples


# ---------------------------------------------------------------- processing

def db_to_lin(db):
    return 10.0 ** (db / 20.0)


def lin_to_db(x):
    return -200.0 if x <= 0 else 20.0 * math.log10(x)


def trim_leading(samples, trim_db):
    thr = int(db_to_lin(trim_db) * 32767)
    for i, s in enumerate(samples):
        if abs(s) > thr:
            # keep a few samples of run-up so a soft attack is not clicked into
            return samples[max(0, i - 8):], i
    return samples, 0


def normalise(samples, peak_db):
    peak = max((abs(s) for s in samples), default=0)
    if peak == 0:
        return samples, 1.0
    gain = db_to_lin(peak_db) * 32767.0 / peak
    out = array.array("h", (max(-32768, min(32767, int(round(s * gain)))) for s in samples))
    return out, gain


# ---------------------------------------------------------------- writing

def write_wav(path, rate, samples, loop_start=None):
    """16-bit mono PCM; with loop_start (samples) appends the cue and ltxt chunks the engine reads."""
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(samples.tobytes())
    if loop_start is None:
        return
    n = len(samples)
    # cue chunk: one point, id 1, at loop_start, in the data chunk
    cue = struct.pack("<I", 1) + struct.pack("<IIIIII", 1, loop_start, 0x61746164, 0, 0, loop_start)
    cue_chunk = b"cue " + struct.pack("<I", len(cue)) + cue
    # LIST adtl ltxt: id 1, length of the loop, purpose "mark" (the stock files' form)
    ltxt = struct.pack("<II", 1, n - loop_start) + b"mark" + struct.pack("<HHHH", 0, 0, 0, 0)
    ltxt_chunk = b"ltxt" + struct.pack("<I", len(ltxt)) + ltxt
    list_chunk = b"LIST" + struct.pack("<I", 4 + len(ltxt_chunk)) + b"adtl" + ltxt_chunk
    with open(path, "r+b") as f:
        f.seek(0, 2)
        f.write(cue_chunk + list_chunk)
        size = f.tell() - 8
        f.seek(4)
        f.write(struct.pack("<I", size))


def read_loop(path):
    """Returns (loop_start, loop_len) from a wav's cue/ltxt chunks, or None."""
    with open(path, "rb") as f:
        d = f.read()
    pos, start, length = 12, None, None
    while pos + 8 <= len(d):
        cid = d[pos:pos + 4]
        sz = struct.unpack("<I", d[pos + 4:pos + 8])[0]
        body = d[pos + 8:pos + 8 + sz]
        if cid == b"cue " and struct.unpack("<I", body[:4])[0] >= 1:
            start = struct.unpack("<I", body[4 + 20:4 + 24])[0]
        # LIST body: "adtl" | "ltxt" size | cue id | loop length | "mark" ...
        if cid == b"LIST" and body[:4] == b"adtl" and body[4:8] == b"ltxt" and body[20:24] == b"mark":
            length = struct.unpack("<I", body[16:20])[0]
        pos += 8 + sz + (sz & 1)
    return None if start is None else (start, length)


# ---------------------------------------------------------------- the check

def describe(path):
    """length, peak, rms, brightness of any wav (stock 8-bit included)."""
    rate, x = read_pcm(path)
    n = len(x)
    if n == 0:
        return dict(length=0.0, peak=-200.0, rms=-200.0, centroid=0.0, zcr=0.0, band="silent")
    peak = max(abs(s) for s in x)
    rms = math.sqrt(sum(s * s for s in x) / n)
    # brightness over the attack: the first 0.3 s, where the Pulse's two cues collided
    win = x[:min(n, int(rate * 0.3))]
    zc = sum(1 for i in range(1, len(win)) if (win[i - 1] < 0) != (win[i] < 0))
    zcr = zc / (len(win) / rate) if len(win) > 1 else 0.0
    centroid = spectral_centroid(win, rate)
    if centroid < 800:
        band = "low"
    elif centroid < 2500:
        band = "mid"
    else:
        band = "high"
    return dict(length=n / rate, peak=lin_to_db(peak), rms=lin_to_db(rms), centroid=centroid,
                zcr=zcr, band=band)


def spectral_centroid(win, rate):
    """A plain DFT on a 1024-sample window (averaged over the attack), pure Python."""
    N = 1024
    if len(win) < N:
        win = list(win) + [0.0] * (N - len(win))
    hops = list(range(0, min(len(win), 4 * N) - N + 1, N)) or [0]
    num = den = 0.0
    twiddle = [complex(math.cos(-2 * math.pi * k / N), math.sin(-2 * math.pi * k / N)) for k in range(N)]
    for h in hops:
        frame = [win[h + i] * (0.5 - 0.5 * math.cos(2 * math.pi * i / N)) for i in range(N)]
        for k in range(1, N // 2):
            acc = 0j
            for i in range(N):
                acc += frame[i] * twiddle[(k * i) % N]
            mag = abs(acc)
            num += mag * (k * rate / N)
            den += mag
    return num / den if den else 0.0


def resolve_neighbour(spec):
    if os.path.isabs(spec) and os.path.exists(spec):
        return spec
    for root in (REPO_SOUND, VALVE_SOUND):
        p = os.path.join(root, spec)
        if not p.lower().endswith(".wav"):
            p += ".wav"
        if os.path.exists(p):
            return p
    die("neighbour not found: " + spec)


def check(out_path, neighbour):
    a, b = describe(out_path), describe(neighbour)
    print("check: %s against %s" % (os.path.relpath(out_path, REPO) if under(out_path, REPO) else out_path,
                                    neighbour))
    print("  %-12s %8s %9s %9s %10s %9s  %s" % ("", "length", "peak", "rms", "centroid", "zcr", "band"))
    for name, d in (("this", a), ("neighbour", b)):
        print("  %-12s %7.3fs %8.1fdB %8.1fdB %9.0fHz %8.0f/s  %s" %
              (name, d["length"], d["peak"], d["rms"], d["centroid"], d["zcr"], d["band"]))
    short = a["length"] < 0.3 and b["length"] < 0.3
    if a["band"] == b["band"] and short:
        print("  LIKELY CLASH: both short and in the same brightness band. Two cues like these landed a"
              " tenth of a second apart on the Pulse and the second was inaudible (ART_DEBT.md).")
    elif a["band"] == b["band"]:
        print("  possible clash: same brightness band. Fine if they never play within half a second.")
    else:
        print("  no clash flagged: different bands.")
    if abs(a["peak"] - b["peak"]) > 6:
        print("  note: peaks differ by %.0f dB; the louder one will dominate the pair." % abs(a["peak"] - b["peak"]))


# ---------------------------------------------------------------- main

def collect(args):
    if args.all:
        found = []
        for root, _, files in os.walk(SRC):
            for f in files:
                if f.lower().endswith(AUDIO_EXT):
                    found.append(os.path.join(root, f))
        if not found:
            die("nothing under " + SRC)
        return found
    if not args.paths:
        die("give a PATH or --all")
    out = []
    for p in args.paths:
        if os.path.isabs(p):
            if not os.path.exists(p):
                die("no such file: " + p)
            out.append(p)
            continue
        cands = [os.path.join(SRC, p)] + [os.path.join(SRC, p + e) for e in AUDIO_EXT]
        hit = next((c for c in cands if os.path.isfile(c)), None)
        if not hit:
            die("not under %s: %s" % (SRC, p))
        out.append(hit)
    return out


def engine_path(src_file):
    """`pulse/brace.wav` for a file under src; the bare name otherwise."""
    if under(src_file, SRC):
        rel = os.path.relpath(src_file, SRC)
    else:
        rel = os.path.basename(src_file)
    return os.path.splitext(rel)[0].replace("\\", "/") + ".wav"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("paths", nargs="*")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--out")
    ap.add_argument("--install", action="store_true")
    ap.add_argument("--no-normalize", action="store_true")
    ap.add_argument("--peak", type=float, default=-1.0)
    ap.add_argument("--trim-db", default="-50")
    ap.add_argument("--loop", nargs="?", const="0", default=None)
    ap.add_argument("--check")
    ap.add_argument("--rate", type=int, default=22050)
    args = ap.parse_args()

    out_root = os.path.abspath(args.out) if args.out else REPO_SOUND
    if under(out_root, SRC):
        die("refusing to write under the hand-made tier " + SRC)
    if args.rate not in (11025, 22050, 44100):
        die("--rate must be 11025, 22050 or 44100")
    trim_db = None if str(args.trim_db).lower() == "none" else float(args.trim_db)

    for src_file in collect(args):
        rel = engine_path(src_file)
        dst = os.path.join(out_root, rel.replace("/", os.sep))
        if under(dst, SRC):
            die("refusing to write under " + SRC)
        os.makedirs(os.path.dirname(dst), exist_ok=True)

        tmp = tempfile.NamedTemporaryFile(suffix=".wav", delete=False)
        tmp.close()
        try:
            run_ffmpeg(src_file, tmp.name, args.rate)
            rate, samples = read_int16(tmp.name)
        finally:
            os.unlink(tmp.name)
        n0 = len(samples)

        cut = 0
        if trim_db is not None:
            samples, cut = trim_leading(samples, trim_db)
        gain = 1.0
        if not args.no_normalize:
            samples, gain = normalise(samples, args.peak)

        loop_start = None
        if args.loop is not None:
            loop_start = int(round(float(args.loop) * rate))
            if loop_start >= len(samples):
                die("--loop start is past the end of the sound")

        write_wav(dst, rate, samples, loop_start)
        peak = max((abs(s) for s in samples), default=0) / 32767.0
        print("%s -> %s: mono 16-bit %d Hz, %.3f s (trimmed %.3f s), peak %.1f dBFS, gain %+.1f dB%s" % (
            src_file, dst, rate, len(samples) / rate, cut / rate, lin_to_db(peak), lin_to_db(gain),
            ", loops from %.3f s" % (loop_start / rate) if loop_start is not None else ""))
        print("  ART_DEBT line to fill in: | <event> | `%s` | made by Andrei, FL Studio, %s | plays beside <neighbour> within <t> s |"
              % (rel, "looping" if loop_start is not None else "one-shot"))

        if args.install:
            inst = os.path.join(INSTALL_SOUND, rel.replace("/", os.sep))
            os.makedirs(os.path.dirname(inst), exist_ok=True)
            shutil.copyfile(dst, inst)
            print("  installed -> " + inst)

        if args.check:
            check(dst, resolve_neighbour(args.check))


if __name__ == "__main__":
    main()
