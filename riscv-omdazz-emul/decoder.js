function bits(value, start, n) {
	value >>= start;
	value &= (1 << n) - 1;
	return value & 0xFFFFFFFF;
}

function uint(value) {
	value &= 0xFFFFFFFF;
	if (value < 0)
		value += 0x100000000;
	return value;
}

RiscvDecoder = function() {
	this.instr  = 0;
	this.opcode = 0;
	this.rd     = 0;
	this.rs1    = 0;
	this.rs2    = 0;
	this.func3  = 0;
	this.func7  = 0;
	this.csr    = 0;
	this.u_imm  = 0;
	this.i_imm  = 0;
	this.b_imm  = 0;
	this.s_imm  = 0;
	this.j_imm  = 0;
	this.z_imm  = 0;

	this.is_lui     = 0;
	this.is_auipc   = 0;
	this.is_jal     = 0;
	this.is_jalr    = 0;
	this.is_branch  = 0;
	this.is_load    = 0;
	this.is_store   = 0;
	this.is_alu     = 0;
	this.is_control = 0;
	this.is_atomic  = 0;
	this.is_r2      = 0;

	this.decode = function(instr) {
		this.instr  = instr;
		this.opcode = instr & 0x7F;
		this.rd     = (instr >> 7) & 0x1F;
		this.rs1    = (instr >> 15) & 0x1F;
		this.rs2    = (instr >> 20) & 0x1F;
		this.func3  = (instr >> 12) & 0x07;
		this.func7  = (instr >> 25) & 0x7F;
		this.csr    = (instr >> 20) & 0xFFF;
		this.is_r2      = (this.opcode & 0x20) != 0;
	};

	this.u_imm  = () => this.instr & 0xFFFFF000;
	this.i_imm  = () => (this.instr >> 20) | (this.instr & 0x80000000 ? 0xFFFFF000 : 0);
	this.b_imm  = () => (bits(this.instr, 7, 1) << 11) |
		(bits(this.instr, 8, 4) << 1) |
		(bits(this.instr, 25, 6) << 5) |
		(bits(this.instr, 31, 1) << 12) | (this.instr & 0x80000000 ? 0xFFFFE000 : 0);
	this.s_imm  = () => (bits(this.instr, 7, 5) << 0) |
		(bits(this.instr, 25, 7) << 5) | (this.instr & 0x80000000 ? 0xFFFFF000 : 0);
	this.j_imm  = () => (bits(this.instr, 12, 8) << 12) |
		(bits(this.instr, 20, 1) << 11) |
		(bits(this.instr, 21, 10) << 1) |
		(bits(this.instr, 31, 1) << 20) | (this.instr & 0x80000000 ? 0xFFE00000 : 0);
	this.z_imm  = () => bits(this.instr, 15, 5);
};
