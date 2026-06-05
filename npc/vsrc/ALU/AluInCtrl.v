`include "vsrc/vsrc_conf.h.v"

module AluInCtrl #(
    parameter integer DATA_LEN  = 32,
    parameter integer INST_CTRL = 12
) (
    input                  clk,
    input  [ DATA_LEN-1:0] imm,
    input  [ DATA_LEN-1:0] rs1,
    input  [ DATA_LEN-1:0] rs2,
    input  [ DATA_LEN-1:0] csr,
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
  // inst_ecall,
  // inst_mret,
  // inst_csr,

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
          12'b100_000_000_000, rs1, imm
        },

        // 2. inst_S (Store): RS1 + imm
        {
          12'b010_000_000_000, rs1, imm
        },

        // 3. inst_compute_imm: RS1 + imm
        {
          12'b001_000_000_000, rs1, imm
        },

        // 4. inst_compute_reg: RS1 + RS2
        {
          12'b000_100_000_000, rs1, rs2
        },

        // 5. inst_lui: 0 + imm (或者直接输出 imm，看你 ALU 0 选通逻辑)
        {
          12'b000_010_000_000, {DATA_LEN{1'b0}}, imm
        },

        // 6. inst_auipc: PC + imm
        {
          12'b000_001_000_000, pc, imm
        },

        // 7. inst_b (Branch): RS1 vs RS2 (ALU 做减法或比较)
        {
          12'b000_000_100_000, rs1, rs2
        },

        // 8. inst_jal: PC + 4 (用于计算返回地址写入寄存器，或 PC + imm 计算目标)
        {
          12'b000_000_010_000, pc_addr, {(DATA_LEN) {1'b0}}
        },

        // 9. inst_jalr: PC + 4
        {
          12'b000_000_001_000, pc_addr, {(DATA_LEN) {1'b0}}
        },

        // 10. inst_ecall, it does not calculate in ALU
        {
          12'b000_000_000_100, {(DATA_LEN) {1'b0}}, {(DATA_LEN) {1'b0}}
        },

        // 11. inst_mret, it does not calculate in ALU
        {
          12'b000_000_000_010, {(DATA_LEN) {1'b0}}, {(DATA_LEN) {1'b0}}
        },

        // 12. inst_csr, it pass csr value through ALU to write to RD
        {
          12'b000_000_000_001, {(DATA_LEN) {1'b0}}, csr
        }
      })
  );

`ifdef DEBUG_ALUINCTRL
  always @(posedge clk) begin  // 假设系统有 clk 信号，在时钟上升沿采样稳定数据
    if (|inst_ctrl) begin  // 仅在有有效控制信号时打印
      $write("[Time=%05t] [ALU_IN_CTRL]: ", $time);

      case (1'b1)
        inst_ctrl[11]: $write("[LOAD  ] ");
        inst_ctrl[10]: $write("[STORE ] ");
        inst_ctrl[9]:  $write("[I-TYPE] ");
        inst_ctrl[8]:  $write("[R-TYPE] ");
        inst_ctrl[7]:  $write("[LUI   ] ");
        inst_ctrl[6]:  $write("[AUIPC ] ");
        inst_ctrl[5]:  $write("[BRANCH] ");
        inst_ctrl[4]:  $write("[JAL   ] ");
        inst_ctrl[3]:  $write("[JALR  ] ");
        inst_ctrl[2]:  $write("[ECALL ] ");
        inst_ctrl[1]:  $write("[MRET  ] ");
        inst_ctrl[0]:  $write("[CSR   ] ");
        default:       $write("[UNKOWN] ");
      endcase

      // 重点排列：输出结果 -> 核心输入参数
      $display("OUT: {0x%h, 0x%h} | RS1: 0x%h, RS2: 0x%h, IMM: 0x%h, PC: 0x%h", alu_in_1, alu_in_2,
               rs1, rs2, imm, pc);
    end
  end
`endif

endmodule
