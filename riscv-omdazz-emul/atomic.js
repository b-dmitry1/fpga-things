Riscv.prototype.do_atomic = function(cpu) {
	let addr = cpu.r1;
	switch ((cpu.dec.func7 >> 2) & 0x1F) {
		case 0x00: // AMOADD
			value = cpu.sys.read32(addr);
			cpu.sys.write32(addr, uint(value + cpu.r2));
			cpu.set_rd(value);
			break;
		case 0x01: // AMOSWAP
			value = cpu.sys.read32(addr);
			cpu.sys.write32(addr, cpu.r2);
			cpu.set_rd(value);
			break;
		case 0x02: // LR
			value = cpu.sys.read32(addr);
			cpu.set_rd(value);
			break;
		case 0x03: // SC
			value = 0;
			if (value == 0)
				cpu.sys.write32(addr, cpu.r2);
			cpu.set_rd(value);
			break;
		case 0x04: // AMOXOR
			value = cpu.sys.read32(addr);
			cpu.sys.write32(addr, value ^ cpu.r2);
			cpu.set_rd(value);
			break;
		case 0x08: // AMOOR
			value = cpu.sys.read32(addr);
			cpu.sys.write32(addr, value | cpu.r2);
			cpu.set_rd(value);
			break;
		case 0x0C: // AMOAND
			value = cpu.sys.read32(addr);
			cpu.sys.write32(addr, value & cpu.r2);
			cpu.set_rd(value);
			break;
	}
}; 

Riscv.prototype.do_fence = function(cpu) {
};
