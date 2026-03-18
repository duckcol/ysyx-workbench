module AluInCtrl #(
    parameter integer DATA_LEN  = 32,
    parameter integer INST_CTRL = 9
) (
    input  [ DATA_LEN-1:0] imm,
    input  [ DATA_LEN-1:0] rs1,
    input  [ DATA_LEN-1:0] rs2,
    input  [ DATA_LEN-1:0] pc_addr,
    input  [ DATA_LEN-1:0] pc,
    input  [INST_CTRL-1:0] inst_ctrl,
    output [ DATA_LEN-1:0] alu_in_1,
    output [ DATA_LEN-1:0] alu_in_2
);
  // wire inst_ctrl form MSB to LSB is:
  // inst_L,
  // inst_S,
  // inst_commpute_imm,
  // inst_commpute_reg,
  // inst_lui,
  // inst_auipc,
  // inst_B,
  // inst_jal,
  // inst_jalr,

  // since the pc_addr is actually current running inst's pc + 4
  // so I need to -4 to get pc = pc_addr - 4 in EXU
  MuxKeyWithDefault #(
      .NR_KEY  (INST_CTRL),
      .KEY_LEN (INST_CTRL),
      .DATA_LEN(DATA_LEN * 2)
  ) ALU_in_data_ctrl (
      .out({alu_in_1, alu_in_2}),
      .key(inst_ctrl),
      .default_out({(DATA_LEN * 2) {1'b0}}),
      .lut({
        // format: {key (inst_ctrl), data (alu_in_1, alu_in_2)}

        // 1. inst_L (Load): RS1 + imm
        {
          9'b100_000_000, rs1, imm
        },

        // 2. inst_S (Store): RS1 + imm
        {
          9'b010_000_000, rs1, imm
        },

        // 3. inst_compute_imm: RS1 + imm
        {
          9'b001_000_000, rs1, imm
        },

        // 4. inst_compute_reg: RS1 + RS2
        {
          9'b000_100_000, rs1, rs2
        },

        // 5. inst_lui: 0 + imm (或者直接输出 imm，看你 ALU 0 选通逻辑)
        {
          9'b000_010_000, {DATA_LEN{1'b0}}, imm
        },

        // 6. inst_auipc: PC + imm
        {
          9'b000_001_000, pc, imm
        },

        // 7. inst_b (Branch): RS1 vs RS2 (ALU 做减法或比较)
        {
          9'b000_000_100, rs1, rs2
        },

        // 8. inst_jal: PC + 4 (用于计算返回地址写入寄存器，或 PC + imm 计算目标)
        // 注意：根据你的表格，此处为 PC + 4
        {
          9'b000_000_010, pc_addr, {(DATA_LEN) {1'b0}}
        },

        // 9. inst_jalr: PC + 4
        {
          9'b000_000_001, pc_addr, {(DATA_LEN) {1'b0}}
        }
      })
  );

endmodule
