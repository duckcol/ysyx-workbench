module RegisterFile #(
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
    output [DATA_WIDTH-1:0] rdata_out
);
  reg [DATA_WIDTH-1:0] rf[2**ADDR_WIDTH-1:0];
  always @(posedge clk) begin
    if (wen) rf[waddr] <= wdata;
  end

  //  for monitor and output 0 when addr == 0
  assign rdata1 = (ren && raddr1 != 0) ? rf[raddr1] : 0;
  assign rdata2 = (ren && raddr2 != 0) ? rf[raddr2] : 0;
  assign rdata_out = (ren && waddr != 0) ? rf[waddr] : 0;
endmodule
