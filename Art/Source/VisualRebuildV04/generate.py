"""V0.4 reference-guided architectural source. Run with Python 3, centimetres/Z-up.

All 84 stair top heights and centres match the saved collision after its existing
runtime taper correction. Openings are geometry, never painted-on windows.
"""
import math,random,json
from pathlib import Path
from mesh import Mesh,polar,add
OUT=Path(__file__).resolve().parents[2]/'Lighthouse'/'SourceMeshes'
T=math.tau

def radius(z): return 314-92*min(z,1560)/1560

def tower():
    m=Mesh('SM_BB_LH_TowerShell')
    # Explicit angular/vertical boundaries permit true reveals through a 26 cm wall.
    openings=[(-.22,.22,0,242)]
    for z in (380,880,1340):
        for a in (.75,2.32,3.89,5.46): openings.append((a-.105,a+.105,z-75,z+75))
    angles=sorted(set([i*T/128 for i in range(129)]+[a%T for lo,hi,b,t in openings for a in (lo,hi)]))
    heights=sorted(set([0,80,150,520,1040,1500,1560]+[h for lo,hi,b,t in openings for h in (b,t)]))
    def hole(a,z):
        return any((lo<a<hi or lo<a-T<hi) and b<z<t for lo,hi,b,t in openings)
    for z,zz in zip(heights,heights[1:]):
        for a,b in zip(angles,angles[1:]):
            if hole((a+b)/2,(z+zz)/2):continue
            for inner,mat in ((False,'TowerPaint'),(True,'InteriorPlaster')):
                r=radius(z)-(26 if inner else 0);rr=radius(zz)-(26 if inner else 0)
                pts=[polar(r,a,z),polar(r,b,z),polar(rr,b,zz),polar(rr,a,zz)]
                uv=[(a*270/160,z/160),(b*270/160,z/160),(b*270/160,zz/160),(a*270/160,zz/160)]
                m.face(list(reversed(pts)) if inner else pts,mat,list(reversed(uv)) if inner else uv,True)
    for lo,hi,b,t in openings:
        for a in (lo,hi):
            m.face([polar(radius(b),a,b),polar(radius(b)-26,a,b),polar(radius(t)-26,a,t),polar(radius(t),a,t)],'CutStone')
        for z in (b,t):m.face([polar(radius(z),lo,z),polar(radius(z),hi,z),polar(radius(z)-26,hi,z),polar(radius(z)-26,lo,z)],'CutStone')
        a=(lo+hi)/2;r=radius((b+t)/2);w=(hi-lo)*r
        # Stone voussoirs/jambs follow the opening, with a lintel and projecting sill.
        for side in (-1,1):
            for z in range(int(b+18),int(t),36):
                c=add(polar(radius(z)+2,a,z),(-math.sin(a)*side*(w/2+10),math.cos(a)*side*(w/2+10),0))
                m.box(c,(23,42,34),'CutStone',math.degrees(a)+90)
        for z in (b-8,t+10):m.box(polar(radius(z)+5,a,z),(w+48,50,18),'CutStone',math.degrees(a)+90)
        if b>0:
            # Recessed pane and metal crossbars. Open doorway has no pane.
            m.box(polar(r-17,a,(b+t)/2),(w,3,t-b),'LanternGlass',math.degrees(a)+90)
            for off in (-w/2,0,w/2):
                c=add(polar(r-10,a,(b+t)/2),(-math.sin(a)*off,math.cos(a)*off,0))
                m.box(c,(4,6,t-b),'DarkIron',math.degrees(a)+90)
            for z in (b,b+50,b+100,t):m.box(polar(radius(z)-10,a,z),(w,6,4),'DarkIron',math.degrees(a)+90)
    # Ring courses are hollow: no accidental ceilings across the stairwell.
    for z in (90,510,1030,1510):
        m.ring(z,radius(z)+8,radius(z)-1,20,'CutStone')
        m.ring(z+13,radius(z)+11,radius(z)+1,5,'DarkIron')
    # Battered plinth with an uninterrupted entry gap at positive X.
    for i in range(5):m.ring(15+i*27,344-i*5,303-i*3,26,'CutStone',96,.27,T-.27)
    # Buttresses stop below the taper, rather than piercing it as vertical strips.
    for i in range(8):
        a=(i+.5)*T/8
        m.rod(polar(331,a,20),polar(302,a,470),13,'CutStone',4)
    # Splayed portal and open timber leaf outside the traversable threshold.
    m.box((331,-113,118),(10,112,224),'AgedWood',-25)
    for z in (48,190):m.box((335,-112,z),(13,108,8),'DarkIron',-25)
    return m

