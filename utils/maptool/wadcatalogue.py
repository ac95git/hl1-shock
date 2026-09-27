"""Builds a browsable catalogue of the textures a map can use: one local HTML page and a PNG per texture.

    python wadcatalogue.py [OUT_DIR]

OUT_DIR defaults to utils/maptool/catalogue/ (git-ignored: everything in it is regenerated from the
WADs). Open OUT_DIR/index.html in a browser. Every texture gets a plain-English family name decoded
from its cryptic prefix (FAMILIES below), flags for the prefixes the compiler and engine read
(animated, toggled, random-tiled, masked, liquid, light, scrolling), an average colour for the
colour filter, and a count of the faces in each map under maps/ that use it. Every texture has a
copy button for its name; clicking the texture shows it tiled 3x3, which is how it will read on a
wall, with a second copy button.

The WADs are the four `mines1.map` loads, read with wadpack.py's reader. A family label marked
with `?` is a guess from the pictures, not from Valve.
"""

import colorsys
import glob
import html
import json
import os
import re
import sys

from PIL import Image

import wadpack

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
VALVE = r"D:\apps\steam\steamapps\common\Half-Life\valve"
WADS = ["halflife", "liquids", "xeno", "decals"]

# Chapter codes, as Valve named its maps: cXaY is chapter X, map Y.
CHAPTERS = [
    ("c0a0", "Black Mesa Inbound (the tram ride)"),
    ("c1a0", "Anomalous Materials"),
    ("c1a1", "Unforeseen Consequences"),
    ("c1a2", "Office Complex"),
    ("c1a3", "\"We've Got Hostiles\""),
    ("c1a4", "Blast Pit"),
    ("c2a1", "Power Up"),
    ("c2a2", "On A Rail"),
    ("c2a3", "Apprehension"),
    ("c2a4d", "Questionable Ethics"),
    ("c2a4e", "Questionable Ethics"),
    ("c2a4f", "Questionable Ethics"),
    ("c2a4g", "Questionable Ethics"),
    ("c2a4", "Residue Processing"),
    ("c2a5", "Surface Tension"),
    ("c3a1", "Forget About Freeman"),
    ("c3a2", "Lambda Core"),
    ("c4a1", "Xen / Interloper"),
    ("c4a2", "Gonarch's Lair"),
    ("c4a3", "Nihilanth"),
    ("ca1x", "Anomalous Materials test chamber ?"),
]

