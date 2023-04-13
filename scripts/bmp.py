import sys
from array import array

if len(sys.argv) != 3:
    print(f"ERROR: Syntax: py {sys.argv[0]} input_file output_file")
    exit()

inputfilename = sys.argv[1]
outputfilename = sys.argv[2]

inputfile = open(inputfilename, "rb")
outputfile = open(outputfilename, "wb")

outputfile.write(array("b", [0x53, 0x49, 0x4D, 0x47]))

inputfile.seek(10)
pixel_offset = int.from_bytes(inputfile.read(4), byteorder="little")

inputfile.seek(18)
width = int.from_bytes(inputfile.read(4), byteorder="little")
height = int.from_bytes(inputfile.read(4), byteorder="little")
outputfile.write(array("i", [width, height]))

bytes_parsed = 0

inputfile.seek(pixel_offset)

while True:
    b = inputfile.read(3)
    if len(b) < 3:
        break

    b = bytearray(b)
    b[0], b[2] = b[2], b[0]
    b.append(0xFF)
    outputfile.write(b)

    bytes_parsed += 3
    if bytes_parsed / 3 == width:
        inputfile.read(bytes_parsed % 4)
        bytes_parsed = 0

print("compiled binary BMP " + outputfilename)
