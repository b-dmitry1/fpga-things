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
    input  wire        ready
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

wire [4:0] rd     = instr[11:7];
wire [4:0] rs1    = instr[19:15];
wire [4:0] rs2    = instr[24:20];
wire [4:0] shamt  = instr[24:20];
wire [4:0] opcode = instr[6:2];

// ALU instruction decode
always @ (posedge clk)
begin
    case (func3)
        3'b000: alu_op <= opcode == 5'b01100 && func7[0] ? OP_MUL : opcode[3] && func7[5] ? OP_SUB : OP_ADD;
        3'b001: alu_op <= OP_SLL;
        3'b010: alu_op <= OP_SLT;
        3'b011: alu_op <= OP_SLTU;
        3'b100: alu_op <= opcode == 5'b01100 && func7[0] ? OP_DIV : OP_XOR;
        3'b101: alu_op <= opcode == 5'b01100 && func7[0] ? OP_DIV : func7[5] ? OP_SRA : OP_SRL;
        3'b110: alu_op <= opcode == 5'b01100 && func7[0] ? OP_REM : OP_OR;
        3'b111: alu_op <= opcode == 5'b01100 && func7[0] ? OP_REM : OP_AND;
    endcase
end

wire is_shift   = (opcode == 5'b00100 || opcode == 5'b01100) && (func3 == 3'b001 || func3 == 3'b101);
wire is_div_rem = opcode == 5'b01100 && func7[0] && func3[2];

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
        OP_MUL:   alu_res <= alu_a * alu_b;
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
// Init (simulator only!)
//////////////////////////////////////////////////////////////////////////////

integer i;
initial
begin
    for (i = 0; i < 32; i = i + 1)
        r[i] = 0;
    pc = 32'd0;
    write_rd = 0;
    div_valid <= 0;
    state = S_IDLE;
end

//////////////////////////////////////////////////////////////////////////////
// State machine
//////////////////////////////////////////////////////////////////////////////

localparam
    S_IDLE        = 4'd0,
    S_FETCH       = 4'd1,
    S_DECODE      = 4'd2,
    S_READ        = 4'd3,
    S_WRITE       = 4'd4,
    S_WRITE_WAIT  = 4'd5,
    S_EXECUTE     = 4'd6,
    S_SHIFT       = 4'd7,
    S_SHIFT_WAIT  = 4'd8,
    S_DIV         = 4'd9,
    S_DIV_WAIT    = 4'd10;

reg  [3:0] state;

always @ (posedge clk or posedge rst)
begin
    if (rst)
    begin
        // Reset vector
        pc <= 32'h0;
        // Force reset r0
        rdata <= 32'd0;
        write_rd <= 1;
        // Start execution
        state <= S_IDLE;
    end
    else
    begin
        case (state)
            S_IDLE:
            begin
                // Save previous result to r[rd]
                if (write_rd)
                    r[rd] <= is_alu_instr ? alu_res : rdata;
                write_rd <= 0;

                // Advance PC and begin instruction fetch
                this_instr_pc <= pc;
                pc <= pc + 32'd4;
                addr <= pc;
                lane <= 4'b1111;
                wr <= 0;
                valid <= 1;
                state <= S_FETCH;
            end
            S_FETCH:
            begin
                if (ready)
                begin
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

                // Select read / write / execute
                if      (opcode == 5'b00000)
                begin
                    // Set read address and begin read
                    addr <= r[rs1] + i_imm;
                    wr <= 0;
                    valid <= 1;
                    state <= S_READ;
                end
                else if (opcode == 5'b01000)
                begin
                    // Set write address
                    addr <= r[rs1] + s_imm;
                    wr <= 1;
                    state <= S_WRITE;
                end
                else if (is_div_rem)
                    state <= S_DIV;
                else if (is_shift)
                    state <= S_SHIFT;
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
                        case (instr[14:12])
                            3'b000: if (r1 == r2)                   pc <= this_instr_pc + b_imm; // BEQ
                            3'b001: if (r1 != r2)                   pc <= this_instr_pc + b_imm; // BNE
                            3'b100: if ($signed(r1) < $signed(r2))  pc <= this_instr_pc + b_imm; // BLT
                            3'b101: if ($signed(r1) >= $signed(r2)) pc <= this_instr_pc + b_imm; // BGE
                            3'b110: if (r1 < r2)                    pc <= this_instr_pc + b_imm; // BLTU
                            3'b111: if (r1 >= r2)                   pc <= this_instr_pc + b_imm; // BGEU
                        endcase
                    end
                endcase

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
                if (ready)
                begin
                    case (func3)
                        3'b000: rdata <=      // LB
                            addr[1:0] == 2'b00 ? {{24{din[7]}}, din[7:0]} :
                            addr[1:0] == 2'b01 ? {{24{din[15]}}, din[15:8]} :
                            addr[1:0] == 2'b10 ? {{24{din[23]}}, din[23:16]} :
                                                 {{24{din[31]}}, din[31:24]};
                        3'b001: rdata <=      // LH
                            addr[1] == 1'b0 ? {{16{din[15]}}, din[15:0]} :
                                              {{16{din[31]}}, din[31:16]};
                        3'b010: rdata <= din; // LW
                        3'b100: rdata <=      // LBU
                            addr[1:0] == 2'b00 ? {24'd0, din[7:0]} :
                            addr[1:0] == 2'b01 ? {24'd0, din[15:8]} :
                            addr[1:0] == 2'b10 ? {24'd0, din[23:16]} :
                                                 {24'd0, din[31:24]};
                        3'b101: rdata <=      // LHU
                            addr[1] == 1'b0 ? {16'd0, din[15:0]} :
                                              {16'd0, din[31:16]};
                    endcase
                    valid <= 0;
                    write_rd <= rd != 5'b00000;
                    state <= S_IDLE;
                end
            end
            S_WRITE:
            begin
                // Set data and select byte lanes
                case (func3)
                    3'b000:
                    begin
                        lane[0] <= addr[1:0] == 2'b00;
                        lane[1] <= addr[1:0] == 2'b01;
                        lane[2] <= addr[1:0] == 2'b10;
                        lane[3] <= addr[1:0] == 2'b11;
                        dout <= {4{r2[7:0]}};
                    end
                    3'b001:
                    begin
                        lane[0] <= addr[1] == 1'b0;
                        lane[1] <= addr[1] == 1'b0;
                        lane[2] <= addr[1] == 1'b1;
                        lane[3] <= addr[1] == 1'b1;
                        dout <= {2{r2[15:0]}};
                    end
                    3'b010:
                    begin
                        lane <= 4'b1111;
                        dout <= r2;
                    end
                endcase
                // Begin write
                valid <= 1;
                state <= S_WRITE_WAIT;
            end
            S_WRITE_WAIT:
            begin
                if (ready)
                begin
                    valid <= 0;
                    state <= S_IDLE;
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
                state <= S_DIV_WAIT;
            end
            S_DIV_WAIT:
            begin
                if (div_ready)
                begin
                    div_valid <= 0;
                    write_rd <= rd != 5'b00000;
                    state <= S_IDLE;
                end
            end
        endcase
    end
end

endmodule
