"""Import V0.4 source modules and build persistent UE materials; run in UE Python."""
from pathlib import Path
import json
import unreal as u
ROOT=Path(__file__).resolve().parent
DEST='/Game/BlackBeacon/Art/Lighthouse'
E=u.MaterialEditingLibrary
AT=u.AssetToolsHelpers.get_asset_tools()

def asset(name,cls,factory,folder='Materials'):
    path=DEST+'/'+folder+'/'+name
    return u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else AT.create_asset(name,DEST+'/'+folder,cls,factory)
def node(m,cls,key,**props):
    old=[n for n in E.get_material_expressions(m) if n.get_editor_property('desc')==key]
    n=old[0] if old else E.create_material_expression(m,cls,0,0)
    n.set_editor_property('desc',key)
    for k,v in props.items():n.set_editor_property(k,v)
    return n
def link(a,b,k):
    if not E.connect_material_expressions(a,'',b,k):raise RuntimeError('Missing input '+k)
def prop(a,p):
    if not E.connect_material_property(a,'',p):raise RuntimeError('Missing property '+str(p))
def scalar(m,k,v):return node(m,u.MaterialExpressionScalarParameter,k,parameter_name=k,default_value=v)
def vector(m,k,v):return node(m,u.MaterialExpressionVectorParameter,k,parameter_name=k,default_value=u.LinearColor(*v))
def custom(m,k,code,inputs,kind=u.CustomMaterialOutputType.CMOT_FLOAT1):
    n=node(m,u.MaterialExpressionCustom,k)
    ins=[]
    for name in inputs:
        ci=u.CustomInput();ci.set_editor_property('input_name',name);ins.append(ci)
    if [str(i.get_editor_property('input_name')) for i in n.get_editor_property('inputs')]!=list(inputs):n.set_editor_property('inputs',ins)
    n.set_editor_property('code',code);n.set_editor_property('output_type',kind)
    for name,v in inputs.items():link(v,n,name)
    return n
def save(m):
    E.layout_material_expressions(m);E.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)
def material(name,blend=None):
    m=asset(name,u.Material,u.MaterialFactoryNew())
    m.set_editor_property('used_with_instanced_static_meshes',True)
    m.set_editor_property('used_with_nanite',True)
    if blend is not None:
        m.set_editor_property('blend_mode',blend);m.set_editor_property('two_sided',True)
        m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
    return m

