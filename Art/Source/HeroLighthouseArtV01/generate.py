"""Additive exterior architecture in cm/Z-up. Never exports a gameplay mesh."""
import json
import math
import random
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / 'VisualRebuildV04'))
sys.path.insert(0, str(ROOT.parent / 'HeroEnvironmentV05'))
from mesh import Mesh, polar, add
from export_obj import write_mesh

TAU = math.tau


def ashlar(mesh, a, b, bottom, top, outer, inner, bevel=1.4):
    """Closed curved stone with recessed bed joints and a chamfered exposed face."""
    count = max(3, math.ceil((b-a)*outer/12))
    rings = []
    for r, lo, hi, inset in ((inner, bottom, top, 0),
                              (outer-bevel, bottom, top, 0),
                              (outer, bottom+bevel, top-bevel, bevel/outer)):
        angles = [a+inset+(b-a-2*inset)*i/count for i in range(count+1)]
        rings.append(([polar(r, angle, lo) for angle in angles],
                      [polar(r, angle, hi) for angle in angles]))
    for lower, upper in rings[2:]:
        for i in range(count):
            mesh.face([lower[i], lower[i+1], upper[i+1], upper[i]], 'Ashlar', smooth=True)
    for (low, high), (next_low, next_high) in zip(rings, rings[1:]):
        for i in range(count):
            mesh.face([low[i+1], low[i], next_low[i], next_low[i+1]], 'Ashlar')
            mesh.face([high[i], high[i+1], next_high[i+1], next_high[i]], 'Ashlar')
        mesh.face([low[0], high[0], next_high[0], next_low[0]], 'Ashlar')
        mesh.face([low[-1], next_low[-1], next_high[-1], high[-1]], 'Ashlar')
    low, high = rings[0]
    for i in range(count):
        mesh.face([low[i+1], low[i], high[i], high[i+1]], 'Ashlar')


def foundation():
    mesh = Mesh('SM_BB_Hero_Foundation')
    rng = random.Random(928)
    # Real staggered bed joints. Keep +/-0.285 radians clear to the existing portal.
    for row in range(8):
        bottom = 3 + row*32
        outer = 351-row*3.6
        edges = [.285]
        step = TAU/28
        next_angle = .285 + step*(.5 if row%2 else 1)
        while next_angle < TAU-.285:
            edges.append(next_angle)
            next_angle += step
        edges.append(TAU-.285)
        for a,b in zip(edges, edges[1:]):
            ashlar(mesh, a+.002, b-.002, bottom+.6, bottom+31.4,
                   outer+rng.uniform(-.65,.65), outer-15)
    # Coping has a sloping drip edge and separate capstones; the portal stays open.
    for i in range(28):
        a=.285+(TAU-.57)*i/28
        b=.285+(TAU-.57)*(i+1)/28
        ashlar(mesh,a+.002,b-.002,258,276,333,300,2)
    # Deep rusticated portal jambs follow existing opening edges. 140cm clear width.
    for side in (-1,1):
        for row in range(7):
            mesh.box((314, side*(91 if row%2 else 94), 18+row*33),
                     (58,42 if row%2 else 48,31), 'Ashlar')
    mesh.box((313,0,258),(62,246,30),'Ashlar')
    mesh.box((314,0,279),(72,262,10),'Ashlar')
    # Segmented relieving arch above the rectangular door; no change to door clearance.
    for i in range(11):
        a=math.pi*i/11+.012
        b=math.pi*(i+1)/11-.012
        inner,outer=92,119
        back=[(296,inner*math.cos(a),273+inner*math.sin(a)),
              (296,outer*math.cos(a),273+outer*math.sin(a)),
              (296,outer*math.cos(b),273+outer*math.sin(b)),
              (296,inner*math.cos(b),273+inner*math.sin(b))]
        front=[(325,p[1],p[2]) for p in back]
        mesh.face(front,'Ashlar')
        mesh.face(list(reversed(back)),'Ashlar')
        for j in range(4):
            k=(j+1)%4
            mesh.face([back[j],front[j],front[k],back[k]],'Ashlar')
    return mesh


def crown():
    mesh=Mesh('SM_BB_Hero_GalleryCorbels')
    # Cast-web brackets reach below the inherited gallery soffit; curved undersides
    # carry a broad shoe into the tower. Two skins plus flanges are closed geometry.
    profile=[(239,1355),(251,1355),(260,1390),(277,1420),(305,1446),
             (342,1463),(382,1470),(386,1528),(370,1528),(342,1510),
             (303,1480),(265,1441),(239,1415)]
    for i in range(16):
        a=(i+.5)*TAU/16
        def point(r,z,t):
            return (r*math.cos(a)-t*math.sin(a),r*math.sin(a)+t*math.cos(a),z)
        # Triangulate the concave profile with Blender-compatible fan-free ear clipping.
        from triangulate import triangulate
        for side in (-1,1):
            for tri in triangulate(profile):
                ps=[point(*profile[k],side*3.5) for k in tri]
                mesh.face(ps if side<0 else list(reversed(ps)),'DarkIron')
        for p,q in zip(profile,profile[1:]+profile[:1]):
            mesh.face([point(*p,-3.5),point(*q,-3.5),point(*q,3.5),point(*p,3.5)],'DarkIron')
        # Flanged edges expose a readable metal section rather than a thin diagonal rod.
        for p,q in zip(profile[:7],profile[1:7]):
            for t in (-6,6):mesh.rod(point(*p,t),point(*q,t),2.1,'DarkIron')
        mesh.box(polar(240,a,1390),(15,32,87),'DarkIron',math.degrees(a))
        for z in (1363,1416):
            for t in (-10,10):
                mesh.rod(point(248,z,t),point(252,z,t),2.8,'DarkIron',6)
    # Separate riveted fascia plates around the gallery's outer edge.
    for i in range(48):
        a=i*TAU/48+.002;b=(i+1)*TAU/48-.002
        mesh.ring(1542,398,393,36,'DarkIron',4,a,b)
        for z in (1530,1553):
            p=polar(398,(a+b)/2,z);q=polar(401,(a+b)/2,z)
            mesh.rod(p,q,2.2,'DarkIron',8)
    mesh.ring(1562,402,392,5,'DarkIron')
    mesh.ring(1522,400,390,5,'DarkIron')
    return mesh


def window_dressings():
    mesh=Mesh('SM_BB_Hero_WindowDressings')
    # Hood moulds, projecting drained sills and tapered stone consoles frame
    # the exact twelve existing openings; all work stays outside the shell.
    for z in (380,880,1340):
        r=314-92*z/1560
        for a in (.75,2.32,3.89,5.46):
            w=.21*r
            mesh.box(polar(r+15,a,z+97),(47,w+65,12),'Ashlar',math.degrees(a))
            mesh.box(polar(r+18,a,z-88),(57,w+62,11),'Ashlar',math.degrees(a))
            for side in (-1,1):
                centre=add(polar(r+10,a,z-104),(-math.sin(a)*side*(w/2+12),math.cos(a)*side*(w/2+12),0))
                mesh.box(centre,(27,15,24),'Ashlar',math.degrees(a))
    return mesh


def main():
    out=ROOT/'Generated'
    audit=[write_mesh(m,out) for m in (foundation(),crown(),window_dressings())]
    for entry in audit:
        with (out/(entry['name']+'.mtl')).open('a') as material:
            material.write('newmtl Ashlar\nKd 0.28 0.27 0.24\nKs 0.03 0.03 0.03\nNs 8\n')
    (ROOT/'MeshAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
    print(json.dumps(audit,indent=2))


if __name__=='__main__':main()
