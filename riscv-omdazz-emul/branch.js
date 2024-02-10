Riscv.prototype.do_branch = function(cpu) {
	let addr = (cpu.pc + cpu.dec.b_imm() - 4) & 0xFFFFFFFF;
	switch (cpu.dec.func3) {
		case 0: // BEQ
			if (cpu.r1 == cpu.r2) cpu.set_pc(addr);
			break;
		case 1: // BNE
			if (cpu.r1 != cpu.r2) cpu.set_pc(addr);
			break;
		case 4: // BLT
			r1s = cpu.r1 & 0x80000000 ? -((cpu.r1 ^ 0xFFFFFFFF) + 1) : cpu.r1;
			r2s = cpu.r2 & 0x80000000 ? -((cpu.r2 ^ 0xFFFFFFFF) + 1) : cpu.r2;
			if (r1s < r2s) cpu.set_pc(addr);
			break;
		case 5: // BGE
			r1s = cpu.r1 & 0x80000000 ? -((cpu.r1 ^ 0xFFFFFFFF) + 1) : cpu.r1;
			r2s = cpu.r2 & 0x80000000 ? -((cpu.r2 ^ 0xFFFFFFFF) + 1) : cpu.r2;
			if (r1s > r2s - 0.5) cpu.set_pc(addr);
			break;
		case 6: // BLTU
			if (cpu.r1 < cpu.r2) cpu.set_pc(addr);
			break;
		case 7: // BGEU
			if (cpu.r1 >= cpu.r2 - 0.5) cpu.set_pc(addr);
			break;
	}
}; 