def gallery():
    m=Mesh('SM_BB_LH_Gallery')
    m.lathe([(1400,226),(1460,249),(1500,288),(1522,335),(1540,378)],'CutStone')
    m.ring(1545,394,277,30,'DarkIron')
    # Last flight rises through the positive-Y hatch onto the existing landing.
    m.ring(1565,391,63,10,'DarkIron',112,math.radians(112),T)
    m.ring(1565,391,210,10,'DarkIron',40,0,math.radians(112))
    m.box((120,-35,1565),(110,120,10),'DarkIron')
    for i in range(16):
        a=i*T/16
        m.rod(polar(229,a,1400),polar(379,a,1532),8,'DarkIron')
        m.rod(polar(244,a,1500),polar(379,a,1532),6,'DarkIron')
        m.box(polar(287,a,1555),(170,9,12),'DarkIron',math.degrees(a))
    for z in (1585,1630,1680):m.ring(z,394,388,6,'DarkIron')
    for i in range(48):
        a=i*T/48;m.rod(polar(391,a,1558),polar(391,a,1684),3.2,'DarkIron')
    # Hollow service drum meets the lantern glazing; doorway aligns with the landing.
    for i in range(16):
        a=i*T/16;b=(i+1)*T/16
        if i in (0,15):continue
        m.face([polar(251,a,1570),polar(251,b,1570),polar(251,b,1740),polar(251,a,1740)],'TowerPaint')
        m.face([polar(237,b,1570),polar(237,a,1570),polar(237,a,1740),polar(237,b,1740)],'InteriorPlaster')
    m.ring(1740,266,234,20,'DarkIron')
    # External maintenance ladder is fixed to the taper, with proper standoff brackets.
    for y in (-26,26):m.rod((radius(200)+22,y,200),(radius(1450)+22,y,1450),3,'DarkIron')
    for z in range(220,1441,30):
        x=radius(z)+22;m.rod((x,-26,z),(x,26,z),2,'DarkIron')
        if z%150<30:
            for y in (-26,26):m.rod((x,y,z),(x-24,y,z),3,'DarkIron')
    return m

def deck():
    m=Mesh('SM_BB_LH_GalleryDeck')
    m.ring(1565,388,63,10,'DarkIron',112,math.radians(112),T)
    m.ring(1565,388,210,10,'DarkIron',40,0,math.radians(112))
    return m

def lantern():
    m=Mesh('SM_BB_LH_LanternRoom')
    # Local optical origin Z=1980; floor is -410. Full-height room, not floating cage.
    for z,r in ((-240,265),(133,270),(145,290)):m.ring(z,r,r-16,16,'DarkIron')
    for i in range(16):
        a=i*T/16
        m.rod(polar(251,a,-235),polar(251,a,140),5,'DarkIron')
        for z in (-116,8,125):m.rod(polar(252,a,z),polar(252,a+T/16,z),3,'DarkIron')
    m.lathe([(147,296),(161,294),(182,275),(212,239),(239,192),(260,130),(277,64),(281,18)],'DarkIron')
    for i in range(16):
        a=i*T/16
        pts=[polar(r+2,a,z) for z,r in ((160,292),(182,275),(212,239),(239,192),(260,130),(277,64))]
        for p,q in zip(pts,pts[1:]):m.rod(p,q,2,'WarmBrass')
    m.rod((0,0,278),(0,0,343),8,'DarkIron');m.rod((0,0,340),(0,0,390),2,'WarmBrass')
    # Fixed cast pedestal/roller bed; optical assembly is separate and rotates with the beam.
    m.ring(-395,84,0,26,'DarkIron');m.ring(-342,39,0,90,'DarkIron')
    m.ring(-288,76,20,20,'WarmBrass');m.ring(-270,84,35,12,'DarkIron')
    m.rod((0,0,-270),(0,0,-96),19,'DarkIron')
    m.ring(-100,100,22,13,'DarkIron')
    for i in range(24):
        a=i*T/24;m.box(polar(78,a,-275),(12,8,16),'WarmBrass',math.degrees(a))
    return m

def optics():
    m=Mesh('SM_BB_LH_FresnelRotor')
    for z in (-85,85):m.ring(z,76,64,8,'WarmBrass')
    for j in range(25):
        z=-72+j*6;r=48+22*math.sqrt(max(0,1-(z/83)**2))
        m.lathe([(z-2.8,r-5),(z,r),(z+2.8,r-5)],'LensGlass',96)
    for i in range(6):
        a=i*T/6;m.rod(polar(76,a,-88),polar(76,a,88),2.5,'WarmBrass')
    # Off-axis counterweight and drive teeth make rotation mechanically legible.
    m.box((-88,0,-78),(25,45,25),'DarkIron')
    m.ring(-91,91,45,8,'DarkIron')
    for i in range(36):m.box(polar(91,i*T/36,-91),(8,6,8),'WarmBrass',i*10)
    return m

