# Educational RISC-V for iverilog

A very simple RISC-V (32 bit, IM) CPU designed to run in Icarus Verilog simulator (iverilog) able to run console C programs.

Execution time:
* Most instructions in 4 cycles (+ROM delay).
* Shifts in 5..9 cycles.
* Div/Rem in 38 cycles.
* Load in 5 cycles (+ROM/RAM/device delay).
* Store in 6 cycles (+RAM/device delay).

## Building / testing on a Windows machine

* Please use Windows version of Icarus Verilog simulator. The iverilog's BIN directory should be added to PATH. If you don't want to edit your PATH add full path to iverilog.exe and "vvp.exe" in "1.bat" file.
* To compile C examples please use riscv64-unknown-elf- toolchain. The toolchain's BIN directory should be added to PATH. If you don't want to edit your PATH add full path to toolchain's executables in "1.bat" file.
* The "1.bat" script will do anything for you.

