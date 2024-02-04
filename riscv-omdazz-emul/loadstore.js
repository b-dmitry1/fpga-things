Riscv.prototype.do_load = function() {
	let addr = this.r1 + this.dec.i_imm();
	let res = 0;
	switch (this.dec.func3) {
		case 0: // LB
			res = this.sys.read8(addr);
			if (res & 0x80) res |= 0xFFFFFF00;
			break;
		case 2: // LW
			res = this.sys.read32(addr);
			break;
		case 4: // LBU
			res = this.sys.read8(addr);
			break;
	}
	this.set_rd(res);
}; 

Riscv.prototype.do_store = function() {
	let addr = this.r1 + this.dec.s_imm();
	switch (this.dec.func3) {
		case 0: // SB
			this.sys.write8(addr, this.r2);
			break;
		case 2: // SW
			this.sys.write32(addr, this.r2);
			break;
	}
}; 
