"""Small deterministic centimetre mesh authoring library; no gameplay collision."""
import math
from pathlib import Path

MATERIALS = ('TowerPaint','DarkIron','WarmBrass','LanternGlass','AgedWood','WetRock','InteriorPlaster','CutStone','LensGlass','Lamp')

def polar(r,a,z):
    return (r*math.cos(a),r*math.sin(a),z)

def add(a,b): return tuple(x+y for x,y in zip(a,b))
def sub(a,b): return tuple(x-y for x,y in zip(a,b))
def mul(a,s): return tuple(x*s for x in a)
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def unit(a): return mul(a,1/max(1e-9,math.sqrt(sum(x*x for x in a))))

class Mesh:
    def __init__(self,name): self.name=name;self.vertices=[];self.faces=[]
    def face(self,points,mat,uv=None,smooth=False):
        start=len(self.vertices)+1
        if uv is None:
            n=unit(cross(sub(points[1],points[0]),sub(points[2],points[0])))
            axis=max(range(3),key=lambda i:abs(n[i])); axes=[i for i in range(3) if i!=axis]
            uv=[(p[axes[0]]/160,p[axes[1]]/160) for p in points]
        self.vertices.extend(zip(points,uv))
        for i in range(1,len(points)-1):
            normal=cross(sub(points[i],points[0]),sub(points[i+1],points[0]))
            if sum(x*x for x in normal)>1e-10:
                self.faces.append((mat,(start,start+i,start+i+1),smooth))
    def box(self,c,size,mat,yaw=0):
        a=math.radians(yaw);co=math.cos(a);si=math.sin(a)
        ps=[]
        for z in (-.5,.5):
            for y in (-.5,.5):
                for x in (-.5,.5):
                    xx=x*size[0];yy=y*size[1]
                    ps.append((c[0]+xx*co-yy*si,c[1]+xx*si+yy*co,c[2]+z*size[2]))
        for ids in ((0,2,3,1),(4,5,7,6),(0,1,5,4),(2,6,7,3),(0,4,6,2),(1,3,7,5)):
            self.face([ps[i] for i in ids],mat)
    def rod(self,a,b,r,mat,sides=8):
        n=unit(sub(b,a));u=unit(cross(n,(0,0,1) if abs(n[2])<.9 else (0,1,0)));v=cross(n,u)
        rings=[]
        for p in (a,b):rings.append([add(p,add(mul(u,r*math.cos(i*math.tau/sides)),mul(v,r*math.sin(i*math.tau/sides)))) for i in range(sides)])
        for i in range(sides):
            j=(i+1)%sides;self.face([rings[0][i],rings[0][j],rings[1][j],rings[1][i]],mat,smooth=True)
        self.face(list(reversed(rings[0])),mat);self.face(rings[1],mat)
    def ring(self,z,outer,inner,height,mat,n=96,start=0,end=math.tau):
        for i in range(n):
            a=start+(end-start)*i/n;b=start+(end-start)*(i+1)/n
            lo=z-height/2;hi=z+height/2
            self.face([polar(outer,a,lo),polar(outer,b,lo),polar(outer,b,hi),polar(outer,a,hi)],mat,smooth=True)
            if inner>0:self.face([polar(inner,b,lo),polar(inner,a,lo),polar(inner,a,hi),polar(inner,b,hi)],mat,smooth=True)
            self.face([polar(inner,a,hi),polar(outer,a,hi),polar(outer,b,hi),polar(inner,b,hi)],mat)
            self.face([polar(inner,b,lo),polar(outer,b,lo),polar(outer,a,lo),polar(inner,a,lo)],mat)
    def lathe(self,profile,mat,n=96):
        for (z,r),(zz,rr) in zip(profile,profile[1:]):
            for i in range(n):
                a=i*math.tau/n;b=(i+1)*math.tau/n
                self.face([polar(r,a,z),polar(r,b,z),polar(rr,b,zz),polar(rr,a,zz)],mat,
                          [(a*270/160,z/160),(b*270/160,z/160),(b*270/160,zz/160),(a*270/160,zz/160)],True)
    def write(self,folder):
        folder=Path(folder);folder.mkdir(parents=True,exist_ok=True)
        with (folder/(self.name+'.obj')).open('w') as f:
            f.write(f'mtllib {self.name}.mtl\no {self.name}\n')
            for (x,y,z),uv in self.vertices:f.write(f'v {x:.4f} {-y:.4f} {z:.4f}\n')
            for p,(u,v) in self.vertices:f.write(f'vt {u:.6f} {v:.6f}\n')
            last=None
            for mat,ids,smooth in sorted(self.faces,key=lambda face:(face[0],face[2])):
                if last is None or mat!=last[0]:f.write(f'usemtl {mat}\n')
                if last is None or smooth!=last[1]:f.write(f's {1 if smooth else "off"}\n')
                last=(mat,smooth)
                f.write('f '+' '.join(f'{i}/{i}' for i in reversed(ids))+'\n')
        (folder/(self.name+'.mtl')).write_text(''.join(f'newmtl {m}\nKd 0.4 0.4 0.4\nKs 0.1 0.1 0.1\nNs 12\n' for m in MATERIALS))
        return {'name':self.name,'vertices':len(self.vertices),'triangles':len(self.faces)}
