@echo off

where cl
if %errorlevel% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist ..\bin mkdir ..\bin

cls

pushd ..\bin

glslc -o shader-v.spv ..\src\vulkan\glsl\shader.vert
glslc -o shader-f.spv ..\src\vulkan\glsl\shader.frag
glslc -o shadow-v.spv ..\src\vulkan\glsl\shadow.vert
glslc -o shadow-f.spv ..\src\vulkan\glsl\shadow.frag

cl /nologo /I C:\VulkanSDK\1.3.250.0\Include /Z7 /Wall /wd5045 /DSLN_DEBUG /DSLN_VULKAN /DSLN_XAUDIO2 /DSLN_WIN64 ..\src\win64.c /link KERNEL32.LIB USER32.LIB ..\build\VULKAN-1.LIB XAUDIO2.LIB

popd
