`include "vsrc/vsrc_conf.h.v"

module CSRctrl #(
    parameter integer DATA_LEN = 32,
    parameter integer ADDR_LEN = 12
) (
    input  [ADDR_LEN-1:0] csr_addr,
    input  [DATA_LEN-1:0] rs1,
    input  [DATA_LEN-1:0] pc,
    output [DATA_LEN-1:0] csr_val,
    output [DATA_LEN-1:0] mtvec_val,
    output [DATA_LEN-1:0] mepc_val,

    input clk,
    input rst,
    input inst_ecall,
    input inst_mret,
    input inst_csr,
    input [2:0] csr_funct3
);
  wire [DATA_LEN-1:0] mstatus_val, mcause_val;
  wire [DATA_LEN-1:0] mtvec_din, mstatus_din, mepc_din, mcause_din;
  wire mtvec_wen, mstatus_wen, mepc_wen, mcause_wen;
  wire [DATA_LEN-1:0] csr_din;

  //  when inst_csr valid,
  //  get csr_val of current csr_addr
  MuxKeyWithDefault #(
      .NR_KEY  (4),
      .KEY_LEN (1 + 12),
      .DATA_LEN(DATA_LEN)
  ) csr_val_mux (
      .out(csr_val),
      .key({inst_csr, csr_addr}),
      .default_out({DATA_LEN{1'b0}}),
      .lut({
        {{1'b1, 12'h305}, mtvec_val},
        {{1'b1, 12'h300}, mstatus_val},
        {{1'b1, 12'h341}, mepc_val},
        {{1'b1, 12'h342}, mcause_val}
      })
  );

  //  when inst_ecall valid
  //  get csr_din of current csr_addr
  MuxKeyWithDefault #(
      .NR_KEY  (3),
      .KEY_LEN (1 + 3),
      .DATA_LEN(DATA_LEN)
  ) csr_din_mux (
      .out(csr_din),
      .key({inst_csr, csr_funct3}),
      .default_out({DATA_LEN{1'b0}}),
      .lut({
        {{1'b1, 3'h1}, rs1},  // csrrw
        {{1'b1, 3'h2}, csr_val | rs1},  // csrrs
        {{1'b1, 3'h3}, csr_val & (~rs1)}  // csrrc
      })
  );

  //  setting wen for CSRs
  assign mtvec_wen   = (csr_addr == 12'h305) & (inst_csr);
  assign mstatus_wen = (csr_addr == 12'h300) & (inst_csr);
  wire mepc_wen_csr = ((csr_addr == 12'h341) & (inst_csr));
  assign mepc_wen = mepc_wen_csr | inst_ecall;
  wire mcause_wen_csr = ((csr_addr == 12'h342) & (inst_csr));
  assign mcause_wen = mepc_wen_csr | inst_ecall;

  //  setting din for CSRs
  assign mtvec_din = (mtvec_wen) ? csr_din : {DATA_LEN{1'b0}};
  assign mstatus_din = (mstatus_wen) ? csr_din : {DATA_LEN{1'b0}};
  assign mepc_din = (mepc_wen_csr) ? csr_din : (inst_ecall) ? pc : {DATA_LEN{1'b0}};
  assign mcause_din = (mcause_wen_csr) ? csr_din : (inst_ecall) ? 32'h11 : {DATA_LEN{1'b0}};

  Reg #(
      .WIDTH    (DATA_LEN),
      .RESET_VAL({DATA_LEN{1'b0}})
  ) mtvec (
      .clk (clk),
      .rst (rst),
      .din (mtvec_din),
      .dout(mtvec_val),
      .wen (mtvec_wen)
  );
  Reg #(
      .WIDTH    (DATA_LEN),
      .RESET_VAL(`NPC_MSTATUS_RST_VAL)
  ) mstatus (
      .clk (clk),
      .rst (rst),
      .din (mstatus_din),
      .dout(mstatus_val),
      .wen (mstatus_wen)
  );
  Reg #(
      .WIDTH    (DATA_LEN),
      .RESET_VAL({DATA_LEN{1'b0}})
  ) mepc (
      .clk (clk),
      .rst (rst),
      .din (mepc_din),
      .dout(mepc_val),
      .wen (mepc_wen)
  );
  Reg #(
      .WIDTH    (DATA_LEN),
      .RESET_VAL({DATA_LEN{1'b0}})
  ) mcause (
      .clk (clk),
      .rst (rst),
      .din (mcause_din),
      .dout(mcause_val),
      .wen (mcause_wen)
  );

`ifdef DEBUG_CSR
  string cur_csr_name = "";
  always @(*) begin
    case (csr_addr)
      12'h305: cur_csr_name = "MTVEC  ";
      12'h300: cur_csr_name = "MSTATUS";
      12'h341: cur_csr_name = "MEPC   ";
      12'h342: cur_csr_name = "MCAUSE ";
      default: cur_csr_name = "INVALID";
    endcase
  end

  always @(posedge clk) begin
    if (rst) begin
      $display("[Time=%05t] [CSR] reset asserted", $time);
    end else begin
      if (inst_csr) begin
        $display("[Time=%05t] [CSR] | %s=0x%08x", $time, cur_csr_name, csr_din);
      end else begin
        if (inst_ecall) begin
          $display("[Time=%05t] [ECALL] | MPEC=0x%08x MCAUSE=0x%08x", $time, mepc_din, mcause_din);
        end
      end
    end
  end
`endif


endmodule
