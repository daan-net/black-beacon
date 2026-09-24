"""Fast geometry-only rebuild after materials exist; also audits draw sections."""
from pathlib import Path
import json
import unreal as u
ROOT=Path(__file__).resolve().parent
DEST='/Game/BlackBeacon/Art/Lighthouse'
AT=u.AssetToolsHelpers.get_asset_tools()
materials={name:u.load_asset(DEST+'/Materials/M_LH_'+name) for name in
           ('TowerPaint','DarkIron','WarmBrass','AgedWood','WetRock','InteriorPlaster','CutStone')}
materials.update({'LanternGlass':u.load_asset(DEST+'/Materials/M_V04_Glazing'),
                  'LensGlass':u.load_asset(DEST+'/Materials/M_V04_LensGlass'),
                  'Lamp':u.load_asset(DEST+'/Materials/M_V04_Practical')})
audit=[]
for f in sorted((ROOT.parents[1]/'Lighthouse'/'SourceMeshes').glob('*.obj')):
    task=u.AssetImportTask();task.filename=str(f);task.destination_path=DEST+'/Meshes'
    task.automated=True;task.replace_existing=True;task.save=True
    AT.import_asset_tasks([task])
    mesh=u.load_asset(DEST+'/Meshes/'+f.stem)
    if not mesh:raise RuntimeError('Missing '+f.stem)
    names=[]
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'))
        if name not in materials:name=str(slot.get_editor_property('material_slot_name'))
        if name not in materials or not materials[name]:raise RuntimeError('Missing surface '+name)
        mesh.set_material(i,materials[name]);names.append(name)
    settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',False)
    mesh.set_editor_property('nanite_settings',settings)
    if f.stem=='SM_BB_LH_GalleryDeck':
        mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    sections=mesh.get_num_sections(0)
    if sections>16:raise RuntimeError('Excess sections on '+f.stem+': '+str(sections))
    audit.append({'mesh':f.stem,'sections':sections,'materials':names,'bounds':str(mesh.get_bounds())})
out=Path(u.Paths.project_saved_dir())/'VisualRebuildV04'
out.mkdir(parents=True,exist_ok=True)
(out/'AssetAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
u.log('BB_V04_MESH_AUDIT_COMPLETE')
