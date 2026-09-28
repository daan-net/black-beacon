"""Geometric regression checks independent of Unreal and render quality."""
import sys, unittest, math, importlib.util
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Art/Source/VisualRebuildV04'))
from generate import stairs,deck,tower,radius
v05_path=Path(__file__).resolve().parents[1]/'Art/Source/HeroEnvironmentV05/generate.py'
v05_spec=importlib.util.spec_from_file_location('v05_source',v05_path)
v05=importlib.util.module_from_spec(v05_spec)
v05_spec.loader.exec_module(v05)
hero_path=Path(__file__).resolve().parents[1]/'Art/Source/HeroLighthouseArtV01'
sys.path.insert(0,str(hero_path))
sys.path.insert(0,str(v05_path.parent))
hero_spec=importlib.util.spec_from_file_location('hero_exterior_source',hero_path/'generate.py')
hero=importlib.util.module_from_spec(hero_spec)
hero_spec.loader.exec_module(hero)

class Architecture(unittest.TestCase):
    def test_exterior_modules_do_not_enter_stairwell_or_gallery_route(self):
        for mesh in (hero.foundation(),hero.crown(),hero.window_dressings()):
            for p,uv in mesh.vertices:
                self.assertGreaterEqual(math.hypot(p[0],p[1]),radius(p[2])-26,
                    f'{mesh.name} enters the original wall interior')
                self.assertLess(p[2],1570,'New exterior mesh enters the gallery walkway')

    def test_portal_dressing_leaves_visible_doorway_open(self):
        # Geometry is noncolliding, but must not visually seal the working route.
        mesh=hero.foundation()
        for mat,ids,smooth in mesh.faces:
            ps=[mesh.vertices[i-1][0] for i in ids]
            if max(p[0] for p in ps)<270 or min(p[2] for p in ps)>=242:continue
            self.assertTrue(all(p[1]>=69.9 for p in ps) or all(p[1]<=-69.9 for p in ps),
                'Foundation projects into the 140cm visible portal')

    def test_stair_tops_preserve_all_gameplay_rises(self):
        m=v05.stairs()
        # Main tread boxes have two top triangles per rise at exactly the saved height.
        horizontal=[]
        for mat,ids,smooth in m.faces:
            ps=[m.vertices[i-1][0] for i in ids]
            if mat=='StairIron' and max(p[2] for p in ps)-min(p[2] for p in ps)<1e-6:
                horizontal.append(ps[0][2])
        for step in range(1,85):
            self.assertTrue(any(abs(z-step*520/28)<1e-6 for z in horizontal),step)

    def test_gallery_hatch_and_walkway(self):
        m=deck()
        # All floor vertices in the hatch's 0..200 degree sector stay outside r=210.
        for p,uv in m.vertices:
            a=math.degrees(math.atan2(p[1],p[0]))%360
            if .01<a<199.99:self.assertGreaterEqual(math.hypot(p[0],p[1]),209.99)
        self.assertTrue(all(1559.99<=p[2]<=1570.01 for p,uv in m.vertices))

    def test_gallery_clears_recorded_step_74_capsule(self):
        m=deck()
        # Actual failed climb: head touched Z=1560 at this XY position.
        centre=(-113.251,38.242)
        probes=[centre]+[(centre[0]+38*math.cos(i*math.tau/32),
                          centre[1]+38*math.sin(i*math.tau/32)) for i in range(32)]
        for mat,ids,smooth in m.faces:
            ps=[m.vertices[i-1][0] for i in ids]
            if any(abs(p[2]-1560)>1e-6 for p in ps):continue
            a,b,c=ps
            def orient(a,b,p):return (b[0]-a[0])*(p[1]-a[1])-(b[1]-a[1])*(p[0]-a[0])
            for probe in probes:
                signs=[orient(a,b,probe),orient(b,c,probe),orient(c,a,probe)]
                inside=all(v>=-1e-6 for v in signs) or all(v<=1e-6 for v in signs)
                self.assertFalse(inside,'Gallery overlaps the failed player capsule footprint')

    def test_tower_taper_clears_stair_outer_edges(self):
        for floor,outer in enumerate((255,210,175)):
            self.assertGreater(radius((floor+1)*520)-26,outer)

    def test_outer_tower_has_open_doorway(self):
        m=tower()
        for mat,ids,smooth in m.faces:
            if mat!='TowerPaint' or not smooth:continue
            ps=[m.vertices[i-1][0] for i in ids]
            c=tuple(sum(p[j] for p in ps)/3 for j in range(3))
            self.assertFalse(5<c[2]<235 and abs(math.atan2(c[1],c[0]))<.219)

if __name__=='__main__':unittest.main()
