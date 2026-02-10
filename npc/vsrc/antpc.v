// antpc is an only toy process core
module antpc #(
    ADDR_LEN = 32,
    INST_LEN = 32,
    REG_LEN = 5,
    OPCODE_LEN = 7,
    MEM_BASE = 32'h80000000
) (
    input clk,
    input sys_rst_l,
    input [INST_LEN-1:0] pmem_read,
    output [ADDR_LEN-1:0] fetch_inst_addr,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] regd,
    // pins for debug
    input [REG_LEN-1:0] debug_reg_addr,
    output [INST_LEN-1:0] debug_reg_data
);

  wire [ADDR_LEN-1:0] cur_inst_addr, target_addr;
  wire cur_inst_j_or_s;
  Program_Counter #(ADDR_LEN, MEM_BASE) pc1 (
      .sys_clk(clk),
      .pc_rst_l(sys_rst_l),
      .pc_addr(cur_inst_addr),
      .target_addr(target_addr),
      .inst_j_or_s(cur_inst_j_or_s)
  );

  wire [INST_LEN-1:0] cur_inst_data;
  IFU #(ADDR_LEN, INST_LEN) inst_fetch_unit1 (
      .rst_l(sys_rst_l),
      .clk(clk),
      .inst_j_or_s(cur_inst_j_or_s),
      .target_addr(target_addr),
      .pc_addr(cur_inst_addr),
      .fetch_addr(fetch_inst_addr),
      .pmem_read_result(pmem_read),
      .fetch_inst(cur_inst_data)
  );

  wire [OPCODE_LEN-1:0] inst_opcode;
  wire [REG_LEN-1:0] inst_regd, inst_reg1, inst_reg2;
  wire [INST_LEN-1:0] inst_imm;
  wire [2:0] inst_funct3;
  IDU #(INST_LEN, REG_LEN, OPCODE_LEN) inst_decode_unit1 (
      .sys_clk(clk),
      .rst_l(sys_rst_l),
      .inst(cur_inst_data),
      .opcode(inst_opcode),
      .funct3(inst_funct3),
      .regd(inst_regd),
      .reg1(inst_reg1),
      .reg2(inst_reg2),
      .imm(inst_imm)
  );

  EXU #(INST_LEN, REG_LEN, OPCODE_LEN) inst_execute_unit1 (
      .clk(clk),
      .opcode(inst_opcode),
      .funct3(inst_funct3),
      .regd(inst_regd),
      .reg1(inst_reg1),
      .reg2(inst_reg2),
      .imm(inst_imm),
      .pc_addr(cur_inst_addr),
      .target_addr(target_addr),
      .cur_inst_j_or_s(cur_inst_j_or_s),
      .result(result),
      .result_reg(regd),
      //  pins for debug
      .debug_reg_addr(debug_reg_addr),
      .debug_reg_data(debug_reg_data)
  );

endmodule
