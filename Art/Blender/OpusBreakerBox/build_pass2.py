"""
BB_BreakerBox_Opus — Pass 2: Complete rebuild with proper construction.
Fixes from Pass 1 inspection:
  - Enclosure is now a proper 5-sided box (one solid mesh minus front face)
  - Door hinged correctly on left edge
  - Camera tracks look at the model center
  - Interior components are properly placed inside the box
  - Reduced samples for faster iteration (128)
  - Better material contrast
"""

import bpy
import bmesh
import math
import os
from mathutils import Vector, Matrix, Euler

OUTPUT_DIR = "/home/a1/WORK/BLACK_BEACON_OPUS_BLENDER/Art/Blender/OpusBreakerBox"

# ─── Core dimensions ───────────────────────────────────────────────────────
BOX_W = 0.40   # width (X)
BOX_H = 0.55   # height (Z)
BOX_D = 0.20   # depth (Y)
WALL  = 0.004  # sheet metal thickness

# ─── Helpers ────────────────────────────────────────────────────────────────

def clear_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        bpy.data.meshes.remove(mesh, do_unlink=True)
    for mat in list(bpy.data.materials):
        bpy.data.materials.remove(mat, do_unlink=True)
    for col in list(bpy.data.collections):
        bpy.data.collections.remove(col)


def new_collection(name):
    col = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(col)
    return col


def link_to(col, obj):
    col.objects.link(obj)
    if obj.name in bpy.context.scene.collection.objects:
        bpy.context.scene.collection.objects.unlink(obj)


def set_smooth(obj):
    if obj.type == 'MESH':
        for poly in obj.data.polygons:
            poly.use_smooth = True


def apply_transforms(obj):
    """Apply all transforms to mesh data."""
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    obj.select_set(False)


# ─── Materials ──────────────────────────────────────────────────────────────

