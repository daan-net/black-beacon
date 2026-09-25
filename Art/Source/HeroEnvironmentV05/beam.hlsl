// Integrate mist along a finite soft cone. No opaque surface or constant core.
float3 d=normalize(D.xyz),ray=normalize(P-C),v=C-O.xyz;
float vd=dot(v,d),rd=dot(ray,d);
float aa=1-(1+K*K)*rd*rd;
float bb=2*(dot(v,ray)-(1+K*K)*vd*rd);
float cc=dot(v,v)-(1+K*K)*vd*vd;
float disc=bb*bb-4*aa*cc;
if(disc<=0 || abs(aa)<1e-6 || abs(rd)<1e-6) return 0;
float r0=(-bb-sqrt(disc))/(2*aa),r1=(-bb+sqrt(disc))/(2*aa);
float start=max(0,min(r0,r1)),end=max(r0,r1);
float z0=-vd/rd,z1=(L-vd)/rd;
start=max(start,min(z0,z1));end=min(end,max(z0,z1));
if(end<=start) return 0;
float result=0;
[unroll] for(int i=0;i<16;i++) {
 float3 q=v+ray*lerp(start,end,(i+.5)/16.0);
 float axial=dot(q,d);
 float radial=length(q-d*axial)/max(14,axial*K);
 float edge=pow(saturate(1-radial*radial),4);
 float mist=.62+.20*sin(dot(q,float3(.0031,.0043,.0017))+T*.48)
  +.12*sin(dot(q,float3(-.007,.002,.004))-T*.81);
 float extinction=exp(-axial/2800)/(1+axial/1900);
 float ends=smoothstep(35,115,axial)*(1-smoothstep(L*.45,L,axial));
 result+=edge*max(.12,mist)*extinction*ends;
}
float phase=.55+.45*pow(abs(dot(ray,d)),3);
return min(.11,result/16*(end-start)/190*Opacity*phase);
