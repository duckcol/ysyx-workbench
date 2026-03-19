module EXU #(
    parameter integer INST_LEN = 32,
    parameter integer REG_LEN = 5,
    parameter integer OPCODE_LEN = 7
) (
    input clk,
    input [OPCODE_LEN-1:0] opcode,
    input [2:0] funct3,
    input [6:0] funct7,
    input [REG_LEN-1:0] regd,
    input [REG_LEN-1:0] reg1,
    input [REG_LEN-1:0] reg2,
    input [INST_LEN-1:0] imm,
    input [INST_LEN-1:0] pc_addr,
    output [INST_LEN-1:0] j_or_b_target_addr,
    output cur_inst_j_or_b,
    output [INST_LEN-1:0] result,
    output [REG_LEN-1:0] result_reg,
    //  pins for debug
    input [REG_LEN-1:0] debug_reg_addr,
    output [INST_LEN-1:0] debug_reg_data
);

  //  so far it only execute inst:
  //  addi, auipc, jal, jalr, ebreak, sw(do nothing)

  //  CULCULATE datain
  //  which is different because of different inst
  //  NOTE:
  //  pc_addr is the current running inst's pc + ４
  wire [INST_LEN-1:0] pc = pc_addr - {{(INST_LEN - 3) {1'b0}}, 3'd4};

  // riscv32e only have 16 reg in the regfile
  // so we need some modification to the rf
  wire [INST_LEN-1:0] rs1, rs2, rd;
  RegisterFileWithZero #(
      .ADDR_WIDTH(REG_LEN - 1),
      .DATA_WIDTH(INST_LEN)
  ) GPR32 (
      .clk       (clk),
      .wdata     (rd),
      .waddr     (regd[REG_LEN-2:0]),
      .raddr1    (reg1[REG_LEN-2:0]),
      .raddr2    (reg2[REG_LEN-2:0]),
      .wen       (rd_write_en),
      .ren       (1'b1),
      .rdata1    (rs1),
      .rdata2    (rs2),
      .wdata_out (result),
      .debug_addr(debug_reg_addr[REG_LEN-2:0]),
      .debug_data(debug_reg_data)
  );

  // AluOpCtrl is to control how
  // other EXU part work
  // depending on current running inst
  // wire inst_ctrl form MSB to LSB is:
  // inst_L,
  // inst_S,
  // inst_commpute_imm,
  // inst_commpute_reg,
  // inst_lui,
  // inst_auipc,
  // inst_B,
  // inst_jal,
  // inst_jalr,
  parameter integer INST_CTRL = 9;
  parameter integer ALUOP_LEN = 4;
  wire [INST_CTRL-1:0] inst_ctrl;
  wire [ALUOP_LEN-1:0] alu_sel;
  wire inst_illegal;
  AluOpCtrl #(
      .ALUOP_LEN (ALUOP_LEN),
      .OPCODE_LEN(OPCODE_LEN)
  ) EXU_controller (
      .clk              (clk),
      .opcode           (opcode),
      .funct3           (funct3),
      .funct7           (funct7),
      .alu_sel          (alu_sel),
      .inst_L           (inst_ctrl[8]),
      .inst_S           (inst_ctrl[7]),
      .inst_commpute_imm(inst_ctrl[6]),
      .inst_commpute_reg(inst_ctrl[5]),
      .inst_lui         (inst_ctrl[4]),
      .inst_auipc       (inst_ctrl[3]),
      .inst_B           (inst_ctrl[2]),
      .inst_jal         (inst_ctrl[1]),
      .inst_jalr        (inst_ctrl[0]),
      .inst_illegal     (inst_illegal)
  );

  // AluInCtrl control which two data
  // will be passed in Alu
  wire [INST_LEN-1:0] alu_in_1;
  wire [INST_LEN-1:0] alu_in_2;
  AluInCtrl #(
      .DATA_LEN (INST_LEN),
      .INST_CTRL(INST_CTRL)
  ) EXU_alu_in_ctrl (
      .clk      (clk),
      .imm      (imm),
      .rs2      (rs2),
      .rs1      (rs1),
      .pc_addr  (pc_addr),
      .pc       (pc),
      .inst_ctrl(inst_ctrl),
      .alu_in_1 (alu_in_1),
      .alu_in_2 (alu_in_2)
  );

  // Alu
  wire [INST_LEN-1:0] alu_out;
  Alu #(
      .DATA_LEN(INST_LEN),
      .OPLEN(ALUOP_LEN)
  ) EXU_alu (
      .clk(clk),
      .a  (alu_in_1),
      .b  (alu_in_2),
      .sel(alu_sel),
      .out(alu_out)
  );

  // TargetAddrCtrl will compute target_addr
  // when inst_J or inst_B is effctive
  wire inst_B_effect = inst_ctrl[2] & alu_out[0];
  assign cur_inst_j_or_b = (inst_ctrl[1] | inst_ctrl[0] | inst_B_effect);
  always_comb begin
    if (cur_inst_j_or_b) begin
      $display("detact inst_J or inst_B_effect");
    end
  end

  TargetAddrCtrl #(
      .ADDR_WIDTH(INST_LEN)
  ) EXU_target_addr_computer (
      .clk               (clk),
      .inst_jal          (inst_ctrl[1]),
      .inst_jalr         (inst_ctrl[0]),
      .inst_B_effect     (inst_B_effect),
      .pc                (pc),
      .rs1               (rs1),
      .imm               (imm),
      // .cur_inst_j_or_b   (cur_inst_j_or_b),
      .j_or_b_target_addr(j_or_b_target_addr)
  );

  // MemCtrl will handle pmem read ans write
  wire [INST_LEN-1:0] MemCtrlRead;
  MemCtrl #(
      .DATA_LEN(INST_LEN)
  ) EXU_MemCtrl (
      .alu_result_addr(alu_out),
      .rs2            (rs2),
      .inst_L         (inst_ctrl[8]),
      .inst_S         (inst_ctrl[7]),
      .funct3         (funct3),
      .read_result    (MemCtrlRead)
  );

  // RdCtrl will handle what data will write in rd
  wire rd_write_en;
  RdCtrl #(
      .DATA_LEN (INST_LEN),
      .INST_CTRL(INST_CTRL)
  ) EXU_RdCtrl (
      .mem_read_result(MemCtrlRead),
      .alu_result     (alu_out),
      .inst_ctrl      (inst_ctrl),
      .data_to_rd     (rd),
      .reg_write_en   (rd_write_en)
  );

endmodule
