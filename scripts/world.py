import sys
from array import array
import json

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
outputfile.write(array("i", [len(data["objects"])]))


models = []

for index, model in enumerate(data["models"]):
    name = model["name"]
    file = model["file"]

    zero_count = 10 - len(file)

    outputfile.write(array("b", file.encode()))
    outputfile.write(array("b", [0] * zero_count))

    models.append(name)

for object in data["objects"]:
    model_index = models.index(object["model"])
    position = object.get("position", [0, 0, 0])
    rotation = object.get("rotation", [0, 0, 0])
    scale = object.get("scale", [1, 1, 1])

    outputfile.write(array("i", [model_index]))
    outputfile.write(array("f", position))
    outputfile.write(array("f", rotation))
    outputfile.write(array("f", scale))
