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
    input [INST_LEN-1:0] pc_addr,
    output [INST_LEN-1:0] target_addr,
    output cur_inst_j_or_s,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] result_reg
);

  //  so far it only execute inst:
  //  addi, auipc, jal, jalr

  //  get datain, which is different because of different inst
  //  pc_addr = pc + ４
  wire [INST_LEN-1:0] data1, data2, datain;
  assign datain = imm + data1;
  MuxKeyWithDefault #(
      .NR_KEY  (4),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN)
  ) data_rd_decoder (
      .out(imm),
      .key(opcode),
      .default_out(32'd0),
      .lut({
        // addi, I
        7'b0010011,
        data1 + imm,
        // auipc, U
        7'b0010111,
        pc_addr - 32'd4 + imm,
        // jal, J
        7'b1101111,
        pc_addr,
        // jalr, I
        7'b1100111,
        pc_addr
      })
  );

  // so far, only for jal and jalr
  assign cur_inst_j_or_s = (opcode == 7'b1101111) | (opcode == 7'b1100111) ? 1 : 0;
  MuxKeyWithDefault #(
      .NR_KEY  (2),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN)
  ) target_addr_MUX (
      .out(target_addr),
      .key(opcode),
      .default_out(32'd0),
      .lut({
        // jal, J
        7'b1101111,
        pc_addr - 32'd4 + imm,
        // jalr, I
        7'b1100111,
        (data1 + imm) & ~32'd1
      })
  );

  assign result_reg = regd;

  RegisterFileWithZero #(
      .ADDR_WIDTH(REG_LEN),
      .DATA_WIDTH(INST_LEN)
  ) GPR32 (
      .clk(clk),
      .wdata(datain),
      .waddr(regd),
      .raddr1(reg1),
      .raddr2(reg2),
      .wen(1'b1),
      .ren(1'b1),
      .rdata1(data1),
      .rdata2(data2),
      .rdata_out(result)
  );

  //  ebreak
  import "DPI-C" function void trigger_ebreak();
  always @(*) begin
    if (opcode == 7'b1110011 && funct3 == 3'b000 && imm == 32'd1) begin
      $display("Time=%02t: trigger ebreak", $time);
      trigger_ebreak();
    end
  end
endmodule

