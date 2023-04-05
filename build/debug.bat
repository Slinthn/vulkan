@echo off

where cl
if %errorlevel% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist ..\bin mkdir ..\bin

cls

pushd ..\bin

py ../scripts/ply.py ../res/cube.ply cube.sm
py ../scripts/ply.py ../res/tower.ply tower.sm

rem glslangValidator -e main -gVS -V -o shader-v.spv ..\src\graphics\glsl\shader.vert
rem glslangValidator -e main -gVS -V -o shader-f.spv ..\src\graphics\glsl\shader.frag
glslc -o shader-v.spv ..\src\graphics\glsl\shader.vert
glslc -o shader-f.spv ..\src\graphics\glsl\shader.frag

cl /nologo /I C:\VulkanSDK\1.3.236.0\Include /Z7 /Wall /wd5045 /DSLN_DEBUG /DSLN_VULKAN /DSLN_WIN64 ..\src\win64.c /link KERNEL32.LIB USER32.LIB ..\build\VULKAN-1.LIB

popd
