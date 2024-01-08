@echo off

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -mcmodel=medany -O0 -nostartfiles -Triscv.ld vectors.S
riscv64-unknown-elf-objcopy -O binary a.out ..\firmware.bin
riscv64-unknown-elf-objcopy -O ihex a.out ..\firmware.hex
riscv64-unknown-elf-objdump -D a.out >a.asm

cd ..
bin2mif 32 firmware.bin

cd ..
c:\altera\13.0sp1\quartus\bin64\quartus_cdb riscv -c top --update_mif
c:\altera\13.0sp1\quartus\bin64\quartus_asm --read_settings_files=on --write_settings_files=off riscv -c top
c:\altera\13.0sp1\quartus\bin64\quartus_pgm -c usb-blaster -m JTAG -o p;output_files\top.sof

pause
