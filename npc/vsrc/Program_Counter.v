module Program_Counter #(
    ADDR_LEN = 32,
    MEM_BASE = 32'h80000000
) (
    input sys_clk,
    input pc_rst_l,
    output [ADDR_LEN-1:0] pc_addr
);
  wire [ADDR_LEN-1:0] next_pc = pc_addr + 4;
  Reg #(
      .WIDTH(ADDR_LEN),
      .RESET_VAL(MEM_BASE)
  ) pc (
      sys_clk,
      ~pc_rst_l,
      next_pc,
      pc_addr,
      1'b1
  );
endmodule

