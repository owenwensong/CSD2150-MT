@echo off
powershell write-host -fore Green getDependencies script started!
powershell write-host -fore Red existing dependencies folder will be deleted!
if exist "../dependencies" rmdir "../dependencies" /s
if exist "../dependencies" goto ERRDIRSTILLEXIST

powershell write-host -fore Yellow creating new dependencies folder
mkdir "../dependencies"
if %errorlevel% GEQ 1 goto ERRDIRCREATIONFAIL

powershell write-host -fore Green 'Setting up: OpenGL Mathematics (GLM)'
git clone https://github.com/g-truc/glm "../dependencies/glm"
if %errorlevel% GEQ 1 goto ERRGITCLONE

powershell write-host -fore Green 'Setting up: tinyddsloader'
git clone https://github.com/benikabocha/tinyddsloader "../dependencies/tinyddsloader"
if %errorlevel% GEQ 1 goto ERRGITCLONE

powershell write-host -fore Green 'Setting up: Open Asset Import Library (assimp)'
git clone https://github.com/assimp/assimp "../dependencies/assimp"
if %errorlevel% GEQ 1 goto ERRGITCLONE
call buildAssimp.bat

powershell write-host -fore Green Setup completed successfully!
goto END



:ERRDIRSTILLEXIST
powershell write-host -fore Red dependencies directory still exists, stopping script.
goto END

:ERRDIRCREATIONFAIL
powershell write-host -fore Red failed to create new dependencies directory
goto END

:ERRGITCLONE
powershell write-host -fore Red failed to clone repository
goto END

:END
pause