def stairs():
    m=Mesh('SM_BB_LH_StairStructure')
    m.ring(5,284,0,10,'CutStone')
    m.box((310,0,15.57),(170,100,6),'CutStone')
    m.rod((0,0,10),(0,0,1540),45,'DarkIron',48)
    for z in range(50,1520,130):m.ring(z,50,43,9,'DarkIron',48)
    # Caged maintenance lamps and their conduit anchor the warm pools to fixtures.
    for z in (325,845,1365):
        m.rod((-167,-167,z-35),(-167,-167,z+35),3,'DarkIron')
        m.rod((-167,-167,z+20),(-125,-120,z+20),3,'DarkIron')
        m.box((-125,-120,z),(18,18,26),'Lamp')
        for off in (-11,11):m.rod((-125+off,-132,z-17),(-125+off,-132,z+17),1.8,'DarkIron')
        for zz in (z-17,z+17):m.box((-125,-120,zz),(29,28,5),'DarkIron')
    for floor,(r,d) in enumerate(((190,130),(155,110),(130,90))):
        points=[]
        for step in range(28):
            a=-step*T/28;top=floor*520+(step+1)*520/28
            # Exact collision footprint, but a consistent 6 cm tread and cast nosing.
            m.box(polar(r,a,top-3),(d,80,6),'DarkIron',math.degrees(a))
            m.box(add(polar(r,a,top-1),(-math.sin(a)*-37,math.cos(a)*-37,0)),(d,5,2),'WarmBrass',math.degrees(a))
            inner=r-d/2;outer=r+d/2-15
            m.rod(polar(45,a,top-19),polar(r+d/2-4,a,top-9),4,'DarkIron')
            m.rod(polar(45,a,top-48),polar(inner+10,a,top-9),3,'DarkIron')
            # Rail stays on the existing outer guard line; first tread remains open.
            if step>0:
                m.rod(polar(outer,a,top-3),polar(outer,a,top+90),2.4,'DarkIron')
                points.append((a,top,outer))
            if step%7==3:
                m.rod(polar(radius(top)-27,a,top-22),polar(r+d/2,a,top-9),4,'DarkIron')
        for (a,z,r0),(b,zz,r1) in zip(points,points[1:]):
            for lift in (45,90):m.rod(polar(r0,a,z+lift),polar(r1,b,zz+lift),2.5,'DarkIron')
        if floor<2:
            # Small radial bridge at the change of radius; no disc sealing the shaft.
            m.box((r-18,4,(floor+1)*520-4),(d+28,68,8),'DarkIron')
            m.rod((48,0,(floor+1)*520-70),(r+20,0,(floor+1)*520-10),6,'DarkIron')
    return m

