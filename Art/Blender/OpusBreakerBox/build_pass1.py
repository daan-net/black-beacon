"""
BB_BreakerBox_Opus — Pass 1: Substantial first modeling pass.
Old industrial lighthouse breaker / control box.
Target: weathered 1930s-era electrical control panel.
"""

import bpy
import bmesh
import math
import os
from mathutils import Vector, Matrix, Euler

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
    # Remove from scene collection if present
    if obj.name in bpy.context.scene.collection.objects:
        bpy.context.scene.collection.objects.unlink(obj)


def set_smooth(obj):
    for poly in obj.data.polygons:
        poly.use_smooth = True


def add_bevel_modifier(obj, width=0.001, segments=2):
    mod = obj.modifiers.new("Bevel", 'BEVEL')
    mod.width = width
    mod.segments = segments
    mod.limit_method = 'ANGLE'
    mod.angle_limit = math.radians(30)
    return mod


def solidify(obj, thickness=0.003):
    mod = obj.modifiers.new("Solidify", 'SOLIDIFY')
    mod.thickness = thickness
    mod.offset = -1
    return mod


# ─── Materials ──────────────────────────────────────────────────────────────

def make_rusted_metal_material():
    """Heavy rusted painted steel — primary enclosure material."""
    mat = bpy.data.materials.new("BB_RustedMetal")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    # Output
    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (1200, 0)

    # Principled BSDF
    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (800, 0)
    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    # Noise for rust pattern
    tex_coord = nodes.new('ShaderNodeTexCoord')
    tex_coord.location = (-600, 200)

    noise1 = nodes.new('ShaderNodeTexNoise')
    noise1.location = (-300, 300)
    noise1.inputs['Scale'].default_value = 15.0
    noise1.inputs['Detail'].default_value = 12.0
    noise1.inputs['Roughness'].default_value = 0.7
    links.new(tex_coord.outputs['Object'], noise1.inputs['Vector'])

    noise2 = nodes.new('ShaderNodeTexNoise')
    noise2.location = (-300, 100)
    noise2.inputs['Scale'].default_value = 45.0
    noise2.inputs['Detail'].default_value = 8.0
    noise2.inputs['Roughness'].default_value = 0.5
    links.new(tex_coord.outputs['Object'], noise2.inputs['Vector'])

    # Color ramp for rust mask
    ramp = nodes.new('ShaderNodeValToRGB')
    ramp.location = (0, 300)
    ramp.color_ramp.elements[0].position = 0.35
    ramp.color_ramp.elements[0].color = (0.0, 0.0, 0.0, 1.0)
    ramp.color_ramp.elements[1].position = 0.65
    ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)
    links.new(noise1.outputs['Fac'], ramp.inputs['Fac'])

    # Paint color (dark grey-green industrial)
    paint_color = nodes.new('ShaderNodeRGB')
    paint_color.location = (200, 500)
    paint_color.outputs[0].default_value = (0.08, 0.09, 0.07, 1.0)

    # Rust color (orange-brown)
    rust_color = nodes.new('ShaderNodeRGB')
    rust_color.location = (200, 300)
    rust_color.outputs[0].default_value = (0.25, 0.08, 0.02, 1.0)

    # Mix paint and rust
    mix_color = nodes.new('ShaderNodeMixRGB')
    mix_color.location = (450, 400)
    mix_color.blend_type = 'MIX'
    links.new(ramp.outputs['Color'], mix_color.inputs['Fac'])
    links.new(paint_color.outputs['Color'], mix_color.inputs['Color1'])
    links.new(rust_color.outputs['Color'], mix_color.inputs['Color2'])
    links.new(mix_color.outputs['Color'], bsdf.inputs['Base Color'])

    # Roughness variation
    rough_ramp = nodes.new('ShaderNodeValToRGB')
    rough_ramp.location = (0, -100)
    rough_ramp.color_ramp.elements[0].position = 0.3
    rough_ramp.color_ramp.elements[0].color = (0.55, 0.55, 0.55, 1.0)
    rough_ramp.color_ramp.elements[1].position = 0.7
    rough_ramp.color_ramp.elements[1].color = (0.85, 0.85, 0.85, 1.0)
    links.new(noise2.outputs['Fac'], rough_ramp.inputs['Fac'])
    links.new(rough_ramp.outputs['Color'], bsdf.inputs['Roughness'])

    # Metallic = 1
    bsdf.inputs['Metallic'].default_value = 1.0

    # Bump
    bump = nodes.new('ShaderNodeBump')
    bump.location = (500, -200)
    bump.inputs['Strength'].default_value = 0.3
    links.new(noise2.outputs['Fac'], bump.inputs['Height'])
    links.new(bump.outputs['Normal'], bsdf.inputs['Normal'])

    return mat


