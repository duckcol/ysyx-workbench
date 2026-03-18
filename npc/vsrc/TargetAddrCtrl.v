module TargetAddrCtrl #(
    parameter integer ADDR_WIDTH = 32
) (
    input                   inst_jal,
    input                   inst_jalr,
    input                   inst_B_effect,
    input                   pc,
    input                   rs1,
    input                   imm,
    output [ADDR_WIDTH-1:0] j_or_b_target_addr,
    output                  cur_inst_j_or_b
);
  assign cur_inst_j_or_b = (inst_jalr | inst_jalr | inst_B_effect);
  MuxKeyWithDefault #(
      .NR_KEY  (3),
      .KEY_LEN (3),
      .DATA_LEN(ADDR_WIDTH)
  ) target_addr_mux (
      .out(j_or_b_target_addr),
      .key({inst_B_effect, inst_jal, inst_jalr}),
      .default_out({(ADDR_WIDTH) {1'b0}}),
      .lut({
        // inst_B_effect
        3'b100,
        pc + imm,
        // jal, J
        3'b010,
        pc + imm,
        // jalr, I
        3'b001,
        (rs1 + imm) & ~(32'd1)  //hope synthesizer simply it for me
      })
  );
endmodule