def mat_rusted_steel():
    """Dark industrial rusted painted steel."""
    mat = bpy.data.materials.new("BB_RustedSteel")
    mat.use_nodes = True
    N = mat.node_tree.nodes
    L = mat.node_tree.links
    N.clear()

    out = N.new('ShaderNodeOutputMaterial'); out.location = (1200, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (800, 0)
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    tc = N.new('ShaderNodeTexCoord'); tc.location = (-800, 200)

    # Large-scale rust pattern
    n1 = N.new('ShaderNodeTexNoise'); n1.location = (-500, 400)
    n1.inputs['Scale'].default_value = 8.0
    n1.inputs['Detail'].default_value = 14.0
    n1.inputs['Roughness'].default_value = 0.65
    L.new(tc.outputs['Object'], n1.inputs['Vector'])

    # Fine detail noise
    n2 = N.new('ShaderNodeTexNoise'); n2.location = (-500, 100)
    n2.inputs['Scale'].default_value = 60.0
    n2.inputs['Detail'].default_value = 8.0
    n2.inputs['Roughness'].default_value = 0.5
    L.new(tc.outputs['Object'], n2.inputs['Vector'])

    # Musgrave for extra grit
    n3 = N.new('ShaderNodeTexNoise'); n3.location = (-500, -200)
    n3.inputs['Scale'].default_value = 120.0
    n3.inputs['Detail'].default_value = 4.0
    L.new(tc.outputs['Object'], n3.inputs['Vector'])

    # Rust mask ramp
    ramp = N.new('ShaderNodeValToRGB'); ramp.location = (-200, 400)
    ramp.color_ramp.elements[0].position = 0.30
    ramp.color_ramp.elements[0].color = (0, 0, 0, 1)
    ramp.color_ramp.elements[1].position = 0.60
    ramp.color_ramp.elements[1].color = (1, 1, 1, 1)
    L.new(n1.outputs['Fac'], ramp.inputs['Fac'])

    # Paint color: dark grey-green, almost black
    paint = N.new('ShaderNodeRGB'); paint.location = (100, 500)
    paint.outputs[0].default_value = (0.025, 0.030, 0.022, 1)

    # Rust color: deep brown-orange
    rust = N.new('ShaderNodeRGB'); rust.location = (100, 300)
    rust.outputs[0].default_value = (0.18, 0.055, 0.015, 1)

    # Mix
    mix = N.new('ShaderNodeMixRGB'); mix.location = (350, 400)
    L.new(ramp.outputs['Color'], mix.inputs['Fac'])
    L.new(paint.outputs['Color'], mix.inputs['Color1'])
    L.new(rust.outputs['Color'], mix.inputs['Color2'])

    # Darken with fine noise for grime
    mix2 = N.new('ShaderNodeMixRGB'); mix2.location = (550, 300)
    mix2.blend_type = 'MULTIPLY'
    mix2.inputs['Fac'].default_value = 0.3
    grime = N.new('ShaderNodeValToRGB'); grime.location = (200, -50)
    grime.color_ramp.elements[0].position = 0.3
    grime.color_ramp.elements[0].color = (0.4, 0.4, 0.4, 1)
    grime.color_ramp.elements[1].position = 0.7
    grime.color_ramp.elements[1].color = (1, 1, 1, 1)
    L.new(n2.outputs['Fac'], grime.inputs['Fac'])
    L.new(mix.outputs['Color'], mix2.inputs['Color1'])
    L.new(grime.outputs['Color'], mix2.inputs['Color2'])

    L.new(mix2.outputs['Color'], bsdf.inputs['Base Color'])

    bsdf.inputs['Metallic'].default_value = 1.0

    # Roughness varies with rust
    rramp = N.new('ShaderNodeValToRGB'); rramp.location = (200, -300)
    rramp.color_ramp.elements[0].position = 0.3
    rramp.color_ramp.elements[0].color = (0.45, 0.45, 0.45, 1)
    rramp.color_ramp.elements[1].position = 0.7
    rramp.color_ramp.elements[1].color = (0.85, 0.85, 0.85, 1)
    L.new(n1.outputs['Fac'], rramp.inputs['Fac'])
    L.new(rramp.outputs['Color'], bsdf.inputs['Roughness'])

    # Bump from fine noise
    bump = N.new('ShaderNodeBump'); bump.location = (550, -200)
    bump.inputs['Strength'].default_value = 0.4
    L.new(n3.outputs['Fac'], bump.inputs['Height'])
    L.new(bump.outputs['Normal'], bsdf.inputs['Normal'])

    return mat


def mat_interior_dark():
    """Very dark interior metal panel."""
    mat = bpy.data.materials.new("BB_InteriorDark")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (300, 0)
    bsdf.inputs['Base Color'].default_value = (0.02, 0.02, 0.018, 1)
    bsdf.inputs['Metallic'].default_value = 0.9
    bsdf.inputs['Roughness'].default_value = 0.75
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def mat_copper():
    """Tarnished copper/brass."""
    mat = bpy.data.materials.new("BB_Copper")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (800, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (500, 0)

    tc = N.new('ShaderNodeTexCoord'); tc.location = (-400, 0)
    n1 = N.new('ShaderNodeTexNoise'); n1.location = (-100, 100)
    n1.inputs['Scale'].default_value = 30.0; n1.inputs['Detail'].default_value = 8.0
    L.new(tc.outputs['Object'], n1.inputs['Vector'])

    ramp = N.new('ShaderNodeValToRGB'); ramp.location = (150, 100)
    ramp.color_ramp.elements[0].position = 0.35
    ramp.color_ramp.elements[0].color = (0.55, 0.28, 0.08, 1)  # polished copper
    ramp.color_ramp.elements[1].position = 0.65
    ramp.color_ramp.elements[1].color = (0.12, 0.20, 0.10, 1)  # verdigris
    L.new(n1.outputs['Fac'], ramp.inputs['Fac'])
    L.new(ramp.outputs['Color'], bsdf.inputs['Base Color'])

    bsdf.inputs['Metallic'].default_value = 1.0
    bsdf.inputs['Roughness'].default_value = 0.30
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def mat_ceramic():
    """Yellowed porcelain insulator."""
    mat = bpy.data.materials.new("BB_Ceramic")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (300, 0)
    bsdf.inputs['Base Color'].default_value = (0.55, 0.50, 0.40, 1)
    bsdf.inputs['Roughness'].default_value = 0.25
    bsdf.inputs['Subsurface Weight'].default_value = 0.03
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def mat_bakelite():
    """Dark bakelite."""
    mat = bpy.data.materials.new("BB_Bakelite")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (300, 0)
    bsdf.inputs['Base Color'].default_value = (0.04, 0.025, 0.015, 1)
    bsdf.inputs['Roughness'].default_value = 0.2
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def mat_wire():
    """Dark rubber insulation."""
    mat = bpy.data.materials.new("BB_Wire")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (300, 0)
    bsdf.inputs['Base Color'].default_value = (0.015, 0.015, 0.015, 1)
    bsdf.inputs['Roughness'].default_value = 0.85
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def mat_glass():
    """Dirty glass."""
    mat = bpy.data.materials.new("BB_Glass")
    mat.use_nodes = True
    N = mat.node_tree.nodes; L = mat.node_tree.links; N.clear()
    out = N.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = N.new('ShaderNodeBsdfPrincipled'); bsdf.location = (300, 0)
    bsdf.inputs['Base Color'].default_value = (0.5, 0.48, 0.4, 1)
    bsdf.inputs['Roughness'].default_value = 0.12
    bsdf.inputs['Transmission Weight'].default_value = 0.65
    bsdf.inputs['IOR'].default_value = 1.5
    L.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


# ─── Geometry Builders ──────────────────────────────────────────────────────

def build_enclosure(col, mats):
    """
    Build the main enclosure as a proper 5-sided box (open front).
    Uses bmesh to create a single cohesive mesh.
    """
    m = mats['steel']
    hw, hd, hh = BOX_W/2, BOX_D/2, BOX_H/2

    bm = bmesh.new()

    # Create a full box, then delete the front face
    # Vertices: 8 corners of the box
    # Bottom face (Z=0)
    v0 = bm.verts.new((-hw, -hd, 0))      # back-left-bottom
    v1 = bm.verts.new(( hw, -hd, 0))      # back-right-bottom
    v2 = bm.verts.new(( hw,  hd, 0))      # front-right-bottom
    v3 = bm.verts.new((-hw,  hd, 0))      # front-left-bottom
    # Top face (Z=BOX_H)
    v4 = bm.verts.new((-hw, -hd, BOX_H))  # back-left-top
    v5 = bm.verts.new(( hw, -hd, BOX_H))  # back-right-top
    v6 = bm.verts.new(( hw,  hd, BOX_H))  # front-right-top
    v7 = bm.verts.new((-hw,  hd, BOX_H))  # front-left-top

    bm.verts.ensure_lookup_table()

    # Back face
    bm.faces.new([v0, v1, v5, v4])
    # Left face
    bm.faces.new([v3, v0, v4, v7])
    # Right face
    bm.faces.new([v1, v2, v6, v5])
    # Top face
    bm.faces.new([v4, v5, v6, v7])
    # Bottom face
    bm.faces.new([v3, v2, v1, v0])
    # Front face — we'll keep it for now (the door covers it, but the enclosure
    # should have a narrow frame around the front opening)

    # Instead of a full front face, create a frame (the door fits inside this frame)
    # Delete nothing; we just make the front face the "door opening" area
    # For simplicity: make the full box, then add a solidify modifier for wall thickness

    mesh = bpy.data.meshes.new("BB_Enclosure")
    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new("BB_Enclosure", mesh)
    obj.data.materials.append(m)
    link_to(col, obj)

    # Solidify for wall thickness
    sol = obj.modifiers.new("Solidify", 'SOLIDIFY')
    sol.thickness = WALL
    sol.offset = -1  # grow inward

    # Bevel edges for realism
    bev = obj.modifiers.new("Bevel", 'BEVEL')
    bev.width = 0.002
    bev.segments = 2
    bev.limit_method = 'ANGLE'
    bev.angle_limit = math.radians(50)

    set_smooth(obj)

    # Top lip / flange
    bpy.ops.mesh.primitive_cube_add(size=1)
    lip = bpy.context.active_object
    lip.name = "BB_TopLip"
    lip.scale = (hw + 0.010, hd + 0.010, 0.005)
    lip.location = (0, 0, BOX_H + 0.003)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    lip.data.materials.append(m)
    bev2 = lip.modifiers.new("Bevel", 'BEVEL')
    bev2.width = 0.001; bev2.segments = 2
    set_smooth(lip)
    link_to(col, lip)

    # Bottom lip
    bpy.ops.mesh.primitive_cube_add(size=1)
    blip = bpy.context.active_object
    blip.name = "BB_BottomLip"
    blip.scale = (hw + 0.010, hd + 0.010, 0.005)
    blip.location = (0, 0, -0.003)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    blip.data.materials.append(m)
    set_smooth(blip)
    link_to(col, blip)

    # ── Ventilation louvers on bottom ──
    for i in range(6):
        bpy.ops.mesh.primitive_cube_add(size=1)
        louver = bpy.context.active_object
        louver.name = f"BB_Louver_{i}"
        lx = -0.10 + i * 0.04
        louver.scale = (0.015, hd - 0.02, 0.002)
        louver.location = (lx, 0, 0.02)
        louver.rotation_euler = (math.radians(30), 0, 0)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        louver.data.materials.append(m)
        link_to(col, louver)

    return obj


def build_door(col, mats):
    """
    Front door panel. Origin at hinge edge (left side).
    Opened 25° outward (+Y rotation around Z).
    """
    m_steel = mats['steel']
    m_glass = mats['glass']

    dw = BOX_W - 0.02   # slightly smaller than enclosure
    dh = BOX_H - 0.02
    dt = WALL

    # Create door panel
    bm = bmesh.new()
    # Door mesh: origin at left edge center
    v0 = bm.verts.new((0,    -dt/2, -dh/2))
    v1 = bm.verts.new((dw,   -dt/2, -dh/2))
    v2 = bm.verts.new((dw,    dt/2, -dh/2))
    v3 = bm.verts.new((0,     dt/2, -dh/2))
    v4 = bm.verts.new((0,    -dt/2,  dh/2))
    v5 = bm.verts.new((dw,   -dt/2,  dh/2))
    v6 = bm.verts.new((dw,    dt/2,  dh/2))
    v7 = bm.verts.new((0,     dt/2,  dh/2))
    bm.verts.ensure_lookup_table()
    bm.faces.new([v3, v2, v1, v0])  # back
    bm.faces.new([v4, v5, v6, v7])  # front
    bm.faces.new([v0, v1, v5, v4])  # bottom
    bm.faces.new([v7, v6, v2, v3])  # top
    bm.faces.new([v0, v4, v7, v3])  # left (hinge side)
    bm.faces.new([v1, v2, v6, v5])  # right (latch side)

    mesh = bpy.data.meshes.new("BB_Door")
    bm.to_mesh(mesh)
    bm.free()

    door = bpy.data.objects.new("BB_Door", mesh)
    door.data.materials.append(m_steel)
    # Hinge position: left edge of door aligns with left edge of enclosure front
    door.location = (-BOX_W/2 + 0.01, BOX_D/2, BOX_H/2)
    door.rotation_euler = (0, 0, math.radians(25))  # open 25°
    link_to(col, door)
    set_smooth(door)

    bev = door.modifiers.new("Bevel", 'BEVEL')
    bev.width = 0.001; bev.segments = 2

    # ── Embossed panel on door front ──
    bpy.ops.mesh.primitive_cube_add(size=1)
    emb = bpy.context.active_object
    emb.name = "BB_DoorEmboss"
    emb.scale = (dw/2 - 0.025, 0.002, dh/2 - 0.03)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    # Move to door-local space
    bm = bmesh.new()
    bm.from_mesh(emb.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(dw/2, dt/2 + 0.002, 0))
    bm.to_mesh(emb.data)
    bm.free()
    emb.parent = door
    emb.data.materials.append(m_steel)
    link_to(col, emb)

    # ── Inspection window (round porthole) ──
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.028, depth=0.006)
    win = bpy.context.active_object
    win.name = "BB_DoorWindow"
    win.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bm = bmesh.new()
    bm.from_mesh(win.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(dw/2 + 0.02, dt/2 + 0.001, 0.06))
    bm.to_mesh(win.data)
    bm.free()
    win.parent = door
    win.data.materials.append(m_glass)
    link_to(col, win)

    # Window ring
    bpy.ops.mesh.primitive_torus_add(major_radius=0.030, minor_radius=0.004,
                                      major_segments=24, minor_segments=8)
    ring = bpy.context.active_object
    ring.name = "BB_WindowRing"
    ring.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bm = bmesh.new()
    bm.from_mesh(ring.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(dw/2 + 0.02, dt/2 + 0.003, 0.06))
    bm.to_mesh(ring.data)
    bm.free()
    ring.parent = door
    ring.data.materials.append(m_steel)
    link_to(col, ring)

    # ── Hinges (2 barrel hinges) ──
    for hz_local in [-dh/2 + 0.05, dh/2 - 0.05]:
        # Barrel
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.006, depth=0.03)
        hn = bpy.context.active_object
        hn.name = f"BB_Hinge_{hz_local:.2f}"
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        bm = bmesh.new()
        bm.from_mesh(hn.data)
        bmesh.ops.translate(bm, verts=bm.verts, vec=(0, 0, hz_local))
        bm.to_mesh(hn.data)
        bm.free()
        hn.parent = door
        hn.data.materials.append(m_steel)
        link_to(col, hn)

        # Hinge plates (on door)
        bpy.ops.mesh.primitive_cube_add(size=1)
        hp = bpy.context.active_object
        hp.name = f"BB_HingePlate_{hz_local:.2f}"
        hp.scale = (0.025, 0.002, 0.012)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        bm = bmesh.new()
        bm.from_mesh(hp.data)
        bmesh.ops.translate(bm, verts=bm.verts, vec=(0.015, dt/2, hz_local))
        bm.to_mesh(hp.data)
        bm.free()
        hp.parent = door
        hp.data.materials.append(m_steel)
        link_to(col, hp)

    # ── Latch mechanism (right side) ──
    # Latch catch on enclosure
    bpy.ops.mesh.primitive_cube_add(size=1)
    catch = bpy.context.active_object
    catch.name = "BB_LatchCatch"
    catch.scale = (0.012, 0.008, 0.03)
    catch.location = (BOX_W/2 - 0.01, BOX_D/2 + 0.006, BOX_H/2)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    catch.data.materials.append(m_steel)
    link_to(col, catch)

    # Latch lever on door
    bpy.ops.mesh.primitive_cube_add(size=1)
    lever = bpy.context.active_object
    lever.name = "BB_LatchLever"
    lever.scale = (0.008, 0.012, 0.045)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bm = bmesh.new()
    bm.from_mesh(lever.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(dw - 0.015, dt/2 + 0.008, 0))
    bm.to_mesh(lever.data)
    bm.free()
    lever.parent = door
    lever.data.materials.append(m_steel)
    link_to(col, lever)

    # D-handle on door
    bpy.ops.mesh.primitive_torus_add(major_radius=0.020, minor_radius=0.003,
                                      major_segments=16, minor_segments=6)
    handle = bpy.context.active_object
    handle.name = "BB_Handle"
    handle.scale = (1, 0.5, 1)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bm = bmesh.new()
    bm.from_mesh(handle.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(dw - 0.05, dt/2 + 0.012, -0.06))
    bm.to_mesh(handle.data)
    bm.free()
    handle.parent = door
    handle.data.materials.append(m_steel)
    link_to(col, handle)

    return door


