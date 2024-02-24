module riscv
(
	input wire         clk,
	input wire         rst,

	output reg  [31:0] addr,
	output reg  [31:0] dout,
	input  wire [31:0] din,
	output reg         wr,
	output reg  [ 3:0] lane,
	output reg         valid,
	input  wire        ready,

	input  wire        timer_irq
);

//////////////////////////////////////////////////////////////////////////////
// Registers / latches / flags
//////////////////////////////////////////////////////////////////////////////

reg [31:0] pc;            // Program counter
reg [31:0] this_instr_pc; // Address of current instruction
reg [31:0] instr;         // Buffer for instruction
reg [31:0] r [0:31];      // Register file
reg [31:0] r1;            // Register rs1 buffer
reg [31:0] r2;            // Register rs2 buffer
reg [31:0] rdata;         // Data to write to register rd
reg        write_rd;      // Set if need to save result to rd
reg        is_alu_instr;  // Set if need to save ALU result to rd
reg [31:0] branch_addr;
reg [31:0] trap_code;     // Interrupt or fault code
reg        trap;          // Set if we have interrupt request or fault
reg        wfi;           // Waiting for interrupt

//////////////////////////////////////////////////////////////////////////////
// CSRs
//////////////////////////////////////////////////////////////////////////////

reg  [31:0] mstatus;
reg  [31:0] mie;
reg  [31:0] mtvec;
reg  [31:0] mscratch;
reg  [31:0] mcause;
reg  [31:0] mtval;
reg  [31:0] mip;
reg  [31:0] mepc;

/*
30: 1 - 32 bit, 2 - 64 bit, 3 - 128 bit
20: user mode implemented
18: supervisor mode implemented
16: quad float
15: packed SIMD
13: user-level interrupts
12: int mul/div
8: rv base isa
7: hypervisor
5: float
4: rv32e
3: double
2: compressed
1: bit-manip
0: atomic
*/
wire [31:0] misa     = 32'b0100_0000_0001_0000_0001_0001_0000_0001;
wire [31:0] vendorid = 32'h12345678;
wire [31:0] archid   = 32'd0;
wire [31:0] impid    = 32'd0;
wire [31:0] hartid   = 32'd0;

reg  [31:0] csr_value;

always @(posedge clk)
begin
	mip[7] <= timer_irq;
end

//////////////////////////////////////////////////////////////////////////////
// Instruction decode
//////////////////////////////////////////////////////////////////////////////

wire [6:0] func7  = instr[31:25];
wire [2:0] func3  = instr[14:12];

wire [31:0] i_imm = {{20{instr[31]}}, instr[31:20]};
wire [31:0] s_imm = {{20{instr[31]}}, instr[31:25], instr[11:7]};
wire [31:0] b_imm = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] u_imm = {instr[31:12], 12'b0};
wire [31:0] j_imm = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};
wire [ 4:0] z_imm = instr[19:15];

wire [4:0] rd     = instr[11:7];
wire [4:0] rs1    = instr[19:15];
wire [4:0] rs2    = instr[24:20];
wire [4:0] opcode = instr[6:2];

wire [11:0] csr   = instr[31:20];

// ALU instruction decode
always @ (posedge clk)
begin
	case (func3)
		3'b000: alu_op <= opcode == 5'b01100 && func7[0] ? OP_MUL : opcode[3] && func7[5] ? OP_SUB : OP_ADD;
		3'b001: alu_op <= opcode == 5'b01100 && func7[0] ? OP_MUL : OP_SLL;
		3'b010: alu_op <= opcode == 5'b01100 && func7[0] ? OP_MUL : OP_SLT;
		3'b011: alu_op <= opcode == 5'b01100 && func7[0] ? OP_MUL : OP_SLTU;
		3'b100: alu_op <= opcode == 5'b01100 && func7[0] ? OP_DIV : OP_XOR;
		3'b101: alu_op <= opcode == 5'b01100 && func7[0] ? OP_DIV : func7[5] ? OP_SRA : OP_SRL;
		3'b110: alu_op <= opcode == 5'b01100 && func7[0] ? OP_REM : OP_OR;
		3'b111: alu_op <= opcode == 5'b01100 && func7[0] ? OP_REM : OP_AND;
	endcase
