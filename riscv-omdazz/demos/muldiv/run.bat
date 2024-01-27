@echo off

for %%I in (.) do set "d=%%~nxI"

cd ..

call compile.bat %d%
if errorlevel 1 goto :error
call write.bat %d%
if errorlevel 1 goto :error

cd %d%
exit /b

:error
cd %d%
pause
exit /b