def build_interior(col, mats):
    """
    All interior electrical components, mounted to the back wall of the enclosure.
    Y positions: back wall is at -BOX_D/2, so components go from -BOX_D/2+WALL inward.
    """
    m_int = mats['interior']
    m_cu  = mats['copper']
    m_cer = mats['ceramic']
    m_bak = mats['bakelite']
    m_wire = mats['wire']

    back_y = -BOX_D/2 + WALL   # inner surface of back wall
    mount_y = back_y + 0.005   # mounting backplate surface

    # ── Mounting backplate ──
    bpy.ops.mesh.primitive_cube_add(size=1)
    bp = bpy.context.active_object
    bp.name = "BB_MountPlate"
    bp.scale = (BOX_W/2 - 0.015, 0.003, BOX_H/2 - 0.02)
    bp.location = (0, mount_y, BOX_H/2)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bp.data.materials.append(m_int)
    link_to(col, bp)

    comp_y = mount_y + 0.005  # component mounting surface

    # ── Main breaker (large knife switch at top) ──
    build_knife_switch(col, mats, (0, comp_y, BOX_H - 0.08), scale=1.5, closed=False,
                       name="Main")

    # ── Bus bars (3 horizontal copper bars) ──
    bar_heights = [0.38, 0.25, 0.12]
    for i, bz in enumerate(bar_heights):
        bpy.ops.mesh.primitive_cube_add(size=1)
        bar = bpy.context.active_object
        bar.name = f"BB_BusBar_{i}"
        bar.scale = (BOX_W/2 - 0.04, 0.003, 0.006)
        bar.location = (0, comp_y + 0.003, bz)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        bar.data.materials.append(m_cu)
        bev = bar.modifiers.new("Bevel", 'BEVEL')
        bev.width = 0.0008; bev.segments = 2
        set_smooth(bar)
        link_to(col, bar)

        # Ceramic standoff insulators under bus bars
        for sx in [-0.11, 0.0, 0.11]:
            bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.007, depth=0.012)
            so = bpy.context.active_object
            so.name = f"BB_Standoff_{i}_{sx}"
            so.rotation_euler = (math.radians(90), 0, 0)
            so.location = (sx, comp_y - 0.003, bz)
            so.data.materials.append(m_cer)
            set_smooth(so)
            link_to(col, so)

    # ── Fuse holders (6: 2 rows × 3) ──
    for row, fz in enumerate([0.17, 0.31]):
        for fi, fx in enumerate([-0.09, 0.0, 0.09]):
            build_fuse(col, mats, (fx, comp_y, fz), name=f"{row}_{fi}")

    # ── Branch circuit knife switches (2x) ──
    build_knife_switch(col, mats, (-0.10, comp_y, 0.42), scale=0.8, closed=True,
                       name="Branch_L")
    build_knife_switch(col, mats, ( 0.10, comp_y, 0.42), scale=0.8, closed=False,
                       name="Branch_R")

    # ── Terminal strip at bottom ──
    bpy.ops.mesh.primitive_cube_add(size=1)
    ts = bpy.context.active_object
    ts.name = "BB_TermStrip"
    ts.scale = (BOX_W/2 - 0.04, 0.004, 0.010)
    ts.location = (0, comp_y + 0.002, 0.05)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    ts.data.materials.append(m_cer)
    set_smooth(ts)
    link_to(col, ts)

    # Terminal screws
    for ti in range(10):
        tx = -0.13 + ti * 0.029
        bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.003, depth=0.004)
        scr = bpy.context.active_object
        scr.name = f"BB_TermScrew_{ti}"
        scr.rotation_euler = (math.radians(90), 0, 0)
        scr.location = (tx, comp_y + 0.008, 0.05)
        scr.data.materials.append(m_cu)
        link_to(col, scr)

    # ── Wire runs ──
    wire_paths = [
        # From bus bar 0 down to terminal strip
        [(-0.09, comp_y + 0.015, 0.12), (-0.09, comp_y + 0.025, 0.085), (-0.09, comp_y + 0.015, 0.05)],
        [(0.0,  comp_y + 0.015, 0.12), (0.0,  comp_y + 0.030, 0.085), (0.0,  comp_y + 0.015, 0.05)],
        [(0.09, comp_y + 0.015, 0.12), (0.09, comp_y + 0.025, 0.085), (0.09, comp_y + 0.015, 0.05)],
        # From main switch to bus bar 2
        [(0.0,  comp_y + 0.020, BOX_H - 0.08), (0.0, comp_y + 0.030, 0.42),
         (-0.05, comp_y + 0.020, 0.38)],
        # Cross connections between bus bars
        [(-0.11, comp_y + 0.015, 0.38), (-0.11, comp_y + 0.025, 0.315),
         (-0.11, comp_y + 0.015, 0.25)],
        [(0.11,  comp_y + 0.015, 0.38), (0.11, comp_y + 0.025, 0.315),
         (0.11, comp_y + 0.015, 0.25)],
    ]

    for wi, path in enumerate(wire_paths):
        curve_data = bpy.data.curves.new(f"BB_WireCurve_{wi}", type='CURVE')
        curve_data.dimensions = '3D'
        curve_data.bevel_depth = 0.0025
        curve_data.bevel_resolution = 3

        spline = curve_data.splines.new('BEZIER')
        spline.bezier_points.add(len(path) - 1)
        for pi, (px, py, pz) in enumerate(path):
            bp = spline.bezier_points[pi]
            bp.co = (px, py, pz)
            bp.handle_left_type = 'AUTO'
            bp.handle_right_type = 'AUTO'

        wire_obj = bpy.data.objects.new(f"BB_Wire_{wi}", curve_data)
        wire_obj.data.materials.append(m_wire)
        link_to(col, wire_obj)


