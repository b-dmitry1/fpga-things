@echo off

set modules=riscv.v div32.v

iverilog test.v %modules%
vvp -n a.out

del /q a.out
