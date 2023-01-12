@echo off

where cl
if %errorlevel% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

if not exist ..\bin mkdir ..\bin

cls

pushd ..\bin

cl /nologo /Z7 /Wall /wd5045 /D_debug ..\src\app.c /link KERNEL32.LIB USER32.LIB

popd
