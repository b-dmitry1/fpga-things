module vdu
(
	input  wire        clk,
	
	output reg  [24:3] video_addr,
	input  wire [63:0] video_data,
	
	output wire        hsync,
	output wire        vsync,
	output reg  [ 7:0] red,
	output reg  [ 7:0] green,
	output reg  [ 7:0] blue,
	
	output reg         backtrace,
	input  wire [31:0] vram_start
);

parameter
	VRAM_PAGE1     = 25'h7A0000,
	VRAM_PAGE2     = 25'h7D0000;

assign     hsync = ~((hcounter >= 10'd664) && (hcounter < 10'd760));
assign     vsync = ~((vcounter >= 10'd490) && (vcounter < 10'd492));

reg        pixel_div;

reg  [9:0] hcounter;
reg  [9:0] vcounter;

reg [63:0] vdata;

always @(posedge clk)
begin
	pixel_div <= ~pixel_div;
	
	// Safe flip area
	backtrace <= vcounter >= 10'd480 && vcounter < 10'd520;

	if (hcounter < 10'd640)
	begin
		red   <= {8{vdata[2]}};
		green <= {8{vdata[1]}};
		blue  <= {8{vdata[0]}};
	end
	else
	begin
		red   <= 8'd0;
		green <= 8'd0;
		blue  <= 8'd0;
	end
	
	if (pixel_div)
	begin
		if (hcounter == 10'd799)
		begin
			hcounter <= 10'd0;
			if (vcounter == 10'd524)
				vcounter <= 10'd0;
			else
				vcounter <= vcounter + 10'd1;
		end
		else
			hcounter <= hcounter + 10'd1;

		if (vcounter < 10'd480 && hcounter < 10'd640)
		begin
			if (&hcounter[3:0])
			begin
				video_addr <= video_addr + 1'd1;
				vdata <= video_data;
			end
			else
				vdata <= vdata[63:4];
		end
		else
		begin
			if (vcounter == 10'd522 && hcounter == 10'd799)
			begin
				vdata <= video_data;
				video_addr <= video_addr + 1'd1;
			end
			if (vcounter == 10'd521)
				video_addr <= vram_start[24:3];
		end
	end
end

endmodule