def make_interior_metal_material():
    """Darker, grimier metal for interior panels."""
    mat = bpy.data.materials.new("BB_InteriorMetal")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.05, 0.05, 0.04, 1.0)
    bsdf.inputs['Metallic'].default_value = 1.0
    bsdf.inputs['Roughness'].default_value = 0.7

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    # Subtle noise bump
    tex_coord = nodes.new('ShaderNodeTexCoord')
    tex_coord.location = (-400, -200)
    noise = nodes.new('ShaderNodeTexNoise')
    noise.location = (-100, -200)
    noise.inputs['Scale'].default_value = 80.0
    noise.inputs['Detail'].default_value = 6.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    bump = nodes.new('ShaderNodeBump')
    bump.location = (200, -200)
    bump.inputs['Strength'].default_value = 0.15
    links.new(noise.outputs['Fac'], bump.inputs['Height'])
    links.new(bump.outputs['Normal'], bsdf.inputs['Normal'])

    return mat


def make_copper_material():
    """Aged copper/brass for bus bars and contacts."""
    mat = bpy.data.materials.new("BB_CopperBrass")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.45, 0.22, 0.06, 1.0)
    bsdf.inputs['Metallic'].default_value = 1.0
    bsdf.inputs['Roughness'].default_value = 0.35

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    # Patina noise
    tex_coord = nodes.new('ShaderNodeTexCoord')
    tex_coord.location = (-400, 200)
    noise = nodes.new('ShaderNodeTexNoise')
    noise.location = (-100, 200)
    noise.inputs['Scale'].default_value = 25.0
    noise.inputs['Detail'].default_value = 10.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    ramp = nodes.new('ShaderNodeValToRGB')
    ramp.location = (100, 200)
    ramp.color_ramp.elements[0].position = 0.4
    ramp.color_ramp.elements[0].color = (0.45, 0.22, 0.06, 1.0)
    ramp.color_ramp.elements[1].position = 0.7
    ramp.color_ramp.elements[1].color = (0.15, 0.25, 0.12, 1.0)  # verdigris
    links.new(noise.outputs['Fac'], ramp.inputs['Fac'])
    links.new(ramp.outputs['Color'], bsdf.inputs['Base Color'])

    return mat


def make_ceramic_material():
    """Off-white ceramic / porcelain for fuse holders and insulators."""
    mat = bpy.data.materials.new("BB_Ceramic")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.65, 0.62, 0.55, 1.0)
    bsdf.inputs['Metallic'].default_value = 0.0
    bsdf.inputs['Roughness'].default_value = 0.3
    # Slight subsurface for porcelain feel
    bsdf.inputs['Subsurface Weight'].default_value = 0.05

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    # Subtle grime
    tex_coord = nodes.new('ShaderNodeTexCoord')
    tex_coord.location = (-400, -200)
    noise = nodes.new('ShaderNodeTexNoise')
    noise.location = (-100, -200)
    noise.inputs['Scale'].default_value = 40.0
    noise.inputs['Detail'].default_value = 4.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    ramp = nodes.new('ShaderNodeValToRGB')
    ramp.location = (100, -200)
    ramp.color_ramp.elements[0].position = 0.45
    ramp.color_ramp.elements[0].color = (0.65, 0.62, 0.55, 1.0)
    ramp.color_ramp.elements[1].position = 0.6
    ramp.color_ramp.elements[1].color = (0.35, 0.30, 0.25, 1.0)
    links.new(noise.outputs['Fac'], ramp.inputs['Fac'])
    links.new(ramp.outputs['Color'], bsdf.inputs['Base Color'])

    return mat


def make_bakelite_material():
    """Dark brown bakelite for switch handles and knobs."""
    mat = bpy.data.materials.new("BB_Bakelite")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.06, 0.03, 0.02, 1.0)
    bsdf.inputs['Metallic'].default_value = 0.0
    bsdf.inputs['Roughness'].default_value = 0.25

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def make_wire_material():
    """Rubber-insulated wire — dark rubber/cloth."""
    mat = bpy.data.materials.new("BB_Wire")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.02, 0.02, 0.02, 1.0)
    bsdf.inputs['Metallic'].default_value = 0.0
    bsdf.inputs['Roughness'].default_value = 0.8

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


def make_glass_material():
    """Dirty glass for the small inspection window."""
    mat = bpy.data.materials.new("BB_DirtyGlass")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new('ShaderNodeOutputMaterial')
    out.location = (800, 0)

    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (400, 0)
    bsdf.inputs['Base Color'].default_value = (0.6, 0.58, 0.5, 1.0)
    bsdf.inputs['Metallic'].default_value = 0.0
    bsdf.inputs['Roughness'].default_value = 0.15
    bsdf.inputs['Transmission Weight'].default_value = 0.7
    bsdf.inputs['IOR'].default_value = 1.5

    links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    return mat


# ─── Geometry Builders ──────────────────────────────────────────────────────

