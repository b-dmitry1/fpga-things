Riscv.prototype.do_csr = function(cpu) {
	let csr = cpu.dec.csr;
	switch (cpu.dec.func3) {
		case 0:
			switch (csr) {
				case 0x000: // ECALL
					cpu.trap = true;
					cpu.trap_code = 8;
					break;
				case 0x001: // EBREAK
					console.log("EBREAK");
					break;
				case 0x105: // WFI
					cpu.wfi = true;
					cpu.mstatus |= 0x08;
					break;
				case 0x302: // MRET
					let status = cpu.mstatus;
					cpu.mstatus = ((status & 0x80) >> 4) | (cpu.mmode << 11) | 0x80;
					cpu.mmode = (status >> 11) & 0x03;
					cpu.pc = cpu.mepc;
					break;
				default:
					console.log("unknown sys: " + csr.toString(16));
					break;
			}
			break;
		case 1: // CSRRW
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, cpu.r1);
			cpu.set_rd(value);
			break;
		case 2: // CSRRS
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, value | cpu.r1);
			cpu.set_rd(value);
			break;
		case 3: // CSRRC
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, value & (cpu.r1 ^ 0xFFFFFFFF));
			cpu.set_rd(value);
			break;
		case 5: // CSRRWI
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, cpu.dec.z_imm());
			cpu.set_rd(value);
			break;
		case 6: // CSRRSI
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, value | cpu.dec.z_imm());
			cpu.set_rd(value);
			break;
		case 7: // CSRRCI
			value = cpu.csr_read(cpu, csr);
			cpu.csr_write(cpu, csr, value & (cpu.dec.z_imm ^ 0xFFFFFFFF));
			cpu.set_rd(value);
			break;
	}
}; 

Riscv.prototype.csr_read = function(cpu, csr) {
	switch (csr) {
		case 0x300: return cpu.mstatus;
		case 0x301: return cpu.misa;
		case 0x304: return cpu.mie;
		case 0x305: return cpu.mtvec;
		case 0x340: return cpu.mscratch;
		case 0x341: return cpu.mepc;
		case 0x342: return cpu.mcause;
		case 0x343: return cpu.mtval;
		case 0x344: return cpu.mip;
		case 0xF11: return cpu.vendorid;
		case 0xF12: return cpu.archid;
		case 0xF13: return cpu.impid;
		case 0xF14: return cpu.hartid;
	}
	return 0;
};

Riscv.prototype.csr_write = function(cpu, csr, value) {
	value = uint(value);
	switch (csr) {
		case 0x300: cpu.mstatus  = value; break;
		case 0x304: cpu.mie      = value; break;
		case 0x305: cpu.mtvec    = value; break;
		case 0x340: cpu.mscratch = value; break;
		case 0x341: cpu.mepc     = value; break;
		case 0x342: cpu.mcause   = value; break;
		case 0x343: cpu.mtval    = value; break;
		case 0x344: cpu.mip      = value; break;
	}
	return 0;
};
