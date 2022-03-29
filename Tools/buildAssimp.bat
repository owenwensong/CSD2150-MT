@echo off
:: This file is designed to run from the Tools folder, byy the getDependencies script.
:: it will try to access the dependencies folder from there

SET CURR_DIR=%cd%
SET ASSIMP_DIR=../dependencies/assimp

if not exist %ASSIMP_DIR% goto ERRNODIR

cd %ASSIMP_DIR%
cmake CMakeLists.txt

SET SOURCE_DIR=.
SET GENERATOR=Visual Studio 16 2019
::assume all names are assimp-vc142....lib, vc142 because vs2019 toolset 142

SET BINARIES_DIR="./build/Win32"
cmake . -G "%GENERATOR%" -A Win32 -S %SOURCE_DIR% -B %BINARIES_DIR%
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

SET BINARIES_DIR="./build/x64"
cmake . -G "%GENERATOR%" -A x64 -S %SOURCE_DIR% -B %BINARIES_DIR%
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

:: remove existing lib files. I assume that all 4 builds succeed here
if exist "%SOURCE_DIR%/lib" rmdir "%SOURCE_DIR%/lib" /s /q
mkdir "%SOURCE_DIR%/lib"

:: move the files to a nicer place for the prop-pages, 32/64 for prop-page architecture macro
move /Y %SOURCE_DIR%/build/Win32/lib %SOURCE_DIR%/lib/32
move /Y %SOURCE_DIR%/build/x64/lib %SOURCE_DIR%/lib/64

:: visual studio files, really big. 
rmdir "%SOURCE_DIR%/build" /s /q

:: rename files for the prop-pages. cd because I'm lazy to get the * to work cleanly with relative path
cd %SOURCE_DIR%/lib
for /r %%i in (*-mtd.exp, *-mt.exp) do ren "%%i" assimp.exp
for /r %%i in (*-mtd.lib, *-mt.lib) do ren "%%i" assimp.lib

goto END

:ERRNODIR
powershell write-host -fore Red assimp directory not found
goto END

:END
cd %CURR_DIR%
