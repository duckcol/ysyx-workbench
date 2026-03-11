module IDU #(
    parameter integer INST_LEN = 32,
    parameter integer REG_LEN = 5,
    parameter integer OPCODE_LEN = 7
) (
    input [INST_LEN-1:0] inst,
    input sys_clk,
    input rst_l,
    output [OPCODE_LEN-1:0] opcode,
    output [2:0] funct3,
    output [REG_LEN-1:0] regd,
    output [REG_LEN-1:0] reg1,
    output [REG_LEN-1:0] reg2,
    output [INST_LEN-1:0] imm
);
  // opcode and funct3 position is fixed
  // so no need to use a MUX to decode them
  assign opcode = inst[6:0];
  assign funct3 = inst[14:12];

  //  the position of imm, rs1, rs2, rd are not fixed,
  //  so I need MUX to decode
  //  so far, I decode inst to get them depending
  //  on opcode, and only support 7 instructions
  MuxKeyWithDefault #(
      .NR_KEY  (7),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN + 3 * REG_LEN)
  ) imm_decoder (
      .out({imm, regd, reg1, reg2}),
      .key(opcode),
      .default_out('d0),
      .lut({
        // addi, I
        7'b0010011,
        {{21{inst[31]}}, inst[30:20], inst[11:7], inst[19:15], 5'd0},
        // auipc, U
        7'b0010111,
        {inst[31:12], {12{1'b0}}, inst[11:7], 5'd0, 5'd0},
        //  lui, U
        7'b0110111,
        {inst[31:12], {12{1'b0}}, inst[11:7], 5'd0, 5'd0},
        // jal, J
        7'b1101111,
        {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0, inst[11:7], 5'd0, 5'd0},
        // jalr, I
        7'b1100111,
        {{21{inst[31]}}, inst[30:20], inst[11:7], inst[19:15], 5'd0},
        // ebreak, I
        7'b1110011,
        {{21{inst[31]}}, inst[30:20], inst[11:7], inst[19:15], 5'd0},
        // sw, S
        7'b0100011,
        {{21{inst[31]}}, inst[30:25], inst[11:8], inst[7], 5'd0, inst[19:15], inst[24:20]}
      })
  );

  //  ebreak
  import "DPI-C" function void trigger_ebreak();
  always @(posedge sys_clk) begin
    if (opcode == 7'b1110011 && funct3 == 3'b000 && imm == 32'd1) begin
      $display("Time=%02t: trigger ebreak", $time);
      trigger_ebreak();
    end
  end

  //monitor signal
  // initial begin
  //   $monitor("Time=%02t: inst=%h | opcode=%b regd=%02d reg1=%02d reg2=%02d imm=%h", $time, inst,
  //            opcode, regd, reg1, reg2, imm);
  // end

  // 替换原有的 $monitor
  always @(posedge sys_clk or negedge rst_l) begin
    if (!rst_l) begin
      // 可选：复位时打印提示
      $display("Time=%0t: [RESET] IDU reset asserted", $time);
    end else begin
      if (inst == 32'd0) begin
        $display("Time=%0t: ALL ZERO instruction", $time);
      end else begin
        $display("Time=%0t: inst=%h | opcode=%b regd=%02d reg1=%02d reg2=%02d imm=%h", $time, inst,
                 opcode, regd, reg1, reg2, imm);
      end
    end
  end

endmodule
