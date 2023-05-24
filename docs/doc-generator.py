import os
import re

functions = {}

for root, dirs, files in os.walk(os.path.join("..", "src")):
    for file in files:

        currentedit = ""
        currenteditname = ""
        functioninfo = None
        path = os.path.join(root, file)
        functions[path] = []

        with open(path) as rfile:
            for line in rfile:
                if line.startswith("/**"):
                    functioninfo = {
                        "meta": {
                            "name": "",
                            "description": "",
                            "signature": ""
                        },
                        "params": {}
                    }
                    continue
                
                if functioninfo == None:
                    continue

                if line.startswith(" * @brief"):
                    currentedit = "meta"
                    currenteditname = "description"
                    functioninfo[currentedit][currenteditname] = line.split(" * @brief")[1]
                    continue

                if line.startswith(" * @param"):
                    info = line.split(" ")
                    currentedit = "params"
                    currenteditname = info[3]
                    functioninfo[currentedit][currenteditname] = info[4:-1]
                    continue

                if line.startswith(" */"):
                    currentedit = "meta"
                    currenteditname = "signature"
                    continue

                if line.startswith("){"):
                    functioninfo[currentedit][currenteditname] += "    );"
                    functions[path].append(functioninfo)
                    functioninfo = None
                    currentedit = ""
                    currenteditname = ""
                    continue

                if currenteditname == "signature" and functioninfo[currentedit][currenteditname] == "":
                    functioninfo[currentedit]["name"] = re.findall(".+ (.+)\(", line)[0]

                if currentedit != "":
                    if currenteditname == "signature":
                        functioninfo[currentedit][currenteditname] += "    "
                        functioninfo[currentedit][currenteditname] += line
                    else:
                        functioninfo[currentedit][currenteditname] += line


with open("docs.md", "w") as file:
    for funcfile in functions:
        print(f"# {funcfile}", file=file)
        for func in functions[funcfile]:
            print(f"## {func['meta']['name']}", file=file)
            print(f"{func['meta']['description']}", file=file)
            print(func["meta"]["signature"], file=file)
            print(file=file)
