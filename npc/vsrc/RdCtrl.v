module RdCtrl #(
    parameter integer DATA_LEN  = 32,
    parameter integer INST_CTRL = 12
) (
    input  [ DATA_LEN-1:0] mem_read_result,
    input  [ DATA_LEN-1:0] alu_result,
    input  [ DATA_LEN-1:0] csr_val,
    input  [INST_CTRL-1:0] inst_ctrl,
    output [ DATA_LEN-1:0] data_to_rd,
    output                 reg_write_en
);

  // 1. 数据选择逻辑
  // LOAD: mem_read_result
  // CSR: csr_val
  // OTHERS: RD
  assign data_to_rd   = inst_ctrl[11] ? mem_read_result : inst_ctrl[1] ? csr_val : alu_result;

  // 2. 写回控制逻辑
  // Store, Branch, ecall, mret不需要写回寄存器
  assign reg_write_en = ~(inst_ctrl[10] | inst_ctrl[5] | inst_ctrl[2] | inst_ctrl[1]);

endmodule
