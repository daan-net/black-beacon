"""V0.5 optical works and machinery. Centimetres, Z up, visual meshes only.

Reuse the reviewed authoring library, never regenerate the V0.4 collision deck.
The lantern optical origin is world Z=1980; the gallery surface is Z=1570.
"""
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT.parent / 'VisualRebuildV04'))
from mesh import Mesh, polar, add
from export_obj import write_mesh

OUT = ROOT / 'Generated'
TAU = math.tau


def merge(m, part, transform=lambda p: p):
    start = len(m.vertices)
    m.vertices.extend((transform(p), uv) for p, uv in part.vertices)
    m.faces.extend((mat, tuple(i + start for i in ids), smooth)
                   for mat, ids, smooth in part.faces)


def wheel(m, c, outer, inner, depth, material, axis='x'):
    part = Mesh('ring')
    part.ring(0, outer, inner, depth, material, 64)
    merge(m, part, lambda p: add(c, (p[2], p[0], p[1]) if axis == 'x' else p))


def bolt(m, c, radius=2.1):
    m.rod(c, add(c, (0, 0, 2.4)), radius, 'WarmBrass', 6)


def casting(m, c, size, bevel, material):
    """Beveled casting, extruded along the engine shaft rather than a sharp cube."""
    x,y,z=[s/2 for s in size]
    section=[(-y,-z+bevel),(-y+bevel,-z),(y-bevel,-z),(y,-z+bevel),
             (y,z-bevel),(y-bevel,z),(-y+bevel,z),(-y,z-bevel)]
    ends=[[add(c,(side*x,yy,zz)) for yy,zz in section] for side in (-1,1)]
    for i in range(8):
        j=(i+1)%8;m.face([ends[0][i],ends[0][j],ends[1][j],ends[1][i]],material)
    m.face(list(reversed(ends[0])),material);m.face(ends[1],material)


def dial(m, c, r):
    # Faces towards the player at negative X. Raised ticks and needle, no fake label.
    wheel(m, c, r, r-1.5, 4, 'WarmBrass')
    m.rod(add(c, (-1.7, 0, 0)), add(c, (-2, 0, 0)), r-1.7, 'DialFace', 40)
    for i in range(11):
        a=-2.3+i*.46
        m.rod(add(c, (-2.3, math.sin(a)*r*.70, math.cos(a)*r*.70)),
              add(c, (-2.3, math.sin(a)*r*.86, math.cos(a)*r*.86)), .35, 'DarkIron', 4)
    m.rod(add(c, (-2.8, -r*.12, -r*.12)), add(c, (-2.8, r*.45, r*.50)), .55, 'DarkIron', 6)
    m.rod(add(c, (-3,0,0)), add(c, (-3.5,0,0)), 1.3, 'WarmBrass', 12)


