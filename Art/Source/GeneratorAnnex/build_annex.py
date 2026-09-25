#!/usr/bin/env python3
"""Deterministically generate HERO QUALITY BLACK BEACON generator annex and machinery OBJ source meshes."""

from __future__ import annotations
import math
import random
from pathlib import Path
import sys

# Import the Mesh class from the lighthouse generator
sys.path.append(str(Path(__file__).resolve().parents[1] / "Lighthouse_Hero"))
from generate_lighthouse_obj import Mesh, RNG

OUT = Path(__file__).resolve().parents[2] / "GeneratorAnnex" / "SourceMeshes"

def architecture():
    m = Mesh("SM_BB_GeneratorAnnex_Arch")
    
    # Floor / Foundation (Damp Floor)
    m.box((0, 0, 75), (380, 780, 150), "DampFloor")
    
    # Walls (Thick Wet Concrete)
    for x in (-180, 180):
        m.box((x, 0, 225), (40, 780, 150), "WetConcrete") # Lower wall
        m.box((x, 0, 375), (40, 780, 150), "WetConcrete_Aged") # Upper wall
        
    for y in (-380, 380):
        # Leave a gap for the door on the West wall (x = -180)
        # We handle this by building the Y walls inside the X walls.
        m.box((0, y, 225), (320, 40, 150), "WetConcrete")
        m.box((0, y, 375), (320, 40, 150), "WetConcrete_Aged")
        
    # Roof Gables
    for y in (-380, 380):
        # We need a triangle, but box is fine for blockout, let's step it for better shape
        for i in range(5):
            w = 320 - i * 60
            m.box((0, y, 460 + i*15), (w, 30, 15), "WetConcrete_Aged")
            
    # Roof (Corrugated Roofing)
    # Instead of a flat box, let's make repeating ridges
    for side in (-1, 1):
        for i in range(40):
            # Roof slope
            rx = side * (50 + i * 4)
            rz = 530 - i * 3
            # Panel
            m.box((rx, 0, rz), (5, 840, 4), "Rust")
            m.box((rx + side*2, 0, rz+2), (5, 840, 4), "Rust")
            
    # Roof trim / Ridge cap
    m.cylinder((0, 0, 540), 15, 840, "OxidizedIron", 8)
    
    # Gutters and drainage
    for x in (-210, 210):
        m.box((x, 0, 410), (18, 840, 18), "ChippedPaint")
        # Hollow inside
        m.box((x, 0, 415), (14, 842, 14), "Rust")
        # Downspouts with brackets
        for y in (-390, 0, 390):
            m.cylinder((x, y, 250), 6, 320, "ChippedPaint", 8)
            # Brackets
            m.box((x - math.copysign(10, x), y, 300), (20, 8, 8), "Rust")
            m.box((x - math.copysign(10, x), y, 150), (20, 8, 8), "Rust")
            # Runoff stain decal under spout
            m.box((x, y, 80), (30, 30, 2), "Rust")
        
    # Heavy industrial door (West side, Y= -100)
    m.box((-190, -100, 250), (24, 140, 240), "OxidizedIron")
    # Door frame / trim
    m.box((-195, -175, 250), (34, 20, 250), "WetConcrete_Aged")
    m.box((-195, -25, 250), (34, 20, 250), "WetConcrete_Aged")
    m.box((-195, -100, 385), (34, 170, 20), "WetConcrete_Aged")
    # Hinges
    for z in (180, 250, 320):
        m.cylinder((-180, -165, z), 4, 15, "Rust", 6)
    # Handle
    m.cylinder((-205, -40, 240), 3, 40, "WarmBrass", 8)
    
    # Small weathered windows
    for y in (-200, 200):
        # Sill
        m.box((195, y, 250), (50, 90, 10), "WetConcrete")
        # Frame
        m.box((190, y, 290), (10, 80, 70), "ChippedPaint")
        # Glass panes
        m.box((190, y, 290), (4, 70, 60), "DirtyGlass")
        # Window bars
        for dy in (-20, 0, 20):
            m.cylinder((200, y + dy, 290), 2, 70, "Rust", 4)
        # Rust streaks under window
        m.box((202, y, 200), (2, 70, 90), "Rust")
            
    # Vents / Chimney
    m.box((150, 300, 520), (60, 60, 200), "WetConcrete_Aged")
    m.cylinder((150, 300, 640), 25, 40, "Rust", 8)
    m.cylinder((150, 300, 665), 30, 10, "ChippedPaint", 12)

    return m

