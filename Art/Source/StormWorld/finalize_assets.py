"""Persist usage/import flags and audit the assets after generation (UE Python)."""
from pathlib import Path
import json
import unreal as u
E=u.MaterialEditingLibrary
ROOT='/Game/BlackBeacon/Storm'
report={}
for name in ('M_StormOcean','M_SeaSpray','M_CoastalMist','M_RainSplash'):
    a=u.load_asset(ROOT+'/Materials/'+name)
    report[name]={'domain':str(a.get_editor_property('material_domain')),'blend':str(a.get_editor_property('blend_mode'))}
    if name in ('M_SeaSpray','M_CoastalMist','M_RainSplash'):
        a.set_editor_property('used_with_instanced_static_meshes',True)
    E.recompile_material(a)
    u.EditorAssetLibrary.save_loaded_asset(a)
cloud=u.load_asset(ROOT+'/Materials/MI_StormCloudNative')
report['MI_StormCloudNative']={'parent':cloud.get_editor_property('parent').get_path_name()}
for name in ('SM_StormOcean','SM_StormCoast'):
    a=u.load_asset(ROOT+'/Meshes/'+name)
    if name=='SM_StormOcean':
        settings=a.get_editor_property('nanite_settings')
        settings.set_editor_property('enabled',False)
        a.set_editor_property('nanite_settings',settings)
        u.EditorAssetLibrary.save_loaded_asset(a)
    bounds=a.get_bounding_box()
    report[name]={'min':str(bounds.min),'max':str(bounds.max),'nanite':a.get_editor_property('nanite_settings').get_editor_property('enabled')}
for name in ('SW_StormWind','SW_StormRain','SW_StormSurf','SW_StormThunder'):
    a=u.load_asset(ROOT+'/Audio/'+name)
    if name!='SW_StormThunder':
        a.set_editor_property('virtualization_mode',u.VirtualizationMode.PLAY_WHEN_SILENT)
    u.EditorAssetLibrary.save_loaded_asset(a)
    report[name]={'loop':a.get_editor_property('looping'),'virtualization':str(a.get_editor_property('virtualization_mode'))}
out=Path(u.Paths.project_saved_dir())/'StormReview'
out.mkdir(exist_ok=True,parents=True)
(out/'AssetAudit.json').write_text(json.dumps(report,indent=2))
print('BB_STORM_FINALIZED',flush=True)
