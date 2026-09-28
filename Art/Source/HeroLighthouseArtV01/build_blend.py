"""Run with Blender --background --python. Editable additive modules, metres/Z-up."""
from pathlib import Path
import bpy
ROOT=Path(__file__).resolve().parent
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for file in sorted((ROOT/'Generated').glob('*.obj')):
    bpy.ops.wm.obj_import(filepath=str(file),forward_axis='NEGATIVE_Y',up_axis='Z')
    for obj in bpy.context.selected_objects:
        # OBJ export reverses Y for Unreal; undo it in the editable source scene.
        obj.scale=(.01,-.01,.01)
        bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
        obj['purpose']='Exterior presentation only. No gameplay collision.'
bpy.context.scene.unit_settings.system='METRIC'
bpy.context.scene['baseline']='06a986f; additive Hero Lighthouse Art V0.1 continuation'
bpy.context.scene['source']='generate.py; existing tower / stairs / lantern deliberately excluded'
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'HeroExterior.blend'))
