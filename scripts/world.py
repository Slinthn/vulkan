import sys
from array import array
import json
import math

if len(sys.argv) != 3:
    print(f"ERROR: Syntax: py {sys.argv[0]} input_file output_file")
    exit()

inputfilename = sys.argv[1]
outputfilename = sys.argv[2]

inputfile = open(inputfilename, "r")
outputfile = open(outputfilename, "wb")
data = json.loads(inputfile.read())

outputfile.write(array("b", [0x53, 0x57, 0, 0]))
outputfile.write(array("i", [len(data["models"])]))
outputfile.write(array("i", [len(data["textures"])]))
outputfile.write(array("i", [len(data["objects"])]))
outputfile.write(array("i", [len(data["point-cuboids"])]))

models = []
textures = []

for index, model in enumerate(data["models"]):
    name = model["name"]
    file = model["file"]

    zero_count = 20 - len(file)

    outputfile.write(array("b", file.encode()))
    outputfile.write(array("b", [0] * zero_count))

    models.append(name)

for index, texture in enumerate(data["textures"]):
    name = texture["name"]
    file = texture["file"]

    zero_count = 20 - len(file)

    outputfile.write(array("b", file.encode()))
    outputfile.write(array("b", [0] * zero_count))

    textures.append(name)

for object in data["objects"]:
    model_index = models.index(object["model"])
    texture_index = textures.index(object["texture"])
    position = object.get("position", [0, 0, 0])
    rotation = object.get("rotation", [0, 0, 0])
    scale = object.get("scale", [1, 1, 1])

    position[1], position[2] = -position[2], position[1]  # Vulkan
    rotation[1], rotation[2] = -rotation[2], rotation[1]  # Vulkan
    scale[1], scale[2] = -scale[2], scale[1]  # Vulkan

    rotation = [math.radians(x) for x in rotation]

    outputfile.write(array("i", [model_index, texture_index]))
    outputfile.write(array("f", position))
    outputfile.write(array("f", rotation))
    outputfile.write(array("f", scale))

for cb in data["point-cuboids"]:
    centre = cb.get("centre", [0, 0, 0])
    centre[1], centre[2] = -centre[2], centre[1]  # Vulkan
    dimension = cb.get("dimension", [0, 0, 0])
    dimension[1], dimension[2] = dimension[2], dimension[1]  # Vulkan

    outputfile.write(array("f", centre))
    outputfile.write(array("f", dimension))
