@echo off

call 1.bat || exit /b

cd ..
c:\altera\13.0sp1\quartus\bin64\quartus_cdb riscv -c top --update_mif || pause && exit /b
c:\altera\13.0sp1\quartus\bin64\quartus_asm --read_settings_files=on --write_settings_files=off riscv -c top || pause && exit /b
c:\altera\13.0sp1\quartus\bin64\quartus_pgm -c usb-blaster -m JTAG -o p;output_files\top.sof