# Stem -> plain-English family. Checked in order, first match wins, against the name with its
# special prefix stripped. Keep the longer stems above the shorter ones they start with.
FAMILIES = [
    ("aaatrigger", "Tool: trigger volume (invisible in game)"),
    ("clip", "Tool: clip (blocks players, invisible)"),
    ("origin", "Tool: origin (rotation centre of a brush entity)"),
    ("sky", "Tool: sky (the skybox shows through)"),
    ("null", "Tool: null (face is removed)"),
    ("invisible", "Tool: invisible"),
    ("translucent", "Tool: translucent"),
    ("scroll", "Scrolling (conveyor, flowing liquid)"),
    ("sunbeam", "Effect: sunbeam"),
    ("laser", "Effect: laser curtain"),
    ("babfl", "Floors ?"),
    ("signc1a", "Signs: chapter 1"),
    ("signc2a", "Signs: chapter 2"),
    ("signc3a", "Signs: chapter 3"),
    ("c3a2sign", "Signs: Lambda Core"),
    ("sign", "Signs and labels"),
    ("exit", "Signs: exit"),
    ("ventsign", "Signs: vent"),
    ("poster", "Posters"),
    ("pinup", "Posters: pin-ups"),
    ("picture", "Pictures and framed art"),
    ("paper", "Paper, notices"),
    ("lab", "Lab: Black Mesa research walls, floors, panels"),
    ("fifties", "Fifties: the old wing (tiles, wood panels, office)"),
    ("fifts", "Fifties: the old wing (tiles, wood panels, office)"),
    ("fities", "Fifties: the old wing (tiles, wood panels, office)"),
    ("fift", "Fifties: the old wing (tiles, wood panels, office)"),
    ("fif", "Fifties: the old wing (tiles, wood panels, office)"),
    ("out", "Outdoor: rock, cliff, dirt, gravel, sand, grass"),
    ("tnnl", "Tunnel: service tunnels, concrete, rock, pipes"),
    ("silo", "Silo: the Blast Pit missile silo"),
    ("generic", "Generic: general-purpose metal, panels, trim"),
    ("gen", "Generic: general-purpose metal, panels, trim"),
    ("crete", "Concrete"),
    ("wetcrete", "Concrete, wet"),
    ("babtech", "Tech: computers, consoles, machine panels"),
    ("drkmtl", "Dark metal"),
    ("metal", "Metal"),
    ("steel", "Metal"),
    ("chromebar", "Metal: chrome"),
    ("trk", "Track: rail track and ties"),
    ("rustyrail", "Rails and rungs: rusty"),
    ("rustyrungs", "Rails and rungs: rusty"),
    ("troomrail", "Rails"),
    ("stairrail", "Rails"),
    ("rail", "Rails"),
    ("trrm", "Train room: the tram and its station ?"),
    ("trn", "Train ?"),
    ("train", "Train"),
    ("subway", "Subway: the tram tunnels"),
    ("tracklite", "Lights: track"),
    ("xcrate", "Crates: Xen-era wooden and metal crates ?"),
    ("bcrate", "Crates: big crates ?"),
    ("crate", "Crates"),
    ("cardbox", "Crates: cardboard boxes"),
    ("barrel", "Barrels"),
    ("nwbarrel", "Barrels"),
    ("freezer", "Freezer: cold storage"),
    ("frost", "Frost: frozen surfaces (Residue Processing)"),
    ("icicle", "Frost: icicles"),
    ("elev", "Elevator"),
    ("pfab", "Prefab: portable buildings"),
    ("prefab", "Prefab: portable buildings"),
    ("duct", "Ducts: air vents"),
    ("flickermon", "Monitors: flickering screens"),
    ("wet", "Wet surfaces"),
    ("flatbed", "Vehicles: flatbed truck"),
    ("tank", "Vehicles: tank"),
    ("tires", "Vehicles: tyres"),
    ("rocket", "Rocket (the satellite launch)"),
    ("ngin", "Engine, machinery"),
    ("button", "Buttons"),
    ("metswitch", "Buttons: switches"),
    ("fusebox", "Buttons: fuse box"),
    ("fuse", "Buttons: fuse box"),
    ("autolck", "Doors: automatic lock"),
    ("autolock", "Doors: automatic lock"),
    ("introdr", "Doors: the intro"),
    ("door", "Doors"),
    ("gate", "Doors: gate"),
    ("glassbrick", "Glass: glass brick"),
    ("glass", "Glass"),
    ("stain", "Stains ?"),
    ("stripes", "Hazard stripes"),
    ("ammo", "Pickup faces: ammo"),
    ("medkit", "Pickup faces: health charger"),
    ("recharge", "Pickup faces: suit charger"),
    ("base", "Base: Hazard Course walls and floors ?"),
    ("con", "Conveyor ?"),
    ("toxicgrn", "Toxic: green sludge"),
    ("watersilo", "Liquid: water"),
    ("waterf", "Liquid: waterfall"),
    ("water", "Liquid: water"),
    ("gwater", "Liquid: green water"),
    ("bwater", "Liquid: blue water"),
    ("rwater", "Liquid: red water"),
    ("radio", "Liquid: radioactive"),
    ("grate", "Grates and grating"),
    ("gratestep", "Grates and grating"),
    ("grid", "Grates and grating"),
    ("fence", "Fences"),
    ("chainlink", "Fences: chain-link"),
    ("bars", "Fences: bars"),
    ("ladder", "Ladders"),
    ("rope", "Rope"),
    ("conduit", "Pipes and conduit"),
    ("genpipe", "Pipes and conduit"),
    ("grass", "Plants: grass"),
    ("shrub", "Plants: shrubs"),
    ("vine", "Plants: vines"),
    ("light", "Lights: fixtures"),
    ("gymlight", "Lights: fixtures"),
    ("emerglight", "Lights: emergency"),
    ("litepanel", "Lights: panels"),
    ("panellite", "Lights: panels"),
    ("skkyli", "Lights: skylight"),
    ("spot", "Lights: spotlight colours"),
    ("clock", "Clocks"),
    ("table", "Furniture"),
    ("genlocker", "Furniture: lockers"),
    ("pepsi", "Vending machine"),
    ("hfloor", "Floors ?"),
    ("baseflr", "Floors"),
    ("tension", "Surface Tension"),
    ("stairs", "Stairs"),
    ("tech", "Xen: alien tech"),
    ("biotech", "Xen: biotech"),
    ("biosphinc", "Xen: biotech"),
    ("xeno", "Xen: rock, flesh, alien ground"),
    ("crys", "Xen: crystals"),
    ("fluid", "Xen: fluid"),
    ("pod", "Xen: pods"),
    ("razor", "Razor wire"),
    ("armsides", "Props ?"),
    ("handleside", "Props ?"),
    ("pillar", "Pillars"),
    ("air", "Props ?"),
    ("black", "Solid colour"),
    ("white", "Solid colour"),
    ("red", "Solid colour"),
    ("blue", "Solid colour"),
    ("yellow", "Solid colour"),
    ("grayscale", "Solid colour"),
    ("fade", "Solid colour ?"),
    ("fill", "Solid colour ?"),
]

