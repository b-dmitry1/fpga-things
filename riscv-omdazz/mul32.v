module mul32(
	input wire clk,

	input wire [31:0] a,
	input wire [31:0] b,
	output reg [63:0] res,

	input wire valid,
	output reg ready
);

reg [31:0] am;
reg [63:0] bm;

// verification only
initial
begin
	ready <= 0;
	am    <= 32'd0;
	bm    <= 64'd0;
	res   <= 64'd0;
end

always @(posedge clk)
begin
	if (am == 32'd0)
	begin
		ready <= 0;
		if (valid)
		begin
			ready <= a == 32'd0;
			am  <= a;
			bm  <= b;
			res <= 64'd0;
		end
	end
	else
	begin
		if (am[0])
			res[63:0] <= res[63:0] + bm[63:0];
		am <= am[31:1];
		bm[63:0] <= {bm[62:0], 1'b0};
		ready <= am == 32'd1;
	end
end

endmodule
