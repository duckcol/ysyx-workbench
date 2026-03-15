// antpc is an toy process core
module antpc #(
    parameter integer ADDR_LEN = 32,
    parameter integer INST_LEN = 32,
    parameter integer REG_LEN = 5,
    parameter integer OPCODE_LEN = 7,
    parameter integer MEM_BASE = 32'h80000000
) (
    input clk,
    input sys_rst_l,
    // input [INST_LEN-1:0] pmem_read_result,
    // output [ADDR_LEN-1:0] pmem_read_addr,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] regd,
    // pins for debug
    input [REG_LEN-1:0] debug_reg_addr,
    output [INST_LEN-1:0] debug_reg_data
);

  wire [ADDR_LEN-1:0] cur_pc_addr, j_or_s_target_addr;
  wire cur_inst_j_or_s;
  Program_Counter #(
      .ADDR_LEN(ADDR_LEN),
      .MEM_BASE(MEM_BASE)
  ) pc1 (
      .sys_clk(clk),
      .pc_rst_l(sys_rst_l),
      .pc_addr(cur_pc_addr),
      .j_or_s_target_addr(j_or_s_target_addr),
      .inst_j_or_s(cur_inst_j_or_s)
  );

  wire [INST_LEN-1:0] cur_inst_data;
  IFU #(
      .ADDR_LEN(ADDR_LEN),
      .INST_LEN(INST_LEN)
  ) inst_fetch_unit1 (
      .rst_l(sys_rst_l),
      .clk(clk),
      .inst_j_or_s(cur_inst_j_or_s),
      .j_or_s_target_addr(j_or_s_target_addr),
      .pc_addr(cur_pc_addr),
      // .pmem_read_addr(pmem_read_addr),
      // .pmem_read_inst(pmem_read_result),
      .cur_inst(cur_inst_data)
  );

  wire [OPCODE_LEN-1:0] inst_opcode;
  wire [REG_LEN-1:0] inst_regd, inst_reg1, inst_reg2;
  wire [INST_LEN-1:0] inst_imm;
  wire [2:0] inst_funct3;
  wire [6:0] inst_funct7;
  IDU #(
      .INST_LEN(INST_LEN),
      .REG_LEN(REG_LEN),
      .OPCODE_LEN(OPCODE_LEN)
  ) inst_decode_unit1 (
      .sys_clk(clk),
      .rst_l(sys_rst_l),
      .inst(cur_inst_data),
      .opcode(inst_opcode),
      .funct3(inst_funct3),
      .funct7(inst_funct7),
      .regd(inst_regd),
      .reg1(inst_reg1),
      .reg2(inst_reg2),
      .imm(inst_imm)
  );

  EXU #(
      .INST_LEN(INST_LEN),
      .REG_LEN(REG_LEN),
      .OPCODE_LEN(OPCODE_LEN)
  ) inst_execute_unit1 (
      .clk(clk),
      .opcode(inst_opcode),
      .funct3(inst_funct3),
      .funct7(inst_funct7),
      .regd(inst_regd),
      .reg1(inst_reg1),
      .reg2(inst_reg2),
      .imm(inst_imm),
      .pc_addr(cur_pc_addr),
      .j_or_s_target_addr(j_or_s_target_addr),
      .cur_inst_j_or_s(cur_inst_j_or_s),
      .result(result),
      .result_reg(regd),
      //  pins for debug
      .debug_reg_addr(debug_reg_addr),
      .debug_reg_data(debug_reg_data)
  );

endmodule