def machinery():
    m = Mesh("SM_BB_GeneratorAnnex_Machinery")
    
    # Support frame (Skid)
    m.box((0, 50, 160), (140, 400, 20), "ChippedPaint")
    # Bolts on skid
    for dx in (-60, 60):
        for dy in (-120, 0, 120, 220):
            m.cylinder((dx, dy, 172), 3, 10, "Rust", 6)
            
    # Engine Block (Oily and Oxidized)
    # Base pan
    m.box((0, 100, 190), (100, 200, 40), "OilyMetal")
    # Main block
    m.box((0, 100, 250), (90, 190, 80), "OxidizedIron")
    # Cooling fins
    for z in range(215, 285, 10):
        m.box((0, 100, z), (95, 195, 4), "OxidizedIron")
    # Cylinder heads
    for y in (50, 100, 150):
        m.cylinder((0, y, 310), 25, 40, "ChippedPaint", 16)
        # Spark plugs / injectors
        m.cylinder((0, y, 335), 4, 15, "WarmBrass", 6)
        
    # Alternator
    m.cylinder((0, -80, 230), 55, 120, "ChippedPaint", 24)
    # Alternator cooling ribs
    for i in range(12):
        angle = math.tau * i / 12
        m.box((math.cos(angle)*50, -80, math.sin(angle)*50 + 230), (10, 110, 10), "OxidizedIron", math.degrees(angle))
        
    # Flywheel and belts
    m.cylinder((0, -10, 230), 65, 20, "Rust", 24)
    # Spokes
    for i in range(4):
        angle = math.tau * i / 4
        m.box((math.cos(angle)*30, -10, math.sin(angle)*30 + 230), (10, 15, 60), "Rust", math.degrees(angle))
    
    # Pulleys and Belt
    m.cylinder((0, 210, 220), 20, 15, "Rust", 16) # Lower pulley
    m.cylinder((0, 210, 310), 15, 15, "Rust", 16) # Upper pulley
    # Belt (approximated with boxes)
    m.box((-17, 210, 265), (4, 10, 90), "OilyMetal", 10)
    m.box((17, 210, 265), (4, 10, 90), "OilyMetal", -10)
    
    # Exhaust manifold and pipes
    for y in (50, 100, 150):
        m.cylinder((60, y, 290), 10, 40, "Rust", 12)
        m.cylinder((75, y, 290), 14, 5, "OxidizedIron", 12) # Flange
    # Collector pipe
    m.cylinder((85, 100, 290), 12, 160, "Rust", 12)
    # Upward exhaust
    m.cylinder((85, 190, 400), 12, 220, "Rust", 12)
    # Muffler
    m.box((85, 190, 420), (35, 35, 80), "Rust")
    
    # Fuel tank (mounted on frame)
    m.box((-100, 220, 350), (70, 120, 90), "ChippedPaint")
    # Tank straps
    m.box((-100, 180, 350), (72, 5, 92), "Rust")
    m.box((-100, 260, 350), (72, 5, 92), "Rust")
    # Cap
    m.cylinder((-100, 220, 400), 10, 10, "WarmBrass", 12)
    # Fuel line (multiple cylinders)
    m.cylinder((-100, 220, 300), 4, 20, "WarmBrass", 6)
    m.cylinder((-50, 220, 290), 4, 100, "WarmBrass", 6, yaw=90)
    
    # Breaker Cabinet / Control Panel
    m.box((140, -50, 280), (20, 140, 200), "ChippedPaint")
    # Door slightly open
    m.box((128, -80, 280), (4, 70, 190), "ChippedPaint", 15)
    m.box((128, -20, 280), (4, 70, 190), "ChippedPaint")
    
    # Inside the panel (Wiring/Switches)
    m.box((132, -80, 280), (4, 60, 170), "OilyMetal")
    for z in range(220, 340, 20):
        m.box((130, -80, z), (5, 40, 10), "OxidizedIron")
        
    # Large Gauges
    for y in (-20, -40):
        m.cylinder((125, y, 350), 12, 5, "WarmBrass", 16)
        m.cylinder((124, y, 350), 10, 1, "DirtyGlass", 16)
    
    # Conduit / Wiring running along walls
    for y in (-150, 0, 150):
        m.cylinder((155, y, 400), 4, 300, "Rust", 6, yaw=90)
        # Wall mounts
        m.box((150, y, 400), (10, 10, 10), "OxidizedIron")
        
    # Oil staining on the floor under the engine
    m.box((0, 100, 151), (140, 260, 2), "OilyMetal")
    
    return m