def annex():
    m=Mesh('SM_BB_LH_AnnexDetails')
    # Saved collision shell is 3.2 x 4.2 m, not the old art's 8.4 m footprint.
    # Wall panels split around real windows; west doorway remains 120 cm clear.
    for x in (-160,160):
        if x<0:
            for y in (-135,135):m.box((x,y,140),(24,150,280),'TowerPaint')
            m.box((x,0,260),(24,120,40),'CutStone')
        else:
            for y in (-163,0,163):m.box((x,y,140),(24,94,280),'TowerPaint')
            for y in (-88,88):
                m.box((x,y,49),(24,56,98),'TowerPaint');m.box((x,y,254),(24,56,52),'TowerPaint')
                m.box((x-2,y,163),(3,56,130),'LanternGlass')
                for off in (-34,34):m.box((x+3,y+off,163),(34,12,152),'CutStone')
                for z in (94,163,232):m.box((x+4,y,z),(34,78,10),'DarkIron')
    for y in (-210,210):
        for x in (-117,117):m.box((x,y,140),(86,24,280),'TowerPaint')
        m.box((0,y,48),(148,24,96),'TowerPaint');m.box((0,y,252),(148,24,56),'TowerPaint')
        m.box((0,y,162),(148,3,128),'LanternGlass')
        for x in (-78,-26,26,78):m.box((x,y-4,162),(7,24,140),'DarkIron')
        for z in (92,162,230):m.box((0,y-5,z),(173,36,9),'CutStone')
    m.box((0,0,-5),(330,430,10),'CutStone')
    for y in (-220,220):
        points=[(-178,y,280),(178,y,280),(0,y,355)]
        m.face(points if y<0 else list(reversed(points)),'TowerPaint')
    # Gable boards, roof thickness, ridge and correctly sloping standing seams.
    for side in (-1,1):
        top=[(0,-237,360),(side*190,-237,280),(side*190,237,280),(0,237,360)]
        underside=[(0,237,351),(side*190,237,271),(side*190,-237,271),(0,-237,351)]
        m.face(top if side>0 else list(reversed(top)),'DarkIron')
        m.face(underside if side>0 else list(reversed(underside)),'AgedWood')
        for y in range(-230,231,32):m.rod((0,y,362),(side*190,y,282),1.6,'DarkIron')
        m.rod((side*191,-240,278),(side*191,240,278),6,'DarkIron')
        m.rod((side*186,204,278),(side*186,204,10),4,'DarkIron')
    m.rod((0,-240,362),(0,240,362),5,'DarkIron')
    for y in (-150,0,150):
        m.rod((-156,y,280),(0,y,342),5,'AgedWood');m.rod((0,y,342),(156,y,280),5,'AgedWood')
        m.rod((-150,y,279),(150,y,279),6,'AgedWood')
    for x in (-157,157):
        for y in (-206,206):
            for z in range(18,276,32):m.box((x,y,z),(34,36,30),'CutStone')
    for y in (-69,69):m.box((-174,y,119),(38,18,238),'CutStone')
    m.box((-177,0,244),(44,158,19),'CutStone')
    m.box((-208,-104,115),(10,106,222),'AgedWood',-38)
    for z in (35,185):m.box((-209,-105,z),(13,102,9),'DarkIron',-38)
    m.box((91,120,363),(49,48,144),'CutStone');m.box((91,120,440),(65,63,12),'DarkIron')
    m.rod((120,110,120),(120,110,390),9,'DarkIron');m.rod((120,110,390),(144,110,410),9,'DarkIron')
    for z in (70,200):m.rod((158,-180,z),(158,170,z),3,'DarkIron')
    m.box((135,-150,170),(22,64,90),'DarkIron')
    for z in range(135,205,12):m.box((120,-150,z),(3,48,4),'WarmBrass')
    # Exterior bulkhead lamps, with emissive inset and a metal protective cage.
    for p in ((-184,97,231),(0,-231,255)):
        m.box(p,(16,20,29),'DarkIron');m.box(add(p,(-2,-3,0)),(18,21,19),'Lamp')
    return m

def rock(m,c,scale,seed):
    rng=random.Random(seed);ph=[rng.uniform(0,T) for _ in range(4)]
    n=32;k=18;grid=[]
    for j in range(k+1):
        lat=-math.pi/2+math.pi*j/k;row=[]
        for i in range(n+1):
            a=T*i/n
            v=1+.14*math.sin(3*a+ph[0])*math.cos(lat*2)+.09*math.sin(7*a+ph[1]+lat*4)+.035*math.sin(13*a+lat*9+ph[2])
            # Sheared strata and lobes, not randomized triangular spikes.
            row.append((c[0]+scale[0]*(math.cos(lat)*math.cos(a)*v+.12*math.sin(lat)),c[1]+scale[1]*math.cos(lat)*math.sin(a)*v,c[2]+scale[2]*math.sin(lat)*(.94+.06*math.sin(a*4+ph[3]))))
        grid.append(row)
    for j in range(k):
        for i in range(n):m.face([grid[j][i],grid[j][i+1],grid[j+1][i+1],grid[j+1][i]],'WetRock',smooth=True)

def rocks():
    m=Mesh('SM_BB_LH_RockPlinth');rng=random.Random(404)
    for i in range(36):
        a=i*T/36
        # Keep entry, annex and the existing shore approach clear.
        if abs(math.sin(a))<.25 or -2.1<a-T<-1.0:continue
        r=rng.uniform(420,790);rock(m,polar(r,a,-80),(rng.uniform(80,170),rng.uniform(75,160),rng.uniform(80,160)),i)
    return m

def coast_rock():
    m=Mesh('SM_BB_CoastalRock');rock(m,(0,0,0),(100,95,115),440);return m

def main():
    meshes=[tower(),gallery(),deck(),lantern(),optics(),stairs(),annex(),rocks(),coast_rock()]
    audit=[m.write(OUT) for m in meshes]
    (Path(__file__).parent/'MeshAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
    print(json.dumps(audit,indent=2))
if __name__=='__main__':main()
