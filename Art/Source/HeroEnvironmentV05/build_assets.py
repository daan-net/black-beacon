"""One scoped UE import/material build for V0.5. Does not regenerate V0.4 assets."""
from pathlib import Path
import json
import unreal as u

ROOT=Path(__file__).resolve().parent
# Reuse expression helpers only. The V0.4 build/import body must never execute.
helper=ROOT.parent/'VisualRebuildV04/build_assets.py'
exec(compile(helper.read_text().split("\nm=material('M_V04_WeatheredSurface')")[0],str(helper),'exec'))
ROOT=Path(__file__).resolve().parent

m=material('M_V04_WeatheredSurface')
expr={str(n.get_editor_property('desc')):n for n in E.get_material_expressions(m)}
expr['surface'].set_editor_property('code',(ROOT/'surface.hlsl').read_text())
expr['micro_normal'].set_editor_property('code',
    'float3 g=float3(sin(P.y*2.7+sin(P.z*3.1)),cos(P.z*2.3+sin(P.x*3.7)),sin(P.x*2.9+P.y*2.1));return normalize(N+(g-N*dot(g,N))*.016);')
save(m)
parent=m
families={
    'TowerPaint':((.48,.47,.42,1),0,.88,0,.26),
    'InteriorPlaster':((.30,.285,.25,1),5,.93,0,.03),
    'DarkIron':((.065,.077,.075,1),1,.67,.48,.12),
    'WarmBrass':((.31,.205,.086,1),2,.42,.78,.04),
    'RoofCopper':((.075,.14,.13,1),2,.64,.65,.22),
    'EngineEnamel':((.055,.115,.102,1),6,.62,.28,.02),
    'StairIron':((.10,.12,.12,1),7,.69,.50,.04),
    'DialFace':((.65,.60,.44,1),8,.73,0,0),
}
materials={name:u.load_asset(DEST+'/Materials/M_LH_'+name) for name in
           ('AgedWood','WetRock','CutStone')}
for name,(c,f,r,metal,w) in families.items():
    instance=asset('M_LH_'+name,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
    E.set_material_instance_parent(instance,parent)
    E.set_material_instance_vector_parameter_value(instance,'BaseColor',u.LinearColor(*c))
    for k,v in {'Family':f,'Roughness':r,'Metallic':metal,'WetAmount':w}.items():
        E.set_material_instance_scalar_parameter_value(instance,k,v)
    E.update_material_instance(instance);u.EditorAssetLibrary.save_loaded_asset(instance)
    materials[name]=instance

# Faceted optical glass responds to the actual practical, while the arc remains power-driven.
m=material('M_V05_OpticalGlass',u.BlendMode.BLEND_TRANSLUCENT)
m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_DEFAULT_LIT)
m.set_editor_property('translucency_lighting_mode',u.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
prop(vector(m,'Tint',(.24,.29,.24,1)),u.MaterialProperty.MP_BASE_COLOR)
prop(scalar(m,'Roughness',.12),u.MaterialProperty.MP_ROUGHNESS)
prop(scalar(m,'Specular',.65),u.MaterialProperty.MP_SPECULAR)
fres=node(m,u.MaterialExpressionFresnel,'facet_edges',exponent=3.0,base_reflect_fraction=.04)
prop(custom(m,'glass_coverage','return .10+F*.30;',{'F':fres}),u.MaterialProperty.MP_OPACITY)
prop(vector(m,'GlassGlow',(.028,.021,.009,1)),u.MaterialProperty.MP_EMISSIVE_COLOR)
save(m);materials['LensGlass']=m
materials['LanternGlass']=u.load_asset(DEST+'/Materials/M_V04_Glazing')
materials['Lamp']=u.load_asset(DEST+'/Materials/M_V04_Practical')

m=u.load_asset('/Game/BlackBeacon/Materials/M_BeamShaft')
expr={str(n.get_editor_property('desc')):n for n in E.get_material_expressions(m)}
expr['v04_scattering'].set_editor_property('code',(ROOT/'beam.hlsl').read_text())
save(m)

audit=[]
for f in sorted((ROOT/'Generated').glob('*.obj')):
    task=u.AssetImportTask();task.filename=str(f);task.destination_path=DEST+'/Meshes'
    task.automated=True;task.replace_existing=True;task.save=True
    options=u.FbxImportUI();options.set_editor_property('import_materials',False)
    options.set_editor_property('import_textures',False);task.options=options
    AT.import_asset_tasks([task])
    mesh=u.load_asset(DEST+'/Meshes/'+f.stem)
    if not mesh:raise RuntimeError('Missing '+f.stem)
    names=[]
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'))
        if name not in materials:name=str(slot.get_editor_property('material_slot_name'))
        if name not in materials or not materials[name]:raise RuntimeError('Missing surface '+name)
        mesh.set_material(i,materials[name]);names.append(name)
    # Opaque high-detail meshes use Nanite's VSM path. Optical/glazing meshes stay conventional.
    nanite=f.stem in ('SM_BB_GeneratorWorks','SM_BB_GeneratorFlywheel','SM_BB_LH_StairStructure')
    settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',nanite)
    mesh.set_editor_property('nanite_settings',settings)
    editor=(u.get_editor_subsystem(u.StaticMeshEditorSubsystem) or u.new_object(u.StaticMeshEditorSubsystem))
    for section in range(mesh.get_num_sections(0)):
        material_index=editor.get_lod_material_slot(mesh,0,section)
        if names[material_index] in ('Lamp','LensGlass','LanternGlass'):
            editor.enable_section_cast_shadow(mesh,False,0,section)
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    audit.append({'mesh':f.stem,'sections':mesh.get_num_sections(0),'materials':names,'bounds':str(mesh.get_bounds())})
out=Path(u.Paths.project_saved_dir())/'VisualRebuildV05'/'Validation'
out.mkdir(parents=True,exist_ok=True)
(out/'AssetAudit.json').write_text(json.dumps(audit,indent=2)+'\n')
u.log('BB_V05_ASSETS_COMPLETE')