def lantern():
    m = Mesh('SM_BB_LH_LanternRoom')
    # Sixteen slender cast columns with distinct shoes, caps and gasket rebates.
    for z, r, h in ((-241,264,14),(-231,258,6),(120,266,12),(143,289,18)):
        m.ring(z,r,r-14,h,'DarkIron')
    for i in range(16):
        a=i*TAU/16
        m.rod(polar(251,a,-405),polar(251,a,132),3.8,'DarkIron',12)
        for z in (-390,-241,111):
            m.box(polar(251,a,z),(13,17,24),'DarkIron',math.degrees(a))
            bolt(m,polar(260,a,z+10),2)
        # One transom gives tall glazing proportions; no horizontal bars at optical centre.
        m.rod(polar(251,a,-121),polar(251,a+TAU/16,-121),2.4,'DarkIron')
        for z in (-230,116):
            m.rod(polar(248,a,z),polar(248,a+TAU/16,z),1.4,'WarmBrass')
        # Curved spandrel braces beneath the cornice.
        for side in (-1,1):
            points=[polar(250,a+side*.11*j/6,84+38*math.sin(j/6*math.pi/2)) for j in range(7)]
            for p,q in zip(points,points[1:]):m.rod(p,q,2.6,'DarkIron')
    # Pane surfaces match the sixteen bays; hide obsolete eight-panel glazing at runtime.
    for i in range(16):
        a=i*TAU/16+.018;b=(i+1)*TAU/16-.018
        for lo,hi in ((-227,-125),(-117,112)):
            m.face([polar(250,a,lo),polar(250,b,lo),polar(250,b,hi),polar(250,a,hi)],'LanternGlass')
    profile=[(152,293),(167,286),(198,258),(230,211),(260,148),(280,74),(285,22)]
    m.lathe(profile,'RoofCopper')
    # Interior ceiling is a separate inward-facing skin.
    inner=Mesh('ceiling');inner.lathe([(z-5,r-3) for z,r in profile],'DarkIron')
    inner.faces=[(mat,tuple(reversed(ids)),s) for mat,ids,s in inner.faces];merge(m,inner)
    for i in range(16):
        a=i*TAU/16
        pts=[polar(r+1.5,a,z) for z,r in profile]
        for p,q in zip(pts,pts[1:]):m.rod(p,q,1.8,'DarkIron')
        m.rod(polar(248,a,125),polar(70,a,275),3,'DarkIron')
    m.lathe([(285,23),(307,23),(308,38),(316,40),(335,12)],'DarkIron',64)
    for i in range(12):m.rod(polar(24,i*TAU/12,287),polar(24,i*TAU/12,308),1.8,'WarmBrass')
    m.rod((0,0,334),(0,0,401),1.8,'WarmBrass')
    # A tapered cast pedestal and four arched feet replace the unsupported disc stack.
    m.ring(-401,64,0,15,'DarkIron')
    m.lathe([(-396,49),(-382,47),(-360,30),(-282,24),(-265,38),(-254,40)],'EngineEnamel',64)
    for i in range(4):
        a=(i+.5)*TAU/4
        m.rod(polar(58,a,-399),polar(25,a,-317),5,'DarkIron')
        bolt(m,polar(56,a,-390),3)
    m.ring(-251,52,20,10,'WarmBrass')
    m.rod((0,0,-248),(0,0,-111),14,'DarkIron',32)
    for z in (-244,-149,-116):m.ring(z,23,13,8,'WarmBrass',64)
    # Roller track supported by four ribs, with open underside visible from the stairs.
    m.ring(-104,83,64,8,'DarkIron')
    for i in range(8):
        a=i*TAU/8
        m.rod(polar(16,a,-143),polar(75,a,-108),3.5,'DarkIron')
        m.rod(polar(62,a,-96),polar(82,a,-96),4,'WarmBrass',12)
    # Exposed reduction drive, oil cups and speed governor beside pedestal.
    for z,r in ((-217,27),(-172,18)):
        wheel(m,(35,-32,z),r,r-6,7,'WarmBrass')
        m.rod((0,0,z),(11,-32,z),5,'DarkIron')
        m.rod((11,-32,z),(42,-32,z),4,'WarmBrass',16)
        for i in range(24):
            a=i*TAU/24
            m.box((35,-32+r*math.cos(a),z+r*math.sin(a)),(9,4,4),'DarkIron')
    m.rod((35,-32,-230),(35,-32,-109),3.5,'WarmBrass')
    m.rod((-28,0,-237),(-28,0,-189),2,'WarmBrass')
    # Existing control footprint: front-facing instruments at the unchanged interaction point.
    m.box((208,0,-361),(51,90,87),'EngineEnamel')
    m.box((180,0,-321),(5,98,34),'DarkIron')
    for y in (-26,26):dial(m,(176,y,-315),10)
    for y in (-28,0,28):
        m.rod((175,y,-344),(169,y,-344),3.2,'WarmBrass',12)
    m.rod((192,-55,-383),(192,-55,-314),3,'DarkIron')
    m.rod((192,-55,-315),(181,-55,-297),2,'WarmBrass')
    return m


