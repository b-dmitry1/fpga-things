Riscv.prototype.do_load = function(cpu) {
	let addr = cpu.r1 + cpu.dec.i_imm();
	let res = 0;
	switch (cpu.dec.func3) {
		case 0: // LB
			res = cpu.sys.read8(addr);
			if (res & 0x80) res |= 0xFFFFFF00;
			break;
		case 1: // LH
			res = cpu.sys.read16(addr);
			if (res & 0x8000) res |= 0xFFFF0000;
			break;
		case 2: // LW
			res = cpu.sys.read32(addr);
			break;
		case 4: // LBU
			res = cpu.sys.read8(addr);
			break;
		case 5: // LHU
			res = cpu.sys.read16(addr);
			break;
	}
	cpu.set_rd(res);
}; 

Riscv.prototype.do_store = function(cpu) {
	let addr = cpu.r1 + cpu.dec.s_imm();
	switch (cpu.dec.func3) {
		case 0: // SB
			cpu.sys.write8(addr, cpu.r2);
			break;
		case 1: // SH
			cpu.sys.write16(addr, cpu.r2);
			break;
		case 2: // SW
			cpu.sys.write32(addr, cpu.r2);
			break;
	}
}; 
