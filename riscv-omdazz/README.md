# RISC-V for OMDAZZ board

A very simple RISC-V (32 bit, IM) CPU for OMDAZZ board. Able to run small C programs.

Execution time:
* Most instructions in 4 cycles (+ROM delay).
* Shifts in 5..9 cycles.
* Div/Rem in 38 cycles.
* Load in 6 cycles (+ROM/RAM/device delay).
* Store in 7 cycles (+RAM/device delay).

## Building / testing on a Windows machine

* Please use Quartus 13.0 (or later).
* To compile C examples please use riscv64-unknown-elf- toolchain. The toolchain's BIN directory should be added to PATH. If you don't want to edit your PATH add full path to toolchain's executables in "1.bat" file.
* The "1.bat" script will do anything for you.

### bin2mif

The "bin2mif" tool's repo: https://github.com/b-dmitry1/bin2mif

Please compile it manually using Visual C++ (on a Windows machine) or GCC (on a Linux machine) and copy to a "test" directory.

### Testing

A small collection of sample programs could be found in a "test" directory.

* The CPU is configured to run at 50 Mhz, max speed is 100 Mhz (using PLL).
* ROM is located at 0x00000000 - 0x00002FFF (12 kbytes).
* RAM is located at 0x00003000 - 0x00003FFF (4 kbytes).
* LED / 7SEG port is located at 0x10000000.

Please don't use standard "printf", "scanf", or other heavy functions. They need 64K+ ROM.
