# The Skill map — the board, cell by cell

The 15×15 board as settled on 2026-09-15, region by region in one grill, then assembled. This is the
layout only; the rules are in [SKILL_TREE.md](SKILL_TREE.md#one-start-open-roads--settled-2026-09-15),
the look in [SKILL_PANEL.md](SKILL_PANEL.md). `skill_map.csv` beside this file is the same grid as data,
one cell per column, for whoever fills `k_SkillDefs`. **Nothing here is built yet**; the table in
`game_shared/skill_defs.h` still holds the 16-column layout of 2026-09-14.

Cells are `(col, row)`, both from 0, top-left origin. A node opens from any owned orthogonal neighbour;
an empty cell (`.`) is a wall. The suit at (7,7) is held from the start.

## The board

```
         SHINOBI (NW, Dash Module)      MEDICAL (N)                     STEALTH (NE, Night Vision)
       c0     c1     c2     c3     c4   c5     c6     c7     c8     c9   c10    c11    c12    c13    c14
 r0  [AirD]  D      .      D      D    Overh  H    [LastS]  H    Leech  C      .     Night   C    [Silnt]
 r1   D      .    Phase    .      D    H      .      .      .      H    C      C      C      .      C
 r2   D      D      D    Reach    D    H      H    MedEx    H      H    Phntm   .      C    CutHd   C
 r3  Reprs   .      D      .    SecW   .      .      H      .      .    .      .    SlipA    .      .
 r4   D      D      .      D    Recov  .      .      H      .      .    SoftS   C    Ambsh    .      C
         MELEE (W)                      HUB                             WEAPON SPECIALIST (E)
 r5  {GlsC}  S      .      S      .    .      .    Fort     .      .    B      B    Headh    B    Demol
 r6   S      .    Speed    S      S    Sure   H      H      H      .    .      .      B      .      B
 r7   S    Bkstb    S    Reach    S    Force  M    (SUIT)   B    Marks  B    QDraw    B      B    Mstry
 r8   S      .      .      .      .    A      A      A      A    ArmEx  .      .      B      .      B
 r9  [Clev]  S      S      S      .    F-Up   .    Batt     .      .    B      B    FastR    B    [Swap]
         ENERGY (SW)                    JUGGERNAUT (S)                  ALIEN (SE, alien Module)
 r10  E    EnDmg    .      .      E    A      .      A      .      A    N      .    HiveC    .      N
 r11  E      .      E      E      E    A      A      A      A    Ricoc  N      N      N    HiveR   N
 r12 Insul   .    EgonF    .      .    A      .      .      .    PWin   HiveA   .      N      .      .
 r13  E      E      E      .      .    A      .    [JMaj]  Matrx  PRech  N     Pack    N    Tethr   N
 r14  .      .    EnEff    E    [Ovrd]  A    PDsch  PRebd   MoK     .    .      .    Recal    N    [AMaj]
```

`[ ]` a Major, `{ }` the keystone, `( )` the suit. Stat nodes: **S** Melee Damage, **B** Bullet Damage,
**H** Healing (Medical) / Max Health (hub rows 6), **A** Max Armour, **E** Energy Damage, **D** Dash
Recovery, **C** Concealment, **N** Hornet Replenish, **M** Melee Damage (hub), and the hub's **B** at (8,7)
is Bullet Damage.

## Counts

| Region | Nodes | Of which Stat | Skills and Majors |
| --- | --- | --- | --- |
| Hub | 17 (the suit included) | 9 | Fortitude, Battery Capacity, Marksman, Melee Force, Sure Footing, Armor Expert, Follow-Up |
| Melee | 17 | 12 | Reach, Speed, Backstab, Cleave, Glass Cannon |
| Weapon Specialist | 19 | 12 | Quick Draw, Headhunter, Demolitions, Fast Reload, Mastery, Swap Surge |
| Medical | 14 | 10 | Med Expert, Overheal, Leech, Last Stand |
| Juggernaut | 18 | 10 | Ricochet, Pulse Window, Pulse Recharge, Defense Matrix, the Major, Matrix on Kill, Pulse Discharge, Pulse Rebound |
| Energy | 15 | 9 | Energy Damage, Insulation, Egon Focus, Energy Efficiency, Overdraw |
| Shinobi | 19 | 13 | Dash Reach, Dash Recovery, Second Wind, Reprisal, Phase, Air Dash |
| Stealth | 17 | 10 | Soft Step, Ambush, Phantom, Slip Away, Nightfall, Cut the Head, Silent Kill |
| Alien | 18 | 11 | Hive Capacity, Hive Replenish, Hive Attack Speed, Pack, Tether, Recall, the Major |
| **Board** | **154** (153 buyable) | 96 | 57 |

Against 100 findable points a thorough player owns about 65% of the buyable nodes; the 71% figure in
SKILL_TREE.md was computed on 140 and this board came out at 154. Both are the same decision.

## Prices, from the suit

Shortest road from a held suit, counting every node bought including the destination. The hub's rim
Minor on the way is one of them.

| Destination | Points | Road |
| --- | --- | --- |
| Melee Speed | 6 | M, Force, (4,7), Reach, (3,6), Speed |
| Backstab | 6 | M, Force, (4,7), Reach, (2,7), Backstab |
| Cleave | 9 | … Backstab, (0,7), (0,8), Cleave |
| Glass Cannon | 9 | … Backstab, (0,7), (0,6), Glass Cannon |
| Follow-Up | 4 | M, Force, (5,8), Follow-Up |
| Quick Draw | 4 | B, Marksman, (10,7), Quick Draw |
| Headhunter, Fast Reload | 7 | … Quick Draw, (12,7), (12,6), Headhunter |
| Mastery | 7 | … Quick Draw, (12,7), (13,7), Mastery |
| Swap Surge | 9 | … Mastery, (14,8), Swap Surge |
| Med Expert | 5 | H, Fortitude, (7,4), (7,3), Med Expert |
| Overheal, Leech | 9 | … Med Expert, (6,2), (5,2), (5,1), Overheal |
| Last Stand | 11 | … Overheal, (6,0), Last Stand |
| Ricochet | 6 | A, Battery, (7,10), (7,11), (8,11), Ricochet |
| Juggernaut Major | 10 | … Ricochet, Window, Recharge, Matrix, Major |
| Energy Damage | 11 | … Backstab, (0,7), (0,8), Cleave, (0,10), Energy Damage — through Cleave, the Gargantua road, long on purpose |
| Overdraw, via Juggernaut | 11 | A, Battery, (7,10), (7,11), (6,11), (5,11), (5,12), (5,13), (5,14), Overdraw |
| Air Dash | 12 | … Glass Cannon, (0,4), Reprisal, (0,2), (0,1), Air Dash |
| Silent Kill | 14 | … Quick Draw, (12,7), (12,6), Headhunter, Ambush, Slip Away, (12,2), (12,1), Nightfall, (13,0), Silent Kill |
| Alien Major | 15 | … Fast Reload, Hive Capacity, (12,11), (12,12), (12,13), Tether, (14,13), Major |

The other way into Energy, (1,9) → (1,10), is the same length: (1,9) is reached only from Cleave. The
Juggernaut door at (5,11) → (4,11) is 7 from the suit and leads to Egon Focus and Overdraw, not to Energy
Damage, so a Juggernaut who wants the energy roads still walks in from the top-right.

## Sizes

Settled 2026-09-15 for the 1:1 view: a **112 px step**, nodes of **36** (Stat pad), **50** (Minor), **62**
(Medium), **74** (Major), the suit's processor **92**. The board is 1680 px square; every screen pans,
a 1080p field shows about eight rows. **Four Skills keep the Major-sized frame** besides the eight
regional Majors: Weapon Mastery, Pulse Discharge, Pulse Rebound and Follow-Up. The Major frame therefore
means "a big Skill", and a region's end is told by position, not by the frame.

## The two fixes made while assembling

- **(5,8) became a Max Armour node.** As drawn, Follow-Up at (5,9) touched only Juggernaut's (5,10), so a
  Melee player reached the Melee × Juggernaut node only through the Juggernaut region. With (5,8) filled
  it is four from the suit.
- **Medical's (5,4) and (9,4) were dropped.** Each touched only the corner region beside it and nothing in
  Medical, since the hub's top corners are empty: dead ends that served nobody. Medical is 14 nodes.

## Doors

Where two regions actually connect: both border cells filled.

| Seam | Doors |
| --- | --- |
| Hub → Melee | Force (5,7) → (4,7); Sure Footing (5,6) → (4,6) |
| Hub → Medical | Fortitude (7,5) → (7,4) |
| Hub → Specialist | Marksman (9,7) → (10,7) |
| Hub → Juggernaut | Battery (7,9) → (7,10); Follow-Up (5,9) → (5,10) |
| Melee ↔ Shinobi | (0,5) Glass Cannon ↔ (0,4); (1,5) ↔ (1,4); (3,5) ↔ (3,4) |
| Melee ↔ Energy | (0,9) Cleave ↔ (0,10); (1,9) ↔ (1,10) Energy Damage |
| Medical ↔ Shinobi | (5,0) Overheal ↔ (4,0); (5,1) ↔ (4,1); (5,2) ↔ (4,2) |
| Medical ↔ Stealth | (9,0) Leech ↔ (10,0); (9,1) ↔ (10,1); (9,2) ↔ (10,2) Phantom |
| Specialist ↔ Stealth | (10,5) ↔ (10,4) Soft Step; (11,5) ↔ (11,4); (12,5) Headhunter ↔ (12,4) Ambush; (14,5) Demolitions ↔ (14,4) |
| Specialist ↔ Alien | (10,9) ↔ (10,10); (12,9) Fast Reload ↔ (12,10) Hive Capacity; (14,9) Swap Surge ↔ (14,10) |
| Juggernaut ↔ Energy | (5,10) ↔ (4,10); (5,11) ↔ (4,11); (5,14) ↔ (4,14) Overdraw |
| Juggernaut ↔ Alien | (9,10) ↔ (10,10); (9,11) Ricochet ↔ (10,11); (9,12) Pulse Window ↔ (10,12) Hive Attack Speed |

A door into a hidden region is a door onto blank pads until its Module is found; a door *from* a hidden
node (Pulse Window into Alien) is closed until the Pulse Module.

## Placement rules the board follows

- A region's Major sits in the cell farthest from its entries, in a corner where the empty cells leave one
  or two approaches.
- A region's entry Skill sits on the road from its door, never on the door cell itself, except where the
  door is the Skill (Soft Step, Overheal, Leech, Hive Capacity), which is a corner region's business.
- Border rows and columns are mostly empty so that doors are chosen, not incidental; a full border row
  (Specialist's top and bottom) hands the choice to the neighbour.
- Hidden nodes never sit on a road another Route needs: the Pulse block is in Juggernaut's far corner, and
  Overdraw's door is an armour node.
