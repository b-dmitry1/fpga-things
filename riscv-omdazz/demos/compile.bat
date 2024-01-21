@echo off

SetLocal EnableExtensions EnableDelayedExpansion

if "%1"=="" echo Please do not use this script directly && goto error

set "c="
for %%f in ("..\lib\*.c") do (
  set c=!c!..\lib\%%f 
)

for %%f in ("%1\*.c") do (
  set c=!c!%%f 
)

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -mcmodel=medany -O2 -nostartfiles -I. -I%1 -I..\lib -T..\lib\riscv.ld ..\lib\vectors.S !c!
if errorlevel 1 goto error
riscv64-unknown-elf-objcopy -O binary a.out firmware.bin
if errorlevel 1 goto error

if exist a.out del a.out

..\bin2mif 32 firmware.bin
move firmware.mif ..
exit /b

:error
exit /b 1