def build_fuse(col, mats, pos, name=""):
    """Single ceramic fuse holder with fuse element."""
    x, y, z = pos
    m_cer = mats['ceramic']
    m_cu  = mats['copper']

    # Fuse base block
    bpy.ops.mesh.primitive_cube_add(size=1)
    base = bpy.context.active_object
    base.name = f"BB_FuseBase_{name}"
    base.scale = (0.016, 0.010, 0.022)
    base.location = (x, y + 0.010, z)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    base.data.materials.append(m_cer)
    bev = base.modifiers.new("Bevel", 'BEVEL')
    bev.width = 0.0015; bev.segments = 2
    set_smooth(base)
    link_to(col, base)

    # Fuse cartridge
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.004, depth=0.032)
    fuse = bpy.context.active_object
    fuse.name = f"BB_Fuse_{name}"
    fuse.location = (x, y + 0.015, z)
    fuse.data.materials.append(m_cer)
    set_smooth(fuse)
    link_to(col, fuse)

    # End caps
    for cz in [-0.015, 0.015]:
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.005, depth=0.004)
        cap = bpy.context.active_object
        cap.name = f"BB_FuseCap_{name}_{cz}"
        cap.location = (x, y + 0.015, z + cz)
        cap.data.materials.append(m_cu)
        set_smooth(cap)
        link_to(col, cap)

    # Contact clips
    for cz in [-0.018, 0.018]:
        bpy.ops.mesh.primitive_cube_add(size=1)
        clip = bpy.context.active_object
        clip.name = f"BB_Clip_{name}_{cz}"
        clip.scale = (0.002, 0.006, 0.010)
        clip.location = (x, y + 0.020, z + cz)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        clip.data.materials.append(m_cu)
        link_to(col, clip)


