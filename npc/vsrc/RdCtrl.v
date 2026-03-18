module RdCtrl #(
    parameter integer DATA_LEN  = 32,
    parameter integer INST_CTRL = 9
) (
    input  [ DATA_LEN-1:0] mem_read_result,
    input  [ DATA_LEN-1:0] alu_result,
    input  [INST_CTRL-1:0] inst_ctrl,
    output [ DATA_LEN-1:0] data_to_rd,
    output                 reg_write_en      // 新增：寄存器写使能
);

  // 1. 数据选择逻辑
  // 只有 Load 指令选内存，其余(计算、跳转、LUI)全部选 ALU
  assign data_to_rd   = inst_ctrl[8] ? mem_read_result : alu_result;

  // 2. 写回控制逻辑 (核心改进)
  // Store (inst_ctrl[7]) 和 Branch (inst_ctrl[2]) 不需要写回寄存器
  assign reg_write_en = ~(inst_ctrl[7] | inst_ctrl[2]);

endmodule
