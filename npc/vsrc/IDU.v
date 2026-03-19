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
    output [6:0] funct7,
    output [REG_LEN-1:0] regd,
    output [REG_LEN-1:0] reg1,
    output [REG_LEN-1:0] reg2,
    output [INST_LEN-1:0] imm
);
  // opcode funct7 and funct3 position is fixed
  // so no need to use a MUX to decode them
  assign opcode = inst[6:0];
  assign funct3 = inst[14:12];
  assign funct7 = inst[31:25];

  parameter TYPE_R = 3'd0, TYPE_I = 3'd1, TYPE_S = 3'd2,
  TYPE_B = 3'd3, TYPE_U = 3'd4, TYPE_J = 3'd5;
  wire [2:0] inst_type;
  MuxKeyWithDefault #(
      .NR_KEY  (10),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(3)
  ) inst_type_decoder (
      .out(inst_type),
      .key(opcode),
      .default_out(3'd6),
      .lut({
        // load
        7'h03,
        TYPE_I,
        // store
        7'h23,
        TYPE_S,
        // compute imm
        7'h13,
        TYPE_I,
        // compute reg
        7'h33,
        TYPE_R,
        // lui only so far
        7'h37,
        TYPE_U,
        // auipc only so far
        7'h17,
        TYPE_U,
        // branch
        7'h63,
        TYPE_B,
        // jal
        7'h6F,
        TYPE_J,
        // jalr
        7'h67,
        TYPE_I,
        // system call
        7'h73,
        TYPE_I
      })
  );
  MuxKeyWithDefault #(
      .NR_KEY  (6),
      .KEY_LEN (3),
      .DATA_LEN(INST_LEN + 3 * REG_LEN)
  ) inst_imm_and_reg_decoder (
      .out({imm, reg2, reg1, regd}),
      .key(inst_type),
      .default_out('d0),
      .lut({
        TYPE_R,
        {{32'd0}, inst[24:20], inst[19:15], inst[11:7]},
        TYPE_I,
        {{{21{inst[31]}}, inst[30:20]}, 5'd0, inst[19:15], inst[11:7]},
        TYPE_S,
        {{{21{inst[31]}}, inst[30:25], inst[11:8], inst[7]}, inst[24:20], inst[19:15], 5'd0},
        TYPE_B,
        {{{20{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0}, inst[24:20], inst[19:15], 5'd0},
        TYPE_U,
        {inst[31:12], {12{1'b0}}, 5'd0, 5'd0, inst[11:7]},
        TYPE_J,
        {{{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0}, 5'd0, 5'd0, inst[11:7]}
      })
  );

  //  ebreak
  import "DPI-C" function void trigger_ebreak();
  always @(posedge sys_clk) begin
    if (opcode == 7'b1110011 && funct3 == 3'b000 && imm == 32'd1) begin
      $display("[Time=%05t] trigger ebreak", $time);
      trigger_ebreak();
    end
  end

  // 替换原有的 $monitor
`ifdef DEBUG_IDU
  string cur_inst_type = "";
  always @(*) begin
    case (inst_type)
      TYPE_R:  cur_inst_type = "TYPE_R";
      TYPE_I:  cur_inst_type = "TYPE_I";
      TYPE_S:  cur_inst_type = "TYPE_S";
      TYPE_B:  cur_inst_type = "TYPE_B";
      TYPE_U:  cur_inst_type = "TYPE_U";
      TYPE_J:  cur_inst_type = "TYPE_J";
      default: cur_inst_type = "TYPE_X";
    endcase
  end

  always @(posedge sys_clk or negedge rst_l) begin
    if (!rst_l) begin
      // 可选：复位时打印提示
      $display("[Time=%05t] [RESET] IDU reset asserted", $time);
    end else begin
      if (inst == 32'd0) begin
        $display("[Time=%05t] [IDU] ALL ZERO instruction", $time);
      end else begin
        $display(
            "[Time=%05t] [IDU] inst=%h | type=%s opcode=%b regd=%02d reg1=%02d reg2=%02d imm=%h",
            $time, inst, cur_inst_type, opcode, regd, reg1, reg2, imm);
      end
    end
  end
`endif

endmodule
