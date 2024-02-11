Riscv = function(sys) {
	//////////////////////////////////////////////////////////////////////////////
	// PUBLIC
	// load, reset, step, tick
	//////////////////////////////////////////////////////////////////////////////

	// Firmware loader
	this.load = async function(url, addr) {
		this.ready = false;
		const response = await fetch(url);
		const buffer = await response.arrayBuffer();
		const program = new Uint32Array(buffer);

		for (let i = 0; i < program.length; i++)
			this.sys.write32(addr | (i * 4), program[i]);

		this.ready = true;
	}; 

	// CPU reset
	this.reset = function() {
		this.pc = 0;
		for (let i = 0; i < 32; i++)
			this.r[i] = 0;
		this.pc = this.start;
		this.r[10] = 0;
		this.r[11] = 0x80FF0000;

		this.mmode    = 3;
		this.trap     = false;
		this.wfi      = false;

		this.mstatus  = 0;
		this.mie      = 0;
		this.mtvec    = 0;
		this.mscratch = 0;
		this.mcause   = 0;
		this.mtval    = 0;
		this.mip      = 0;
		this.mepc     = 0;
	};

	this.dump = function() {
		console.log("pc = " + this.pc.toString(16));
		let s = "";
		for (let i = 0; i < 32; i++) {
			if (i % 8 == 0)
				s = "";
			s += this.r[i].toString(16) + '  ';
			if (i % 8 == 7)
				console.log(s);
		}
	};

	// Execute 1 next instruction
	this.step = function() {
		if (!this.ready)
			return;

		if (this.wfi)
			return;

		// Check interrupts
		if (this.trap) {
			this.trap = false;
			this.mcause = this.trap_code;
			this.mtval = 0;
			this.mepc = this.trap_code & 0x80000000 ? this.pc : uint(this.pc - 4);
			this.mstatus = uint(((this.mstatus & (1 << 3)) << 4) | (this.mmode << 11));
			this.mmode = 3;
			this.pc = this.mtvec;
		}

		// Fetch next instruction
		this.fetch();

		// Decode instruction
		this.dec.decode(this.instr);

		// Load register values
		this.r1 = this.r[this.dec.rs1];
		this.r2 = this.dec.is_r2 ? this.r[this.dec.rs2] : this.dec.i_imm();

		// Execute
		this.jmpTable[this.dec.opcode](this);
	};

	this.tick = function(us) {
		const timer = sys.getDevice(0x11);

		if ("function" != typeof timer.tick)
			return;

		timer.tick(us);

		if (timer.irq)
			this.mip |= 1 << 7;
		else
			this.mip &= ~(1 << 7);

		this.mip = uint(this.mip);

		if ((this.mip & this.mie & (1 << 7)) && (this.mstatus & (1 << 3))) {
			this.trap = true;
			this.trap_code = 0x80000007;
		}

		this.wfi = false;
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
	this.start = 0;
	this.instr = 0;
	this.r[0] = 0;
	this.ready = false;
	this.mmode = 3;
	this.trap = false;
	this.trap_code = 0;

	// CSRs
	this.mstatus  = 0;
	this.mie      = 0;
	this.mtvec    = 0;
	this.mscratch = 0;
	this.mcause   = 0;
	this.mtval    = 0;
	this.mip      = 0;
	this.mepc     = 0;
	this.misa     = 0x40101101;
	this.vendorid = 0x12345678;
	this.archid   = 0;
	this.impid    = 0;
	this.hartid   = 0;

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

	this.do_lui = function(cpu) {
		cpu.set_rd(cpu.dec.u_imm());
	}

	this.do_auipc = function(cpu) {
		cpu.set_rd(cpu.pc + cpu.dec.u_imm() - 4);
	}

	this.do_jal = function(cpu) {
		cpu.set_rd(cpu.pc);
		cpu.set_pc(cpu.pc + cpu.dec.j_imm() - 4);
	}

	this.do_jalr = function(cpu) {
		cpu.set_rd(cpu.pc);
		cpu.set_pc(cpu.r1 + cpu.dec.i_imm());
	}

	this.do_alu = function(cpu) {
		cpu.set_rd(cpu.alu(cpu.dec.func7, cpu.dec.func3, cpu.dec.opcode, cpu.r1, cpu.r2));
	}

	this.do_undefined = function(cpu) {
		console.log("undefined: " + cpu.instr.toString(16));
	}

	this.jmpTable = [];
	this.jmpTable.length = 128;
	for (let i = 0; i < 128; i++) {
		this.jmpTable[i] = this.do_undefined;
	}

	this.jmpTable[0x37] = this.do_lui;
	this.jmpTable[0x17] = this.do_auipc;
	this.jmpTable[0x6F] = this.do_jal;
	this.jmpTable[0x67] = this.do_jalr;
	this.jmpTable[0x63] = this.do_branch;
	this.jmpTable[0x03] = this.do_load;
	this.jmpTable[0x23] = this.do_store;
	this.jmpTable[0x13] = this.do_alu;
	this.jmpTable[0x33] = this.do_alu;
	this.jmpTable[0x73] = this.do_csr;
	this.jmpTable[0x2F] = this.do_atomic;
	this.jmpTable[0x0F] = this.do_fence;
};
