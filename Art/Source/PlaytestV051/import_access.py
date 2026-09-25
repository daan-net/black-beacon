"""Geometry-only import: no material rebuild, no collision-deck import."""
from pathlib import Path
import unreal as u
ROOT=Path(__file__).resolve().parent
BASE='/Game/BlackBeacon/Art/Lighthouse'
editor=u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
if not editor:editor=u.new_object(u.StaticMeshEditorSubsystem)
for file in sorted((ROOT/'Generated').glob('*.obj')):
    path=BASE+'/Meshes/'+file.stem
    original=u.load_asset(path)
    surfaces={str(slot.get_editor_property('imported_material_slot_name')):slot.get_editor_property('material_interface') for slot in original.get_editor_property('static_materials')}
    task=u.AssetImportTask();task.filename=str(file);task.destination_path=BASE+'/Meshes'
    task.automated=True;task.replace_existing=True;task.save=False
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=u.load_asset(path)
    for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'))
        if name not in surfaces:raise RuntimeError('Missing original surface: '+name)
        mesh.set_material(index,surfaces[name])
        if name in ('LanternGlass','Lamp'):editor.enable_section_cast_shadow(mesh,False,0,index)
    settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',False);mesh.set_editor_property('nanite_settings',settings)
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    u.log('V051_ACCESS_IMPORTED '+file.stem)
