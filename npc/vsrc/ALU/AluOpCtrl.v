`include "vsrc/vsrc_conf.h.v"

module AluOpCtrl #(
    parameter integer ALUOP_LEN  = 4,
    parameter integer OPCODE_LEN = 7
) (
    input                   clk,
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
    output                  inst_ecall,
    output                  inst_mret,
    output                  inst_csr,
    output                  inst_illegal
);

  // --- 严谨的归一化逻辑 (Safe Normalization) ---
  // 将不需要的funct7和funct3化为0

  // 1. 判断是否需要 funct7
  wire use_f7 =
  // inst_commpute_reg
  (opcode == 7'h33) ||
  // inst_commpute_imm shift
  (opcode == 7'h13 && (funct3 == 3'h1 || funct3 == 3'h5)) ||
  // inst_ecall and inst_mret
  (opcode == 7'h73 && (funct3 == 3'h0));

  wire [6:0] f7_final = use_f7 ? funct7 : 7'h00;

  // 2. 判断是否需要 funct3 (Load/Store/LUI/AUIPC/JAL 在 ALU 阶段都只做加法，不依赖 f3)
  // 注意：B-type 依然需要 f3 来区分 beq/bne/blt 等 ALU 比较操作
  wire use_f3 =
  // inst_commpute_imm
  (opcode == 7'h13) ||
  // inst_commpute_reg
  (opcode == 7'h33) ||
  // inst_B
  (opcode == 7'h63) ||
  // inst_csr
  (opcode == 7'h73);

  wire [2:0] f3_final = use_f3 ? funct3 : 3'h0;

  wire decode_hit;

  localparam InstLhit = 12'b100_000_000_000;
  localparam InstShit = 12'b010_000_000_000;
  localparam InstComputeImmhit = 12'b001_000_000_000;
  localparam InstComputeReghit = 12'b000_100_000_000;
  localparam InstLUIhit = 12'b000_010_000_000;
  localparam InstAUIPChit = 12'b000_001_000_000;
  localparam InstBhit = 12'b000_000_100_000;
  localparam InstJALhit = 12'b000_000_010_000;
  localparam InstJALRhit = 12'b000_000_001_000;
  localparam InstECALLhit = 12'b000_000_000_100;
  localparam InstMREThit = 12'b000_000_000_010;
  localparam InstCSRhit = 12'b000_000_000_001;

  MuxKeyWithDefault #(
      .NR_KEY  (36),
      .KEY_LEN (OPCODE_LEN + 3 + 7),
      .DATA_LEN(ALUOP_LEN + 12 + 1)
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
        inst_ecall,
        inst_mret,
        inst_csr,
        decode_hit
      }),
      .key({opcode, f3_final, f7_final}),
      .default_out(17'd0),
      .lut({
        // -----------------------------------------------------------------------
        // 格式:
        // {
        //    {opcode, f3, f7},     (key)
        //    {alu_sel, flags, hit} (out)
        // }
        // -----------------------------------------------------------------------

        // Load & Store (归一化后 f3=0, f7=0)
        {
          {7'h03, 3'h0, 7'h00}, {4'd1, InstLhit, 1'b1}  // Load (LB/LW/...)
        },
        {
          {7'h23, 3'h0, 7'h00}, {4'd1, InstShit, 1'b1}  // Store (SB/SW/...)
        },

        // I-Type Compute (依赖 f3, 移位指令依赖 f7)
        {
          {7'h13, 3'h0, 7'h00}, {4'd1, InstComputeImmhit, 1'b1}  // addi
        },
        {
          {7'h13, 3'h2, 7'h00}, {4'd11, InstComputeImmhit, 1'b1}  // slti
        },
        {
          {7'h13, 3'h3, 7'h00}, {4'd12, InstComputeImmhit, 1'b1}  // sltiu
        },
        {
          {7'h13, 3'h4, 7'h00}, {4'd3, InstComputeImmhit, 1'b1}  // xori
        },
        {
          {7'h13, 3'h6, 7'h00}, {4'd5, InstComputeImmhit, 1'b1}  // ori
        },
        {
          {7'h13, 3'h7, 7'h00}, {4'd4, InstComputeImmhit, 1'b1}  // andi
        },
        {
          {7'h13, 3'h1, 7'h00}, {4'd6, InstComputeImmhit, 1'b1}  // slli
        },
        {
          {7'h13, 3'h5, 7'h00}, {4'd10, InstComputeImmhit, 1'b1}  // srli
        },
        {
          {7'h13, 3'h5, 7'h20}, {4'd9, InstComputeImmhit, 1'b1}  // srai
        },

        // R-Type Compute (依赖 f3 和 f7)
        {
          {7'h33, 3'h0, 7'h00}, {4'd1, InstComputeReghit, 1'b1}  // add
        },
        {
          {7'h33, 3'h0, 7'h20}, {4'd2, InstComputeReghit, 1'b1}  // sub
        },
        {
          {7'h33, 3'h1, 7'h00}, {4'd6, InstComputeReghit, 1'b1}  // sll
        },
        {
          {7'h33, 3'h2, 7'h00}, {4'd11, InstComputeReghit, 1'b1}  // slt
        },
        {
          {7'h33, 3'h3, 7'h00}, {4'd12, InstComputeReghit, 1'b1}  // sltu
        },
        {
          {7'h33, 3'h4, 7'h00}, {4'd3, InstComputeReghit, 1'b1}  // xor
        },
        {
          {7'h33, 3'h5, 7'h00}, {4'd10, InstComputeReghit, 1'b1}  // srl
        },
        {
          {7'h33, 3'h5, 7'h20}, {4'd9, InstComputeReghit, 1'b1}  // sra
        },
        {
          {7'h33, 3'h6, 7'h00}, {4'd5, InstComputeReghit, 1'b1}  // or
        },
        {
          {7'h33, 3'h7, 7'h00}, {4'd4, InstComputeReghit, 1'b1}  // and
        },

        // U-Type (归一化后 f3=0, f7=0)
        {
          {7'h37, 3'h0, 7'h00}, {4'd1, InstLUIhit, 1'b1}  // lui
        },
        {
          {7'h17, 3'h0, 7'h00}, {4'd1, InstAUIPChit, 1'b1}  // auipc
        },

        // B-Type (依赖 f3)
        {
          {7'h63, 3'h0, 7'h00}, {4'd7, InstBhit, 1'b1}  // beq
        },
        {
          {7'h63, 3'h1, 7'h00}, {4'd8, InstBhit, 1'b1}  // bne
        },
        {
          {7'h63, 3'h4, 7'h00}, {4'd11, InstBhit, 1'b1}  // blt
        },
        {
          {7'h63, 3'h5, 7'h00}, {4'd13, InstBhit, 1'b1}  // bge
        },
        {
          {7'h63, 3'h6, 7'h00}, {4'd12, InstBhit, 1'b1}  // bltu
        },
        {
          {7'h63, 3'h7, 7'h00}, {4'd14, InstBhit, 1'b1}  // bgeu
        },

        // J-Type & I-Jump (归一化后 f3=0, f7=0)
        {
          {7'h6f, 3'h0, 7'h00}, {4'd1, InstJALhit, 1'b1}  // jal
        },
        {
          {7'h67, 3'h0, 7'h00}, {4'd1, InstJALRhit, 1'b1}  // jalr
        },

        // ecall(依赖f3)
        {
          {7'h73, 3'h0, 7'h00}, {4'd1, InstECALLhit, 1'b1}
        },

        // mret(依赖f7)
        {
          {7'h73, 3'h0, 7'h18}, {4'd1, InstMREThit, 1'b1}
        },

        // csrr(依赖f3)
        {
          {7'h73, 3'h1, 7'h00}, {4'd1, InstCSRhit, 1'b1}  // carrw
        },
        {
          {7'h73, 3'h2, 7'h00}, {4'd1, InstCSRhit, 1'b1}  // csrrs
        },
        {
          {7'h73, 3'h3, 7'h00}, {4'd1, InstCSRhit, 1'b1}  // csrrc
        }
      })
  );

  assign inst_illegal = ~decode_hit;
  // --- AluOpCtrl Monitor ---
`ifdef DEBUG_ALUOPCTRL
  always @(posedge clk) begin
    // 1. 捕捉非法指令 (关键！)
    if (inst_illegal) begin
      $display(
          "[Time:%05t] [ALU_OP_CTRL_ERROR] Illegal Instruction! Opcode: 7'h%h, F3: 3'h%h, F7: 7'h%h",
          $time, opcode, funct3, funct7);
    end  // 2. 正常指令译码跟踪
    else if (|{
      inst_L,
      inst_S,
      inst_commpute_imm,
      inst_commpute_reg,
      inst_lui,
      inst_auipc,
      inst_B,
      inst_jal,
      inst_jalr,
      inst_ecall,
      inst_mret,
      inst_csr
      })
      begin
      $write("[Time:%05t] [ALU_OP_CTRL] ", $time);

      // 使用 case(1'b1) 判定当前指令大类
      case (1'b1)
        inst_L:            $write("Type: LOAD   ");
        inst_S:            $write("Type: STORE  ");
        inst_commpute_imm: $write("Type: I-ALU  ");
        inst_commpute_reg: $write("Type: R-ALU  ");
        inst_lui:          $write("Type: LUI    ");
        inst_auipc:        $write("Type: AUIPC  ");
        inst_B:            $write("Type: BRANCH ");
        inst_jal:          $write("Type: JAL    ");
        inst_jalr:         $write("Type: JALR   ");
        inst_ecall:        $write("Type: ECALL  ");
        inst_mret:         $write("Type: MRET   ");
        inst_csr:          $write("Type: CSR    ");
      endcase

      // 打印具体的控制参数
      $display("| AluSel: %2d | Raw: {Op:0x%h, F3:0x%h, F7:0x%h}", alu_sel, opcode, funct3, funct7);
    end
  end
`endif
endmodule