DECAL_FAMILIES = [
    ("blood", "Blood"), ("yblood", "Blood: alien (yellow)"), ("bigblood", "Blood"),
    ("bloodhand", "Blood: handprints"), ("hand", "Blood: handprints"),
    ("shot", "Bullet holes"), ("bigshot", "Bullet holes"), ("dent", "Dents"), ("ding", "Dings"),
    ("scorch", "Scorch marks"), ("smscorch", "Scorch marks"), ("explos", "Scorch marks"),
    ("gaussshot", "Scorch marks"), ("crack", "Cracks"), ("fault", "Cracks"), ("break", "Cracks"),
    ("rust", "Grime: rust"), ("moss", "Grime: moss"), ("lime", "Grime: limescale"),
    ("oil", "Grime: oil"), ("drip", "Grime: drips"), ("water", "Grime: water stains"),
    ("spit", "Grime"), ("mommablob", "Grime"), ("gargstomp", "Grime"),
    ("tire", "Tyre marks"), ("foot", "Footprints"), ("graf", "Graffiti"), ("lambda", "Graffiti: lambda"),
    ("arrow", "Arrows (floor and wall)"), ("turn", "Arrows (floor and wall)"),
    ("caps", "Letters A-Z"), ("64#", "Numbers"), ("large#", "Numbers"), ("med#", "Numbers"),
    ("small#", "Numbers"), ("stripe", "Stripes"), ("pstripe", "Stripes: parking"),
    ("biohaz", "Symbols"), ("bproof", "Symbols"), ("target", "Symbols"), ("ammo", "Symbols"),
    ("crouch", "Symbols"), ("littleman", "Symbols"), ("marker", "Symbols"), ("moustache", "Symbols"),
    ("rotatescrape", "Grime: scrape"), ("reflect", "Effect: reflection"), ("247", "Symbols"),
    ("c1a", "Decals: chapter 1 set pieces"), ("c2a", "Decals: chapter 2 set pieces"),
]

PREFIXES = [
    ("+", "+0..+9 animated frames; +a..+j the frames a trigger toggles to"),
    ("-", "-0..-9 random tiling: the compiler scatters variants"),
    ("{", "Masked: the blue colour is see-through (needs render mode Solid on a brush entity)"),
    ("!", "Liquid: the brush becomes water, slime or lava by its contents"),
    ("~", "Emits light: has an entry in lights.rad, so a face of it lights the room"),
]

HUES = [(20, "red"), (45, "orange"), (70, "yellow"), (160, "green"), (260, "blue"), (330, "purple"), (361, "red")]


def split_prefix(name):
    """('+~', 'light1') for '+0~LIGHT1': the flag characters, then the stem. Prefixes stack."""
    n, flags = name.lower(), ""
    if n[:1] in "+-" and len(n) > 2:
        flags, n = n[:1], n[2:]
    while n[:1] in ("{", "!", "~") and n:
        flags, n = flags + n[:1], n[1:]
    return flags, n