end

wire is_shift   = (opcode == 5'b00100 || ({func7[0], opcode} == 6'b001100)) && (func3 == 3'b001 || func3 == 3'b101);
wire is_div_rem = opcode == 5'b01100 && func7[0] && func3[2];
wire is_mul     = opcode == 5'b01100 && func7[0] && (!func3[2]);

wire is_lr     = opcode == 5'b01011 && func7[6:2] == 5'b00010;
wire is_sc     = opcode == 5'b01011 && func7[6:2] == 5'b00011;
wire is_amoadd = opcode == 5'b01011 && func7[6:2] == 5'b00000;
wire is_lr_amo = opcode == 5'b01011 && func7[6:2] != 5'b00011;

// Atomics, syscall, interrupt return
wire is_csr    = opcode == 5'b11100 && func3 != 3'd0;
wire is_ecall  = opcode == 5'b11100 && func3 == 3'd0 && csr[2:0] == 3'b000;
wire is_ebreak = opcode == 5'b11100 && func3 == 3'd0 && csr[2:0] == 3'b001;
wire is_mret   = opcode == 5'b11100 && func3 == 3'd0 && csr[2:0] == 3'b010;
wire is_wfi    = opcode == 5'b11100 && func3 == 3'd0 && csr[2:0] == 3'b101;
wire is_fence  = opcode == 5'b00011;

//////////////////////////////////////////////////////////////////////////////
// ALU
//////////////////////////////////////////////////////////////////////////////

localparam
	OP_ADD       = 4'd0,
	OP_SUB       = 4'd1,
	OP_AND       = 4'd2,
	OP_OR        = 4'd3,
	OP_XOR       = 4'd4,
	OP_SLT       = 4'd5,
	OP_SLTU      = 4'd6,
	OP_SLL       = 4'd8,
	OP_SRL       = 4'd9,
	OP_SRA       = 4'd10,
	OP_MUL       = 4'd11,
	OP_DIV       = 4'd12,
	OP_REM       = 4'd13;

reg  [ 3:0] alu_op;
wire [31:0] alu_a = r1;
wire [31:0] alu_b = r2;
reg  [31:0] alu_res;

// ALU
always @ (posedge clk)
begin
	is_alu_instr <= opcode == 5'b00100 || opcode == 5'b01100;

	case (alu_op)
		OP_ADD:   alu_res <= alu_a +  alu_b;
		OP_SUB:   alu_res <= alu_a -  alu_b;
		OP_AND:   alu_res <= alu_a &  alu_b;
		OP_OR:    alu_res <= alu_a |  alu_b;
		OP_XOR:   alu_res <= alu_a ^  alu_b;
		OP_SLT:   alu_res <= $signed(alu_a) < $signed(alu_b);
		OP_SLTU:  alu_res <= alu_a <  alu_b;
		OP_SLL:   alu_res <= result_sll;
		OP_SRL:   alu_res <= result_srl;
		OP_SRA:   alu_res <= result_sra;
		OP_MUL:   alu_res <= result_mul;
		OP_DIV:   alu_res <= div_q;
		OP_REM:   alu_res <= div_r;
	endcase
end

//////////////////////////////////////////////////////////////////////////////
// Bit shifter
//////////////////////////////////////////////////////////////////////////////