def build_knife_switch(col, mats, pos, scale=1.0, closed=True, name=""):
    """Knife switch with ceramic base, copper contacts, bakelite handle."""
    x, y, z = pos
    m_cer = mats['ceramic']
    m_cu  = mats['copper']
    m_bak = mats['bakelite']
    s = scale

    # Base plate
    bpy.ops.mesh.primitive_cube_add(size=1)
    base = bpy.context.active_object
    base.name = f"BB_SwBase_{name}"
    base.scale = (0.022*s, 0.006*s, 0.045*s)
    base.location = (x, y + 0.006*s, z)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    base.data.materials.append(m_cer)
    set_smooth(base)
    link_to(col, base)

    # Jaw contacts (bottom pair)
    for jx in [-0.010*s, 0.010*s]:
        bpy.ops.mesh.primitive_cube_add(size=1)
        jaw = bpy.context.active_object
        jaw.name = f"BB_SwJaw_{name}_{jx}"
        jaw.scale = (0.003*s, 0.005*s, 0.016*s)
        jaw.location = (x + jx, y + 0.013*s, z - 0.018*s)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        jaw.data.materials.append(m_cu)
        link_to(col, jaw)

    # Blade
    bpy.ops.mesh.primitive_cube_add(size=1)
    blade = bpy.context.active_object
    blade.name = f"BB_SwBlade_{name}"
    blade.scale = (0.018*s, 0.002*s, 0.038*s)

    if closed:
        blade.location = (x, y + 0.015*s, z)
        blade.rotation_euler = (math.radians(5), 0, 0)
    else:
        blade.location = (x, y + 0.015*s, z - 0.018*s)
        blade.rotation_euler = (math.radians(-45), 0, 0)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    blade.data.materials.append(m_cu)
    link_to(col, blade)

    # Handle knob
    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.010*s, depth=0.012*s)
    knob = bpy.context.active_object
    knob.name = f"BB_SwKnob_{name}"
    knob.rotation_euler = (math.radians(90), 0, 0)
    if closed:
        knob.location = (x, y + 0.020*s, z + 0.030*s)
    else:
        knob.location = (x, y + 0.045*s, z - 0.050*s)
    knob.data.materials.append(m_bak)
    set_smooth(knob)
    link_to(col, knob)


