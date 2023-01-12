@echo off

where cl
if %errorlevel% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist ..\bin mkdir ..\bin

cls

pushd ..\bin

cl /nologo /I C:\VulkanSDK\1.3.236.0\Include /Z7 /Wall /wd5045 /D_debug ..\src\app.c /link KERNEL32.LIB USER32.LIB ..\build\VULKAN-1.LIB

popd
