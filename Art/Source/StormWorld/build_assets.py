"""Build native UE 5.8 storm assets from original sources. Run in Editor Python."""
from pathlib import Path
import unreal as u
ROOT=Path(__file__).resolve().parent
DEST='/Game/BlackBeacon/Storm'
E=u.MaterialEditingLibrary
AT=u.AssetToolsHelpers.get_asset_tools()

def material(name,volume=False,translucent=False):
    path=DEST+'/Materials/'+name
    m=u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else AT.create_asset(name,DEST+'/Materials',u.Material,u.MaterialFactoryNew())
    m.set_editor_property('blend_mode',u.BlendMode.BLEND_ADDITIVE if volume else u.BlendMode.BLEND_TRANSLUCENT if translucent else u.BlendMode.BLEND_OPAQUE)
    m.set_editor_property('material_domain',u.MaterialDomain.MD_VOLUME if volume else u.MaterialDomain.MD_SURFACE)
    if volume: m.set_editor_property('used_with_volumetric_cloud',True)
    if translucent:
        m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
        m.set_editor_property('two_sided',True)
        m.set_editor_property('used_with_instanced_static_meshes',True)
    return m

def node(m,cls,key,**props):
    matches=[n for n in E.get_material_expressions(m) if n.get_editor_property('desc')==key]
    n=matches[0] if matches else E.create_material_expression(m,cls,0,0)
    n.set_editor_property('desc',key)
    for k,v in props.items(): n.set_editor_property(k,v)
    return n

def link(a,b,name,out=''):
    if not E.connect_material_expressions(a,out,b,name): raise RuntimeError('Connect failed '+name)
def prop(a,p):
    if not E.connect_material_property(a,'',p): raise RuntimeError('Property failed '+str(p))
def scalar(m,name,value):return node(m,u.MaterialExpressionScalarParameter,name,parameter_name=name,default_value=value)
def vector(m,name,r,g,b):return node(m,u.MaterialExpressionVectorParameter,name,parameter_name=name,default_value=u.LinearColor(r,g,b,1))
def custom(m,key,code,inputs,kind):
    custom_inputs=[]
    for k in inputs:
        item=u.CustomInput();item.set_editor_property('input_name',k);custom_inputs.append(item)
    n=node(m,u.MaterialExpressionCustom,key)
    existing=n.get_editor_property('inputs')
    if [str(i.get_editor_property('input_name')) for i in existing] != list(inputs):
        n.set_editor_property('inputs',custom_inputs)
    n.set_editor_property('code',code)
    n.set_editor_property('output_type',kind)
    for k,v in inputs.items():
        if isinstance(v,tuple):link(v[0],n,k,v[1])
        else:link(v,n,k)
    return n

def save(m):
    E.layout_material_expressions(m);E.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)

# Import only these original meshes/audio files; leave the existing map and inputs alone.
for f in sorted((ROOT/'Generated').iterdir()):
    if f.suffix not in ('.obj','.wav'): continue
    folder=DEST+('/Meshes' if f.suffix=='.obj' else '/Audio')
    task=u.AssetImportTask();task.filename=str(f);task.destination_path=folder;task.automated=True;task.replace_existing=True;task.save=True
    if not u.EditorAssetLibrary.does_asset_exist(folder+'/'+f.stem) or '-StormReimport' in u.SystemLibrary.get_command_line():
        AT.import_asset_tasks([task])
    a=u.load_asset(folder+'/'+f.stem)
    if not a: raise RuntimeError('Import missing '+f.stem)
    if f.suffix=='.wav':
        a.set_editor_property('looping',f.stem!='SW_StormThunder')
        a.set_editor_property('volume',1.0)
        if f.stem!='SW_StormThunder': a.set_editor_property('virtualization_mode',u.VirtualizationMode.PLAY_WHEN_SILENT)
    else:
        if f.stem=='SM_StormOcean':
            settings=a.get_editor_property('nanite_settings')
            settings.set_editor_property('enabled',False)
            a.set_editor_property('nanite_settings',settings)
            a.set_editor_property('positive_bounds_extension',u.Vector(0,0,250))
            a.set_editor_property('negative_bounds_extension',u.Vector(0,0,250))
        if f.stem=='SM_StormCoast':
            a.get_editor_property('body_setup').set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    u.EditorAssetLibrary.save_loaded_asset(a)

