import bpy
import bmesh
from array import array

models = []

for object in bpy.data.collections["models"].objects:
    bm = bmesh.new()
    bm.from_mesh(object.data)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bmesh.ops.scale(bm, vec=object.scale, verts=bm.verts)

    mesh = bpy.data.meshes.new("")
    bm.to_mesh(mesh)

    file = open(f"C:\\Projects\\vulkan\\bin\\{object.name}.sm".lower(), "wb")
    # Signature
    file.write(array("b", [0x53, 0x4D, 0, 0]))

    # Metadata
    file.write(array("i", [len(mesh.vertices), len(mesh.polygons) * 3]))
    
    for vertex in mesh.vertices:
        file.write(array("f", [vertex.co.x, -vertex.co.z, vertex.co.y,\
            vertex.normal.x, vertex.normal.y, vertex.normal.z,\
            0, 0]))  # TODO: UV
    
    for polygon in mesh.polygons:
        # TODO: is this check necessary? i don't think so
        if len(polygon.vertices) != 3:
            print("Something has gone wrong.")
        
        file.write(array("i", polygon.vertices))
    
    models.append(object.name)
    
    bm.free()
    file.close()
    
file = open("C:\\Projects\\vulkan\\bin\\world.sw", "wb")

# Signature
file.write(array("b", [0x53, 0x57, 0, 0]))

# Metadata
# TODO: point cuboid
file.write(array("i", [len(models), len(bpy.data.collections["objects"].objects), 0]))

for model in models:
    path = f"{model}.sm"
    zeros = 20 - len(path)
    file.write(path.encode())
    file.write(array("b", [0] * zeros))

for object in bpy.data.collections["objects"].objects:
    file.write(object.original.name.encode())

file.close()
