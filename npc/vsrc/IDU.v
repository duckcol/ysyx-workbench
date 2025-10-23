module IDU #(
    INST_LEN = 32,
    REG_LEN = 5,
    OPCODE_LEN = 7
) (
    input [INST_LEN-1:0] inst,
    output [OPCODE_LEN-1:0] opcode,
    output [2:0] funct3,
    output [REG_LEN-1:0] regd,
    output [REG_LEN-1:0] reg1,
    output [REG_LEN-1:0] reg2,
    output [INST_LEN-1:0] imm
);
  //so far I only decode TYPE_I inst to implement addi
  assign opcode = inst[6:0];
  assign funct3 = inst[14:12];
  assign regd = inst[11:7];
  assign reg1 = inst[19:15];
  assign reg2 = inst[24:20];
  assign imm = {{21{inst[31]}}, inst[30:20]};

  //monitor signal
  initial begin
    $monitor("Time=%02t: inst=%h | opcode=%b regd=%02d reg1=%02d reg2=%02d imm=%h", $time, inst,
             opcode, regd, reg1, reg2, imm);
  end

endmodule
