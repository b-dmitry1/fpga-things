.text
#.section .isr_vector
#.align 2

	la t0, 0x10000000

data_loop:
	la t1, 0xEE0000
	la t2, 0x1000000
	mulhu t1, t1, t2   # should be 0xEE00
	la t2, 0x77 ^ 0xFF
	xor t1, t1, t2     # should be 0xEE88
	call wait1
	la t1, 0xFFFF
	sw t1, 0(t0)
	la t1, 0xDD00 | 0x3D ^ 0xFF
	call wait1
	la t1, 0xFFFF
	sw t1, 0(t0)
	la t1, 0xBB00 | 0x73 ^ 0xFF
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