def build_conduits(col, mats):
    """Entry conduits — pipes entering top, bottom, and side."""
    m = mats['steel']

    conduit_specs = [
        # (position, rotation, radius, length, name)
        ((0.08, -0.03, BOX_H + 0.06), (0, 0, 0), 0.014, 0.12, "Top"),
        ((-0.08, -0.03, -0.05), (0, 0, 0), 0.014, 0.10, "Bottom"),
        ((BOX_W/2 + 0.035, -0.02, 0.30), (0, math.radians(90), 0), 0.011, 0.07, "Side"),
    ]

    for pos, rot, rad, length, cname in conduit_specs:
        # Pipe
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=rad, depth=length)
        pipe = bpy.context.active_object
        pipe.name = f"BB_Conduit_{cname}"
        pipe.location = pos
        pipe.rotation_euler = rot
        pipe.data.materials.append(m)
        set_smooth(pipe)
        link_to(col, pipe)

        # Coupling nut (hex)
        nut_pos = list(pos)
        if cname == "Top":
            nut_pos[2] = BOX_H + 0.003
        elif cname == "Bottom":
            nut_pos[2] = -0.003
        else:
            nut_pos[0] = BOX_W/2 + 0.003

        bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=rad + 0.005, depth=0.012)
        nut = bpy.context.active_object
        nut.name = f"BB_ConduitNut_{cname}"
        nut.location = nut_pos
        nut.rotation_euler = rot
        nut.data.materials.append(m)
        link_to(col, nut)


