module Program_Counter #(
    parameter integer ADDR_LEN = 32,
    parameter integer MEM_BASE = 32'h80000000
) (
    input sys_clk,
    input pc_rst_l,
    input inst_j_or_s,
    input [ADDR_LEN-1:0] target_addr,
    output [ADDR_LEN-1:0] pc_addr
);
  //  simple MUX to differ normal or jump/switch
  wire [ADDR_LEN-1:0] next_pc = inst_j_or_s ? target_addr : pc_addr;

  Reg #(
      .WIDTH(ADDR_LEN),
      .RESET_VAL(MEM_BASE)
  ) pc (
      .clk (sys_clk),
      .rst (~pc_rst_l),
      .din (next_pc + 4),
      .dout(pc_addr),
      .wen (1'b1)
  );
endmodule

