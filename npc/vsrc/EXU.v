module EXU #(
    parameter integer INST_LEN = 32,
    parameter integer REG_LEN = 5,
    parameter integer OPCODE_LEN = 7
) (
    input clk,
    input [OPCODE_LEN-1:0] opcode,
    input [2:0] funct3,
    input [REG_LEN-1:0] regd,
    input [REG_LEN-1:0] reg1,
    input [REG_LEN-1:0] reg2,
    input [INST_LEN-1:0] imm,
    input [INST_LEN-1:0] pc_addr,
    output [INST_LEN-1:0] j_or_s_target_addr,
    output cur_inst_j_or_s,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] result_reg,
    //  pins for debug
    input [REG_LEN-1:0] debug_reg_addr,
    output [INST_LEN-1:0] debug_reg_data
);

  //  so far it only execute inst:
  //  addi, auipc, jal, jalr, ebreak, sw(do nothing)

  //  get datain, which is different because of different inst
  //  pc_addr = pc + ４
  wire [INST_LEN-1:0] data1, data2, datain;
  MuxKeyWithDefault #(
      .NR_KEY  (5),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN)
  ) data_rd_decoder (
      .out(datain),
      .key(opcode),
      .default_out(32'd0),
      .lut({
        // addi, I
        7'b0010011,
        data1 + imm,
        // auipc, U
        7'b0010111,
        pc_addr - 32'd4 + imm,
        // lui, U
        7'b0110111,
        imm,
        // jal, J
        7'b1101111,
        pc_addr,
        // jalr, I
        7'b1100111,
        pc_addr
      })
  );

  // so far, only for jal and jalr
  assign cur_inst_j_or_s =
  (opcode == 7'b1101111) | (opcode == 7'b1100111 && funct3 == 3'b000)
  ? 1 : 0;
  MuxKeyWithDefault #(
      .NR_KEY  (2),
      .KEY_LEN (OPCODE_LEN),
      .DATA_LEN(INST_LEN)
  ) target_addr_mux (
      .out(j_or_s_target_addr),
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

  // riscv32e only have 16 reg in the regfile
  // so we need some modification to the rf
  RegisterFileWithZero #(
      .ADDR_WIDTH(REG_LEN - 1),
      .DATA_WIDTH(INST_LEN)
  ) GPR32 (
      .clk(clk),
      .wdata(datain),
      .waddr(regd[REG_LEN-2:0]),
      .raddr1(reg1[REG_LEN-2:0]),
      .raddr2(reg2[REG_LEN-2:0]),
      .wen(1'b1),
      .ren(1'b1),
      .rdata1(data1),
      .rdata2(data2),
      .wdata_out(result),
      .debug_addr(debug_reg_addr[REG_LEN-2:0]),
      .debug_data(debug_reg_data)
  );

endmodule