def make_enclosure(col, mat_rust):
    """
    Main steel enclosure box: ~40cm W × 55cm H × 20cm D.
    Sheet metal with folded edges, ventilation slots at bottom.
    """
    W, H, D = 0.40, 0.55, 0.20

    # Back panel
    bpy.ops.mesh.primitive_cube_add(size=1)
    back = bpy.context.active_object
    back.name = "BB_Enclosure_Back"
    back.scale = (W/2, 0.002, H/2)
    back.location = (0, -D/2, H/2)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    back.data.materials.append(mat_rust)
    link_to(col, back)

    # Side panels (left, right)
    for side_name, x_pos, x_scale_sign in [("Left", -W/2, 1), ("Right", W/2, 1)]:
        bpy.ops.mesh.primitive_cube_add(size=1)
        side = bpy.context.active_object
        side.name = f"BB_Enclosure_Side_{side_name}"
        side.scale = (0.002, D/2, H/2)
        side.location = (x_pos, 0, H/2)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        side.data.materials.append(mat_rust)
        link_to(col, side)

    # Top panel
    bpy.ops.mesh.primitive_cube_add(size=1)
    top = bpy.context.active_object
    top.name = "BB_Enclosure_Top"
    top.scale = (W/2, D/2, 0.002)
    top.location = (0, 0, H)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    top.data.materials.append(mat_rust)
    link_to(col, top)

    # Bottom panel with ventilation slots
    bpy.ops.mesh.primitive_cube_add(size=1)
    bottom = bpy.context.active_object
    bottom.name = "BB_Enclosure_Bottom"
    bottom.scale = (W/2, D/2, 0.002)
    bottom.location = (0, 0, 0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bottom.data.materials.append(mat_rust)
    link_to(col, bottom)

    # Ventilation slots on bottom
    for i in range(5):
        bpy.ops.mesh.primitive_cube_add(size=1)
        slot = bpy.context.active_object
        slot.name = f"BB_Vent_Slot_{i}"
        slot_x = -0.12 + i * 0.06
        slot.scale = (0.02, 0.06, 0.003)
        slot.location = (slot_x, 0.02, -0.001)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        link_to(col, slot)

    # Top reinforcement lip / flange
    bpy.ops.mesh.primitive_cube_add(size=1)
    lip = bpy.context.active_object
    lip.name = "BB_Top_Lip"
    lip.scale = (W/2 + 0.008, D/2 + 0.008, 0.006)
    lip.location = (0, 0, H + 0.004)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    lip.data.materials.append(mat_rust)
    link_to(col, lip)

    # Bottom flange
    bpy.ops.mesh.primitive_cube_add(size=1)
    blip = bpy.context.active_object
    blip.name = "BB_Bottom_Lip"
    blip.scale = (W/2 + 0.008, D/2 + 0.008, 0.006)
    blip.location = (0, 0, -0.004)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    blip.data.materials.append(mat_rust)
    link_to(col, blip)

    return back


def make_door(col, mat_rust, mat_glass):
    """
    Front door panel with hinges, latch, handle, and small inspection window.
    Positioned slightly open (ajar ~15 degrees) to show interior.
    """
    W, H = 0.38, 0.53
    D = 0.003

    # Door panel
    bpy.ops.mesh.primitive_cube_add(size=1)
    door = bpy.context.active_object
    door.name = "BB_Door_Panel"
    door.scale = (W/2, D/2, H/2)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    # Move origin to hinge side (left edge)
    # First move mesh so that the left edge is at origin
    bm = bmesh.new()
    bm.from_mesh(door.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(W/2, 0, 0))
    bm.to_mesh(door.data)
    bm.free()

    door.location = (-W/2 + 0.01, 0.10, H/2 + 0.01)
    door.rotation_euler = (0, 0, math.radians(25))  # open 25 degrees
    door.data.materials.append(mat_rust)
    link_to(col, door)

    # Door reinforcement frame (embossed rectangle on door face)
    bpy.ops.mesh.primitive_cube_add(size=1)
    frame = bpy.context.active_object
    frame.name = "BB_Door_Frame_Emboss"
    frame.scale = (W/2 - 0.025, 0.003, H/2 - 0.03)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bm = bmesh.new()
    bm.from_mesh(frame.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(W/2, 0, 0))
    bm.to_mesh(frame.data)
    bm.free()
    frame.location = (-W/2 + 0.01, 0.10 + D/2 + 0.002, H/2 + 0.01)
    frame.rotation_euler = (0, 0, math.radians(25))
    frame.data.materials.append(mat_rust)
    frame.parent = door
    frame.matrix_parent_inverse = door.matrix_world.inverted()
    link_to(col, frame)

    # Inspection window (small round porthole)
    bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=0.03, depth=0.008)
    window = bpy.context.active_object
    window.name = "BB_Door_Window"
    window.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    # Position on door
    bm = bmesh.new()
    bm.from_mesh(window.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(W/2 - 0.04, 0, 0.08))
    bm.to_mesh(window.data)
    bm.free()
    window.location = (-W/2 + 0.01, 0.10 + D/2 + 0.001, H/2 + 0.01)
    window.rotation_euler = (0, 0, math.radians(25))
    window.data.materials.append(mat_glass)
    window.parent = door
    window.matrix_parent_inverse = door.matrix_world.inverted()
    link_to(col, window)

    # Window frame ring
    bpy.ops.mesh.primitive_torus_add(
        major_radius=0.032, minor_radius=0.004,
        major_segments=24, minor_segments=8
    )
    w_ring = bpy.context.active_object
    w_ring.name = "BB_Door_WindowRing"
    w_ring.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bm = bmesh.new()
    bm.from_mesh(w_ring.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(W/2 - 0.04, 0.003, 0.08))
    bm.to_mesh(w_ring.data)
    bm.free()
    w_ring.location = (-W/2 + 0.01, 0.10 + D/2 + 0.001, H/2 + 0.01)
    w_ring.rotation_euler = (0, 0, math.radians(25))
    w_ring.data.materials.append(mat_rust)
    w_ring.parent = door
    w_ring.matrix_parent_inverse = door.matrix_world.inverted()
    link_to(col, w_ring)

    # Hinges (2x)
    for hz in [0.08, 0.45]:
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.008, depth=0.04)
        hinge = bpy.context.active_object
        hinge.name = f"BB_Hinge_{hz:.0f}"
        hinge.location = (-W/2 + 0.01, 0.10, hz)
        hinge.data.materials.append(mat_rust)
        link_to(col, hinge)

        # Hinge plates
        for yoff in [-0.004, 0.004]:
            bpy.ops.mesh.primitive_cube_add(size=1)
            plate = bpy.context.active_object
            plate.name = f"BB_HingePlate_{hz:.0f}_{yoff}"
            plate.scale = (0.02, 0.002, 0.015)
            plate.location = (-W/2 + 0.01 + 0.015, 0.10 + yoff, hz)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
            plate.data.materials.append(mat_rust)
            link_to(col, plate)

    # Latch / hasp (right side of door)
    bpy.ops.mesh.primitive_cube_add(size=1)
    latch_base = bpy.context.active_object
    latch_base.name = "BB_Latch_Base"
    latch_base.scale = (0.015, 0.006, 0.04)
    latch_base.location = (W/2 - 0.02, 0.10 + 0.01, H/2)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    latch_base.data.materials.append(mat_rust)
    link_to(col, latch_base)

    # Latch lever
    bpy.ops.mesh.primitive_cube_add(size=1)
    latch_lever = bpy.context.active_object
    latch_lever.name = "BB_Latch_Lever"
    latch_lever.scale = (0.008, 0.015, 0.05)
    latch_lever.location = (W/2 - 0.02, 0.10 + 0.02, H/2)
    latch_lever.rotation_euler = (math.radians(15), 0, 0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    latch_lever.data.materials.append(mat_rust)
    link_to(col, latch_lever)

    # Handle — simple D-handle
    bpy.ops.mesh.primitive_torus_add(
        major_radius=0.025, minor_radius=0.004,
        major_segments=16, minor_segments=8
    )
    handle = bpy.context.active_object
    handle.name = "BB_Door_Handle"
    handle.scale = (1, 0.5, 1)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    # Cut in half — we'll just use the full torus for now, it reads as a pull handle
    bm = bmesh.new()
    bm.from_mesh(handle.data)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(W/2 - 0.06, 0.003, -0.03))
    bm.to_mesh(handle.data)
    bm.free()
    handle.location = (-W/2 + 0.01, 0.10 + D/2 + 0.001, H/2 + 0.01)
    handle.rotation_euler = (0, 0, math.radians(25))
    handle.data.materials.append(mat_rust)
    handle.parent = door
    handle.matrix_parent_inverse = door.matrix_world.inverted()
    link_to(col, handle)

    return door


