"""Scoped CC0 imports and reusable PBR materials; never edits legacy assets/maps."""
import json
from pathlib import Path
import unreal as u

ROOT = Path(__file__).resolve().parents[3]
SOURCE = Path(__file__).resolve().parent
DEST = '/Game/BlackBeacon/EnvironmentKitV1'
E = u.MaterialEditingLibrary
AT = u.AssetToolsHelpers.get_asset_tools()


def asset(name, cls, factory, folder='Materials'):
    return u.load_asset(f'{DEST}/{folder}/{name}') or AT.create_asset(name, f'{DEST}/{folder}', cls, factory)


def node(material, cls, **properties):
    expression = E.create_material_expression(material, cls, 0, 0)
    for key, value in properties.items():
        expression.set_editor_property(key, value)
    return expression


def custom(material, code, inputs, kind=u.CustomMaterialOutputType.CMOT_FLOAT3):
    expression = node(material, u.MaterialExpressionCustom, code=code, output_type=kind)
    custom_inputs = []
    for key in inputs:
        item = u.CustomInput()
        item.set_editor_property('input_name', key)
        custom_inputs.append(item)
    expression.set_editor_property('inputs', custom_inputs)
    for key, value in inputs.items():
        assert E.connect_material_expressions(value, '', expression, key)
    return expression


def scalar(material, name, value):
    return node(material, u.MaterialExpressionScalarParameter, parameter_name=name, default_value=value)


def save(material):
    E.layout_material_expressions(material)
    E.recompile_material(material)
    u.EditorAssetLibrary.save_loaded_asset(material)


manifest = json.loads((SOURCE / 'assets.json').read_text())
textures = {}
meshes = {}
for entry in manifest:
    slug = entry['id']
    textures[slug] = {}
    for channel, file in entry['files'].items():
        task = u.AssetImportTask()
        task.filename = str(ROOT / file['local'])
        task.destination_path = DEST + ('/Meshes' if channel == 'mesh' else '/Textures')
        task.automated = True
        task.replace_existing = True
        task.save = True
        task.destination_name = ('SM_' if channel == 'mesh' else 'T_') + slug + ('' if channel == 'mesh' else '_' + channel)
        expected = task.destination_path + '/' + task.destination_name
        imported = u.load_asset(expected)
        if imported and ('-KitMaterialsOnly' in u.SystemLibrary.get_command_line()
                         or not (slug == 'coast_rocks_05' and channel == 'mesh')
                         or u.EditorAssetLibrary.get_metadata_tag(imported, 'BB.SourceLOD') == '0'):
            # Continue an interrupted import without rebuilding completed assets.
            if channel == 'mesh':
                meshes[slug] = imported
            else:
                textures[slug][channel] = imported
            continue
        if channel == 'mesh':
            options = u.FbxImportUI()
            options.import_materials = False
            options.import_textures = False
            options.static_mesh_import_data.combine_meshes = False
            options.static_mesh_import_data.auto_generate_collision = False
            options.static_mesh_import_data.import_mesh_lods = False
            task.options = options
            if slug == 'coast_rocks_05':
                task.destination_name = ''
        AT.import_asset_tasks([task])
        candidates = [u.load_asset(path) for path in task.imported_object_paths]
        if slug == 'coast_rocks_05' and channel == 'mesh':
            u.log('Coast source objects: ' + str(task.imported_object_paths))
            imported = next((a for a in candidates if isinstance(a, u.StaticMesh) and 'LOD0' in a.get_name().upper()), None)
            if not imported:
                raise RuntimeError('Expected explicit coastal LOD0; refusing overlapping LOD import')
            if u.EditorAssetLibrary.does_asset_exist(expected):
                u.EditorAssetLibrary.delete_asset(expected)
            for other in candidates:
                if other != imported:
                    u.EditorAssetLibrary.delete_asset(other.get_path_name())
        else:
            imported = u.load_asset(expected)
        if not imported:
            # Interchange may retain the FBX source object name; resolve the actual result.
            imported = next((a for a in candidates if isinstance(a, u.StaticMesh if channel == 'mesh' else u.Texture2D)), None)
        if not imported:
            raise RuntimeError('Import failed: ' + file['local'])
        if imported.get_path_name().split('.')[0] != expected:
            assert u.EditorAssetLibrary.rename_asset(imported.get_path_name(), expected)
            imported = u.load_asset(expected)
        if channel == 'mesh':
            meshes[slug] = imported
            settings = imported.get_editor_property('nanite_settings')
            settings.set_editor_property('enabled', slug != 'barrel_03')
            imported.set_editor_property('nanite_settings', settings)
            if slug == 'coast_rocks_05':
                u.EditorAssetLibrary.set_metadata_tag(imported, 'BB.SourceLOD', '0')
        else:
            imported.set_editor_property('srgb', channel == 'Diffuse')
            compression = u.TextureCompressionSettings.TC_NORMALMAP if channel == 'nor_dx' else (
                u.TextureCompressionSettings.TC_DEFAULT if channel == 'Diffuse' else u.TextureCompressionSettings.TC_MASKS)
            imported.set_editor_property('compression_settings', compression)
            textures[slug][channel] = imported
        u.EditorAssetLibrary.save_loaded_asset(imported)

