.text

.globl asm_sll
asm_sll:
	sll a0, a0, a1
	ret

.globl asm_slli1
asm_slli1:
	slli a0, a0, 1
	ret
.globl asm_slli2
asm_slli2:
	slli a0, a0, 2
	ret
.globl asm_slli4
asm_slli4:
	slli a0, a0, 4
	ret
.globl asm_slli5
asm_slli5:
	slli a0, a0, 5
	ret
.globl asm_slli8
asm_slli8:
	slli a0, a0, 8
	ret
.globl asm_slli13
asm_slli13:
	slli a0, a0, 13
	ret
.globl asm_slli16
asm_slli16:
	slli a0, a0, 16
	ret
.globl asm_slli31
asm_slli31:
	slli a0, a0, 31
	ret

.globl asm_srl
asm_srl:
	srl a0, a0, a1
	ret

.globl asm_sra
asm_sra:
	sra a0, a0, a1
	ret