def make_interior_components(col, mat_interior, mat_copper, mat_ceramic, mat_bakelite, mat_wire):
    """
    Interior electrical components:
    - Mounting backplate
    - 3 rows of bus bars (copper)
    - 6 ceramic fuse holders with fuses
    - 2 knife switches
    - Terminal strips
    - Wire runs
    """
    W, H, D = 0.36, 0.50, 0.16

    # Mounting backplate (slightly proud of enclosure back)
    bpy.ops.mesh.primitive_cube_add(size=1)
    backplate = bpy.context.active_object
    backplate.name = "BB_Interior_Backplate"
    backplate.scale = (W/2 - 0.01, 0.003, H/2 - 0.02)
    backplate.location = (0, -D/2 + 0.015, H/2 + 0.02)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    backplate.data.materials.append(mat_interior)
    link_to(col, backplate)

    # ── Bus bars (3 horizontal copper bars) ──
    for i, bz in enumerate([0.12, 0.30, 0.48]):
        bpy.ops.mesh.primitive_cube_add(size=1)
        bar = bpy.context.active_object
        bar.name = f"BB_BusBar_{i}"
        bar.scale = (W/2 - 0.03, 0.003, 0.008)
        bar.location = (0, -D/2 + 0.025, bz)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        bar.data.materials.append(mat_copper)
        add_bevel_modifier(bar, 0.001, 2)
        link_to(col, bar)

        # Bus bar mounting standoffs (ceramic insulators)
        for sx in [-0.12, 0, 0.12]:
            bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.008, depth=0.015)
            standoff = bpy.context.active_object
            standoff.name = f"BB_Standoff_{i}_{sx}"
            standoff.rotation_euler = (math.radians(90), 0, 0)
            standoff.location = (sx, -D/2 + 0.018, bz)
            standoff.data.materials.append(mat_ceramic)
            link_to(col, standoff)

    # ── Ceramic fuse holders (6 total, 2 rows of 3) ──
    for row, fz in enumerate([0.18, 0.36]):
        for fi, fx in enumerate([-0.10, 0.0, 0.10]):
            # Fuse base (ceramic block)
            bpy.ops.mesh.primitive_cube_add(size=1)
            fbase = bpy.context.active_object
            fbase.name = f"BB_FuseBase_{row}_{fi}"
            fbase.scale = (0.018, 0.012, 0.025)
            fbase.location = (fx, -D/2 + 0.035, fz)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
            fbase.data.materials.append(mat_ceramic)
            add_bevel_modifier(fbase, 0.002, 2)
            link_to(col, fbase)

            # Fuse element (cylinder on top)
            bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.005, depth=0.035)
            fuse = bpy.context.active_object
            fuse.name = f"BB_Fuse_{row}_{fi}"
            fuse.location = (fx, -D/2 + 0.035, fz)
            fuse.rotation_euler = (0, 0, 0)
            fuse.data.materials.append(mat_ceramic)
            link_to(col, fuse)

            # Fuse end caps (copper)
            for cap_z in [-0.016, 0.016]:
                bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.006, depth=0.005)
                cap = bpy.context.active_object
                cap.name = f"BB_FuseCap_{row}_{fi}_{cap_z}"
                cap.location = (fx, -D/2 + 0.035, fz + cap_z)
                cap.data.materials.append(mat_copper)
                link_to(col, cap)

            # Contact clips (copper strips on fuse base)
            for clip_z in [-0.020, 0.020]:
                bpy.ops.mesh.primitive_cube_add(size=1)
                clip = bpy.context.active_object
                clip.name = f"BB_FuseClip_{row}_{fi}_{clip_z}"
                clip.scale = (0.003, 0.008, 0.012)
                clip.location = (fx, -D/2 + 0.042, fz + clip_z)
                bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
                clip.data.materials.append(mat_copper)
                link_to(col, clip)

    # ── Knife switches (2x) ──
    for si, (sx, sz) in enumerate([(-0.12, 0.42), (0.12, 0.42)]):
        # Switch base plate
        bpy.ops.mesh.primitive_cube_add(size=1)
        sw_base = bpy.context.active_object
        sw_base.name = f"BB_SwitchBase_{si}"
        sw_base.scale = (0.025, 0.008, 0.05)
        sw_base.location = (sx, -D/2 + 0.035, sz)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        sw_base.data.materials.append(mat_ceramic)
        link_to(col, sw_base)

        # Jaw contacts (bottom)
        for jaw_x in [-0.012, 0.012]:
            bpy.ops.mesh.primitive_cube_add(size=1)
            jaw = bpy.context.active_object
            jaw.name = f"BB_SwitchJaw_{si}_{jaw_x}"
            jaw.scale = (0.003, 0.006, 0.018)
            jaw.location = (sx + jaw_x, -D/2 + 0.042, sz - 0.02)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
            jaw.data.materials.append(mat_copper)
            link_to(col, jaw)

        # Blade (the knife part — angled if "on")
        bpy.ops.mesh.primitive_cube_add(size=1)
        blade = bpy.context.active_object
        blade.name = f"BB_SwitchBlade_{si}"
        blade.scale = (0.02, 0.002, 0.04)
        blade.location = (sx, -D/2 + 0.045, sz - 0.015)
        if si == 0:
            blade.rotation_euler = (math.radians(-30), 0, 0)  # thrown open
        else:
            blade.rotation_euler = (math.radians(5), 0, 0)  # closed
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        blade.data.materials.append(mat_copper)
        link_to(col, blade)

        # Handle (bakelite knob on blade)
        bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.012, depth=0.015)
        knob = bpy.context.active_object
        knob.name = f"BB_SwitchKnob_{si}"
        knob.rotation_euler = (math.radians(90), 0, 0)
        knob.location = (sx, -D/2 + 0.050, sz + 0.015 if si == 1 else sz - 0.045)
        knob.data.materials.append(mat_bakelite)
        link_to(col, knob)

    # ── Terminal strip at bottom ──
    bpy.ops.mesh.primitive_cube_add(size=1)
    term_strip = bpy.context.active_object
    term_strip.name = "BB_TerminalStrip"
    term_strip.scale = (W/2 - 0.04, 0.005, 0.012)
    term_strip.location = (0, -D/2 + 0.030, 0.06)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    term_strip.data.materials.append(mat_ceramic)
    link_to(col, term_strip)

    # Terminal screws
    for ti in range(8):
        tx = -0.12 + ti * 0.035
        bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.004, depth=0.006)
        screw = bpy.context.active_object
        screw.name = f"BB_TermScrew_{ti}"
        screw.rotation_euler = (math.radians(90), 0, 0)
        screw.location = (tx, -D/2 + 0.037, 0.06)
        screw.data.materials.append(mat_copper)
        link_to(col, screw)

    # ── Wire runs (simple curves from bus bars to terminals) ──
    wire_paths = [
        [(-0.10, -0.06, 0.12), (-0.10, -0.05, 0.09), (-0.10, -0.06, 0.06)],
        [(0.0, -0.06, 0.12), (0.0, -0.04, 0.09), (0.0, -0.06, 0.06)],
        [(0.10, -0.06, 0.12), (0.10, -0.05, 0.09), (0.10, -0.06, 0.06)],
        [(-0.10, -0.06, 0.30), (-0.12, -0.04, 0.24), (-0.10, -0.06, 0.18)],
        [(0.10, -0.06, 0.30), (0.12, -0.04, 0.24), (0.10, -0.06, 0.18)],
    ]

    for wi, path in enumerate(wire_paths):
        curve_data = bpy.data.curves.new(f"BB_WireCurve_{wi}", type='CURVE')
        curve_data.dimensions = '3D'
        curve_data.bevel_depth = 0.003
        curve_data.bevel_resolution = 4

        spline = curve_data.splines.new('BEZIER')
        spline.bezier_points.add(len(path) - 1)
        for pi, (px, py, pz) in enumerate(path):
            bp = spline.bezier_points[pi]
            bp.co = (px, py, pz)
            bp.handle_left_type = 'AUTO'
            bp.handle_right_type = 'AUTO'

        wire_obj = bpy.data.objects.new(f"BB_Wire_{wi}", curve_data)
        wire_obj.data.materials.append(mat_wire)
        link_to(col, wire_obj)