reg [31:0] result_sll;
reg [31:0] result_srl;
reg [31:0] result_sra;
reg [4:0] shift;
always @(posedge clk)
begin
	if          (shift[4]) // by 16
	begin
		shift[4]   <= 1'b0;
		result_sll <= {result_sll[15:0], 16'd0};
		result_srl <= {16'd0, result_srl[31:16]};
		result_sra <= {{16{result_sra[31]}}, result_sra[31:16]};
	end else if (shift[3]) // by 8
	begin
		shift[3]   <= 1'b0;
		result_sll <= {result_sll[23:0], 8'd0};
		result_srl <= {8'd0, result_srl[31:8]};
		result_sra <= {{8{result_sra[31]}}, result_sra[31:8]};
	end else if (shift[2]) // by 4
	begin
		shift[2]   <= 1'b0;
		result_sll <= {result_sll[27:0], 4'd0};
		result_srl <= {4'd0, result_srl[31:4]};
		result_sra <= {{4{result_sra[31]}}, result_sra[31:4]};
	end else if (shift[1]) // by 2
	begin
		shift[1]   <= 1'b0;
		result_sll <= {result_sll[29:0], 2'd0};
		result_srl <= {2'd0, result_srl[31:2]};
		result_sra <= {{2{result_sra[31]}}, result_sra[31:2]};
	end else if (shift[0]) // by 1
	begin
		shift[0]   <= 1'b0;
		result_sll <= {result_sll[30:0], 1'd0};
		result_srl <= {1'd0, result_srl[31:1]};
		result_sra <= {{1{result_sra[31]}}, result_sra[31:1]};
	end else if (state == S_SHIFT) // Start signal
	begin
		shift      <= r2[4:0];
		result_sll <= r1;
		result_srl <= r1;
		result_sra <= r1;
	end
end

//////////////////////////////////////////////////////////////////////////////
// MUL
//////////////////////////////////////////////////////////////////////////////

reg  [31:0] result_mul;
wire [63:0] mul_q;
reg         mul_valid;
wire        mul_ready;
mul32 i_mul32
(
	.clk(clk),
	.a  (r1),
	.b  (r2),
	.res (mul_q),
	.valid      (mul_valid),
	.ready      (mul_ready)
);

reg [63:0] imul_res;
always @(posedge clk)
	imul_res <= r1 * r2;

//////////////////////////////////////////////////////////////////////////////
// DIV / REM
//////////////////////////////////////////////////////////////////////////////

reg         div_valid;
wire        div_ready;
wire [31:0] div_q;
wire [31:0] div_r;
wire        signed_div = ~func3[0];
div32 i_div(
	.clk        (clk),
	.denom      (r1),
	.num        (r2),
	.q          (div_q),
	.r          (div_r),
	.valid      (div_valid),
	.ready      (div_ready),
	.signed_div (signed_div)
);

//////////////////////////////////////////////////////////////////////////////
// State machine
//////////////////////////////////////////////////////////////////////////////

localparam
	S_IDLE        = 4'd0,
	S_FETCH       = 4'd1,
	S_DECODE      = 4'd2,
	S_READ        = 4'd3,
	S_WRITE       = 4'd4,
	S_EXECUTE     = 4'd5,
	S_SHIFT       = 4'd6,
	S_SHIFT_WAIT  = 4'd7,
	S_DIV         = 4'd8,
	S_MUL         = 4'd9,
	S_MUL2        = 4'd10,
	S_ATOMIC_READ = 4'd11,
	S_ATOMIC_WRITE= 4'd12,
	S_CSR_READ    = 4'd13,
	S_CSR_MODIFY  = 4'd14,
	S_CSR_WRITE   = 4'd15;

reg  [3:0] state;

wire [31:0] store_addr = r1 + s_imm;
reg         misaligned;
reg [ 3:0]  next_lane;
reg [31:0]  next_addr;

reg mmode;
reg phase;

always @ (posedge clk or posedge rst)
begin
	if (rst)
	begin
		// Reset vector
		pc       <= 32'h0;

		// Force reset r0
		rdata    <= 32'd0;
		write_rd <= 1;
		instr    <= 32'd0;

		// Disable interrupts
		mstatus  <= 32'h3;
		trap     <= 0;
		mmode    <= 1;

		// Start execution
		state    <= S_IDLE;
	end
	else
	begin
		case (state)
			S_IDLE:
			begin
				phase <= 0;

				// Save previous result to r[rd]
				if (write_rd)
					r[rd] <= is_alu_instr ? alu_res : rdata;
				write_rd <= 0;

				// Interrupt or fault reaction
				if (trap)
				begin
					wfi <= 0;

					mmode   <= 1;
					mcause  <= trap_code;
					mtval   <= trap_code[31] ? 32'd0 : pc;

					pc      <= mtvec;
					mstatus <= {19'b0, mmode, mmode, 3'b000, mstatus[3], 3'b000, 1'b0, 3'b000};
					mepc    <= pc;
					trap    <= 0;
				end
				else
				begin
					if (mstatus[3])
					begin
						if (mip[7] && mie[7])
						begin
							// Timer interrupt
							wfi <= 0;
							trap_code <= 32'h80000007;
							trap <= 1;
						end
					end
				end

				if (~wfi)
					state <= S_FETCH;
			end
			S_FETCH:
			begin
				this_instr_pc <= pc;
				addr          <= pc;
				lane          <= 4'b1111;
				wr            <= 0;
				valid         <= 1;

				if (ready)
				begin
					pc    <= pc + 32'd4;
					instr <= din;
					valid <= 0;
					state <= S_DECODE;
				end
			end
			S_DECODE:
			begin
				// Load register values
				r1 <= r[rs1];
				r2 <= opcode[3] ? r[rs2] : i_imm;
				
				branch_addr <= this_instr_pc + b_imm;

				// Select read / write / execute
				if      (opcode == 5'b00000)
					state <= S_READ;
				else if (opcode == 5'b01000)
					state <= S_WRITE;
				else if (is_div_rem)
					state <= S_DIV;
				else if (is_mul)
					state <= S_MUL;
				else if (is_shift)
					state <= S_SHIFT;
				else if (is_lr_amo)
					state <= S_ATOMIC_READ;
				else if (is_sc)
					state <= S_ATOMIC_WRITE;
				else if (is_csr)
					state <= S_CSR_READ;
				else
					state <= S_EXECUTE;
			end
			S_EXECUTE:
			begin
				// Jumps and branches
				case (opcode)
					5'b11011: pc <= this_instr_pc + j_imm; // JAL
					5'b11001: pc <= r1 + i_imm;            // JALR
					5'b11000:
					begin
						case (func3)
							3'b000: if (r1 == r2)                   pc <= branch_addr; // BEQ
							3'b001: if (r1 != r2)                   pc <= branch_addr; // BNE
							3'b100: if ($signed(r1) < $signed(r2))  pc <= branch_addr; // BLT
							3'b101: if ($signed(r1) >= $signed(r2)) pc <= branch_addr; // BGE
							3'b110: if (r1 < r2)                    pc <= branch_addr; // BLTU
							3'b111: if (r1 >= r2)                   pc <= branch_addr; // BGEU
						endcase
					end
				endcase

				if (is_ecall)
				begin
					trap      <= 1;
					trap_code <= 32'h8;
				end

				if (is_mret)
				begin
					pc      <= mepc;
					mstatus <= {19'b0, mmode, mmode, 3'b000, 1'b1, 3'b000, mstatus[7], 3'b000};
					mmode   <= mstatus[11];
				end

				if (is_wfi)
				begin
					wfi        <= 0;
					mstatus[3] <= 1;
				end

				case (opcode)
					5'b01101: rdata <= u_imm;                 // LUI
					5'b00101: rdata <= this_instr_pc + u_imm; // AUIPC
					5'b11011: rdata <= pc;                    // JAL
					5'b11001: rdata <= pc;                    // JALR
				endcase

				write_rd <= rd != 5'b00000 &&
					(opcode == 5'b00100 || opcode == 5'b01100 || opcode == 5'b01101 ||
					opcode == 5'b00101 || opcode == 5'b11011 || opcode == 5'b11001);

				state <= S_IDLE;
			end
			S_READ:
			begin
				addr <= r1 + i_imm;
				wr <= 0;
				valid <= 1;
				if (ready)
				begin
					case (func3)
						3'b000:
							rdata <=      // LB
							addr[1:0] == 2'b00 ? {{24{din[7]}}, din[7:0]} :
							addr[1:0] == 2'b01 ? {{24{din[15]}}, din[15:8]} :
							addr[1:0] == 2'b10 ? {{24{din[23]}}, din[23:16]} :
												 {{24{din[31]}}, din[31:24]};
						3'b001:
						begin
							// LH
							case (addr[1:0])
								2'b00: rdata <= {{16{din[15]}}, din[15: 0]};
								2'b01: rdata <= {{16{din[23]}}, din[23: 8]};
								2'b10: rdata <= {{16{din[31]}}, din[31:16]};
								2'b11:
								begin
									if (~phase)
									begin
										rdata[7:0] <= din[31:24];
										r1 <= r1 + 3'd4;
										phase <= 1;
									end
									else
										rdata[31:8] <= {{16{din[7]}}, din[7:0]};
								end
							endcase
						end
						3'b010:
						begin
							// LW
							case (addr[1:0])
								2'b00: rdata <= din;
								2'b01:
								begin
									if (~phase)
									begin
										rdata[23:0] <= din[31:8];
										r1 <= r1 + 3'd4; // easiest way
										phase <= 1;
									end
									else
										rdata[31:24] <= din[7:0];
								end
								2'b10:
								begin
									if (~phase)
									begin
										rdata[15:0] <= din[31:16];
										r1 <= r1 + 3'd4;
										phase <= 1;
									end
									else
										rdata[31:16] <= din[15:0];
								end
								2'b11:
								begin
									if (~phase)
									begin
										rdata[7:0] <= din[31:24];
										r1 <= r1 + 3'd4;
										phase <= 1;
									end
									else
										rdata[31:8] <= din[23:0];
								end
							endcase
						end

						3'b100: rdata <=      // LBU
							addr[1:0] == 2'b00 ? {24'd0, din[7:0]} :
							addr[1:0] == 2'b01 ? {24'd0, din[15:8]} :
							addr[1:0] == 2'b10 ? {24'd0, din[23:16]} :
												 {24'd0, din[31:24]};
						3'b101:
							// LHU
							case (addr[1:0])
								2'b00: rdata <= {16'd0, din[15: 0]};
								2'b01: rdata <= {16'd0, din[23: 8]};
								2'b10: rdata <= {16'd0, din[31:16]};
								2'b11:
								begin
									if (~phase)
									begin
										rdata[7:0] <= din[31:24];
										r1 <= r1 + 3'd4;
										phase <= 1;
									end
									else
										rdata[31:8] <= {16'd0, din[7:0]};
								end
							endcase
					endcase
					write_rd <= rd != 5'b00000;
					valid <= 0;
					if (~phase &&
						((func3 == 3'b001 && addr[1:0] == 2'b11) ||
						(func3 == 3'b010 && addr[1:0] != 2'b00) ||
						(func3 == 3'b101 && addr[1:0] == 2'b11)))
						state <= S_READ;
					else
						state <= S_IDLE;
				end
			end
			S_WRITE:
			begin
				if (~phase)
				begin
					misaligned <= 0;
					if (misaligned)
					begin
						lane <= next_lane;
						addr <= {next_addr[31:2], 2'b00};
					end
					else
					begin
						addr <= {store_addr[31:2], 2'b00};
						// Set data and select byte lanes
						case (func3)
							3'b000:
							begin
								lane[0] <= store_addr[1:0] == 2'b00;
								lane[1] <= store_addr[1:0] == 2'b01;
								lane[2] <= store_addr[1:0] == 2'b10;
								lane[3] <= store_addr[1:0] == 2'b11;
								dout <= {4{r2[7:0]}};
							end
							3'b001:
							begin
								// 16 bit write
								case (store_addr[1:0])
									2'b00:
									begin
										// Aligned
										lane <= 4'b0011;
										dout <= {2{r2[15:0]}};
									end
									2'b01:
									begin
										lane <= 4'b0110;
										dout <= {r2[7:0], r2[15:0], r2[15:8]};
									end
									2'b10:
									begin
										// Aligned
										lane <= 4'b1100;
										dout <= {2{r2[15:0]}};
									end
									2'b11:
									begin
										misaligned <= 1;
										lane <= 4'b1000;
										dout <= {r2[7:0], r2[15:0], r2[15:8]};
										next_lane <= 4'b0001;
										next_addr <= store_addr + 3'd4;
									end
								endcase
							end
							3'b010:
							begin
								// 32 bit write
								case (store_addr[1:0])
									2'b00:
									begin
										// Aligned
										lane <= 4'b1111;
										dout <= r2;
									end
									2'b01:
									begin
										misaligned <= 1;
										lane <= 4'b1110;
										dout <= {r2[23:0], r2[31:24]};
										next_lane <= 4'b0001;
										next_addr <= store_addr + 3'd4;
									end
									2'b10:
									begin
										misaligned <= 1;
										lane <= 4'b1100;
										dout <= {r2[15:0], r2[31:16]};
										next_lane <= 4'b0011;
										next_addr <= store_addr + 3'd4;
									end
									2'b11:
									begin
										misaligned <= 1;
										lane <= 4'b1000;
										dout <= {r2[7:0], r2[31:8]};
										next_lane <= 4'b0111;
										next_addr <= store_addr + 3'd4;
									end
								endcase
							end
						endcase
					end
				end

				phase <= 1;
				wr <= 1;
				valid <= 1;

				if (ready)
				begin
					valid <= 0;
					if (~misaligned)
						state <= S_IDLE;
					else
						phase <= 0;
				end
			end
			S_SHIFT:
				state <= S_SHIFT_WAIT;
			S_SHIFT_WAIT:
			begin
				if (shift == 5'b00000)
				begin
					write_rd <= rd != 5'b00000;
					state <= S_IDLE;
				end
			end
			S_DIV:
			begin
				div_valid <= 1;
				if (div_ready)
				begin
					div_valid <= 0;
					write_rd <= rd != 5'b00000;
					state <= S_EXECUTE;
				end
			end
			S_MUL:
			begin
				mul_valid <= 1;
				if (mul_ready)
				begin
					result_mul <= func3[1:0] != 2'b00 ? imul_res[63:32]/*mul_q[63:32]*/ : imul_res[31:0]; //mul_q[31:0];
					mul_valid <= 0;
					state <= S_MUL2;
				end
			end
			S_MUL2:
				state <= S_EXECUTE;
			S_ATOMIC_READ:
			begin
				addr <= r1;
				wr <= 0;
				valid <= 1;
				if (ready)
				begin
					valid <= 0;
					rdata <= din;
					write_rd <= rd != 5'b00000;
					if (is_lr)
						state <= S_IDLE;
					else
						state <= S_ATOMIC_WRITE;
				end
			end
			S_ATOMIC_WRITE:
			begin
				addr <= r1;
				case (func7[6:2])
					5'b00000: dout <= r2 + rdata; // AMOADD
					5'b00100: dout <= r2 ^ rdata; // AMOXOR
					5'b01100: dout <= r2 & rdata; // AMOAND
					5'b01000: dout <= r2 | rdata; // AMOOR
					default:  dout <= r2;
				endcase
				wr <= 1;
				valid <= 1;
				if (ready)
				begin
					valid <= 0;
					rdata <= is_sc ? 32'd0 : rdata;
					write_rd <= rd != 5'b00000;
					state <= S_IDLE;
				end
			end
			S_CSR_READ:
			begin
				case (csr)
					12'h300: csr_value <= mstatus;
					12'h301: csr_value <= misa;
					12'h304: csr_value <= mie;
					12'h305: csr_value <= mtvec;
					12'h340: csr_value <= mscratch;
					12'h341: csr_value <= mepc;
					12'h342: csr_value <= mcause;
					12'h343: csr_value <= mtval;
					12'h344: csr_value <= mip;
					12'hF11: csr_value <= vendorid;
					12'hF12: csr_value <= archid;
					12'hF13: csr_value <= impid;
					12'hF14: csr_value <= hartid;
					default: csr_value <= 32'd0;
				endcase
				state <= S_CSR_MODIFY;
			end
			S_CSR_MODIFY:
			begin
				rdata <= csr_value;

				case (func3)
					3'b001: csr_value <= r1;                 // CSRRW
					3'b010: csr_value <= csr_value |  r1;    // CSRRS
					3'b011: csr_value <= csr_value & ~r1;    // CSRRC
					3'b101: csr_value <= z_imm;              // CSRRWI
					3'b110: csr_value <= csr_value |  z_imm; // CSRRSI
					3'b111: csr_value <= csr_value & ~z_imm; // CSRRCI
				endcase

				state <= S_CSR_WRITE;
			end
			S_CSR_WRITE:
			begin
				case (csr)
					12'h300: mstatus  <= csr_value;
					12'h304: mie      <= csr_value;
					12'h305: mtvec    <= csr_value;
					12'h340: mscratch <= csr_value;
					12'h341: mepc     <= csr_value;
					12'h342: mcause   <= csr_value;
					12'h343: mtval    <= csr_value;
				endcase

				write_rd <= rd != 5'b00000;
				state <= S_IDLE;
			end
		endcase
	end
end

endmodule