def build_mounting(col, mats):
    """Wall mounting brackets and bolts on back."""
    m = mats['steel']
    for mz in [0.08, BOX_H - 0.08]:
        for mx in [-0.13, 0.13]:
            # L-bracket
            bpy.ops.mesh.primitive_cube_add(size=1)
            br = bpy.context.active_object
            br.name = f"BB_Bracket_{mx}_{mz}"
            br.scale = (0.018, 0.020, 0.003)
            br.location = (mx, -BOX_D/2 - 0.018, mz)
            bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
            br.data.materials.append(m)
            link_to(col, br)

            # Bolt head
            bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.005, depth=0.004)
            bolt = bpy.context.active_object
            bolt.name = f"BB_Bolt_{mx}_{mz}"
            bolt.rotation_euler = (math.radians(90), 0, 0)
            bolt.location = (mx, -BOX_D/2 - 0.032, mz)
            bolt.data.materials.append(m)
            link_to(col, bolt)


def build_nameplate(col, mats):
    """Brass nameplate and danger sign on front of door area."""
    m_cu = mats['copper']
    m_st = mats['steel']

    # Nameplate — will be on the door, but for simplicity place at fixed position
    bpy.ops.mesh.primitive_cube_add(size=1)
    np = bpy.context.active_object
    np.name = "BB_NamePlate"
    np.scale = (0.035, 0.001, 0.012)
    np.location = (0.05, BOX_D/2 + 0.006, BOX_H - 0.05)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    np.data.materials.append(m_cu)
    bev = np.modifiers.new("Bevel", 'BEVEL')
    bev.width = 0.0004; bev.segments = 2
    link_to(col, np)

    # Rivets
    for rx in [-0.025, 0.025]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=4, radius=0.0015)
        rv = bpy.context.active_object
        rv.name = f"BB_Rivet_{rx}"
        rv.location = (0.05 + rx, BOX_D/2 + 0.008, BOX_H - 0.05)
        rv.data.materials.append(m_cu)
        link_to(col, rv)

    # Danger plate
    bpy.ops.mesh.primitive_cube_add(size=1)
    dp = bpy.context.active_object
    dp.name = "BB_DangerPlate"
    dp.scale = (0.030, 0.001, 0.020)
    dp.location = (0.05, BOX_D/2 + 0.006, 0.12)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    dp.data.materials.append(m_st)
    link_to(col, dp)


def build_ground_stud(col, mats):
    """Grounding lug on enclosure side."""
    m_cu = mats['copper']

    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.005, depth=0.012)
    stud = bpy.context.active_object
    stud.name = "BB_GroundStud"
    stud.rotation_euler = (0, math.radians(90), 0)
    stud.location = (BOX_W/2 + 0.004, 0, 0.06)
    stud.data.materials.append(m_cu)
    set_smooth(stud)
    link_to(col, stud)

    bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.007, depth=0.004)
    nut = bpy.context.active_object
    nut.name = "BB_GroundNut"
    nut.rotation_euler = (0, math.radians(90), 0)
    nut.location = (BOX_W/2 + 0.010, 0, 0.06)
    nut.data.materials.append(m_cu)
    link_to(col, nut)


# ─── Scene Setup ────────────────────────────────────────────────────────────

def setup_lighting():
    """Three-point + interior fill."""
    # Key (warm, like a nearby work lamp)
    bpy.ops.object.light_add(type='AREA', radius=0.4)
    key = bpy.context.active_object
    key.name = "Key"
    key.location = (0.5, 0.7, 0.7)
    key.data.energy = 60
    key.data.color = (1.0, 0.85, 0.65)
    key.data.size = 0.4
    # Point at model center
    track = key.constraints.new('TRACK_TO')
    track.target = bpy.data.objects.new("KeyTarget", None)
    bpy.context.scene.collection.objects.link(track.target)
    track.target.location = (0, 0, BOX_H/2)
    track.track_axis = 'TRACK_NEGATIVE_Z'
    track.up_axis = 'UP_Y'

    # Fill (cool ambient)
    bpy.ops.object.light_add(type='AREA', radius=0.6)
    fill = bpy.context.active_object
    fill.name = "Fill"
    fill.location = (-0.6, 0.4, 0.4)
    fill.data.energy = 20
    fill.data.color = (0.7, 0.78, 1.0)
    fill.data.size = 0.6

    # Rim (back edge definition)
    bpy.ops.object.light_add(type='AREA', radius=0.3)
    rim = bpy.context.active_object
    rim.name = "Rim"
    rim.location = (-0.2, -0.6, 0.6)
    rim.data.energy = 35
    rim.data.color = (0.85, 0.88, 1.0)
    rim.data.size = 0.25

    # Interior point light (inside box)
    bpy.ops.object.light_add(type='POINT', radius=0.03)
    inte = bpy.context.active_object
    inte.name = "InteriorLight"
    inte.location = (0, 0.02, BOX_H/2)
    inte.data.energy = 3
    inte.data.color = (1.0, 0.9, 0.7)
    inte.data.shadow_soft_size = 0.04


