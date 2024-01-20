@echo off

call ..\quartusdir.bat
if not exist %quartusdir%/quartus_cdb.exe echo "Can't find Quartus compiler in %quartusdir%" && pause && exit /b

for %%I in (.) do set "d1=%%~nxI"

cd ..
%quartusdir%quartus_cdb riscv -c top --update_mif || pause && exit /b
%quartusdir%quartus_asm --read_settings_files=on --write_settings_files=off riscv -c top || pause && exit /b
%quartusdir%quartus_pgm -c usb-blaster -m JTAG -o p;output_files\top.sof
cd %d1%