def optics():
    m=Mesh('SM_BB_LH_FresnelRotor')
    # Compound catadioptric drum: upper/lower prism courses and four interrupted cages.
    for z in (-89,89):m.ring(z,77,65,7,'WarmBrass')
    for z in range(-78,79,7):
        radius=51+20*math.sqrt(max(0,1-(z/89)**2))
        m.lathe([(z-3.2,radius-4),(z-1,radius),(z+2.8,radius-1),(z+3.2,radius-4)],'LensGlass',128)
    for i in range(4):
        a=math.pi/4+i*TAU/4
        m.rod(polar(76,a,-91),polar(76,a,91),2.4,'WarmBrass',12)
        for z in (-80,0,80):m.box(polar(77,a,z),(8,8,6),'DarkIron',math.degrees(a))
    # A forward bull's-eye lens with actual concentric prism facets, normal along +X.
    lens=Mesh('bullseye')
    for j in range(9):
        r=5+j*6.7
        lens.lathe([(0,r),(5.5*(1-r/75),r+5.9),(-1.5,r+6.3)],'LensGlass',128)
    merge(m,lens,lambda p:(75+p[2],p[0],p[1]))
    wheel(m,(75,0,0),66,62,6,'WarmBrass')
    for a in (math.pi/4,3*math.pi/4,5*math.pi/4,7*math.pi/4):
        m.rod((75,64*math.cos(a),64*math.sin(a)),(60,70*math.cos(a),70*math.sin(a)),2,'DarkIron')
    m.ring(-93,86,61,6,'DarkIron')
    for i in range(64):
        a=i*TAU/64;m.box(polar(86,a,-93),(6,4,7),'WarmBrass',math.degrees(a))
    for i in range(6):
        a=i*TAU/6;m.rod(polar(12,a,-91),polar(65,a,-91),2.5,'DarkIron')
    m.box((-86,0,-78),(17,36,20),'DarkIron')
    # Burner/socket remains physical and dark when the power-driven arc is off.
    m.rod((0,0,-89),(0,0,-25),8,'DialFace',24)
    m.ring(-23,13,4,9,'WarmBrass',48)
    m.rod((0,0,24),(0,0,63),4,'DarkIron',24)
    return m


def arc():
    m=Mesh('SM_BB_LH_ArcSource')
    m.lathe([(-24,0),(-21,4),(-14,6),(14,6),(21,4),(24,0)],'Lamp',48)
    return m


