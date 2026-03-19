module Alu #(
    parameter integer DATA_LEN = 32,  // 建议默认 32
    parameter integer OPLEN = 4
) (
    input                     clk,
    input      [DATA_LEN-1:0] a,
    input      [DATA_LEN-1:0] b,
    input      [   OPLEN-1:0] sel,
    output reg [DATA_LEN-1:0] out
);

  // 移位量截断：对于 32 位数据，只取低 5 位
  wire [4:0] shamt = b[4:0];

  always @(*) begin
    case (sel)
      4'd1: out = a + b;
      4'd2: out = a - b;
      4'd3: out = a ^ b;
      4'd4: out = a & b;
      4'd5: out = a | b;
      4'd6: out = a << shamt;
      4'd7: out = {{(DATA_LEN - 1) {1'b0}}, (a == b)};
      4'd8: out = {{(DATA_LEN - 1) {1'b0}}, (a != b)};
      // 算术右移：显式转换 a 为有符号数
      4'd9: out = $unsigned($signed(a) >>> shamt);
      4'd10: out = a >> shamt;
      // 有符号比较 (SLT)
      4'd11: out = {{(DATA_LEN - 1) {1'b0}}, ($signed(a) < $signed(b))};
      // 无符号比较 (SLTU)
      4'd12: out = {{(DATA_LEN - 1) {1'b0}}, (a < b)};
      // 有符号大于等于 (BGE)
      4'd13: out = {{(DATA_LEN - 1) {1'b0}}, ($signed(a) >= $signed(b))};
      // 无符号大于等于 (BGEU)
      4'd14: out = {{(DATA_LEN - 1) {1'b0}}, (a >= b)};
      default: out = {DATA_LEN{1'b0}};
    endcase
  end
  // --- ALU 计算监视器 ---
`ifdef DEBUG_ALU
  // 辅助函数：将 sel 编号转换为易读的操作符字符串
  function [63:0] get_op_str(input [OPLEN-1:0] s);
    case (s)
      4'd1: get_op_str = "+";
      4'd2: get_op_str = "-";
      4'd3: get_op_str = "^";
      4'd4: get_op_str = "&";
      4'd5: get_op_str = "|";
      4'd6: get_op_str = "<<";
      4'd7: get_op_str = "==";
      4'd8: get_op_str = "!=";
      4'd9: get_op_str = ">>>";  // 算术右移
      4'd10: get_op_str = ">>";  // 逻辑右移
      4'd11: get_op_str = "< (s)";  // SLT
      4'd12: get_op_str = "< (u)";  // SLTU
      4'd13: get_op_str = ">= (s)";  // BGE
      4'd14: get_op_str = ">= (u)";  // BGEU
      default: get_op_str = "UNK";
    endcase
  endfunction

  always @(posedge clk) begin
    // 只有当 sel 不为 0 且非复位时打印（假设 0 是 NOP 或 IDLE）
    if (sel != 4'd0) begin
      $display("[Time=%05t] [ALU] 0x%h  %4s  0x%h => RESULT: 0x%h", $time, a, get_op_str(sel), b,
               out);

      // 针对移位操作，额外打印一下移位量 shamt 方便纠错
      if (sel == 4'd6 || sel == 4'd9 || sel == 4'd10) begin
        $display(" (Shift Amount: %0d)", b[4:0]);
      end
    end
  end
`endif
endmodule
