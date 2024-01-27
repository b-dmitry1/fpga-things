.text

# This test prints "FPGA" on a 7-segment display
# Control codes are the result of different instructions

	la t0, 0x13000000

data_loop:
	# MULHU test
	la t1, 0xEE0000
	la t2, 0x1000000
	mulhu t1, t1, t2   # should be 0xEE00

	# XOR test
	la t2, 0x77 ^ 0xFF
	xor t1, t1, t2     # should be 0xEE88

	# JAL test
	call wait1

	la t1, 0xFFFF
	sw t1, 0(t0)

	# MUL test
	la t1, 0xDD
	la t2, 0x100
	mul t1, t1, t2

	# ORI test
	ori t1, t1, 0x3D ^ 0xFF

	call wait1

	la t1, 0xFFFF
	sw t1, 0(t0)

	# DIVU test
	la t1, 0xBB0000
	la t2, 0x100
	divu t1, t1, t2

	# ADDI test
	addi t1, t1, 0x73 ^ 0xFF

	call wait1

	la t1, 0xFFFF
	sw t1, 0(t0)

	la t1, 0x7700 | 0x71 ^ 0xFF
	sw t1, 0(t0)

	call wait1

	la t1, 0xFFFF
	sw t1, 0(t0)

	j data_loop

wait1:
	sw t1, 0(t0)

	la t2, 0
	la t3, 200
wait_loop:
	addi t2, t2, 1
	bltu t2, t3, wait_loop

	ret
