module EXU #(
    INST_LEN = 32,
    REG_LEN = 5,
    OPCODE_LEN = 7
) (
    input clk,
    input [OPCODE_LEN-1:0] opcode,
    input [2:0] funct3,
    input [REG_LEN-1:0] regd,
    input [REG_LEN-1:0] reg1,
    input [REG_LEN-1:0] reg2,
    input [INST_LEN-1:0] imm,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] result_reg
);

  //  so far it only execute inst addi
  wire [INST_LEN-1:0] data1, data2, datain;
  //  only wen when inst == addi
  wire wen = (opcode == 7'b0010011) && (funct3 == 3'b000) ? 1 : 0;
  RegisterFileWithZero #(
      .ADDR_WIDTH(REG_LEN),
      .DATA_WIDTH(INST_LEN)
  ) GPR32 (
      .clk(clk),
      .wdata(datain),
      .waddr(regd),
      .raddr1(reg1),
      .raddr2(reg2),
      .wen(wen),
      .ren(1'b1),
      .rdata1(data1),
      .rdata2(data2),
      .rdata_out(result)
  );

  assign datain = imm + data1;
  assign result_reg = regd;

  //  ebreak
  import "DPI-C" function void trigger_ebreak();
  always @(*) begin
    if (opcode == 7'b1110011 && funct3 == 3'b000 && imm == 32'd1) begin
      $display("Time=%02t: trigger ebreak", $time);
      trigger_ebreak();
    end
  end
endmodule

