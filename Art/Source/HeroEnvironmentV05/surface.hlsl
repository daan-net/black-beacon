struct Field {
 float hash(float3 p) { return frac(sin(dot(p,float3(127.1,311.7,74.7)))*43758.5453); }
 float noise(float3 p) {
  float3 i=floor(p),f=frac(p);f=f*f*(3-2*f);
  return lerp(lerp(lerp(hash(i),hash(i+float3(1,0,0)),f.x),lerp(hash(i+float3(0,1,0)),hash(i+float3(1,1,0)),f.x),f.y),
   lerp(lerp(hash(i+float3(0,0,1)),hash(i+float3(1,0,1)),f.x),lerp(hash(i+float3(0,1,1)),hash(i+1),f.x),f.y),f.z);
 }
}; Field f;
float3 weights=pow(abs(N),4);weights/=max(dot(weights,1),.001);
// Two incommensurate texture scales break the obvious repeated paint squares.
float3 q=P/190;
float tex=dot(Texture2DSample(Tex,TexSampler,q.yz).rgb,float3(.333,.333,.333))*weights.x
 +dot(Texture2DSample(Tex,TexSampler,q.xz).rgb,float3(.333,.333,.333))*weights.y
 +dot(Texture2DSample(Tex,TexSampler,q.xy).rgb,float3(.333,.333,.333))*weights.z;
float broad=f.noise(P/123),fine=f.noise(P/3.7),grain=f.noise(P/1.1);
float streak=f.noise(P*float3(.055,.055,.0015));
float salt=smoothstep(.58,.77,broad*.65+streak*.35);
float runoff=smoothstep(.44,.71,streak)*(.45+.55*broad);
float damp=Wet*saturate(runoff*.6+smoothstep(.2,.75,N.z)*broad*.55);
float3 c=Color.rgb*(.82+.24*fine);
float r=Rough+(.5-fine)*.16;
if(Family<.5) {
 float peel=smoothstep(.48,.67,broad*.50+fine*.25+streak*.25);
 c=lerp(Color.rgb*(.67+.5*tex),float3(.13,.125,.11),peel*.72);
 c=lerp(c,Color.rgb*.48,runoff*.52);
 c=lerp(c,float3(.62,.61,.54),salt*.18);
} else if(Family<1.5) {
 float rust=smoothstep(.49,.72,broad*.7+fine*.3);
 c=lerp(c,float3(.125,.057,.027),rust*.8);r+=rust*.17;
} else if(Family<2.5) {
 float oxide=smoothstep(.42,.72,broad*.8+fine*.2);
 c=lerp(c,float3(.043,.081,.070),oxide*.6);r+=oxide*.20;
} else if(Family<3.5) {
 c*=.65+.42*f.noise(P*float3(.10,.10,.002));
} else if(Family<4.5) {
 c*=.55+tex*.55+broad*.35;r+=.07*(1-broad);
} else if(Family<5.5) {
 // Subtle curved masonry courses beneath sheltered lime plaster.
 float u=atan2(P.y,P.x)*265;
 float row=floor(P.z/29);
 float course=abs(frac(P.z/29)-.5)*2;
 float joint=abs(frac((u+row*43)/83)-.5)*2;
 float mortar=max(smoothstep(.96,.998,course),smoothstep(.978,.999,joint))*.14;
 c=Color.rgb*(.78+.20*broad+.14*tex)*(1-mortar);
 c=lerp(c,c*.6,runoff*.25);
} else if(Family<6.5) {
 // Painted machinery with exposed casting and oxidation in the worn zones.
 float wear=smoothstep(.47,.70,broad*.5+fine*.5);
 c=lerp(c,float3(.065,.054,.039),wear*.65);r+=wear*.11;
} else if(Family<7.5) {
 // Small raised tread chevrons only on upward surfaces; rails remain cast iron.
 float2 d=float2(P.x+P.y,P.x-P.y)/5.5;
 float bar=pow(saturate(1-abs(frac(d.x)-.5)*6),3)*step(.52,frac(d.y));
 float tread=bar*smoothstep(.75,.98,N.z);
 c*=.80+tread*.48;r-=tread*.09;
} else {
 c=Color.rgb*(.9+.1*broad);
}
c*=1-damp*.24;
return float4(c,clamp(r-damp*.20,.30,.96));
