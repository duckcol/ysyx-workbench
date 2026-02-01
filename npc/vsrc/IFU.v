module IFU #(
    ADDR_LEN = 32,
    INST_LEN = 32
) (
    input rst_l,
    input clk,
    input inst_j_or_s,
    input [ADDR_LEN-1:0] target_addr,
    input [ADDR_LEN-1:0] pc_addr,
    input [INST_LEN-1:0] pmem_read_result,
    output [ADDR_LEN-1:0] fetch_addr,
    output [INST_LEN-1:0] fetch_inst
);
  Reg #(
      .WIDTH(INST_LEN),
      .RESET_VAL(32'd0)
  ) IR (
      .clk (clk),
      .rst (~rst_l),
      .din (pmem_read_result),
      .dout(fetch_inst),
      .wen (1'b1)
  );
  assign fetch_addr = inst_j_or_s ? target_addr : pc_addr;
endmodule
