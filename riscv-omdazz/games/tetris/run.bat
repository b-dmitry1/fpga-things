@echo off

for %%I in (.) do set "d=%%~nxI"

cd ..
call compile.bat %d% || cd%d% && pause && exit /b
call write.bat %d% || cd%d% && pause && exit /b
cd %d%
