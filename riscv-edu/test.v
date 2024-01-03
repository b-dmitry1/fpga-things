module test;

reg         clk;
reg         rst;

wire [31:0] addr;
wire [31:0] din;
reg  [31:0] dout;
wire [ 3:0] lane;
wire        wr;
wire        valid;
reg         ready;

riscv target
(
    .clk   (clk),
    .rst   (rst),
    .addr  (addr),
    .dout  (din),
    .din   (dout),
    .lane  (lane),
    .wr    (wr),
    .valid (valid),
    .ready (ready)
);

reg  [31:0] ram [0:65535];

integer fp, c, i;

initial
begin
    fp = $fopen("firmware.bin", "rb");
    c = $fread(ram, fp);
    $fclose(fp);

    // Reverse bytes
    for (i = 0; i < c / 4; i = i + 1)
        ram[i] = {ram[i][7:0], ram[i][15:8], ram[i][23:16], ram[i][31:24]};
end

initial
begin
    #10;
    rst <= 0;
    #10;
    rst <= 1;
    #10;
    rst <= 0;
    #10;

    ready <= 1;

    $display("Press Ctrl+C to stop the simulation");
	
    forever
    begin
        clk <= 1;
        #10;
        clk <= 0;
        #10;
        ready <= valid;
        if (valid)
        begin
            if (wr)
            begin
                if (addr[31:20] == 12'h000)
                begin
                    if (lane[0])
                        ram[addr[17:2]][ 7: 0] <= din[7:0];
                    if (lane[1])
                        ram[addr[17:2]][15: 8] <= din[15:8];
                    if (lane[2])
                        ram[addr[17:2]][23:16] <= din[23:16];
                    if (lane[3])
                        ram[addr[17:2]][31:24] <= din[31:24];
                end
                if (addr[31:20] == 12'h100)
                    $write("%c", din[7:0]);
            end
            else
            begin
                if (addr[31:20] == 12'h000)
                    dout <= ram[addr[17:2]];
            end
        end
    end
    $finish;
end


endmodule