def props():
    m = Mesh("SM_BB_GeneratorAnnex_Props")
    
    # Workbench
    m.box((-120, -300, 240), (80, 200, 10), "AgedWood")
    for dx in (-35, 35):
        for dy in (-90, 90):
            m.box((-120 + dx, -300 + dy, 195), (10, 10, 90), "OxidizedIron")
            
    # Shelves
    m.box((-120, -300, 330), (60, 180, 5), "AgedWood")
    m.box((-120, -300, 380), (60, 180, 5), "AgedWood")
    
    # Barrels/Cans (Oily and rusted)
    for p in ((-120, 320), (-80, 340), (-140, 350)):
        m.cylinder((p[0], p[1], 195), 25, 90, "ChippedPaint", 16)
        # Ribs on barrel
        m.torus(170, 25, 2, "Rust", 16, 4)
        m.torus(220, 25, 2, "Rust", 16, 4)
        m.cylinder((p[0], p[1], 240), 10, 5, "WarmBrass", 8)
        # Oil stain under barrels
        m.box((p[0], p[1], 151), (40, 40, 2), "OilyMetal")
        
    # Crates
    m.box((120, 300, 180), (60, 60, 60), "AgedWood")
    m.box((120, 300, 240), (50, 50, 50), "AgedWood")
    m.box((130, 230, 175), (50, 80, 50), "AgedWood")
    
    # Tools / Clutter on workbench
    m.box((-120, -320, 248), (15, 5, 5), "Rust")
    m.box((-110, -280, 248), (5, 25, 5), "OilyMetal")
    
    # Industrial practical lamps (hanging with detailed cages)
    for y in (-200, 0, 200):
        # Cable
        m.cylinder((0, y, 500), 2, 100, "OilyMetal", 6)
        # Shade
        m.cylinder((0, y, 450), 25, 15, "ChippedPaint", 16, top_radius=10, bottom_radius=25)
        # Bulb
        m.cylinder((0, y, 440), 8, 12, "DirtyGlass", 8)
        # Cage
        for r in range(4):
            angle = math.tau * r / 4
            m.box((math.cos(angle)*15, y+math.sin(angle)*15, 435), (2, 2, 25), "Rust", math.degrees(angle))
        
    return m

# Add yaw support to Mesh class monkey-patch
def patch_mesh():
    original_cylinder = Mesh.cylinder
    def cylinder(self, center, radius, height, material, sides=32, top_radius=None, bottom_radius=None, start=0.0, yaw=0.0):
        x, y, z = center
        rb = radius if bottom_radius is None else bottom_radius
        rt = radius if top_radius is None else top_radius
        lower, upper = [], []
        ca, sa = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
        for i in range(sides):
            a = math.radians(start) + math.tau * i / sides
            lx, ly, lz = math.cos(a) * rb, math.sin(a) * rb, -height * 0.5
            ux, uy, uz = math.cos(a) * rt, math.sin(a) * rt, height * 0.5
            # Apply yaw
            lower.append(self.v((x + lx*ca - lz*sa, y + ly, z + lx*sa + lz*ca)))
            upper.append(self.v((x + ux*ca - uz*sa, y + uy, z + ux*sa + uz*ca)))
        for i in range(sides):
            j = (i + 1) % sides
            self.quad(material, lower[i], lower[j], upper[j], upper[i])
        self.face(material, *reversed(lower))
        self.face(material, *upper)
    Mesh.cylinder = cylinder

def main():
    patch_mesh()
    OUT.mkdir(parents=True, exist_ok=True)
    for mesh in (architecture(), machinery(), props()):
        mesh.write(OUT)

if __name__ == "__main__":
    main()
