module uart
(
	input wire       clk,
	
	input wire [7:0] din,
	output reg [9:0] dout,
	input wire       wr,
	input wire       valid,
	
	output reg       txd
);

localparam
	BIT_TIME	  = 12'd433;

localparam
	S_IDLE     = 2'd0,
	S_TRANSMIT = 2'd1;
	
reg [11:0] tdiv;
reg [1:0]  tstate;
reg [9:0]  tdata;

always @(posedge clk)
begin
	tdiv <=
		tstate == S_IDLE ? 12'd0 :
		tdiv == BIT_TIME ? 12'd0 :
		tdiv + 1'd1;
	
	txd <=
		tstate == S_IDLE ? 1'b1 :
		tdata[0];
	
	dout[9] <= tstate == S_IDLE;
	
	case (tstate)
		S_IDLE:
		begin
			if (valid)
			begin
				tdata <= {1'b1, din, 1'b0};
				tstate <= S_TRANSMIT;
			end
		end
		S_TRANSMIT:
		begin
			if (tdiv == BIT_TIME)
			begin
				tdata <= {1'b0, tdata[9:1]};
				if (tdata == 10'd1)
					tstate <= S_IDLE;
			end
		end
	endcase
end

endmodule

/*
module uart(
	input wire clk,
	
	input wire [7:0] din,
	output wire [7:0] dout,
	input wire wr_in,
	output reg wr_out,
	output reg rd_toggle,

	input wire rxd,
	output reg txd
);

localparam
	BIT_TIME	= 9'd433;

localparam
	IDLE = 4'd0,
	TXSTART = 4'd1,
	TXBIT0 = 4'd2,
	TXBIT1 = 4'd3,
	TXBIT2 = 4'd4,
	TXBIT3 = 4'd5,
	TXBIT4 = 4'd6,
	TXBIT5 = 4'd7,
	TXBIT6 = 4'd8,
	TXBIT7 = 4'd9,
	TXSTOP = 4'd10;

localparam
	RXSTART = 4'd1,
	RXDATA = 4'd2,
	RXSTOP = 4'd3;

reg [8:0] tdiv;
reg [3:0] txstate;
reg [7:0] txdata;

reg [8:0] rdiv;
reg [3:0] rxstate;
reg [7:0] rxdata;
reg [3:0] rxbits;

assign dout = rxdata;

always @(posedge clk)
begin
	if (txstate == IDLE)
	begin
		txd <= 1'b1;
		if (wr_in != wr_out)
		begin
			txdata <= din;
			wr_out <= ~wr_out;
			txstate <= TXSTART;
		end
	end
	if (tdiv == 9'd433)
	begin
		tdiv <= 9'd0;
		case (txstate)
			TXSTART:
			begin
				txd <= 1'b0;
				txstate <= TXBIT0;
			end
			TXBIT0:
			begin
				txd <= txdata[0];
				txstate <= TXBIT1;
			end
			TXBIT1:
			begin
				txd <= txdata[1];
				txstate <= TXBIT2;
			end
			TXBIT2:
			begin
				txd <= txdata[2];
				txstate <= TXBIT3;
			end
			TXBIT3:
			begin
				txd <= txdata[3];
				txstate <= TXBIT4;
			end
			TXBIT4:
			begin
				txd <= txdata[4];
				txstate <= TXBIT5;
			end
			TXBIT5:
			begin
				txd <= txdata[5];
				txstate <= TXBIT6;
			end
			TXBIT6:
			begin
				txd <= txdata[6];
				txstate <= TXBIT7;
			end
			TXBIT7:
			begin
				txd <= txdata[7];
				txstate <= TXSTOP;
			end
			TXSTOP:
			begin
				txd <= 1'b1;
				txstate <= IDLE;
			end
		endcase
	end
	else
	begin
		tdiv <= tdiv + 1'd1;
	end
	
	if (rdiv == 9'd0)
	begin
		case (rxstate)
			IDLE:
			begin
				if (!rxd)
				begin
					rdiv <= BIT_TIME >> 1;
					rxstate <= RXSTART;
				end
				else
				begin
					rdiv <= BIT_TIME >> 2;
				end
			end
			RXSTART:
			begin
				rdiv <= BIT_TIME;
				rxbits <= 4'd7;
				rxstate <= RXDATA;
			end
			RXDATA:
			begin
				rdiv <= BIT_TIME;
				rxdata <= {rxd, rxdata[7:1]};
				rxbits <= rxbits - 4'd1;
				if (rxbits == 4'd0)
					rxstate <= RXSTOP;
			end
			RXSTOP:
			begin
				rdiv <= BIT_TIME;
				rd_toggle <= ~rd_toggle;
				rxstate <= IDLE;
			end
		endcase
	end
	else
	begin
		rdiv <= rdiv - 1'd1;
	end
end

endmodule
*/
