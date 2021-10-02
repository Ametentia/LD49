@ECHO OFF
SETLOCAL

IF NOT EXIST "..\build" (mkdir "..\build")

pushd "..\build" > NUL

SET COMPILER_FLAGS=-nologo -W4 -wd4305 -wd4201 -wd4505 -wd4244 -I "..\base"
SET LINKER_FLAGS=-incremental:no gdi32.lib kernel32.lib user32.lib shell32.lib shcore.lib pathcch.lib ole32.lib

REM Build renderer
REM
call "..\base\renderer\windows_wgl.bat" debug

REM Debug build
REM
cl -Od -Zi %COMPILER_FLAGS% "..\code\windows_ludum.cpp" -Fe"ludum.exe" -link %LINKER_FLAGS%

REM Release build
REM
REM cl -O2 -WX %COMPILER_FLAGS% "..\code\windows_ludum.cpp" -Fe"ludum.exe" -link %LINKER_FLAGS%

popd > NUL

ENDLOCAL
