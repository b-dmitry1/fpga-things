Riscv = function(sys) {
	//////////////////////////////////////////////////////////////////////////////
	// PUBLIC
	// load, reset, step
	//////////////////////////////////////////////////////////////////////////////

	// Firmware loader
	this.load = async function(url) {
		this.ready = false;
		const response = await fetch(url);
		const buffer = await response.arrayBuffer();
		const program = new Uint32Array(buffer);

		for (let i = 0; i < program.length; i++)
			this.sys.write32(i * 4, program[i]);

		this.ready = true;
	}; 

	// CPU reset
	this.reset = function() {
		this.pc = 0;
		this.r[0] = 0;
	}

	// Execute 1 next instruction
	this.step = function() {
		if (!this.ready) return;

		// Fetch next instruction
		this.fetch();

		// Decode instruction
		this.dec.decode(this.instr);

		// Load register values
		this.r1 = this.r[this.dec.rs1];
		this.r2 = this.dec.is_r2 ? this.r[this.dec.rs2] : this.dec.i_imm();

		// Execute
		if (this.dec.is_lui) {
			this.set_rd(this.dec.u_imm());
			return;
		}

		if (this.dec.is_auipc) {
			this.set_rd(this.pc + this.dec.u_imm() - 4);
			return;
		}

		if (this.dec.is_jal) {
			this.set_rd(this.pc);
			this.set_pc(this.pc + this.dec.j_imm() - 4);
			return;
		}

		if (this.dec.is_jalr) {
			this.set_rd(this.pc);
			this.set_pc(this.r1 + this.dec.i_imm());
			return;
		}

		if (this.dec.is_branch) {
			this.do_branch();
			return;
		}

		if (this.dec.is_load) {
			this.do_load();
			return;
		}

		if (this.dec.is_store) {
			this.do_store();
			return;
		}

		if (this.dec.is_alu) {
			this.set_rd(this.do_alu(this.dec.func7, this.dec.func3, this.dec.opcode, this.r1, this.r2));
			return;
		}
	};

	//////////////////////////////////////////////////////////////////////////////
	// PRIVATE
	// do not use directly
	//////////////////////////////////////////////////////////////////////////////

	// Registers
	this.r = new Uint32Array(32);
	this.r1 = 0;
	this.r2 = 0;
	this.pc = 0;
	this.instr = 0;
	this.r[0] = 0;
	this.ready = false;

	// System controller
	this.sys = sys;

	// Instruction decoder
	this.dec = new RiscvDecoder();

	this.set_pc = function(value) {
		this.pc = uint(value);
	};

	this.get_reg = function(n) {
		return this.r[n & 0x1F] & 0xFFFFFFFF;
	};

	this.set_rd = function(v) {
		n = this.dec.rd & 0x1F;
		if (n != 0)
			this.r[n] = uint(v);
	};

	this.fetch = function() {
		this.instr = this.sys.read32(this.pc);
		this.set_pc(this.pc + 4);
	};
};
