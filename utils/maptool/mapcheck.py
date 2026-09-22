"""The read-back for a J.A.C.K. export: what changed since a git ref, an optional region
listing, then install, compile with the four VHLT tools and summarise the log.

    mapcheck.py maps/<name>.map [--ref HEAD] [--region x0 y0 z0 x1 y1 z1]
                                 [--layout-only] [--no-compile]

--layout-only runs CSG and BSP alone (fullbright, seconds); the default runs all four.
The compilers run with the current directory on D:, because worldspawn's wad paths are
drive-relative. Paths for the tools and the install are the constants below.

Before anything else it compares the .map's timestamp with its .jmf in maps/jmf/ (the editor's
copy; the .map is the source of truth, docs/MAP_WORKFLOW.md): a .jmf newer than the .map means
Andrei saved and did not export; a .map newer than the .jmf means a text edit is not in the
editor yet and his next export would overwrite it.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

# Child processes write straight to the console; keep our own lines in order with theirs.
sys.stdout.reconfigure(line_buffering=True)

TOOLS = r'D:\Apps\J.A.C.K\halflife'
INSTALL = r'D:\Apps\steam\steamapps\common\Half-Life\topmod\maps'
COMPILE_CWD = 'D:\\'


def check_jmf(map_path):
    """The two ways the .map and its .jmf drift apart, from timestamps alone."""
    jmf = os.path.join(os.path.dirname(map_path), 'jmf',
                       os.path.splitext(os.path.basename(map_path))[0] + '.jmf')
    if not os.path.exists(jmf):
        return
    dt = os.stat(jmf).st_mtime - os.stat(map_path).st_mtime
    age = f'{abs(dt) / 3600:.1f} h' if abs(dt) >= 3600 else f'{abs(dt) / 60:.0f} min'
    if dt > 2:
        print(f'  WARNING: {os.path.basename(jmf)} is {age} newer than the .map: '
              'saved in J.A.C.K. and not exported? The diff below is against a stale map.')
    elif dt < -2:
        print(f'  WARNING: the .map is {age} newer than {os.path.basename(jmf)}: '
              'a text edit J.A.C.K. has not seen. Open the .map in J.A.C.K. and overwrite the .jmf '
              'before editing, or the next export loses it.')


def git_show(ref, relpath):
    out = subprocess.run(['git', 'show', f'{ref}:{relpath}'], capture_output=True)
    if out.returncode != 0:
        return None
    fd, tmp = tempfile.mkstemp(suffix='.map')
    with os.fdopen(fd, 'wb') as f:
        f.write(out.stdout)
    return tmp


def semantic_diff(ref, map_path):
    rel = os.path.relpath(map_path, subprocess.run(['git', 'rev-parse', '--show-toplevel'],
                                                   capture_output=True, text=True).stdout.strip())
    rel = rel.replace('\\', '/')
    old = git_show(ref, rel)
    print(f'=== changes since {ref} ({rel}) ===')
    if old is None:
        print(f'  not in {ref}: a new map, nothing to compare')
        return
    try:
        subprocess.run([sys.executable, os.path.join(HERE, 'mapsemdiff.py'), old, map_path], check=False)
    finally:
        os.unlink(old)


def region(map_path, box):
    print('=== brushes in the region ===')
    subprocess.run([sys.executable, os.path.join(HERE, 'brushes_near.py'), map_path] + [str(v) for v in box])


def compile_map(map_path, layout_only):
    name = os.path.splitext(os.path.basename(map_path))[0]
    dst = os.path.join(INSTALL, name + '.map')
    shutil.copyfile(map_path, dst)
    base = os.path.join(INSTALL, name)
    steps = [('hlcsg', ['-low', '-wadautodetect']), ('hlbsp', ['-low'])]
    if not layout_only:
        steps += [('hlvis', ['-low']), ('hlrad', ['-low'])]
    print(f'=== compile ({"CSG+BSP" if layout_only else "all four"}) -> {base}.bsp ===')
    t0 = time.time()
    for tool, flags in steps:
        r = subprocess.run([os.path.join(TOOLS, tool + '.exe')] + flags + [base],
                           cwd=COMPILE_CWD, capture_output=True, text=True)
        if r.returncode != 0:
            print(f'  {tool} exited {r.returncode}')
    print(f'  {time.time() - t0:.1f} s')
    summarise_log(base + '.log', base + '.bsp')


def summarise_log(log_path, bsp_path):
    if not os.path.exists(log_path):
        print('  no log written')
        return
    text = open(log_path, encoding='utf-8', errors='replace').read()
    lines = text.splitlines()
    leaks = [l for l in lines if 'LEAK' in l]
    errors = [l for l in lines if re.match(r'\s*Error', l)]
    warnings = [l.strip() for l in lines if l.strip().startswith('Warning')]
    kinds = {}
    for w in warnings:
        key = re.sub(r'\(.*', '', w).strip()          # drop the coordinates and the entity
        key = re.sub(r'\bat\b.*', '', key).strip()
        kinds[key] = kinds.get(key, 0) + 1
    lights = [l.strip() for l in lines if 'direct lights' in l]
    texlights = [l.strip() for l in lines if 'texlights' in l.lower()]
    print('=== log ===')
    print(f'  leaks: {len(leaks)}' + ('  <-- BSP failed; load the pointfile in J.A.C.K.' if leaks else ''))
    print(f'  errors: {len(errors)}')
    for e in errors[:10]:
        print('   ', e.strip())
    print(f'  warnings: {len(warnings)}')
    for k, n in sorted(kinds.items(), key=lambda kv: -kv[1]):
        print(f'    {n:4d}  {k}')
    for l in texlights[:2] + lights[:1]:
        print('  ', l)
    if os.path.exists(bsp_path):
        st = os.stat(bsp_path)
        fresh = time.time() - st.st_mtime < 600
        print(f'  bsp: {st.st_size} bytes, {"just written" if fresh else "STALE, not rewritten"}')
    else:
        print('  bsp: none')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('map')
    ap.add_argument('--ref', default='HEAD')
    ap.add_argument('--region', nargs=6, type=float, metavar=('x0', 'y0', 'z0', 'x1', 'y1', 'z1'))
    ap.add_argument('--layout-only', action='store_true')
    ap.add_argument('--no-compile', action='store_true')
    a = ap.parse_args()
    check_jmf(a.map)
    semantic_diff(a.ref, a.map)
    if a.region:
        region(a.map, a.region)
    if not a.no_compile:
        compile_map(a.map, a.layout_only)


if __name__ == '__main__':
    main()
