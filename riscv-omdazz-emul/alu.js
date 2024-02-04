Riscv.prototype.do_alu = function(func7, func3, opcode, r1, r2) {
	if (opcode == 0x33 && func7 == 1) {
		switch (func3) {
			case 0:
				return r1 * r2;
			case 4: // DIV
				r1s = r1 & 0x80000000 ? -((r1 ^ 0xFFFFFFFF) + 1) : r1;
				r2s = r2 & 0x80000000 ? -((r2 ^ 0xFFFFFFFF) + 1) : r2;
				return r1s / r2s;
			case 5: // DIVU
				return r1 / r2;
			case 6: // MOD
				r1s = r1 & 0x80000000 ? -((r1 ^ 0xFFFFFFFF) + 1) : r1;
				r2s = r2 & 0x80000000 ? -((r2 ^ 0xFFFFFFFF) + 1) : r2;
				return r1s % r2s;
			case 7: // MODU
				return r1 % r2;
			default:
				console.log("Not implemented: mul/div func3 = " + func3);
				return 0;
		}
	} else {
		switch (func3) {
			case 0:
				if (opcode == 0x33 && func7 == 0x20)
					return (r1 - r2) & 0xFFFFFFFF; // SUB
				return (r1 + r2) & 0xFFFFFFFF; // ADD
			case 1: // SLL
				return r1 << (r2 & 0x1F);
			case 2: // SLT
				r1s = r1 & 0x80000000 ? -((r1 ^ 0xFFFFFFFF) + 1) : r1;
				r2s = r2 & 0x80000000 ? -((r2 ^ 0xFFFFFFFF) + 1) : r2;
				return r1s < r2s;
			case 3: // SLTU
				return r1 < r2;
			case 4: // XOR
				return r1 ^ r2;
			case 5: // SRL / SRA
				let res = r1;
				let shamt = r2 & 0x1F;

				if (func7 == 0x20) {
					res >>= shamt;
					if (res < 0)
						res += 0x100000000;
					res &= 0xFFFFFFFF;
					return res; // SRA
				}
	
				if (shamt >= 1) {
					res >>= 1;
					res &= 0x7FFFFFFF;
					shamt--;
				}
				res >>= shamt;

				return res & 0xFFFFFFFF; // SRL
			case 6: // OR
				return r1 | r2;
			case 7: // AND
				return r1 & r2;
		}
	}
	return 0;
};
