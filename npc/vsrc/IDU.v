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
  // opcode, funct3, rd, rs1, and rs2 position is fixed
  // so no need to use a MUX to decode them
  assign opcode = inst[6:0];
  assign funct3 = inst[14:12];
  assign regd   = inst[11:7];
  assign reg1   = inst[19:15];
  assign reg2   = inst[24:20];

  //  imm's position is not fixed,
  //  so I need MUX to decode
  //  so far, I decode inst to get imm depending
  //  on opcode, and only support 4 instructions
  MuxKeyWithDefault #(
      .NR_KEY  (4),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN)
  ) imm_decoder (
      .out(imm),
      .key(opcode),
      .default_out(32'd0),
      .lut({
        // addi, I
        7'b0010011,
        {{21{inst[31]}}, inst[30:20]},
        // auipc, U
        7'b0010111,
        {inst[31:12], {12{1'b0}}},
        // jal, J
        7'b1101111,
        {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0},
        // jalr, I
        7'b1100111,
        {{21{inst[31]}}, inst[30:20]}
      })
  );

  //monitor signal
  initial begin
    $monitor("Time=%02t: inst=%h | opcode=%b regd=%02d reg1=%02d reg2=%02d imm=%h", $time, inst,
             opcode, regd, reg1, reg2, imm);
  end

endmodule
