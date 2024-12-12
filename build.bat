@echo off
cls

Rem check if msvc needs to be set up
where cl >nul 2>nul
if %ERRORLEVEL% neq 0 call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul

set COMMON_COMPILER_FLAGS= -Zi -nologo -std:c11 -wd5105 -GR-
set COMMON_LINKER_FLAGS= opengl32.lib user32.lib gdi32.lib dsound.lib

if not exist bin mkdir bin
pushd bin

cl  %COMMON_COMPILER_FLAGS% ..\source\win32.c /link %COMMON_LINKER_FLAGS% -out:win32.exe

popd