def family(stem, table):
    for key, label in CHAPTERS:
        if stem.startswith(key) and table is FAMILIES:
            return "Chapter: " + label
    for key, label in table:
        if stem.startswith(key):
            return label
    return "Other ?"


def colour_bucket(rgb):
    r, g, b = (c / 255.0 for c in rgb)
    h, l, s = colorsys.rgb_to_hls(r, g, b)
    if l < 0.14:
        return "black"
    if s < 0.16 or l > 0.9:
        return "white" if l > 0.7 else "grey"
    if 0.05 < h * 360 < 45 and l < 0.45:
        return "brown"
    deg = h * 360
    for top, name in HUES:
        if deg < top:
            return name
    return "red"


def texture_image(lump, w, h, offs, decal):
    palpos = offs[3] + (w // 8) * (h // 8)
    pal = list(lump[palpos + 2:palpos + 2 + 768])
    idx = lump[offs[0]:offs[0] + w * h]
    if decal:
        colour = tuple(pal[765:768])
        alpha = Image.frombytes("L", (w, h), bytes(idx))
        im = Image.new("RGBA", (w, h), colour + (0,))
        im.putalpha(alpha)
        return im
    im = Image.frombytes("P", (w, h), bytes(idx))
    im.putpalette(pal)
    return im.convert("RGBA")


def map_usage():
    """{map name: {TEXTURE name upper: face count}} for every .map under maps/."""
    out = {}
    face = re.compile(r"^\s*\(.*?\)\s*\(.*?\)\s*\(.*?\)\s*(\S+)\s*\[", re.M)
    for path in sorted(glob.glob(os.path.join(REPO, "maps", "*.map"))):
        counts = {}
        with open(path, encoding="latin1") as fh:
            for m in face.finditer(fh.read()):
                t = m.group(1).upper()
                counts[t] = counts.get(t, 0) + 1
        out[os.path.splitext(os.path.basename(path))[0]] = counts
    return out


def build(out_dir):
    usage = map_usage()
    entries = []
    for wad in WADS:
        path = os.path.join(VALVE, wad + ".wad")
        img_dir = os.path.join(out_dir, wad)
        os.makedirs(img_dir, exist_ok=True)
        decal = wad == "decals"
        for i, (name, w, h, offs, _size, _pal, lump) in enumerate(wadpack.read(path)):
            if w is None:
                continue
            im = texture_image(lump, w, h, offs, decal)
            if name.startswith("{") and not decal:
                # The engine draws palette index 255 of a masked texture as see-through.
                idx = lump[offs[0]:offs[0] + w * h]
                im.putalpha(Image.frombytes("L", (w, h), bytes(0 if b == 255 else 255 for b in idx)))
            rel = "%s/%04d.png" % (wad, i)
            im.save(os.path.join(out_dir, rel))
            opaque = [p[:3] for p in im.resize((16, 16), Image.BOX).get_flattened_data() if p[3] > 128] or [(128, 128, 128)]
            avg = tuple(sum(c[k] for c in opaque) // len(opaque) for k in range(3))
            prefix, stem = split_prefix(name)
            used = {m: c[name.upper()] for m, c in usage.items() if name.upper() in c}
            entries.append({
                "n": name, "w": wad, "x": w, "y": h, "f": rel,
                "fam": family(stem, DECAL_FAMILIES if decal else FAMILIES),
                "p": prefix, "scroll": stem.startswith("scroll"),
                "c": "#%02x%02x%02x" % avg, "cb": colour_bucket(avg), "u": used,
            })
        print("%s: %d" % (wad, sum(1 for e in entries if e["w"] == wad)))
    page = PAGE.replace("/*DATA*/", json.dumps(entries, separators=(",", ":")))
    page = page.replace("/*PREFIXES*/", json.dumps(PREFIXES))
    page = page.replace("/*MAPS*/", json.dumps(sorted(usage)))
    with open(os.path.join(out_dir, "index.html"), "w", encoding="utf-8") as fh:
        fh.write(page)
    print("wrote " + os.path.join(out_dir, "index.html"))


PAGE = r"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Texture Catalogue</title>
<style>
:root{--bg:#16181b;--panel:#1f2226;--line:#33373d;--text:#e6e6e6;--dim:#9aa0a6;--accent:#f0a030}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:14px/1.4 system-ui,sans-serif}
header{position:sticky;top:0;z-index:2;background:var(--panel);border-bottom:1px solid var(--line);padding:10px 16px;display:flex;flex-wrap:wrap;gap:8px;align-items:center}
header input,header select{background:var(--bg);color:var(--text);border:1px solid var(--line);border-radius:4px;padding:6px 8px;font:inherit}
header input[type=search]{width:260px}
label{color:var(--dim);display:flex;gap:4px;align-items:center}
.sw{width:22px;height:22px;border-radius:50%;border:2px solid var(--line);cursor:pointer}
.sw.on{border-color:var(--accent)}
#count{color:var(--dim);margin-left:auto}
#legend{padding:8px 16px;color:var(--dim);font-size:12px;border-bottom:1px solid var(--line)}
#legend b{color:var(--text);font-family:monospace}
main{padding:12px 16px}
h2{font-size:15px;margin:18px 0 6px;color:var(--accent);font-weight:600}
h2 small{color:var(--dim);font-weight:400}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(120px,1fr));gap:8px}
.card{background:var(--panel);border:1px solid var(--line);border-radius:4px;padding:4px;cursor:pointer}
.card:hover{border-color:var(--accent)}
.card img{width:100%;aspect-ratio:1;object-fit:contain;image-rendering:pixelated;background:#555 repeating-conic-gradient(#444 0 25%,#555 0 50%) 0 0/16px 16px;display:block}
.card .n{font-family:monospace;font-size:12px;word-break:break-all;margin-top:3px}
.card .m{color:var(--dim);font-size:11px}
.card .u{color:var(--accent);font-size:11px}
#big{position:fixed;inset:0;background:rgba(0,0,0,.85);display:none;z-index:5;align-items:center;justify-content:center;flex-direction:column;gap:10px;padding:16px}
#big.on{display:flex}
#tile{width:min(90vw,70vh);aspect-ratio:1;background-repeat:repeat;image-rendering:pixelated;border:1px solid var(--line)}
#bigname{font-family:monospace;font-size:18px}
#bigmeta{color:var(--dim);text-align:center}
.cp{float:right;background:var(--bg);color:var(--dim);border:1px solid var(--line);border-radius:3px;font:11px system-ui;padding:1px 6px;cursor:pointer}
.cp:hover{color:var(--accent);border-color:var(--accent)}
#bigcp{font-size:14px;padding:6px 14px;float:none}
.cp.ok{color:#6c6;border-color:#6c6}
</style></head><body>
<header>
<input type="search" id="q" placeholder="Search name or family (e.g. rock, rust, c2a2)">
<select id="wad"><option value="">All WADs</option><option>halflife</option><option>liquids</option><option>xeno</option><option>decals</option></select>
<select id="fam"><option value="">All families</option></select>
<select id="pre"><option value="">Any prefix</option><option value="none">Plain</option><option value="+">+ animated/toggle</option><option value="-">- random tile</option><option value="{">{ masked</option><option value="!">! liquid</option><option value="~">~ light</option></select>
<select id="map"><option value="">Any use</option></select>
<span id="sws"></span>
<label><input type="checkbox" id="grp" checked> group by family</label>
<span id="count"></span>
</header>
<div id="legend"></div>
<main id="out"></main>
<div id="big"><div id="bigname"></div><button class="cp" id="bigcp">Copy name</button><div id="tile"></div><div id="bigmeta"></div></div>
<script>
const D=/*DATA*/, P=/*PREFIXES*/, MAPS=/*MAPS*/;
const COLS={black:'#111',grey:'#888',white:'#eee',brown:'#6b4a2e',red:'#b33',orange:'#d80',yellow:'#cc3',green:'#4a4',blue:'#36c',purple:'#94c'};
let col='';
const $=id=>document.getElementById(id);
$('legend').innerHTML='Prefixes: '+P.map(([k,v])=>'<b>'+k+'</b> '+v).join(' &nbsp;·&nbsp; ')+' &nbsp;·&nbsp; <b>scroll…</b> scrolls on a func_conveyor. <b>copy</b> puts a name on the clipboard; click a texture to see it tiled. A family ending in ? is a guess.';
[...new Set(D.map(e=>e.fam))].sort().forEach(f=>$('fam').add(new Option(f,f)));
MAPS.forEach(m=>$('map').add(new Option('used in '+m,m)));
$('map').add(new Option('unused in any map','-'));
$('sws').innerHTML=Object.entries(COLS).map(([k,v])=>`<span class="sw" title="${k}" data-c="${k}" style="background:${v}"></span>`).join(' ');
$('sws').onclick=e=>{const c=e.target.dataset.c;if(!c)return;col=col===c?'':c;document.querySelectorAll('.sw').forEach(s=>s.classList.toggle('on',s.dataset.c===col));draw()};
const esc=s=>s.replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
function card(e,i){const u=Object.entries(e.u).map(([m,c])=>m+' ×'+c).join(', ');
 return `<div class="card" data-i="${i}"><img loading="lazy" src="${e.f}" alt=""><div class="n">${esc(e.n)}</div><div class="m"><button class="cp" data-i="${i}">copy</button>${e.x}×${e.y} · ${e.w}</div>${u?`<div class="u">${esc(u)}</div>`:''}</div>`}
function draw(){
 const q=$('q').value.toLowerCase().trim(),w=$('wad').value,f=$('fam').value,p=$('pre').value,m=$('map').value;
 const hits=[];D.forEach((e,i)=>{
  if(w&&e.w!==w)return; if(f&&e.fam!==f)return; if(col&&e.cb!==col)return;
  if(p==='none'?e.p:p&&!e.p.includes(p))return;
  if(m==='-'?Object.keys(e.u).length:m&&!e.u[m])return;
  if(q&&!(e.n.toLowerCase().includes(q)||e.fam.toLowerCase().includes(q)))return;
  hits.push(i)});
 $('count').textContent=hits.length+' of '+D.length;
 if(!$('grp').checked){$('out').innerHTML='<div class="grid">'+hits.map(i=>card(D[i],i)).join('')+'</div>';return}
 const g={};hits.forEach(i=>(g[D[i].fam]=g[D[i].fam]||[]).push(i));
 $('out').innerHTML=Object.keys(g).sort().map(k=>`<h2>${esc(k)} <small>${g[k].length}</small></h2><div class="grid">${g[k].map(i=>card(D[i],i)).join('')}</div>`).join('');
}
['q','wad','fam','pre','map','grp'].forEach(id=>$(id).addEventListener('input',draw));
// navigator.clipboard can be missing or refused on a file:// page; the textarea path still works there.
function copy(text,btn){
 const done=()=>{const t=btn.textContent;btn.textContent='copied';btn.classList.add('ok');setTimeout(()=>{btn.textContent=t;btn.classList.remove('ok')},1000)};
 const old=()=>{const a=document.createElement('textarea');a.value=text;document.body.appendChild(a);a.select();document.execCommand('copy');a.remove();done()};
 if(navigator.clipboard&&window.isSecureContext)navigator.clipboard.writeText(text).then(done,old);else old()}
let shown=null;
$('out').onclick=ev=>{
 const b=ev.target.closest('button.cp');if(b){copy(D[b.dataset.i].n,b);return}
 const c=ev.target.closest('.card');if(!c)return;const e=D[c.dataset.i];shown=e;
 $('bigname').textContent=e.n;$('tile').style.backgroundImage=`url(${e.f})`;
 $('tile').style.backgroundSize=(100/3)+'% '+(100/3)+'%';
 const u=Object.entries(e.u).map(([m,c])=>m+': '+c+' faces').join(', ');
 $('bigmeta').textContent=`${e.fam} · ${e.x}×${e.y} · ${e.w}.wad${u?' · '+u:''} · click anywhere to close`;
 $('big').classList.add('on')};
$('big').onclick=ev=>{if(ev.target.id==='bigcp'){copy(shown.n,ev.target);return}$('big').classList.remove('on')};
document.onkeydown=e=>{if(e.key==='Escape')$('big').classList.remove('on')};
draw();
</script></body></html>
"""


if __name__ == "__main__":
    build(sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "catalogue"))
