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

* The CPU is configured to run at 50 Mhz.
* ROM is located at 0x00000000 - 0x00002FFF (12 kbytes).
* RAM is located at 0x00003000 - 0x00003FFF (4 kbytes).
* LED / 7SEG port is located at 0x10000000.
* SDRAM is located at 0x80000000 - 0x807FFFFF (8 mbytes).
* Frame buffer can be placed anywhere in a SDRAM.

Please don't use standard "printf", "scanf", or other heavy functions. They need 64K+ ROM.

### SDRAM controller

By default the SDRAM controller is configured to work with 8 MByte 16-bit SDRAM. If you want more memory (32 or 64 megabytes)
replace the SDRAM IC and set "ras_bits" and "cas_bits" parameters in sdram.v file as defined in a datasheet.
For a common 16-bit 32 MByte SDRAM the values should be: ras_bits = 13 and cas_bits = 9. Unfortunatelly there is no A12 line
on the board so you'll need to connect it manually to any free pin of FPGA.

### Video controller

The displayed video window (framebuffer) may reside anywhere in a SDRAM. To place a window where you want it to be just program
the 8-byte aligned address to a "vram_start" register located at 0x12000000 (write only).

A smooth hardware vertical scrolling can be achieved by adding +/- 320 bytes to a "vram_start" register.

The video mode is 640 x 480 pixels with 8 displayed colors packed in a 4-bit values. Each byte of a video memory contains 2 pixels so
the screen size is 640 x 480 x 4 bit = 153600 bytes. The stride is 320 bytes.
