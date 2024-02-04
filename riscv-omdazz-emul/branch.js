Riscv.prototype.do_branch = function() {
	let addr = (this.pc + this.dec.b_imm() - 4) & 0xFFFFFFFF;
	switch (this.dec.func3) {
		case 0: // BEQ
			if (this.r1 == this.r2) this.set_pc(addr);
			break;
		case 1: // BNE
			if (this.r1 != this.r2) this.set_pc(addr);
			break;
		case 4: // BLT
			r1s = this.r1 & 0x80000000 ? -((this.r1 ^ 0xFFFFFFFF) + 1) : this.r1;
			r2s = this.r2 & 0x80000000 ? -((this.r2 ^ 0xFFFFFFFF) + 1) : this.r2;
			if (r1s < r2s) this.set_pc(addr);
			break;
		case 5: // BGE
			r1s = this.r1 & 0x80000000 ? -((this.r1 ^ 0xFFFFFFFF) + 1) : this.r1;
			r2s = this.r2 & 0x80000000 ? -((this.r2 ^ 0xFFFFFFFF) + 1) : this.r2;
			if (r1s > r2s - 0.5) this.set_pc(addr);
			break;
		case 6: // BLTU
			if (this.r1 < this.r2) this.set_pc(addr);
			break;
		case 7: // BGEU
			if (this.r1 >= this.r2 - 0.5) this.set_pc(addr);
			break;
	}
}; 
