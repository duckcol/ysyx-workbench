module Alu #(
    parameter integer DATALEN = 32,  // 建议默认 32
    parameter integer OPLEN   = 4
) (
    input [DATALEN-1:0] a,
    input [DATALEN-1:0] b,
    input [OPLEN-1:0] sel,
    output reg [DATALEN-1:0] out
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
      4'd7: out = {{(DATALEN - 1) {1'b0}}, (a == b)};
      4'd8: out = {{(DATALEN - 1) {1'b0}}, (a != b)};
      // 算术右移：显式转换 a 为有符号数
      4'd9: out = $unsigned($signed(a) >>> shamt);
      4'd10: out = a >> shamt;
      // 有符号比较 (SLT)
      4'd11: out = {{(DATALEN - 1) {1'b0}}, ($signed(a) < $signed(b))};
      // 无符号比较 (SLTU)
      4'd12: out = {{(DATALEN - 1) {1'b0}}, (a < b)};
      // 有符号大于等于 (BGE)
      4'd13: out = {{(DATALEN - 1) {1'b0}}, ($signed(a) >= $signed(b))};
      // 无符号大于等于 (BGEU)
      4'd14: out = {{(DATALEN - 1) {1'b0}}, (a >= b)};
      default: out = {DATALEN{1'b0}};
    endcase
  end
endmodule
