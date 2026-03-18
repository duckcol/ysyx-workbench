module AluOpCtrl #(
    parameter integer ALUOP_LEN  = 4,
    parameter integer OPCODE_LEN = 7
) (
    input  [OPCODE_LEN-1:0] opcode,
    input  [           2:0] funct3,
    input  [           6:0] funct7,
    output [ ALUOP_LEN-1:0] alu_sel,
    output                  inst_L,
    output                  inst_S,
    output                  inst_commpute_imm,
    output                  inst_commpute_reg,
    output                  inst_lui,
    output                  inst_auipc,
    output                  inst_B,
    output                  inst_jal,
    output                  inst_jalr,
    output                  inst_illegal
);

  // --- 严谨的归一化逻辑 (Safe Normalization) ---

  // 1. 判断是否需要 funct7 (只有 R-type 和 I-type 移位指令需要)
  wire use_f7 = (opcode == 7'h33) || (opcode == 7'h13 && (funct3 == 3'h1 || funct3 == 3'h5));
  wire [6:0] f7_final = use_f7 ? funct7 : 7'h00;

  // 2. 判断是否需要 funct3 (Load/Store/LUI/AUIPC/JAL 在 ALU 阶段都只做加法，不依赖 f3)
  // 注意：B-type 依然需要 f3 来区分 beq/bne/blt 等 ALU 比较操作
  wire use_f3 = (opcode == 7'h13) || (opcode == 7'h33) || (opcode == 7'h63);
  wire [2:0] f3_final = use_f3 ? funct3 : 3'h0;

  wire decode_hit;

  MuxKeyWithDefault #(
      .NR_KEY  (31),
      .KEY_LEN (OPCODE_LEN + 3 + 7),
      .DATA_LEN(ALUOP_LEN + 9 + 1)
  ) control_alu_op (
      .out({
        alu_sel,
        inst_L,
        inst_S,
        inst_commpute_imm,
        inst_commpute_reg,
        inst_lui,
        inst_auipc,
        inst_B,
        inst_jal,
        inst_jalr,
        decode_hit
      }),
      .key({opcode, f3_final, f7_final}),
      .default_out(14'd0),
      .lut({
        // -----------------------------------------------------------------------
        // 格式: {opcode, f3, f7}, {alu_sel, flags, hit}
        // -----------------------------------------------------------------------

        // Load & Store (归一化后 f3=0, f7=0)
        {
          7'h03, 3'h0, 7'h00
        },
        {4'd1, 9'b100_000_000, 1'b1},  // Load (LB/LW/...)
        {7'h23, 3'h0, 7'h00},
        {4'd1, 9'b010_000_000, 1'b1},  // Store (SB/SW/...)

        // I-Type Compute (依赖 f3, 移位指令依赖 f7)
        {
          7'h13, 3'h0, 7'h00
        },
        {4'd1, 9'b001_000_000, 1'b1},  // addi
        {7'h13, 3'h2, 7'h00},
        {4'd11, 9'b001_000_000, 1'b1},  // slti
        {7'h13, 3'h3, 7'h00},
        {4'd12, 9'b001_000_000, 1'b1},  // sltiu
        {7'h13, 3'h4, 7'h00},
        {4'd3, 9'b001_000_000, 1'b1},  // xori
        {7'h13, 3'h6, 7'h00},
        {4'd5, 9'b001_000_000, 1'b1},  // ori
        {7'h13, 3'h7, 7'h00},
        {4'd4, 9'b001_000_000, 1'b1},  // andi
        {7'h13, 3'h1, 7'h00},
        {4'd6, 9'b001_000_000, 1'b1},  // slli
        {7'h13, 3'h5, 7'h00},
        {4'd10, 9'b001_000_000, 1'b1},  // srli
        {7'h13, 3'h5, 7'h20},
        {4'd9, 9'b001_000_000, 1'b1},  // srai

        // R-Type Compute (依赖 f3 和 f7)
        {
          7'h33, 3'h0, 7'h00
        },
        {4'd1, 9'b000_100_000, 1'b1},  // add
        {7'h33, 3'h0, 7'h20},
        {4'd2, 9'b000_100_000, 1'b1},  // sub
        {7'h33, 3'h1, 7'h00},
        {4'd6, 9'b000_100_000, 1'b1},  // sll
        {7'h33, 3'h2, 7'h00},
        {4'd11, 9'b000_100_000, 1'b1},  // slt
        {7'h33, 3'h3, 7'h00},
        {4'd12, 9'b000_100_000, 1'b1},  // sltu
        {7'h33, 3'h4, 7'h00},
        {4'd3, 9'b000_100_000, 1'b1},  // xor
        {7'h33, 3'h5, 7'h00},
        {4'd10, 9'b000_100_000, 1'b1},  // srl
        {7'h33, 3'h5, 7'h20},
        {4'd9, 9'b000_100_000, 1'b1},  // sra
        {7'h33, 3'h6, 7'h00},
        {4'd5, 9'b000_100_000, 1'b1},  // or
        {7'h33, 3'h7, 7'h00},
        {4'd4, 9'b000_100_000, 1'b1},  // and

        // U-Type (归一化后 f3=0, f7=0)
        {
          7'h37, 3'h0, 7'h00
        },
        {4'd1, 9'b000_010_000, 1'b1},  // lui
        {7'h17, 3'h0, 7'h00},
        {4'd1, 9'b000_001_000, 1'b1},  // auipc

        // B-Type (依赖 f3)
        {
          7'h63, 3'h0, 7'h00
        },
        {4'd7, 9'b000_000_100, 1'b1},  // beq
        {7'h63, 3'h1, 7'h00},
        {4'd8, 9'b000_000_100, 1'b1},  // bne
        {7'h63, 3'h4, 7'h00},
        {4'd11, 9'b000_000_100, 1'b1},  // blt
        {7'h63, 3'h5, 7'h00},
        {4'd13, 9'b000_000_100, 1'b1},  // bge
        {7'h63, 3'h6, 7'h00},
        {4'd12, 9'b000_000_100, 1'b1},  // bltu
        {7'h63, 3'h7, 7'h00},
        {4'd14, 9'b000_000_100, 1'b1},  // bgeu

        // J-Type & I-Jump (归一化后 f3=0, f7=0)
        {
          7'h6f, 3'h0, 7'h00
        },
        {4'd1, 9'b000_000_010, 1'b1},  // jal
        {7'h67, 3'h0, 7'h00},
        {4'd1, 9'b000_000_001, 1'b1}  // jalr
      })
  );

  assign inst_illegal = ~decode_hit;

endmodule
