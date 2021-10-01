@ECHO OFF
SETLOCAL

IF NOT EXIST "..\build" (mkdir "..\build")

pushd "..\build" > NUL

SET COMPILER_FLAGS=-nologo -W4
SET LINKER_FLAGS=-incremental:no kernel32.lib

REM Debug build
REM
cl -Od -Zi %COMPILER_FLAGS% "..\code\windows_ludum.cpp" -Fe"ludum.exe" -link %LINKER_FLAGS%

REM Release build
REM
REM cl -O2 -WX %COMPILER_FLAGS% "..\code\windows_ludum.cpp" -Fe"ludum.exe" -link %LINKER_FLAGS%

popd > NUL

ENDLOCAL