# Use the engine's layered cloud/weather/erosion model, with a project storm preset.
path=DEST+'/Materials/MI_StormCloudNative'
a=u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else AT.create_asset('MI_StormCloudNative',DEST+'/Materials',u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
E.set_material_instance_parent(a,u.load_asset('/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst'))
for k,v in {'Cloud_GlobalCoverage':.1,'Cloud_GlobalDensity':.008,'StormClouds':.85,'Layout_CloudGlobalScale':64.0}.items():
    E.set_material_instance_scalar_parameter_value(a,k,v)
for k,v in {'Layout_WindControls':(.927,.375,0,.35),'Storm_LightningAnim':(0,0,0,0),'Storm_AlbedoColor':(.38,.42,.48,1)}.items():
    E.set_material_instance_vector_parameter_value(a,k,u.LinearColor(*v))
E.update_material_instance(a);u.EditorAssetLibrary.save_loaded_asset(a)

m=material('M_StormOcean')
m.set_editor_property('tangent_space_normal',False)
p=node(m,u.MaterialExpressionWorldPosition,'world')
t=node(m,u.MaterialExpressionTime,'time')
wind=vector(m,'WindDirection',.927,.375,0)
strength=scalar(m,'StormStrength',1)
waves=custom(m,'waves','''
float2 d=normalize(W.xy+float2(0.0001,0));
float2 d2=normalize(float2(d.x*.7-d.y*.6,d.x*.6+d.y*.7));
float2 d3=normalize(float2(d.x*.6+d.y*.8,-d.x*.8+d.y*.6));
float a=dot(P.xy,d)*0.002244-T*1.37;
float b=dot(P.xy,d2)*0.004333-T*1.91;
float c=dot(P.xy,d3)*0.008727-T*2.53;
float h=sin(a)*95+sin(b)*42+sin(c)*18;
float2 gradient=cos(a)*95*.002244*d+cos(b)*42*.004333*d2+cos(c)*18*.008727*d3;
float chop=sin(dot(P.xy,float2(.021,.016))-T*3.7)*.05;
float foam=smoothstep(.97,1.28,sin(a)+.38*sin(b));
foam*=smoothstep(-.35,.65,sin(P.x*.014+sin(P.y*.012)+T*.7));
return float4(h*S,(gradient.x+chop)*S,(gradient.y+chop)*S,foam);
''',{'P':p,'T':t,'W':wind,'S':strength},u.CustomMaterialOutputType.CMOT_FLOAT4)
prop(custom(m,'displacement','return float3(0,0,V.x);',{'V':waves},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
prop(custom(m,'normal','return normalize(float3(-V.y,-V.z,1));',{'V':waves},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_NORMAL)
prop(custom(m,'watercolor','return lerp(float3(.012,.030,.038),float3(.30,.37,.39),V.w);',{'V':waves},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_BASE_COLOR)
prop(custom(m,'roughness','return lerp(.25,.68,V.w);',{'V':waves},u.CustomMaterialOutputType.CMOT_FLOAT1),u.MaterialProperty.MP_ROUGHNESS)
prop(custom(m,'foamglow','return float3(.004,.006,.007)*V.w;',{'V':waves},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_EMISSIVE_COLOR)
prop(scalar(m,'Specular',.5),u.MaterialProperty.MP_SPECULAR)
prop(scalar(m,'Metallic',0),u.MaterialProperty.MP_METALLIC)
save(m)

for name,alpha in [('M_SeaSpray',.32),('M_CoastalMist',.085),('M_RainSplash',.27)]:
    m=material(name,translucent=True)
    uv=node(m,u.MaterialExpressionTextureCoordinate,'uv')
    data=node(m,u.MaterialExpressionPerInstanceCustomData,'life',data_index=0,const_default_value=1.0)
    time=node(m,u.MaterialExpressionTime,'time')
    if name=='M_RainSplash':
        code='float a=frac(T*.7+Life);float r=length(UV-.5)*2;return exp(-pow((r-a*.8)*18,2))*pow(1-a,3)*.27;'
    else:
        code=f'''float2 v=(UV-.5)*2;float r=dot(v,v);
float n=.65+.35*sin(UV.x*22+sin(UV.y*17+T*.5));
return saturate(1-r)*saturate(1-r)*n*Life*{alpha};'''
    opacity=custom(m,'opacity',code,{'UV':uv,'Life':data,'T':time},u.CustomMaterialOutputType.CMOT_FLOAT1)
    fade=node(m,u.MaterialExpressionDepthFade,'softcontact',fade_distance_default=100.0)
    link(opacity,fade,'Opacity');prop(fade,u.MaterialProperty.MP_OPACITY)
    prop(vector(m,'MistColor',.12,.16,.18),u.MaterialProperty.MP_EMISSIVE_COLOR)
    save(m)

# Persist renderer usages needed by already imported Nanite coastal/hero meshes.
for master_path in ['/Game/BlackBeacon/Materials/M_WetBasaltRock','/Game/BlackBeacon/Materials/M_CoastSurface']:
    master=u.load_asset(master_path)
    master.set_editor_property('used_with_nanite',True)
    E.recompile_material(master)
    u.EditorAssetLibrary.save_loaded_asset(master)

path=DEST+'/Materials/MI_StormGround'
a=u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else AT.create_asset('MI_StormGround',DEST+'/Materials',u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
E.set_material_instance_parent(a,u.load_asset('/Game/BlackBeacon/Materials/M_WetBasaltRock'))
E.set_material_instance_vector_parameter_value(a,'BaseColor',u.LinearColor(.55,.60,.64,1))
for k,v in {'Metallic':0,'Roughness':.83,'Specular':.28,'WetAmount':.48,'WetRoughness':.46}.items():E.set_material_instance_scalar_parameter_value(a,k,v)
E.update_material_instance(a);u.EditorAssetLibrary.save_loaded_asset(a)
print('BB_STORM_ASSETS_COMPLETE',flush=True)


import runpy
runpy.run_path(str(ROOT/'finalize_assets.py'))
