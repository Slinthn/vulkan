import sys
from array import array
from PIL import Image

if len(sys.argv) != 3:
    print(f"ERROR: Syntax: py {sys.argv[0]} input_file output_file")
    exit()

inputfilename = sys.argv[1]
outputfilename = sys.argv[2]

inputfile = Image.open(inputfilename)
outputfile = open(outputfilename, "wb")

pixels = inputfile.load()

outputfile.write(b"ST\0\0")
outputfile.write(array("i", inputfile.size))

for x in range(inputfile.size[0]):
    for y in range(inputfile.size[1]):
        outputfile.write(array("f", [pixels[x, y]]))