def make_conduit_entries(col, mat_rust):
    """Conduit entry points — pipes entering top and bottom of the box."""
    # Top conduit
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.015, depth=0.12)
    top_conduit = bpy.context.active_object
    top_conduit.name = "BB_Conduit_Top"
    top_conduit.location = (0.08, -0.05, 0.55 + 0.06)
    top_conduit.data.materials.append(mat_rust)
    link_to(col, top_conduit)

    # Conduit nut/coupling
    bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.020, depth=0.015)
    nut_top = bpy.context.active_object
    nut_top.name = "BB_ConduitNut_Top"
    nut_top.location = (0.08, -0.05, 0.55 + 0.005)
    nut_top.data.materials.append(mat_rust)
    link_to(col, nut_top)

    # Bottom conduit
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.015, depth=0.10)
    bot_conduit = bpy.context.active_object
    bot_conduit.name = "BB_Conduit_Bottom"
    bot_conduit.location = (-0.08, -0.05, -0.05)
    bot_conduit.data.materials.append(mat_rust)
    link_to(col, bot_conduit)

    bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.020, depth=0.015)
    nut_bot = bpy.context.active_object
    nut_bot.name = "BB_ConduitNut_Bottom"
    nut_bot.location = (-0.08, -0.05, -0.005)
    nut_bot.data.materials.append(mat_rust)
    link_to(col, nut_bot)

    # Side conduit (right side)
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.012, depth=0.08)
    side_conduit = bpy.context.active_object
    side_conduit.name = "BB_Conduit_Side"
    side_conduit.rotation_euler = (0, math.radians(90), 0)
    side_conduit.location = (0.20 + 0.04, -0.03, 0.30)
    side_conduit.data.materials.append(mat_rust)
    link_to(col, side_conduit)

    bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.016, depth=0.012)
    nut_side = bpy.context.active_object
    nut_side.name = "BB_ConduitNut_Side"
    nut_side.rotation_euler = (0, math.radians(90), 0)
    nut_side.location = (0.20 + 0.005, -0.03, 0.30)
    nut_side.data.materials.append(mat_rust)
    link_to(col, nut_side)


