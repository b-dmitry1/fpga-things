module top
(
	input  wire       clk,
	input  wire       rst_n,

	output reg  [3:0] led,
	output reg  [3:0] dig,
	output reg  [7:0] seg,
	
	output wire       txd,
	input  wire       rxd
);

wire [31:0] addr;
wire [31:0] din;
reg  [31:0] dout;
wire [ 3:0] lane;
wire        wr;
wire        valid;
reg         ready;

riscv i_cpu
(
    .clk   (clk),
    .rst   (~rst_n),
    .addr  (addr),
    .dout  (din),
    .din   (dout),
    .lane  (lane),
    .wr    (wr),
    .valid (valid),
    .ready (ready)
);

wire [31:0] sram_dout;
reg sram_wr;
sram i_sram
(
    .clock     (clk),
	 .data_a    (din),
	 .address_a (addr[13:2]),
	 .wren_a    (sram_wr),
	 .byteena_a (lane),
	 .q_a       (sram_dout)
);

wire [9:0] uart_dout;
uart i_uart
(
	.clk   (clk),
	.din   (din[7:0]),
	.dout  (uart_dout),
	.wr    (wr),
	.valid (valid && (addr[31:20] == 12'h200)),
	.txd   (txd)
);

localparam
	S_IDLE         = 3'd0,
	S_READ_SRAM    = 3'd1,
	S_WRITE_SRAM   = 3'd2,
	S_CYCLE_END    = 3'd3;

reg [2:0] state;

always @(posedge clk or negedge rst_n)
begin
	if (~rst_n)
	begin
		sram_wr <= 0;
		state <= S_IDLE;
	end
	else
	begin
		led[3] <= txd;
		case (state)
			S_IDLE:
			begin
				if (valid)
				begin
					if (wr)
					begin
						if (addr[31:20] == 12'h000)
						begin
							sram_wr <= 1;
							state <= S_WRITE_SRAM;
						end
						if (addr[31:20] == 12'h100)
						begin
							{led[2:0], dig, seg} <= din[14:0];
							ready <= 1;
							state <= S_CYCLE_END;
						end
						if (addr[31:20] == 12'h200)
						begin
							ready <= 1;
							state <= S_CYCLE_END;
						end
					end
					else
					begin
						if (addr[31:20] == 12'h000)
						begin
							state <= S_READ_SRAM;
						end
						if (addr[31:20] == 12'h200)
						begin
							dout <= uart_dout;
							state <= S_CYCLE_END;
						end
					end
				end
			end
			S_READ_SRAM:
			begin
				dout <= sram_dout;
				state <= S_CYCLE_END;
			end
			S_WRITE_SRAM:
			begin
				sram_wr <= 0;
				state <= S_CYCLE_END;
			end
			S_CYCLE_END:
			begin
				ready <= valid;
				if (!valid)
					state <= S_IDLE;
			end
		endcase
	end
end

endmodule
