module top
(
	input  wire        clk,
	input  wire        rst_n,

	output reg  [3:0]  led,
	output reg  [3:0]  dig,
	output reg  [7:0]  seg,
	
	input  wire [3:0]  buttons,
	
	output wire        txd,
	input  wire        rxd,

	output wire        spi_cs_n,
	input  wire        spi_miso,
	output wire        spi_mosi,
	output wire        spi_sck,

	output wire        red,
	output wire        green,
	output wire        blue,
	output wire        hsync,
	output wire        vsync,
	
	output wire [11:0] sa,
	output wire  [1:0] sba,
	output wire  [1:0] sdqm,
	inout  wire [15:0] sd,
	output wire        ras_n,
	output wire        cas_n,
	output wire        we_n,
	output wire        cs_n,
	output wire        sclk,
	output wire        scke
);

//////////////////////////////////////////////////////////////////////////////
// Memory map
//////////////////////////////////////////////////////////////////////////////

wire sram_area        = addr[31:28] == 4'h0;
wire gpio_area        = addr[31:24] == 8'h10;
wire uart_area        = addr[31:24] == 8'h11;
wire vdu_io_area      = addr[31:24] == 8'h12;
wire timer_area       = addr[31:24] == 8'h13;
wire seg_area         = addr[31:24] == 8'h14;
wire spi_area         = addr[31:24] == 8'h15;
wire sdram_area       = addr[31];

//////////////////////////////////////////////////////////////////////////////
// CPU
//////////////////////////////////////////////////////////////////////////////

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
	.din   (sdram_area ? sdram_dout : dout),
	.lane  (lane),
	.wr    (wr),
	.valid (valid),
	.ready (ready || sdram_ready || spi_ready)
);

//////////////////////////////////////////////////////////////////////////////
// Boot ROM / RAM
//////////////////////////////////////////////////////////////////////////////

wire [31:0] sram_dout;
reg sram_wr;
sram i_sram
(
	.clock     (clk),
	.data_a    (din),
	.address_a (addr[13:2]),
	.wren_a    (valid && wr && sram_area),
	.byteena_a (lane),
	.q_a       (sram_dout)
);

//////////////////////////////////////////////////////////////////////////////
// GPIO
//////////////////////////////////////////////////////////////////////////////

reg [31:0] gpio;
always @(posedge clk)
begin
	if (valid && wr && gpio_area)
		gpio <= din;
end

//////////////////////////////////////////////////////////////////////////////
// UART
//////////////////////////////////////////////////////////////////////////////

wire [9:0] uart_dout;
uart i_uart
(
	.clk   (clk),
	.din   (din[7:0]),
	.dout  (uart_dout),
	.wr    (wr),
	.valid (valid && uart_area),
	.txd   (txd)
);

//////////////////////////////////////////////////////////////////////////////
// SPI
//////////////////////////////////////////////////////////////////////////////

wire [7:0] spi_dout;
wire       spi_ready;
spi i_spi
(
	.clk   (clk),
	.din   (din[31:0]),
	.dout  (spi_dout),
	.lane  (lane),
	.wr    (wr),
	.valid (valid && spi_area),
	.ready (spi_ready),
	.cs_n  (spi_cs_n),
	.miso  (spi_miso),
	.mosi  (spi_mosi),
	.sck   (spi_sck)
);

//////////////////////////////////////////////////////////////////////////////
// TIMER
//////////////////////////////////////////////////////////////////////////////

reg [31:0] timer_value;
reg [ 4:0] timer_div;
always @(posedge clk)
begin
	// Increment every 1 us
	timer_div   <= timer_div == 5'd19 ? 5'd0 : timer_div + 1'd1;
	timer_value <= timer_div == 5'd19 ? timer_value + 32'd1 : timer_value;
end

//////////////////////////////////////////////////////////////////////////////
// 7-segment display and leds
//////////////////////////////////////////////////////////////////////////////

