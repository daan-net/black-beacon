"""Deterministic, original storm mesh/audio sources; Python standard library only."""
from pathlib import Path
import math, random, wave, array
ROOT=Path(__file__).resolve().parent
OUT=ROOT/'Generated'
OUT.mkdir(exist_ok=True)

def segment_distance(x,y,ax,ay,bx,by):
    t=max(0,min(1,((x-ax)*(bx-ax)+(y-ay)*(by-ay))/((bx-ax)**2+(by-ay)**2)))
    return math.hypot(x-ax-t*(bx-ax),y-ay-t*(by-ay))
def coast_distance(x,y):
    return min(segment_distance(x,y,-4700,-600,-500,0)-850,
               math.hypot(x-500,y-100)-2200,
               segment_distance(x,y,-1200,600,-4700,2700)-650,
               math.hypot(x+5200,y-4200)-750)
def grid(name, extent, count, terrain=False):
    vertices=[]; faces=[]; ids={}
    def vert(i,j):
        if (i,j) not in ids:
            x=-extent+2*extent*i/count; y=-extent+2*extent*j/count
            if not terrain:
                x=math.copysign((abs(x)/extent)**2*extent,x); y=math.copysign((abs(y)/extent)**2*extent,y)
            d=coast_distance(x,y)
            z=-max(0,d+120)*1.25 if terrain else 0
            ids[i,j]=len(vertices)+1; vertices.append((x,y,z))
        return ids[i,j]
    for j in range(count):
        for i in range(count):
            x=-extent+2*extent*(i+.5)/count; y=-extent+2*extent*(j+.5)/count
            if terrain and coast_distance(x,y)>480: continue
            a,b,c,d=vert(i,j),vert(i+1,j),vert(i+1,j+1),vert(i,j+1)
            faces.extend(((a,b,c),(a,c,d)))
    with (OUT/(name+'.obj')).open('w') as f:
        f.write('o '+name+'\n')
        for x,y,z in vertices: f.write(f'v {x:.3f} {-y:.3f} {z:.3f}\n')
        for x,y,z in vertices: f.write(f'vt {x/350:.5f} {y/350:.5f}\n')
        # OBJ is right-handed; Interchange flips Y when converting to UE coordinates.
        for a,b,c in faces: f.write('f '+' '.join(f'{i}/{i}' for i in (a,c,b))+'\n')
    print(name,len(vertices),len(faces))
grid('SM_StormOcean',80000,256)
grid('SM_StormCoast',6500,130,True)

# Layered colored noise with slow envelopes; original synthesized assets, not recordings.
def sound(name,seconds,kind,seed,loop=True):
    rng=random.Random(seed); rate=24000; n=int(seconds*rate); buf=[]
    lo=mid=slow=0.0
    for i in range(n):
        t=i/rate; white=rng.uniform(-1,1)
        lo+=.014*(white-lo); mid+=.20*(white-mid); slow+=.0025*(white-slow)
        if kind=='wind':
            gust=.60+.22*math.sin(t*.73)+.13*math.sin(t*1.41+.6)
            v=gust*(1.5*lo+.17*mid+.018*white)
        elif kind=='rain':
            env=.72+.12*math.sin(t*.95)+.09*math.sin(t*2.13)
            v=env*(.22*(white-mid)+.4*mid)
            if rng.random()<.0018: v+=rng.uniform(.15,.38)
        elif kind=='surf':
            phase=(t/7.3)%1; swell=math.sin(math.pi*phase)**4
            env=.12+.88*swell
            v=env*(1.8*lo+.42*mid+.10*white)+.55*slow
        else:
            env=(1-math.exp(-t*12))*math.exp(-t*.35)
            rolls=.58+.22*math.sin(t*3.7)+.17*math.sin(t*7.1)
            v=env*rolls*(5.8*lo+3.8*slow+.25*mid)
        buf.append(math.tanh(v*1.4)*.72)
    if loop:
        fade=rate
        # Crossfade end into start and drop the duplicated first second.
        for j in range(fade):
            a=j/fade; buf[n-fade+j]=buf[n-fade+j]*(1-a)+buf[j]*a
        buf=buf[fade:]
    else:
        for j in range(rate): buf[-rate+j]*=(1-j/rate)
    pcm=array.array('h',(int(max(-1,min(1,v))*32767) for v in buf))
    with wave.open(str(OUT/(name+'.wav')),'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(rate); f.writeframes(pcm.tobytes())
    rms=math.sqrt(sum(v*v for v in buf)/len(buf))
    print(name,'RMS dBFS',round(20*math.log10(rms),1),'peak',round(max(abs(v) for v in buf),3))
for args in [('SW_StormWind',25,'wind',32),('SW_StormRain',19,'rain',51),('SW_StormSurf',30,'surf',71),('SW_StormThunder',11,'thunder',81,False)]: sound(*args)
