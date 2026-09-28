// World-centimetre weathering: no texture squares or stretched UVs.
struct Weather {
 float hash(float3 p) { return frac(sin(dot(p,float3(127.1,311.7,74.7)))*43758.5453); }
 float noise(float3 p) {
  float3 i=floor(p), f=frac(p); f=f*f*(3-2*f);
  return lerp(lerp(lerp(hash(i),hash(i+float3(1,0,0)),f.x),lerp(hash(i+float3(0,1,0)),hash(i+float3(1,1,0)),f.x),f.y),
   lerp(lerp(hash(i+float3(0,0,1)),hash(i+float3(1,0,1)),f.x),lerp(hash(i+float3(0,1,1)),hash(i+1),f.x),f.y),f.z);
 }
 float fbm(float3 p) { return noise(p)*.57+noise(p*2.07+17.1)*.28+noise(p*4.13-9.3)*.15; }
}; Weather w;
float macro=w.fbm(P/83.0), fine=w.noise(P/2.8);
float warp=w.noise(P/37.0);
float patches=w.fbm(P/18.0+warp*.35);
float loss=smoothstep(.50,.61,patches)*(.35+.65*smoothstep(.35,.65,macro));
float rim=smoothstep(.48,.515,patches)*(1-smoothstep(.515,.55,patches));
float streak=w.fbm(P*float3(.046,.046,.0018));
float runoff=smoothstep(.48,.7,streak)*(.3+.7*macro);
float damp=(1-smoothstep(40,330,P.z))*.25+runoff*.10;
float3 c; float rough;
if (Stone>.5) {
 c=lerp(float3(.17,.18,.17),float3(.31,.30,.265),macro)*(.88+.18*fine);
 c*=1-damp*.42;
 rough=.86+fine*.09-damp*.28;
} else {
 float3 chalk=float3(.61,.595,.53)*(.88+.16*macro);
 float3 substrate=lerp(float3(.285,.265,.23),float3(.36,.34,.30),fine);
 c=lerp(chalk,substrate,loss*.85);
 c=lerp(c,float3(.70,.67,.58),rim*.20);
 c=lerp(c,c*float3(.62,.58,.49),runoff*.42);
 c*=1-damp*.28;
 rough=.91+loss*.035-damp*.2;
}
return float4(c,clamp(rough,.63,.97));
