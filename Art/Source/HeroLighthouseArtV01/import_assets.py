"""Scoped import: three new opaque meshes and one isolated exterior material family."""
from pathlib import Path
import json
import unreal as u

ROOT=Path(__file__).resolve().parent
helper=ROOT.parent/'VisualRebuildV04/build_assets.py'
exec(compile(helper.read_text().split("\nm=material('M_V04_WeatheredSurface')")[0],str(helper),'exec'))
ROOT=Path(__file__).resolve().parent
m=material('M_Hero_ExteriorFinish')
p=node(m,u.MaterialExpressionWorldPosition,'world')
n=node(m,u.MaterialExpressionVertexNormalWS,'normal')
stone=scalar(m,'Stone',0)
surface=custom(m,'weathering',(ROOT/'finish.hlsl').read_text(),{'P':p,'Stone':stone},u.CustomMaterialOutputType.CMOT_FLOAT4)
prop(custom(m,'base','return V.rgb;',{'V':surface},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_BASE_COLOR)
prop(custom(m,'rough','return V.a;',{'V':surface}),u.MaterialProperty.MP_ROUGHNESS)
prop(scalar(m,'Metallic',0),u.MaterialProperty.MP_METALLIC)
prop(scalar(m,'Specular',.25),u.MaterialProperty.MP_SPECULAR)
m.set_editor_property('tangent_space_normal',False)
prop(custom(m,'pores',
    'float3 g=float3(sin(P.y*1.2+sin(P.z*.8)),cos(P.z*1.1+sin(P.x*.9)),sin(P.x*1.3+P.y*.7));return normalize(N+(g-N*dot(g,N))*.025);',
    {'P':p,'N':n},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_NORMAL)
save(m)
stone_material=asset('MI_Hero_Ashlar',u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
E.set_material_instance_parent(stone_material,m)
E.set_material_instance_scalar_parameter_value(stone_material,'Stone',1)
E.update_material_instance(stone_material)
u.EditorAssetLibrary.save_loaded_asset(stone_material)
materials={'Ashlar':stone_material,'DarkIron':u.load_asset(DEST+'/Materials/M_LH_DarkIron')}
audit=[]
for file in sorted((ROOT/'Generated').glob('*.obj')):
    task=u.AssetImportTask();task.filename=str(file);task.destination_path=DEST+'/Meshes'
    task.automated=True;task.replace_existing=True;task.save=False
    options=u.FbxImportUI();options.import_materials=False;options.import_textures=False
    task.options=options
    AT.import_asset_tasks([task])
    mesh=u.load_asset(DEST+'/Meshes/'+file.stem)
    if not mesh:raise RuntimeError('Missing mesh '+file.stem)
    names=[]
    for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'))
        if name not in materials:raise RuntimeError('Unexpected material '+name)
        mesh.set_material(index,materials[name]);names.append(name)
    settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',True)
    mesh.set_editor_property('nanite_settings',settings)
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    audit.append({'mesh':file.stem,'materials':names,'bounds':str(mesh.get_bounds())})
output=Path(u.Paths.project_saved_dir())/'HeroLighthouseArtV01'
output.mkdir(parents=True,exist_ok=True)
(output/'AssetAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
u.log('HERO_EXTERIOR_IMPORT_COMPLETE')
