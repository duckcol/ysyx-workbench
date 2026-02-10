module RegisterFileWithZero #(
    ADDR_WIDTH = 1,
    DATA_WIDTH = 1
) (
    input clk,
    input [DATA_WIDTH-1:0] wdata,
    input [ADDR_WIDTH-1:0] waddr,
    input [ADDR_WIDTH-1:0] raddr1,
    input [ADDR_WIDTH-1:0] raddr2,
    input wen,
    //  pins for monitor
    input ren,
    output [DATA_WIDTH-1:0] rdata1,
    output [DATA_WIDTH-1:0] rdata2,
    output [DATA_WIDTH-1:0] rdata_out,
    //  pins for debug
    input [ADDR_WIDTH-1:0] debug_addr,
    output [DATA_WIDTH-1:0] debug_data
);
  wire internel_wen = wen && (waddr != 0);
  RegisterFile #(
      .ADDR_WIDTH(ADDR_WIDTH),
      .DATA_WIDTH(DATA_WIDTH)
  ) internal_rf (
      .clk(clk),
      .wdata(wdata),
      .waddr(waddr),
      .raddr1(raddr1),
      .raddr2(raddr2),
      .wen(internel_wen),
      .ren(ren),
      .rdata1(rdata1),
      .rdata2(rdata2),
      .rdata_out(rdata_out),
      .debug_addr(debug_addr),
      .debug_data(debug_data)
  );
endmodule
