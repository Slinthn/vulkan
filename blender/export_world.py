import bpy
import bmesh
from array import array
import math

# SM
sw_all = [obj for obj in bpy.data.objects if obj.type == "MESH"]
sw_meshes = []
sw_objects = []
sw_textures = []
sw_pointcuboids = []

for mesh in sw_all:
    try:
        objtype = mesh.data["objtype"].lower()
    except KeyError:
        objtype = "model"
    
    if objtype == "model":
        sw_objects.append(mesh)
        if mesh.data.name not in sw_meshes:
            sw_meshes.append(mesh.data.name)
    elif objtype == "point-cuboid":
        sw_pointcuboids.append(mesh)
    
for mesh in sw_objects:
    bm = bmesh.new()
    bm.from_mesh(mesh.data)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bmesh.ops.scale(bm, vec=mesh.scale, verts=bm.verts)
    
    newmesh = bpy.data.meshes.new("")
    bm.to_mesh(newmesh)

    file = open(f"C:\\projects\\vulkan\\bin\\{mesh.data.name}.sm".lower(), "wb")

    # Signature
    file.write(array("b", [0x53, 0x4D, 0, 0]))

    # Metadata
    file.write(array("i", [len(newmesh.vertices), len(newmesh.polygons) * 3]))
    
    for vertex in newmesh.vertices:
        file.write(array("f", [vertex.co.x, vertex.co.z, vertex.co.y,\
            vertex.normal.x, vertex.normal.y, vertex.normal.z,\
            0, 0]))  # TODO: UV
    
    for polygon in newmesh.polygons:
        file.write(array("i", polygon.vertices))
    
    bm.free()
    file.close()
    
# Textures
for ob in sw_all:
    for mat_slot in ob.material_slots:
        if mat_slot.material and mat_slot.material.node_tree:
            sw_textures.extend([x for x in mat_slot.material.node_tree.nodes if x.type == "TEX_IMAGE"])

sw_textures = list(dict.fromkeys(sw_textures))

# SW
file = open("C:\\projects\\vulkan\\bin\\world.sw", "wb")

# Signature
file.write(array("b", [0x53, 0x57, 0, 0]))

# Metadata
file.write(array("i", [len(sw_meshes), len(sw_textures), len(sw_objects), len(sw_pointcuboids)]))

for mesh in sw_meshes:
    path = f"{mesh}.sm"
    zeros = 20 - len(path)
    file.write(path.encode())
    file.write(array("b", [0] * zeros))

for tex in sw_textures:
    path = f"{tex.image.name.split('.')[0]}.simg"
    zeros = 20 - len(path)
    file.write(path.encode())
    file.write(array("b", [0] * zeros))
    
    tfile = open(f"C:\\projects\\vulkan\\bin\\{path}", "wb")
    
    # Signature
    tfile.write(array("b", [0x53, 0x49, 0x4D, 0x47]))
    
    # Metadata
    tfile.write(array("i", tex.image.size))
    
    # Pixels
    tfile.write(array("B", [int(pix * 255) for pix in tex.image.pixels]))
    
    tfile.close()

for mesh in sw_objects:
    texture_id = 0
    for mat_slot in mesh.material_slots:
        if mat_slot.material and mat_slot.material.node_tree:
            for x in mat_slot.material.node_tree.nodes:
                if x.type != "TEX_IMAGE":
                    continue
                
                sw_textures.index(tex)
                break
            else:
                continue
        break

    
    file.write(array("i", [sw_meshes.index(mesh.data.name), texture_id]))
    file.write(array("f", [mesh.location.x, mesh.location.z, mesh.location.y]))
    file.write(array("f", [mesh.rotation_euler.x, -mesh.rotation_euler.z, mesh.rotation_euler.y]))  # TODO: radians
    file.write(array("f", [1, 1, 1]))  # TODO: do i need this?

for pc in sw_pointcuboids:
    file.write(array("f", [pc.location.x, pc.location.z, pc.location.y]))
    file.write(array("f", [pc.scale.x, pc.scale.z, pc.scale.y]))

file.close()
