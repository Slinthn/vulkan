@echo off

where cl
if %errorlevel% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist ..\bin mkdir ..\bin

cls

pushd ..\bin

py ../scripts/ply.py ../res/counter.ply counter.sm
py ../scripts/ply.py ../res/shelf.ply shelf.sm
py ../scripts/ply.py ../res/shop.ply shop.sm

py ../scripts/world.py ../scripts/world.json ../bin/world.sw

py ../scripts/bmp.py ../res/cat.bmp ../bin/cat.simg
py ../scripts/bmp.py ../res/dog.bmp ../bin/dog.simg

glslc -o shader-v.spv ..\src\vulkan\glsl\shader.vert
glslc -o shader-f.spv ..\src\vulkan\glsl\shader.frag

cl /nologo /I C:\VulkanSDK\1.3.236.0\Include /Z7 /Wall /wd5045 /DSLN_DEBUG /DSLN_VULKAN /DSLN_WIN64 ..\src\win64.c /link KERNEL32.LIB USER32.LIB ..\build\VULKAN-1.LIB

popd
