module uart
(
	input wire        clk,
	
	input wire [31:0] din,
	output reg [31:0] dout,
	input wire        wr,
	input wire        valid,
	
	output reg        txd
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
	
	dout <= tstate == S_IDLE ? 32'h6000 : 32'h0;
	
	case (tstate)
		S_IDLE:
		begin
			if (valid && wr)
			begin
				tdata <= {1'b1, din[7:0], 1'b0};
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
