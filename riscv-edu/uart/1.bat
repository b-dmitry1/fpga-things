@echo off

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -mcmodel=medany -O0 -nostartfiles -Tmyriscv.ld vectors.S syscalls.c main.c
riscv64-unknown-elf-objcopy -O binary a.out ..\firmware.bin
riscv64-unknown-elf-objdump -D a.out >a.asm

pause