m=material('M_V04_WeatheredSurface')
p=node(m,u.MaterialExpressionWorldPosition,'world')
n=node(m,u.MaterialExpressionVertexNormalWS,'normal')
color=vector(m,'BaseColor',(.5,.48,.42,1))
family=scalar(m,'Family',0)
rough=scalar(m,'Roughness',.85)
wet=scalar(m,'WetAmount',.2)
tex=node(m,u.MaterialExpressionTextureObjectParameter,'RockAlbedo',parameter_name='RockAlbedo',texture=u.load_asset('/Game/BlackBeacon/Textures/T_LighthousePaintAlbedo'))
# World-space triplanar mapping removes the old scaled-primitive UV dependence.
surface=custom(m,'surface',r'''
struct Field {
 float hash(float3 p) { return frac(sin(dot(p,float3(127.1,311.7,74.7)))*43758.5453); }
 float noise(float3 p) {
  float3 i=floor(p),f=frac(p);f=f*f*(3-2*f);
  return lerp(lerp(lerp(hash(i),hash(i+float3(1,0,0)),f.x),lerp(hash(i+float3(0,1,0)),hash(i+float3(1,1,0)),f.x),f.y),
   lerp(lerp(hash(i+float3(0,0,1)),hash(i+float3(1,0,1)),f.x),lerp(hash(i+float3(0,1,1)),hash(i+1),f.x),f.y),f.z);
 }
}; Field f;
float3 weights=pow(abs(N),4);weights/=max(dot(weights,1),.001);
float scale=Family>3.5 ? 230 : 180;
float3 tx=Texture2DSample(Tex,TexSampler,P.yz/scale).rgb;
float3 ty=Texture2DSample(Tex,TexSampler,P.xz/scale).rgb;
float3 tz=Texture2DSample(Tex,TexSampler,P.xy/scale).rgb;
float textureValue=dot(tx*weights.x+ty*weights.y+tz*weights.z,float3(.333,.333,.333));
float broad=f.noise(P/160),fine=f.noise(P/7),streak=f.noise(P*float3(.05,.05,.002));
float chip=smoothstep(.48,.67,broad*.58+fine*.18+streak*.24);
float damp=Wet*smoothstep(.40,.68,broad)*saturate(.5+N.z*.5);
float3 c=Color.rgb*(.77+.26*fine);
if(Family<.5) c=lerp(Color.rgb*(.66+.50*textureValue),float3(.13,.12,.09),chip*.75);
else if(Family<1.5) c=lerp(c,float3(.18,.072,.025),smoothstep(.53,.75,broad)*.6);
else if(Family<2.5) c*=.7+.5*broad;
else if(Family<3.5) c*=.55+.45*f.noise(P*float3(.08,.08,.004));
else if(Family<4.5) c*=.55+textureValue*.6+broad*.35;
else c*=.83+.20*broad;
c*=lerp(1,.72,damp);
return float4(c,clamp(Rough+fine*.10-damp*.21,.38,.96));
''',{'P':p,'N':n,'Color':color,'Family':family,'Rough':rough,'Wet':wet,'Tex':tex},u.CustomMaterialOutputType.CMOT_FLOAT4)
prop(custom(m,'base','return V.rgb;',{'V':surface},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_BASE_COLOR)
prop(custom(m,'rough','return V.a;',{'V':surface}),u.MaterialProperty.MP_ROUGHNESS)
prop(scalar(m,'Metallic',0),u.MaterialProperty.MP_METALLIC)
prop(scalar(m,'Specular',.28),u.MaterialProperty.MP_SPECULAR)
# Small world-scale surface normals, independent of UV stretching.
m.set_editor_property('tangent_space_normal',False)
prop(custom(m,'micro_normal','float3 g=float3(cos(P.x*.8+sin(P.z*.4)),sin(P.y*.9+P.z*.3),cos(P.z*.7+P.x*.2));return normalize(N+(g-N*dot(g,N))*.055);',{'P':p,'N':n},u.CustomMaterialOutputType.CMOT_FLOAT3),u.MaterialProperty.MP_NORMAL)
save(m)

families={
 'TowerPaint':((.53,.51,.44,1),0,.88,0,.18),
 'DarkIron':((.055,.065,.062,1),1,.70,.35,.16),
 'WarmBrass':((.32,.19,.065,1),2,.48,.70,.06),
 'AgedWood':((.18,.105,.055,1),3,.84,0,.10),
 'WetRock':((.22,.245,.25,1),4,.80,0,.65),
 'InteriorPlaster':((.33,.29,.23,1),5,.92,0,0),
 'CutStone':((.24,.235,.21,1),5,.89,0,.25),
 'LanternGlass':((.10,.08,.045,1),5,.65,0,0),
}
materials={}
for name,(c,f,r,metal,w) in families.items():
    instance=asset('M_LH_'+name,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
    E.clear_all_material_instance_parameters(instance);E.set_material_instance_parent(instance,m)
    E.set_material_instance_vector_parameter_value(instance,'BaseColor',u.LinearColor(*c))
    for k,v in {'Family':f,'Roughness':r,'Metallic':metal,'WetAmount':w}.items():E.set_material_instance_scalar_parameter_value(instance,k,v)
    texture=u.load_asset('/Game/BlackBeacon/Textures/'+('T_WetBasaltAlbedo' if name=='WetRock' else 'T_LighthousePaintAlbedo'))
    E.set_material_instance_texture_parameter_value(instance,'RockAlbedo',texture)
    E.update_material_instance(instance);u.EditorAssetLibrary.save_loaded_asset(instance);materials[name]=instance

m=material('M_V04_Glazing',u.BlendMode.BLEND_TRANSLUCENT)
prop(vector(m,'GlassTint',(.032,.047,.050,1)),u.MaterialProperty.MP_EMISSIVE_COLOR)
fres=node(m,u.MaterialExpressionFresnel,'grazing',exponent=4.0,base_reflect_fraction=.015)
prop(custom(m,'glassalpha','return .035+F*.12;',{'F':fres}),u.MaterialProperty.MP_OPACITY);save(m);materials['LanternGlass']=m
m=material('M_V04_LensGlass',u.BlendMode.BLEND_TRANSLUCENT)
prop(vector(m,'GlassTint',(.25,.18,.07,1)),u.MaterialProperty.MP_EMISSIVE_COLOR)
prop(scalar(m,'Opacity',.27),u.MaterialProperty.MP_OPACITY);save(m);materials['LensGlass']=m
m=material('M_V04_Practical')
prop(vector(m,'Glow',(1.8,.75,.21,1)),u.MaterialProperty.MP_EMISSIVE_COLOR)
prop(vector(m,'Base',(.35,.18,.045,1)),u.MaterialProperty.MP_BASE_COLOR);save(m);materials['Lamp']=m

# Reuse imported slots by name, never delete materials referenced by the saved map.
for f in sorted((ROOT.parents[1]/'Lighthouse'/'SourceMeshes').glob('*.obj')):
    task=u.AssetImportTask();task.filename=str(f);task.destination_path=DEST+'/Meshes';task.automated=True;task.replace_existing=True;task.save=True
    AT.import_asset_tasks([task])
    mesh=u.load_asset(DEST+'/Meshes/'+f.stem)
    if not mesh:raise RuntimeError('Mesh import failed '+f.stem)
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'))
        if name not in materials:name=str(slot.get_editor_property('material_slot_name'))
        if name not in materials:raise RuntimeError('Unmapped material '+name+' on '+f.stem)
        mesh.set_material(i,materials[name])
    settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',False);mesh.set_editor_property('nanite_settings',settings)
    if f.stem=='SM_BB_LH_GalleryDeck':
        mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    u.EditorAssetLibrary.save_loaded_asset(mesh)

# The shaft integrates a soft density field along the camera ray inside the real
# beam cone; additive scattering cannot turn into an opaque solid surface.
m=u.load_asset('/Game/BlackBeacon/Materials/M_BeamShaft')
m.set_editor_property('blend_mode',u.BlendMode.BLEND_ADDITIVE)
m.set_editor_property('two_sided',False)
m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
p=node(m,u.MaterialExpressionWorldPosition,'v04_world')
camera=node(m,u.MaterialExpressionCameraPositionWS,'v04_camera')
t=node(m,u.MaterialExpressionTime,'v04_time')
inputs={'P':p,'C':camera,'T':t,'O':vector(m,'BeamOriginWS',(0,0,1980,1)),
'D':vector(m,'BeamDirectionWS',(1,0,0,1)),'L':scalar(m,'VisualLengthCm',7000),
'K':scalar(m,'BeamTanHalfAngle',.04366),'Opacity':scalar(m,'BeamOpacity',.012)}
shaft=custom(m,'v04_scattering',r'''
float3 d=normalize(D.xyz),ray=normalize(P-C),v=C-O.xyz;
float vd=dot(v,d),rd=dot(ray,d);
float aa=dot(ray,ray)-(1+K*K)*rd*rd;
float bb=2*(dot(v,ray)-(1+K*K)*vd*rd);
float cc=dot(v,v)-(1+K*K)*vd*vd;
float disc=bb*bb-4*aa*cc;
if(disc<0) return 0;
float r0=(-bb-sqrt(disc))/(2*aa),r1=(-bb+sqrt(disc))/(2*aa);
float start=max(0,min(r0,r1)),end=max(r0,r1);
float z0=(0-vd)/rd,z1=(L-vd)/rd;
start=max(start,min(z0,z1));end=min(end,max(z0,z1));
if(end<=start) return 0;
float result=0;
[unroll] for(int i=0;i<12;i++) {
 float3 q=v+ray*lerp(start,end,(i+.5)/12.0);
 float axial=dot(q,d);float radial=length(q-d*axial)/max(22,axial*K);
 float edge=pow(saturate(1-radial*radial),3);
 float mist=.75+.15*sin(dot(q,float3(.003,.005,.002))+T*.6)+.10*sin(q.z*.014-T*.4);
 float falloff=exp(-axial/L*3)*smoothstep(0,100,axial)*(1-smoothstep(L*.7,L,axial));
 result+=edge*falloff*mist;
}
return min(.16,result/12*(end-start)/280*Opacity);
''',inputs)
prop(shaft,u.MaterialProperty.MP_OPACITY)
tint=vector(m,'BeamTint',(1,.72,.42,1));prop(tint,u.MaterialProperty.MP_EMISSIVE_COLOR);save(m)

# Add crossing short waves, broken caps and localized shore wash to the existing sea.
m=u.load_asset('/Game/BlackBeacon/Storm/Materials/M_StormOcean')
for n in E.get_material_expressions(m):
    if n.get_editor_property('desc')=='waves':
        code=n.get_editor_property('code')
        if 'crossPhase' in code:continue
        code=code.replace('float h=sin(a)*95+sin(b)*42+sin(c)*18;', '''float crossPhase=dot(P.xy,float2(-.011,.007))-T*2.13+sin(a*.47);
float h=sin(a)*95+sin(b)*42+sin(c)*18+sin(crossPhase)*14+sin(a+b*.7)*11;''')
        code=code.replace('smoothstep(.97,1.28','smoothstep(.65,1.22')
        code=code.replace('return float4(h*S', '''float shore=0;
float2 sites[7]={float2(-4800,-1450),float2(-3400,-1250),float2(-1650,-1050),float2(400,-2080),float2(2380,-900),float2(2370,1600),float2(-5500,3450)};
for(int i=0;i<7;i++) {float dist=length(P.xy-sites[i]);shore=max(shore,exp(-dist*dist/180000)*(.45+.3*sin(dist*.025-T*1.7)));}
foam=saturate(foam+shore);
return float4(h*S''')
        n.set_editor_property('code',code)
save(m)
print('BB_V04_ASSETS_COMPLETE',flush=True)