def make_mounting_hardware(col, mat_rust):
    """Wall mounting brackets and bolts."""
    for mz in [0.08, 0.47]:
        for mx in [-0.15, 0.15]:
            # L-bracket
            bpy.ops.mesh.primitive_cube_add(size=1)
            bracket = bpy.context.active_object
            bracket.name = f"BB_Bracket_{mx}_{mz}"
            bracket.scale = (0.02, 0.025, 0.003)
            bracket.location = (mx, -0.10 - 0.025, mz)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
            bracket.data.materials.append(mat_rust)
            link_to(col, bracket)

            # Bolt head
            bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.006, depth=0.005)
            bolt = bpy.context.active_object
            bolt.name = f"BB_Bolt_{mx}_{mz}"
            bolt.rotation_euler = (math.radians(90), 0, 0)
            bolt.location = (mx, -0.10 - 0.04, mz)
            bolt.data.materials.append(mat_rust)
            link_to(col, bolt)


def make_label_plate(col, mat_copper):
    """Small brass/copper name plate on front of door."""
    bpy.ops.mesh.primitive_cube_add(size=1)
    plate = bpy.context.active_object
    plate.name = "BB_NamePlate"
    plate.scale = (0.04, 0.001, 0.015)
    plate.location = (0, 0.105, 0.50)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    plate.data.materials.append(mat_copper)
    add_bevel_modifier(plate, 0.0005, 2)
    link_to(col, plate)

    # Plate mounting rivets
    for rx in [-0.03, 0.03]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=4, radius=0.002)
        rivet = bpy.context.active_object
        rivet.name = f"BB_PlateRivet_{rx}"
        rivet.location = (rx, 0.107, 0.50)
        rivet.data.materials.append(mat_copper)
        link_to(col, rivet)