def generator():
    m=Mesh('SM_BB_GeneratorWorks')
    # Fits the existing annex and collider, in real centimetres without inherited block scale.
    for y in (-51,51):
        m.box((0,y,-79),(230,12,14),'DarkIron')
        for x in (-93,83):
            m.box((x,y,-66),(22,24,12),'DarkIron');bolt(m,(x,y,-58),3)
    casting(m,(-29,0,-43),(100,67,50),8,'EngineEnamel')
    for x in (-60,-29,2):
        m.rod((x,0,-28),(x,0,37),21,'EngineEnamel',32)
        for z in (-12,-3,6,15,24,33):
            wheel(m,(x,0,z),23,18,2.8,'DarkIron',axis='z')
        casting(m,(x,0,42),(28,45,12),3,'EngineEnamel')
        for y in (-14,14):bolt(m,(x,y,48),2.3)
        m.rod((x,-24,31),(x,-34,61),3,'WarmBrass')
        m.rod((x,22,32),(x,36,49),4,'DarkIron')
    m.rod((-64,36,49),(20,36,49),7,'DarkIron',16)
    m.rod((20,36,49),(20,36,146),6,'DarkIron',16)
    m.rod((20,36,146),(100,36,146),6,'DarkIron',16)
    for z in (83,119):wheel(m,(20,36,z),9,5,4,'WarmBrass',axis='z')
    # Alternator: cast end bells, ventilation slots, cooling ribs, terminal box.
    m.rod((28,0,-26),(100,0,-26),31,'EngineEnamel',48)
    for x in (30,37,91,100):wheel(m,(x,0,-26),34,24,5,'DarkIron')
    for i in range(18):
        a=i*TAU/18
        m.rod((40,31*math.cos(a),-26+31*math.sin(a)),(89,31*math.cos(a),-26+31*math.sin(a)),1.9,'DarkIron')
    for i in range(10):
        a=i*TAU/10
        m.rod((103,12*math.cos(a),-26+12*math.sin(a)),(103,25*math.cos(a),-26+25*math.sin(a)),2,'DarkIron')
    m.box((64,0,12),(33,34,20),'EngineEnamel')
    m.rod((73,-18,12),(73,-43,12),3,'DarkIron')
    m.rod((73,-43,12),(105,-43,57),3,'DarkIron')
    # Switchboard stands on feet and is readable from the entrance.
    for y in (-41,41):m.box((114,y,-20),(9,9,120),'DarkIron')
    m.box((114,0,60),(18,105,86),'EngineEnamel')
    m.box((103,0,63),(3,96,76),'DarkIron')
    for y in (-25,25):dial(m,(100,y,79),15)
    for y in (-32,0,32):
        m.rod((99,y,42),(92,y,42),5,'DialFace',16)
        m.rod((90,y,42),(87,y,53),2,'WarmBrass')
    # Fuel filter, injection rail, oil sump covers and pipework.
    m.rod((-63,-39,-4),(-63,-39,24),7,'WarmBrass',24)
    m.rod((-63,-39,26),(4,-39,26),2,'WarmBrass')
    for x in (-60,-29,2):
        m.rod((x,-39,26),(x,-16,47),1.2,'WarmBrass')
        dial(m,(x,-35,-37),9)
    # The service light sits in front of the engine to reveal its casting and fins.
    lamp=Mesh('service_lamp')
    lamp.rod((0,0,130),(0,0,233),1.5,'DarkIron')
    lamp.lathe([(137,22),(144,10),(147,8)],'EngineEnamel',48)
    lamp.rod((0,0,119),(0,0,134),4,'Lamp',24)
    for i in range(6):
        a=i*TAU/6;lamp.rod(polar(8,a,116),polar(8,a,137),.8,'DarkIron')
    lamp.ring(116,9,6,2,'DarkIron',32)
    merge(m,lamp,lambda p:add(p,(-65,-30,0)))
    return m


def flywheel():
    m=Mesh('SM_BB_GeneratorFlywheel')
    wheel(m,(0,0,0),45,36,11,'DarkIron')
    wheel(m,(-6,0,0),44,40,3,'WarmBrass')
    m.rod((-11,0,0),(12,0,0),9,'DarkIron',24)
    for i in range(8):
        a=i*TAU/8
        m.rod((0,7*math.cos(a),7*math.sin(a)),(0,38*math.cos(a+.12),38*math.sin(a+.12)),3.2,'EngineEnamel')
    return m


def stairs():
    # Deliberately adopt the pre-pause source lamps, with tread/collision dimensions unchanged.
    import importlib.util
    spec=importlib.util.spec_from_file_location('v04_source',ROOT.parent/'VisualRebuildV04/generate.py')
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    m=base.stairs()
    m.faces=[('StairIron' if mat=='DarkIron' else mat,ids,s) for mat,ids,s in m.faces]
    # Riveted feet, wall-side pipe riser and landing fittings give the existing stair a construction language.
    for floor,(r,d) in enumerate(((190,130),(155,110),(130,90))):
        for step in range(1,28):
            a=-step*TAU/28;top=floor*520+(step+1)*520/28
            m.box(polar(r+d/2-15,a,top+1),(12,12,4),'DarkIron',math.degrees(a))
        z=(floor+1)*520
        m.ring(z-10,54,44,15,'WarmBrass',48)
    return m


def main():
    meshes=[lantern(),optics(),arc(),generator(),flywheel(),stairs()]
    audit=[]
    for m in meshes:
        audit.append(write_mesh(m,OUT))
        used={mat for mat,ids,s in m.faces}
        with (OUT/(m.name+'.mtl')).open('a') as f:
            for mat in sorted(used-{'TowerPaint','DarkIron','WarmBrass','LanternGlass','AgedWood','WetRock','InteriorPlaster','CutStone','LensGlass','Lamp'}):
                f.write(f'newmtl {mat}\nKd 0.25 0.25 0.25\n')
    (ROOT/'MeshAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
    print(json.dumps(audit,indent=2))


if __name__=='__main__':main()
