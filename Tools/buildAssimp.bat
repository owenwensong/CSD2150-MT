@echo off
:: This file is designed to run from the Tools folder, by the getDependencies script.
:: it will try to access the dependencies folder from there

SET CURR_DIR=%cd%
SET ASSIMP_DIR=../dependencies/assimp

if not exist %ASSIMP_DIR% goto ERRNODIR

cd %ASSIMP_DIR%
cmake CMakeLists.txt

SET SOURCE_DIR=.
SET GENERATOR=Visual Studio 16 2019
:: names are all dynamic since all moves happen for all dll and entire lib folder

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

:: move the files to a nicer place for the prop-pages, 32/64 for prop-page platform architecture macro
move /Y "%SOURCE_DIR%/build/Win32/lib" "%SOURCE_DIR%/lib/32"
move /Y "%SOURCE_DIR%/build/x64/lib" "%SOURCE_DIR%/lib/64"

:: move dlls
for /r "%SOURCE_DIR%/build/Win32/bin/Debug" %%x in (*.dll) do move /Y "%%x" "%SOURCE_DIR%/lib/32/Debug"
for /r "%SOURCE_DIR%/build/Win32/bin/Release" %%x in (*.dll) do move /Y "%%x" "%SOURCE_DIR%/lib/32/Release"
for /r "%SOURCE_DIR%/build/x64/bin/Debug" %%x in (*.dll) do move /Y "%%x" "%SOURCE_DIR%/lib/64/Debug"
for /r "%SOURCE_DIR%/build/x64/bin/Release" %%x in (*.dll) do move /Y "%%x" "%SOURCE_DIR%/lib/64/Release"

:: copy static library config. Using dll so cancel this
::copy /y "%SOURCE_DIR%/build/x64/include/assimp" "%SOURCE_DIR%/include/assimp"

:: visual studio files, really big. 
rmdir "%SOURCE_DIR%/build" /s /q

:: rename files for the prop-pages.
for /r "%SOURCE_DIR%/lib" %%x in (*-mtd.exp, *-mt.exp) do ren "%%x" assimp.exp
for /r "%SOURCE_DIR%/lib" %%x in (*-mtd.lib, *-mt.lib) do ren "%%x" assimp.lib

goto END

:ERRNODIR
powershell write-host -fore Red 'assimp directory not found'
goto END

:END
cd %CURR_DIR%
