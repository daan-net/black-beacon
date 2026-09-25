"""Explicit OBJ normals: UE's Interchange OBJ importer ignores smoothing groups."""
from mesh import sub, cross, unit, add


def write_mesh(mesh, folder):
    audit=mesh.write(folder)
    faces=sorted(mesh.faces,key=lambda face:(face[0],face[2]))
    sums={}
    normals=[]
    def key(index, material):
        return tuple(round(v,4) for v in mesh.vertices[index-1][0])+(material,)
    for material,ids,smooth in faces:
        a,b,c=[mesh.vertices[i-1][0] for i in ids]
        normal=cross(sub(b,a),sub(c,a));normals.append(normal)
        if smooth:
            for i in ids:
                k=key(i,material);sums[k]=add(sums.get(k,(0,0,0)),normal)
    unique={};normal_ids=[]
    for (material,ids,smooth),normal in zip(faces,normals):
        indices=[]
        for i in ids:
            n=unit(sums[key(i,material)] if smooth else normal)
            n=tuple(round(v,7) for v in (n[0],-n[1],n[2]))
            if n not in unique:unique[n]=len(unique)+1
            indices.append(unique[n])
        normal_ids.append(indices)
    with (folder/(mesh.name+'.obj')).open('w') as f:
        f.write(f'mtllib {mesh.name}.mtl\no {mesh.name}\n')
        for (x,y,z),uv in mesh.vertices:f.write(f'v {x:.4f} {-y:.4f} {z:.4f}\n')
        for p,(u,v) in mesh.vertices:f.write(f'vt {u:.6f} {v:.6f}\n')
        for x,y,z in unique:f.write(f'vn {x:.7f} {y:.7f} {z:.7f}\n')
        last=None
        for (material,ids,smooth),ns in zip(faces,normal_ids):
            if material!=last:f.write(f'usemtl {material}\n');last=material
            f.write('f '+' '.join(f'{i}/{i}/{n}' for i,n in reversed(list(zip(ids,ns))))+'\n')
    audit['normals']=len(unique)
    return audit
