"""Ear clipping for the small simple counterclockwise cast-bracket profile."""
def triangulate(points):
    def turn(a,b,c):
        return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    indices=list(range(len(points)))
    area=sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(points,points[1:]+points[:1]))
    if area<0:indices.reverse()
    triangles=[]
    while len(indices)>3:
        for j,b in enumerate(indices):
            a,c=indices[j-1],indices[(j+1)%len(indices)]
            if turn(points[a],points[b],points[c])<=0:continue
            if any(all(turn(points[x],points[y],points[p])>=0 for x,y in ((a,b),(b,c),(c,a)))
                   for p in indices if p not in (a,b,c)):continue
            triangles.append((a,b,c));indices.pop(j);break
        else:raise ValueError('Bracket profile is not a simple polygon')
    return triangles+[tuple(indices)]
