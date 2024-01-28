@echo off

call ..\quartusdir.bat
if not exist %quartusdir%/quartus_cdb.exe echo "Can't find Quartus compiler in %quartusdir%" && exit /b

for %%I in (.) do set "d1=%%~nxI"

cd ..

%quartusdir%quartus_cdb riscv -c top --update_mif
if errorlevel 1 goto error
%quartusdir%quartus_asm --read_settings_files=on --write_settings_files=off riscv -c top
if errorlevel 1 goto error
%quartusdir%quartus_pgm -c usb-blaster -m JTAG -o p;output_files\top.sof
if errorlevel 1 goto error

cd %d1%
exit /b

:error
cd %d1%
exit /b 1