def make_danger_plate(col, mat_rust):
    """Small warning/danger sign (just a small rectangular plate)."""
    bpy.ops.mesh.primitive_cube_add(size=1)
    danger = bpy.context.active_object
    danger.name = "BB_DangerPlate"
    danger.scale = (0.035, 0.001, 0.025)
    danger.location = (0, 0.105, 0.15)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    danger.data.materials.append(mat_rust)
    link_to(col, danger)


def make_ground_stud(col, mat_copper, mat_rust):
    """Grounding stud on the side of the enclosure."""
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.006, depth=0.015)
    stud = bpy.context.active_object
    stud.name = "BB_GroundStud"
    stud.rotation_euler = (0, math.radians(90), 0)
    stud.location = (0.201, 0, 0.08)
    stud.data.materials.append(mat_copper)
    link_to(col, stud)

    # Nut
    bpy.ops.mesh.primitive_cylinder_add(vertices=6, radius=0.008, depth=0.005)
    nut = bpy.context.active_object
    nut.name = "BB_GroundNut"
    nut.rotation_euler = (0, math.radians(90), 0)
    nut.location = (0.209, 0, 0.08)
    nut.data.materials.append(mat_copper)
    link_to(col, nut)

    # Wire lug
    bpy.ops.mesh.primitive_cube_add(size=1)
    lug = bpy.context.active_object
    lug.name = "BB_GroundLug"
    lug.scale = (0.002, 0.012, 0.008)
    lug.location = (0.213, 0, 0.08)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    lug.data.materials.append(mat_copper)
    link_to(col, lug)


# ─── Lighting & Camera Setup ───────────────────────────────────────────────

def setup_studio_lighting():
    """Three-point lighting with warm key + cool fill, period-appropriate."""
    # Key light (warm, simulating nearby oil lamp / bulb)
    bpy.ops.object.light_add(type='AREA', radius=0.5)
    key = bpy.context.active_object
    key.name = "Key_Light"
    key.location = (0.6, 0.8, 0.8)
    key.rotation_euler = (math.radians(-35), math.radians(25), math.radians(-15))
    key.data.energy = 80
    key.data.color = (1.0, 0.85, 0.6)
    key.data.size = 0.5

    # Fill light (cool, ambient/environment light)
    bpy.ops.object.light_add(type='AREA', radius=0.8)
    fill = bpy.context.active_object
    fill.name = "Fill_Light"
    fill.location = (-0.8, 0.5, 0.5)
    fill.rotation_euler = (math.radians(-20), math.radians(-40), 0)
    fill.data.energy = 25
    fill.data.color = (0.7, 0.8, 1.0)
    fill.data.size = 0.8

    # Rim light (from behind, edge definition)
    bpy.ops.object.light_add(type='AREA', radius=0.3)
    rim = bpy.context.active_object
    rim.name = "Rim_Light"
    rim.location = (-0.3, -0.8, 0.7)
    rim.rotation_euler = (math.radians(-25), math.radians(160), 0)
    rim.data.energy = 50
    rim.data.color = (0.9, 0.9, 1.0)
    rim.data.size = 0.3

    # Small interior light (inside the box, to light up the interior)
    bpy.ops.object.light_add(type='POINT', radius=0.05)
    interior = bpy.context.active_object
    interior.name = "Interior_Light"
    interior.location = (0, 0.02, 0.30)
    interior.data.energy = 5
    interior.data.color = (1.0, 0.9, 0.7)
    interior.data.shadow_soft_size = 0.05


def add_camera(name, location, rotation, lens=50):
    bpy.ops.object.camera_add()
    cam = bpy.context.active_object
    cam.name = name
    cam.location = location
    cam.rotation_euler = Euler(rotation)
    cam.data.lens = lens
    cam.data.clip_start = 0.01
    cam.data.clip_end = 100
    return cam


