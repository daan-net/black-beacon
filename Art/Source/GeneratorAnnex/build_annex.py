#!/usr/bin/env python3
"""Deterministically generate modular BLACK BEACON generator annex and machinery OBJ source meshes."""

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
    
    # Base footprint is approximately 400x800. Center is at (0,0,0).
    # Walls (thick wet stone/concrete)
    for x in (-180, 180):
        m.box((x, 0, 150), (40, 800, 300), "WetRock")
    for y in (-380, 380):
        m.box((0, y, 150), (400, 40, 300), "WetRock")
        
    # Roof (aged, rusty corrugated metal, sloped)
    # Gable ends
    for y in (-380, 380):
        m.box((0, y, 350), (400, 30, 100), "TowerPaint")
    
    # Roof panels
    m.box((-100, 0, 360), (220, 840, 10), "DarkIron", -20)
    m.box((100, 0, 360), (220, 840, 10), "DarkIron", 20)
    
    # Ridge cap
    m.cylinder((0, 0, 395), 15, 840, "DarkIron", 8)
    
    # Gutters and drainage
    for x in (-205, 205):
        m.box((x, 0, 310), (15, 840, 15), "DarkIron")
        # Downspouts
        m.cylinder((x, -390, 150), 6, 300, "DarkIron", 8)
        m.cylinder((x, 390, 150), 6, 300, "DarkIron", 8)
        
    # Heavy industrial door (West side)
    m.box((-190, 0, 120), (20, 140, 240), "DarkIron")
    # Door frame
    m.box((-195, -80, 120), (30, 20, 240), "TowerPaint")
    m.box((-195, 80, 120), (30, 20, 240), "TowerPaint")
    m.box((-195, 0, 250), (30, 180, 20), "TowerPaint")
    
    # Small weathered windows
    for y in (-200, 200):
        m.box((190, y, 180), (30, 80, 60), "LanternGlass")
        # Window bars
        for dy in (-20, 0, 20):
            m.cylinder((190, y + dy, 180), 2, 60, "DarkIron", 4)
            
    # Chimney / Vents
    m.box((150, 300, 420), (60, 60, 200), "WetRock")
    m.cylinder((150, 300, 540), 25, 40, "DarkIron", 8)

    return m

def machinery():
    m = Mesh("SM_BB_GeneratorAnnex_Machinery")
    
    # Main Generator Block
    # Engine block
    m.box((0, 100, 60), (120, 240, 120), "DarkIron")
    # Alternator (cylinder)
    m.cylinder((0, -80, 60), 55, 120, "DarkIron", 16)
    # Flywheel housing
    m.cylinder((0, -10, 60), 65, 20, "DarkIron", 16)
    
    # Support frame
    m.box((0, 50, 10), (140, 360, 20), "DarkIron")
    for dx in (-60, 60):
        for dy in (-120, 120):
            m.cylinder((dx, dy, -5), 8, 30, "DarkIron", 8)
            
    # Exhaust manifold and pipes
    for y in (40, 80, 120, 160):
        m.cylinder((70, y, 80), 10, 40, "WarmBrass", 8)
    m.cylinder((90, 100, 80), 12, 160, "DarkIron", 8)
    # Exhaust pipe going up and out
    m.cylinder((90, 180, 200), 12, 240, "DarkIron", 8)
    m.box((90, 180, 320), (40, 40, 60), "DarkIron") # Muffler
    m.cylinder((90, 180, 400), 10, 100, "DarkIron", 8)
    
    # Fuel tank (mounted on wall or frame)
    m.box((-100, 250, 150), (80, 140, 100), "TowerPaint")
    m.cylinder((-100, 250, 205), 15, 10, "WarmBrass", 12)
    # Fuel pipe
    m.cylinder((-100, 250, 80), 5, 40, "DarkIron", 6)
    
    # Electrical/Breaker panels
    m.box((160, -100, 150), (10, 200, 180), "TowerPaint")
    m.box((155, -150, 150), (12, 60, 120), "DarkIron")
    m.box((155, -50, 150), (12, 60, 120), "DarkIron")
    # Conduits
    for y in (-150, -50):
        m.cylinder((155, y, 250), 4, 100, "DarkIron", 6)
        
    # Large Gauges / Valves on the engine
    m.cylinder((-65, 100, 80), 12, 10, "WarmBrass", 16)
    m.cylinder((-65, 150, 80), 12, 10, "WarmBrass", 16)
    
    # Cooling fan / radiator
    m.box((0, 230, 60), (100, 20, 100), "DarkIron")
    m.cylinder((0, 215, 60), 40, 10, "WarmBrass", 16)
    
    return m

def props():
    m = Mesh("SM_BB_GeneratorAnnex_Props")
    
    # Workbench
    m.box((-120, -300, 90), (80, 200, 10), "AgedWood")
    for dx in (-35, 35):
        for dy in (-90, 90):
            m.box((-120 + dx, -300 + dy, 45), (10, 10, 90), "DarkIron")
            
    # Shelf
    m.box((-120, -300, 180), (60, 180, 5), "AgedWood")
    m.box((-120, -300, 230), (60, 180, 5), "AgedWood")
    
    # Barrels/Cans
    for p in ((-120, 320), (-80, 340), (-140, 350)):
        m.cylinder((p[0], p[1], 45), 25, 90, "TowerPaint", 12)
        m.cylinder((p[0], p[1], 90), 10, 5, "WarmBrass", 8)
        
    # Crates
    m.box((120, 300, 30), (60, 60, 60), "AgedWood")
    m.box((120, 300, 90), (50, 50, 50), "AgedWood")
    m.box((130, 230, 25), (50, 80, 50), "AgedWood")
    
    # Tools on workbench
    m.box((-120, -320, 98), (15, 5, 5), "DarkIron")
    m.box((-110, -280, 98), (5, 25, 5), "DarkIron")
    
    # Industrial practical lamp (hanging)
    for y in (-200, 0, 200):
        m.cylinder((0, y, 320), 4, 60, "DarkIron", 6)
        m.cylinder((0, y, 290), 20, 10, "WarmBrass", 16, top_radius=5, bottom_radius=20)
        m.cylinder((0, y, 285), 6, 10, "LanternGlass", 8)
        
    return m

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for mesh in (architecture(), machinery(), props()):
        mesh.write(OUT)
        # Update the MTL file generated by mesh.write to include more materials if needed
        mtl_path = OUT / f"{mesh.name}.mtl"
        with mtl_path.open("a", encoding="ascii") as f:
            # We already have TowerPaint, DarkIron, WarmBrass, LanternGlass, AgedWood, WetRock
            pass

if __name__ == "__main__":
    main()
