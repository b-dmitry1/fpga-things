module sdram(
	input  wire        clk,
	input  wire        clk1,
	input  wire        reset_n,

	input  wire [24:2] cpu_addr,
	input  wire [31:0] cpu_din,
	output reg  [31:0] cpu_dout,
	input  wire [ 3:0] cpu_lane,
	input  wire        cpu_wr,
	input  wire        cpu_valid,
	output reg         cpu_ready,
	
	input  wire [24:3] video_addr,
	output reg  [63:0] video_dout,

	output reg  [12:0] a,
	output reg  [ 1:0] ba,
	output reg  [ 1:0] dqm,
	inout  reg  [15:0] d,
	output wire        ras_n,
	output wire        cas_n,
	output wire        we_n,
	output wire        cs_n,
	output wire        sclk,
	output reg         scke
);

// Config
parameter
	cas_bits		= 8,
	ras_bits		= 12;

//////////////////////////////////////////////////////////////////////////////
// Startup delay
//////////////////////////////////////////////////////////////////////////////

reg [19:0] start;
reg start1, start2, start3;
always @(posedge clk or negedge reset_n)
begin
	if (~reset_n)
		start <= 20'd0;
	else
		start <= start[19] ? start : start + 20'd1;
end
always @(posedge clk)
begin
	start3 = start[19];
	start2 = |start[19:18];
	start1 = |start[19:17];
end

//////////////////////////////////////////////////////////////////////////////
// SDRAM commands
//////////////////////////////////////////////////////////////////////////////

localparam    //   SRCW
	NOP       = 4'b0000,
	PRECHARGE = 4'b1101,
	REFRESH   = 4'b1110,
	LOADMODE  = 4'b1111,
	ACTIVE    = 4'b1100,
	READ      = 4'b1010,
	WRITE     = 4'b1011;

reg [3:0]         cmd;
assign cs_n    = ~cmd[3];
assign ras_n   = ~cmd[2];
assign cas_n   = ~cmd[1];
assign we_n    = ~cmd[0];

//////////////////////////////////////////////////////////////////////////////
// Current row register
//////////////////////////////////////////////////////////////////////////////

reg [13:0] row;
reg row_active;

reg [24:3] cur_video_addr;
reg video_request;

//////////////////////////////////////////////////////////////////////////////
// Clock output and refresh
//////////////////////////////////////////////////////////////////////////////

assign sclk = clk1;
reg [9:0] refresh;
always @(posedge clk)
begin
	scke <= start1;
	refresh <= state == S_REFRESH ? 10'd0 : refresh + 10'd1;
end

//////////////////////////////////////////////////////////////////////////////
// FSM
//////////////////////////////////////////////////////////////////////////////

localparam
	S_START        = 0,
	S_IDLE         = 1,
	S_PRECHARGE    = 2,
	S_DELAY        = 3,
	S_READ_CPU     = 4,
	S_READ_CPU_1   = 5,
	S_READ_CPU_2   = 6,
	S_READ_CPU_3   = 7,
	S_WRITE_CPU    = 8,
	S_WRITE_CPU_1  = 9,
	S_WRITE_CPU_2  = 10,
	S_REFRESH      = 11,
	S_REFRESH_1    = 12,
	S_REFRESH_2    = 13,
	S_REFRESH_3    = 14,
	S_REFRESH_DONE = 15,
	S_READ_VIDEO   = 16,
	S_READ_VIDEO_1 = 17,
	S_READ_VIDEO_2 = 18,
	S_READ_VIDEO_3 = 19,
	S_READ_VIDEO_4 = 20,
	S_READ_VIDEO_5 = 21;

reg [4:0] state;

