@echo off

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -mcmodel=medany -O0 -nostartfiles -I. -I../lib -T../lib/riscv.ld ../lib/vectors.S ../lib/board.c ../lib/syscalls.c ../lib/vga16color.c sdcard.c main.c || pause && exit /b
riscv64-unknown-elf-objcopy -O binary a.out ..\firmware.bin || pause && exit /b

if exist a.out del a.out

cd ..
bin2mif 32 firmware.bin
