@ECHO OFF
SETLOCAL

IF NOT EXIST "..\build" (mkdir "..\build")

pushd "..\build" > NUL

SET COMPILER_FLAGS=-nologo -W4 -wd4505 -wd4201 -wd4244 -I "..\base"
SET LINKER_FLAGS=-incremental:no gdi32.lib kernel32.lib user32.lib shell32.lib shcore.lib pathcch.lib ole32.lib

REM Build
REM
cl -Od -Zi %COMPILER_FLAGS% "..\amtpack\windows_amtpack.cpp" -Fe"amtpack.exe" -link %LINKER_FLAGS%

popd "..\build" > NUL
