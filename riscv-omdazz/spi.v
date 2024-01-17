module spi(
	input  wire        clk,
	
	input  wire [31:0] din,
	output wire [ 7:0] dout,
	input  wire [ 3:0] lane,
	input  wire        wr,
	input  wire        valid,
	output reg         ready,

	output wire        cs_n,
	input  wire        miso,
	output wire        mosi,
	output reg         sck
);

localparam
	IDLE	= 2'd0,
	HIGH	= 2'd1,
	LOW	= 2'd2;

reg [1:0] state;

reg cs;

reg [7:0] div;
reg [7:0] preset;

reg [7:0] sin;
reg [7:0] sout;
reg [2:0] bits;

assign cs_n = ~cs;
assign mosi = sout[7];

assign dout = sin;

always @(posedge clk)
begin
	ready <= 0;

	if (div == preset)
	begin
		div <= 8'd0;

		case (state)
			IDLE:
			begin
				if (wr & valid)
				begin
					if (lane[0])
						sout   <= din[7:0];
					if (lane[2])
						cs     <= din[16];
					if (lane[3])
						preset <= din[31:24];
					bits <= 3'd7;
					state <= HIGH;
				end
			end
			HIGH:
			begin
				sck   <= 1'b1;
				state <= LOW;
			end
			LOW:
			begin
				sin  <= {sin[6:0], miso};
				sout <= {sout[6:0], 1'b0};
				sck  <= 1'b0;
				bits <= bits - 3'd1;
				if (bits == 3'd0)
				begin
					ready <= 1;
					state <= IDLE;
				end
				else
				begin
					state <= HIGH;
				end
			end
		endcase
	end
	else
	begin
		div <= div + 8'd1;
	end
end

endmodule