def setup_cameras():
    """Five camera angles for the benchmark renders."""
    cams = {}

    # 01 — Front 3/4 view (closed perspective, showing the door ajar)
    cams['01_front_closed_3quarter'] = add_camera(
        "Cam_01_Front3Q",
        (0.55, 0.55, 0.35),
        (math.radians(65), 0, math.radians(135)),
        lens=50
    )

    # 02 — Door open interior view (looking into the box)
    cams['02_door_open_interior'] = add_camera(
        "Cam_02_Interior",
        (0.15, 0.35, 0.30),
        (math.radians(75), 0, math.radians(160)),
        lens=35
    )

    # 03 — Detail closeup (fuse holders and switches)
    cams['03_detail_closeup'] = add_camera(
        "Cam_03_Detail",
        (0.05, 0.20, 0.28),
        (math.radians(80), 0, math.radians(170)),
        lens=85
    )

    # 04 — Side construction (showing conduits, brackets, box thickness)
    cams['04_side_construction'] = add_camera(
        "Cam_04_Side",
        (0.50, 0.0, 0.30),
        (math.radians(80), 0, math.radians(90)),
        lens=50
    )

    # 05 — Wireframe overlay (same as 01 angle, render as wireframe)
    cams['05_wireframe'] = add_camera(
        "Cam_05_Wireframe",
        (0.55, 0.55, 0.35),
        (math.radians(65), 0, math.radians(135)),
        lens=50
    )

    return cams


def setup_render_settings():
    """Cycles render at reasonable quality for review."""
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 256
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1920
    scene.render.resolution_y = 1080
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'

    # World background — dark neutral
    world = bpy.data.worlds.new("BB_World")
    scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes['Background']
    bg.inputs['Color'].default_value = (0.012, 0.012, 0.015, 1.0)
    bg.inputs['Strength'].default_value = 0.3


def render_all_cameras(cams, output_dir, pass_name="pass1"):
    """Render from each camera and save."""
    scene = bpy.context.scene

    for cam_name, cam_obj in cams.items():
        scene.camera = cam_obj

        if cam_name == '05_wireframe':
            # Enable wireframe overlay via freestyle
            scene.render.use_freestyle = True
            scene.view_layers[0].use_freestyle = True
            # Also set a wireframe modifier approach — simpler: just render with
            # freestyle lines enabled
            lineset = scene.view_layers[0].freestyle_settings.linesets.new("Wireframe")
            lineset.select_edge_mark = False
            lineset.select_silhouette = True
            lineset.select_border = True
            lineset.select_crease = True
            # Make lines visible
            linestyle = lineset.linestyle
            linestyle.color = (0.8, 0.85, 0.9)
            linestyle.thickness = 1.2

        filepath = os.path.join(output_dir, f"{cam_name}.png")
        scene.render.filepath = filepath
        bpy.ops.render.render(write_still=True)
        print(f"Rendered: {filepath}")

        if cam_name == '05_wireframe':
            scene.render.use_freestyle = False


# ─── Main ───────────────────────────────────────────────────────────────────

def main():
    output_dir = os.path.dirname(bpy.data.filepath) if bpy.data.filepath else "/tmp"
    # Override to our target
    output_dir = "/home/a1/WORK/BLACK_BEACON_OPUS_BLENDER/Art/Blender/OpusBreakerBox"

    clear_scene()

    # Create collection
    col = new_collection("BB_BreakerBox")

    # Create materials
    mat_rust = make_rusted_metal_material()
    mat_interior = make_interior_metal_material()
    mat_copper = make_copper_material()
    mat_ceramic = make_ceramic_material()
    mat_bakelite = make_bakelite_material()
    mat_wire = make_wire_material()
    mat_glass = make_glass_material()

    # Build geometry
    print("Building enclosure...")
    make_enclosure(col, mat_rust)

    print("Building door...")
    make_door(col, mat_rust, mat_glass)

    print("Building interior components...")
    make_interior_components(col, mat_interior, mat_copper, mat_ceramic, mat_bakelite, mat_wire)

    print("Building conduit entries...")
    make_conduit_entries(col, mat_rust)

    print("Building mounting hardware...")
    make_mounting_hardware(col, mat_rust)

    print("Building label plate...")
    make_label_plate(col, mat_copper)
    make_danger_plate(col, mat_rust)

    print("Building ground stud...")
    make_ground_stud(col, mat_copper, mat_rust)

    # Apply smooth shading to all mesh objects
    for obj in col.objects:
        if obj.type == 'MESH':
            set_smooth(obj)

    # Setup scene
    print("Setting up lighting...")
    setup_studio_lighting()

    print("Setting up cameras...")
    cams = setup_cameras()

    print("Configuring render settings...")
    setup_render_settings()

    # Save the .blend file
    blend_path = os.path.join(output_dir, "BB_BreakerBox_Opus.blend")
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print(f"Saved: {blend_path}")

    # Render all cameras
    print("Rendering pass 1...")
    render_all_cameras(cams, output_dir, "pass1")

    print("Pass 1 complete!")


if __name__ == "__main__":
    main()