always @(posedge clk or negedge reset_n)
begin
	if (~reset_n)
	begin
		state <= S_START;
	end
	else
	begin
		video_request <= video_addr != cur_video_addr;
		cpu_ready <= 0;
		case (state)
			S_START:
			begin
				cmd <= NOP;
				d <= 16'hZZZZ;
				dqm <= 2'b00;
				ba <= 2'b00;
				if (start2)
					state <= S_IDLE;
				row_active <= 1'b0;
			end
			S_IDLE:
			begin
				cmd <= NOP;
				d <= 16'hZZZZ;
				dqm <= 2'b00;
				if (~start3)
				begin
				end
				else if (refresh[8])
				begin
					cmd <= PRECHARGE;
					a <= 13'h400;
					ba <= 2'b00;
					row_active <= 1'b0;
					state <= S_REFRESH;
				end
				else if (video_request)
				begin
					if (row_active)
					begin
						if (row == video_addr[2 + ras_bits + cas_bits : cas_bits + 1])
						begin
							state <= S_READ_VIDEO;
						end
						else
						begin
							cmd <= PRECHARGE;
							a <= 13'h400;
							ba <= 2'b00;
							row_active <= 1'b0;
						end
					end
					else
					begin
						cmd <= ACTIVE;
						a <= video_addr[ras_bits + cas_bits : cas_bits + 1];
						ba <= video_addr[2 + ras_bits + cas_bits : 1 + ras_bits + cas_bits];
						row <= video_addr[2 + ras_bits + cas_bits : cas_bits + 1];
						row_active <= 1'b1;
						state <= S_READ_VIDEO;
					end
				end
				else if (!cpu_wr && cpu_valid)
				begin
					if (row_active)
					begin
						if (row == cpu_addr[2 + ras_bits + cas_bits : cas_bits + 1])
						begin
							state <= S_READ_CPU;
						end
						else
						begin
							cmd <= PRECHARGE;
							a <= 13'h400;
							ba <= 2'b00;
							row_active <= 1'b0;
						end
					end
					else
					begin
						cmd <= ACTIVE;
						a <= cpu_addr[ras_bits + cas_bits : cas_bits + 1];
						ba <= cpu_addr[2 + ras_bits + cas_bits : 1 + ras_bits + cas_bits]; // 
						row <= cpu_addr[2 + ras_bits + cas_bits : cas_bits + 1]; // 22:9
						row_active <= 1'b1;
						state <= S_READ_CPU;
					end
				end
				else if (cpu_wr && cpu_valid)
				begin
					if (row_active)
					begin
						if (row == cpu_addr[2 + ras_bits + cas_bits : cas_bits + 1])
						begin
							state <= S_WRITE_CPU;
						end
						else
						begin
							cmd <= PRECHARGE;
							a <= 13'h400;
							ba <= 2'b00;
							row_active <= 1'b0;
						end
					end
					else
					begin
						cmd <= ACTIVE;
						a <= cpu_addr[ras_bits + cas_bits : cas_bits + 1];
						ba <= cpu_addr[2 + ras_bits + cas_bits : 1 + ras_bits + cas_bits];
						row <= cpu_addr[2 + ras_bits + cas_bits : cas_bits + 1];
						row_active <= 1'b1;
						state <= S_WRITE_CPU;
					end
				end
			end
			S_REFRESH:
			begin
				cmd <= REFRESH;
				state <= S_REFRESH_1;
			end
			S_REFRESH_1:
			begin
				cmd <= NOP;
				state <= S_REFRESH_2;
			end
			S_REFRESH_2:
				state <= S_REFRESH_3;
			S_REFRESH_3:
				state <= S_REFRESH_DONE;
			S_REFRESH_DONE:
			begin
				cmd <= LOADMODE;
				a <= 13'h220;
				ba <= 2'b00;
				state <= S_DELAY;
			end
			S_DELAY:
			begin
				cmd <= NOP;
				state <= S_IDLE;
			end
			S_READ_CPU:
			begin
				cmd <= READ;
				a <= {4'b00, cpu_addr[cas_bits:2], 1'b0};
				state <= S_READ_CPU_1;
			end
			S_READ_CPU_1:
			begin
				a <= {4'b00, cpu_addr[cas_bits:2], 1'b1};
				state <= S_READ_CPU_2;
			end
			S_READ_CPU_2:
			begin
				cmd <= NOP;
				cpu_dout[15:0] <= d;
				state <= S_READ_CPU_3;
			end
			S_READ_CPU_3:
			begin
				cpu_dout[31:16] <= d;
				cpu_ready <= 1;
				state <= S_DELAY;
			end
			S_WRITE_CPU:
			begin
				cmd <= WRITE;
				a <= {4'b00, cpu_addr[cas_bits:2], 1'b0};
				d <= cpu_din[15:0];
				dqm <= ~cpu_lane[1:0];
				state <= S_WRITE_CPU_1;
			end
			S_WRITE_CPU_1:
			begin
				cmd <= WRITE;
				a <= {4'b00, cpu_addr[cas_bits:2], 1'b1};
				d <= cpu_din[31:16];
				dqm <= ~cpu_lane[3:2];
				cpu_ready <= 1;
				state <= S_WRITE_CPU_2;
			end
			S_WRITE_CPU_2:
			begin
				cmd <= NOP;
				d <= 16'hZZZZ;
				dqm <= 2'b00;
				state <= S_IDLE;
			end
			S_READ_VIDEO:
			begin
				cmd <= READ;
				cur_video_addr <= video_addr;
				a <= {4'b00, video_addr[cas_bits:3], 2'd0};
				state <= S_READ_VIDEO_1;
			end
			S_READ_VIDEO_1:
			begin
				a <= {4'b00, video_addr[cas_bits:3], 2'd1};
				state <= S_READ_VIDEO_2;
			end
			S_READ_VIDEO_2:
			begin
				a <= {4'b00, video_addr[cas_bits:3], 2'd2};
				video_dout[15:0] <= d;
				state <= S_READ_VIDEO_3;
			end
			S_READ_VIDEO_3:
			begin
				a <= {4'b00, video_addr[cas_bits:3], 2'd3};
				video_dout[31:16] <= d;
				state <= S_READ_VIDEO_4;
			end
			S_READ_VIDEO_4:
			begin
				cmd <= NOP;
				video_dout[47:32] <= d;
				state <= S_READ_VIDEO_5;
			end
			S_READ_VIDEO_5:
			begin
				video_dout[63:48] <= d;
				state <= S_IDLE;
			end
			default:
				state <= S_START;
		endcase
	end
end

endmodule