reg [31:0] seg_value;
reg [15:0] seg_div;
reg seg_blank;
always @(posedge clk or negedge rst_n)
begin
	if (~rst_n)
	begin
		seg_value <= 32'd0;
	end
	else
	begin
		led <= gpio[15:12];

		seg_blank <= ~|seg_value;
		
		// Increment every 1 us
		seg_div <= seg_div + 1'd1;
		if (seg_blank)
		begin
			dig <= gpio[11:8];
			seg <= gpio[ 7:0];
		end
		else
		begin
			case (seg_div[15:14])
				2'd0: begin seg <= ~seg_value[ 7: 0]; dig <= 4'b1110; end
				2'd1: begin seg <= ~seg_value[15: 8]; dig <= 4'b1101; end
				2'd2: begin seg <= ~seg_value[23:16]; dig <= 4'b1011; end
				2'd3: begin seg <= ~seg_value[31:24]; dig <= 4'b0111; end
			endcase
		end
		
		if (valid && wr && seg_area)
		begin
			if (lane[0]) seg_value[ 7: 0] <= din[ 7: 0];
			if (lane[1]) seg_value[15: 8] <= din[15: 8];
			if (lane[2]) seg_value[23:16] <= din[23:16];
			if (lane[3]) seg_value[31:24] <= din[31:24];
		end
	end
end

//////////////////////////////////////////////////////////////////////////////
// PLL
//////////////////////////////////////////////////////////////////////////////

wire sdram_clk;
pll1 i_pll
(
	.inclk0 (clk),
	.c0     (sdram_clk)
);

//////////////////////////////////////////////////////////////////////////////
// SDRAM controller + PLL
//////////////////////////////////////////////////////////////////////////////

wire [31:0] sdram_dout;
wire sdram_ready;
sdram i_sdram(
	.clk        (clk),
	.clk1       (sdram_clk),
	.reset_n    (rst_n),

	.cpu_addr   (addr[22:2]),
	.cpu_din    (din),
	.cpu_dout   (sdram_dout),
	.cpu_lane   (lane),
	.cpu_wr     (wr),
	.cpu_valid  (valid && sdram_area),
	.cpu_ready  (sdram_ready),
	
	.video_addr (video_addr),
	.video_dout (video_data),

	.a          (sa),
	.ba         (sba),
	.dqm        (sdqm),
	.d          (sd),
	.ras_n      (ras_n),
	.cas_n      (cas_n),
	.we_n       (we_n),
	.cs_n       (cs_n),
	.sclk       (sclk),
	.scke       (scke)
);

//////////////////////////////////////////////////////////////////////////////
// VGA controller
//////////////////////////////////////////////////////////////////////////////

wire [24:3] video_addr;
wire [63:0] video_data;
wire [7:0] red1;
wire [7:0] green1;
wire [7:0] blue1;
assign red = red1[7];
assign green = green1[7];
assign blue = blue1[7];
wire backtrace;
wire vdu_io_dout = backtrace;
reg  [31:0] vram_start;
vdu i_vdu
(
	.clk         (clk),
	
	.video_addr  (video_addr),
	.video_data  (video_data),
	
	.hsync       (hsync),
	.vsync       (vsync),
	.red         (red1),
	.green       (green1),
	.blue        (blue1),
	.backtrace   (backtrace),
	.vram_start  (vram_start)
);

always @(posedge clk)
begin
	if (valid && wr && vdu_io_area)
		vram_start <= din;
end

//////////////////////////////////////////////////////////////////////////////
// System controller
//////////////////////////////////////////////////////////////////////////////

reg last_valid;

always @(posedge clk or negedge rst_n)
begin
	if (~rst_n)
	begin
		ready <= 0;
		last_valid <= 0;
	end
	else
	begin
		// Peripherial to CPU data bus
		last_valid <= valid;
		if (last_valid && valid && ~ready)
		begin
			dout <=
				sram_area ? sram_dout :
				uart_area ? uart_dout :
				vdu_io_area ? vdu_io_dout :
				gpio_area ? {buttons} :
				timer_area ? timer_value :
				spi_area ? spi_dout :
				32'hFFFFFFFF;
		end

		// "Ready" control
		if (sdram_area || (wr && spi_area))
			ready <= 0;
		else
			ready <= last_valid && valid && ~ready;
	end
end

endmodule