def setup_cameras():
    """Five cameras, all using Track To constraints to look at model center."""
    center = Vector((0, 0, BOX_H/2))
    cams = {}

    specs = {
        '01_front_closed_3quarter': {
            'loc': (0.45, 0.40, 0.35),
            'lens': 50,
        },
        '02_door_open_interior': {
            'loc': (0.20, 0.30, 0.30),
            'lens': 35,
        },
        '03_detail_closeup': {
            'loc': (0.08, 0.18, 0.28),
            'lens': 85,
        },
        '04_side_construction': {
            'loc': (0.45, -0.02, 0.28),
            'lens': 50,
        },
        '05_wireframe': {
            'loc': (0.45, 0.40, 0.35),
            'lens': 50,
        },
    }

    # Shared target empty
    target = bpy.data.objects.new("CamTarget", None)
    bpy.context.scene.collection.objects.link(target)
    target.location = center

    for cam_name, spec in specs.items():
        bpy.ops.object.camera_add()
        cam = bpy.context.active_object
        cam.name = f"Cam_{cam_name}"
        cam.location = spec['loc']
        cam.data.lens = spec['lens']
        cam.data.clip_start = 0.005
        cam.data.clip_end = 50

        # Track to model center
        track = cam.constraints.new('TRACK_TO')
        track.target = target
        track.track_axis = 'TRACK_NEGATIVE_Z'
        track.up_axis = 'UP_Y'

        cams[cam_name] = cam

    return cams


def setup_render():
    """Configure Cycles rendering."""
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 128   # reduced for iteration speed
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1920
    scene.render.resolution_y = 1080
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'

    # World
    world = bpy.data.worlds.new("BB_World")
    scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes['Background']
    bg.inputs['Color'].default_value = (0.010, 0.010, 0.012, 1)
    bg.inputs['Strength'].default_value = 0.2


def render_all(cams):
    """Render all cameras."""
    scene = bpy.context.scene

    for cam_name, cam_obj in cams.items():
        scene.camera = cam_obj

        if cam_name == '05_wireframe':
            scene.render.use_freestyle = True
            vl = scene.view_layers[0]
            vl.use_freestyle = True
            ls = vl.freestyle_settings.linesets.new("WF")
            ls.select_silhouette = True
            ls.select_border = True
            ls.select_crease = True
            ls.linestyle.color = (0.75, 0.80, 0.85)
            ls.linestyle.thickness = 1.2

        filepath = os.path.join(OUTPUT_DIR, f"{cam_name}.png")
        scene.render.filepath = filepath
        bpy.ops.render.render(write_still=True)
        print(f"  ✓ {cam_name}.png")

        if cam_name == '05_wireframe':
            scene.render.use_freestyle = False


# ─── Main ───────────────────────────────────────────────────────────────────

def main():
    print("=" * 60)
    print("BB_BreakerBox_Opus — Pass 2")
    print("=" * 60)

    clear_scene()

    col = new_collection("BB_BreakerBox")

    # Materials
    mats = {
        'steel':   mat_rusted_steel(),
        'interior': mat_interior_dark(),
        'copper':  mat_copper(),
        'ceramic': mat_ceramic(),
        'bakelite': mat_bakelite(),
        'wire':    mat_wire(),
        'glass':   mat_glass(),
    }

    # Build
    print("Building enclosure...")
    build_enclosure(col, mats)

    print("Building door...")
    build_door(col, mats)

    print("Building interior...")
    build_interior(col, mats)

    print("Building conduits...")
    build_conduits(col, mats)

    print("Building mounting hardware...")
    build_mounting(col, mats)

    print("Building nameplate...")
    build_nameplate(col, mats)

    print("Building ground stud...")
    build_ground_stud(col, mats)

    # Scene
    print("Setting up lighting...")
    setup_lighting()

    print("Setting up cameras...")
    cams = setup_cameras()

    print("Configuring render...")
    setup_render()

    # Save
    blend_path = os.path.join(OUTPUT_DIR, "BB_BreakerBox_Opus.blend")
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print(f"Saved: {blend_path}")

    # Render
    print("Rendering...")
    render_all(cams)

    print("\nPass 2 complete!")


if __name__ == "__main__":
    main()