master = asset('M_EK_ScannedSurface', u.Material, u.MaterialFactoryNew())
E.delete_all_material_expressions(master)
master.set_editor_property('used_with_instanced_static_meshes', True)
master.set_editor_property('used_with_nanite', True)
master.set_editor_property('tangent_space_normal', False)
position = node(master, u.MaterialExpressionWorldPosition)
normal = node(master, u.MaterialExpressionVertexNormalWS)
uv = node(master, u.MaterialExpressionTextureCoordinate)
world = scalar(master, 'WorldAligned', 0)
repeat = scalar(master, 'RepeatCm', 200)
wet = scalar(master, 'Wetness', 0.15)
weather = scalar(master, 'Runoff', 0)
rough_floor = scalar(master, 'MinimumRoughness', 0.38)
tint = node(master, u.MaterialExpressionVectorParameter, parameter_name='Tint', default_value=u.LinearColor(1, 1, 1, 1))
saturation = scalar(master, 'Saturation', 0.8)
samples = {}
objects = {}
for channel in ('Diffuse', 'nor_dx', 'arm'):
    texture = node(master, u.MaterialExpressionTextureObjectParameter,
                   parameter_name=channel, texture=textures['painted_plaster_wall'][channel])
    if channel == 'nor_dx':
        texture.set_editor_property('sampler_type', u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    elif channel == 'arm':
        texture.set_editor_property('sampler_type', u.MaterialSamplerType.SAMPLERTYPE_MASKS)
    objects[channel] = texture
    samples[channel] = custom(master, '''
float3 w=pow(abs(N),4); w/=max(dot(w,1),.001);
float3 a=Texture2DSample(T,Tsampler,UV).rgb;
if (World>.5) a=Texture2DSample(T,Tsampler,P.yz/Repeat).rgb*w.x
    +Texture2DSample(T,Tsampler,P.xz/Repeat).rgb*w.y
    +Texture2DSample(T,Tsampler,P.xy/Repeat).rgb*w.z;
return a;
'''.replace('Tsampler', 'TSampler'), {'T': texture, 'UV': uv, 'P': position, 'N': normal, 'World': world, 'Repeat': repeat})

mask = custom(master, '''
float stripe=.5+.5*sin(P.x*.017+P.y*.023+sin(P.x*.049-P.y*.031)*2);
float broad=.5+.5*sin(P.x*.004+P.y*.007+sin(P.z*.002));
float footing=1-smoothstep(50,360,P.z);
return saturate(Wet*(.3+.7*broad)+Runoff*(pow(stripe,7)*.22+footing*.28));
''', {'P': position, 'Wet': wet, 'Runoff': weather}, u.CustomMaterialOutputType.CMOT_FLOAT1)
base = custom(master, '''
float lum=dot(C,float3(.2126,.7152,.0722));
return lerp(lum.xxx,C,Sat)*Tint.rgb*lerp(1,.60,Damp);
''', {'C': samples['Diffuse'], 'Sat': saturation, 'Tint': tint, 'Damp': mask})
rough = custom(master, 'return clamp(ARM.g-Damp*.24,Floor,.96);',
               {'ARM': samples['arm'], 'Damp': mask, 'Floor': rough_floor}, u.CustomMaterialOutputType.CMOT_FLOAT1)
metal = custom(master, 'return ARM.b;', {'ARM': samples['arm']}, u.CustomMaterialOutputType.CMOT_FLOAT1)
ao = custom(master, 'return lerp(1,ARM.r,.55);', {'ARM': samples['arm']}, u.CustomMaterialOutputType.CMOT_FLOAT1)

# Normal maps retain their proper tangent basis on props; walls blend three
# world-space projections so scaled architecture never stretches the scan.
normal_sample = node(master, u.MaterialExpressionTextureSampleParameter2D,
                     parameter_name='nor_dx', texture=textures['painted_plaster_wall']['nor_dx'],
                     sampler_type=u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
transform = node(master, u.MaterialExpressionTransform,
                 transform_source_type=u.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_TANGENT,
                 transform_type=u.MaterialVectorCoordTransform.TRANSFORM_WORLD)
assert E.connect_material_expressions(normal_sample, '', transform, '')
mapped_normal = custom(master, '''
if(World<.5) return normalize(TangentWorld);
float3 w=pow(abs(N),4); w/=max(dot(w,1),.001);
float2 x=Texture2DSample(T,TSampler,P.yz/Repeat).rg*2-1;
float2 y=Texture2DSample(T,TSampler,P.xz/Repeat).rg*2-1;
float2 z=Texture2DSample(T,TSampler,P.xy/Repeat).rg*2-1;
float3 a=float3(sqrt(saturate(1-dot(x,x)))*sign(N.x),x.x,x.y);
float3 b=float3(y.x,sqrt(saturate(1-dot(y,y)))*sign(N.y),y.y);
float3 c=float3(z.x,z.y,sqrt(saturate(1-dot(z,z)))*sign(N.z));
return normalize(a*w.x+b*w.y+c*w.z);
''', {'P': position, 'N': normal, 'T': objects['nor_dx'], 'World': world, 'Repeat': repeat, 'TangentWorld': transform})
for expression, prop in ((base, u.MaterialProperty.MP_BASE_COLOR), (rough, u.MaterialProperty.MP_ROUGHNESS),
                         (metal, u.MaterialProperty.MP_METALLIC), (ao, u.MaterialProperty.MP_AMBIENT_OCCLUSION),
                         (mapped_normal, u.MaterialProperty.MP_NORMAL)):
    assert E.connect_material_property(expression, '', prop)
save(master)

presets = {
    'MI_EK_ExteriorPlaster': ('painted_plaster_wall', (0.86, 0.86, 0.79, 1), .16, 1, 1),
    'MI_EK_InteriorPlaster': ('painted_plaster_wall', (0.63, 0.60, 0.52, 1), 0, 0, 1),
    'MI_EK_CoastLedge': ('coast_rocks_05', (0.65, 0.73, 0.77, 1), .65, 0, 0),
    'MI_EK_RockFace': ('rock_face_02', (0.65, 0.73, 0.77, 1), .45, 0, 0),
    'MI_EK_FuelDrum': ('barrel_03', (0.65, 0.70, 0.69, 1), .05, 0, 0),
}
for name, (slug, color, damp, runoff, aligned) in presets.items():
    instance = asset(name, u.MaterialInstanceConstant, u.MaterialInstanceConstantFactoryNew())
    E.set_material_instance_parent(instance, master)
    for channel, texture in textures[slug].items():
        E.set_material_instance_texture_parameter_value(instance, channel, texture)
    for key, value in {'Wetness': damp, 'Runoff': runoff, 'WorldAligned': aligned}.items():
        E.set_material_instance_scalar_parameter_value(instance, key, value)
    E.set_material_instance_vector_parameter_value(instance, 'Tint', u.LinearColor(*color))
    E.update_material_instance(instance)
    u.EditorAssetLibrary.save_loaded_asset(instance)
    if slug in meshes:
        for index in range(len(meshes[slug].get_editor_property('static_materials'))):
            meshes[slug].set_material(index, instance)
        u.EditorAssetLibrary.save_loaded_asset(meshes[slug])

report = {slug: {'path': mesh.get_path_name(), 'bounds': str(mesh.get_bounds()),
                 'materials': len(mesh.get_editor_property('static_materials'))} for slug, mesh in meshes.items()}
(ROOT / 'Saved/EnvironmentKitV1/ImportAudit.json').write_text(json.dumps(report, indent=2) + '\n')
u.log('ENVIRONMENT_KIT_V1_IMPORT_COMPLETE')
