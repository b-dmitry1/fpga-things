@echo off

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -mcmodel=medany -O0 -nostartfiles -Triscv.ld vectors.S || pause && exit /b
riscv64-unknown-elf-objcopy -O binary a.out ..\firmware.bin || pause && exit /b
riscv64-unknown-elf-objdump -D a.out >a.asm

del a.out

cd ..
bin2mif 32 firmware.bin
move firmware.mif ..
