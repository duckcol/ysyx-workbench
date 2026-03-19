module TargetAddrCtrl #(
    parameter integer ADDR_WIDTH = 32
) (
    input                   clk,
    input                   inst_jal,
    input                   inst_jalr,
    input                   inst_B_effect,
    input  [ADDR_WIDTH-1:0] pc,
    input  [ADDR_WIDTH-1:0] rs1,
    input  [ADDR_WIDTH-1:0] imm,
    // output                  cur_inst_j_or_b,
    output [ADDR_WIDTH-1:0] j_or_b_target_addr
);
  // assign cur_inst_j_or_b = (inst_jalr | inst_jalr | inst_B_effect);
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

  // --- Target Address Control Monitor ---
`ifdef DEBUG_TARGET_ADDR_CTRL
  always @(clk) begin
    // 只有在发生跳转/分支且有效时才监控
    if (inst_jal | inst_jalr | inst_B_effect) begin
      $write("[Time:%05t] [JMP_ADDR] ", $time);

      case (1'b1)
        inst_B_effect: $write("Type: BRANCH_TAKEN | Base: PC(0x%h) + Offs: 0x%h", pc, imm);
        inst_jal:      $write("Type: JAL | Base: PC(0x%h) + Offs: 0x%h", pc, imm);
        inst_jalr:     $write("Type: JALR | Base: RS1(0x%h) + Offs: 0x%h", rs1, imm);
        default:       $write("Type: UNKNOWN | ");
      endcase

      // 打印最终计算出的目标地址
      $display(" => TARGET: 0x%h", j_or_b_target_addr);

      // 特殊检查：对于 RISC-V，跳转地址通常必须是 2 字节对齐（如果是压缩指令集）或 4 字节对齐
      if (j_or_b_target_addr[1:0] != 2'b00) begin
        $display("   [WARNING] Target address 0x%h is not 4-byte aligned!", j_or_b_target_addr);
      end
    end
  end
`endif
endmodule
