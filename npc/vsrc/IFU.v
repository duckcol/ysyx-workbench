module IFU #(
    parameter integer ADDR_LEN = 32,
    parameter integer INST_LEN = 32
) (
    input rst_l,
    input clk,
    input inst_j_or_b,
    input [ADDR_LEN-1:0] j_or_b_target_addr,
    input [ADDR_LEN-1:0] pc_addr,
    // input [INST_LEN-1:0] pmem_read_inst,
    // output [ADDR_LEN-1:0] pmem_read_addr,
    output [INST_LEN-1:0] cur_inst
);
  //  get instruction from memory
  wire [INST_LEN-1:0] pmem_read_inst;
  wire [INST_LEN-1:0] pmem_read_addr;
  assign pmem_read_addr = inst_j_or_b ? j_or_b_target_addr : pc_addr;
  import "DPI-C" function int pmem_read(input int raddr);
  assign pmem_read_inst = pmem_read(pmem_read_addr);
  Reg #(
      .WIDTH(INST_LEN),
      .RESET_VAL(32'h000000013)
  ) IR (
      .clk (clk),
      .rst (~rst_l),
      .din (pmem_read_inst),
      .dout(cur_inst),
      .wen (1'b1)
  );
  //  for monitor
`ifdef DEBUG_IFU
  always @(clk) begin
    if (rst_l) begin
      $display("[Time=%05t] [IFU] DPI-C  func pmem_read:       %08h", $time, pmem_read(
               pmem_read_addr));
      $display("[Time=%05t] [IFU] input  wire pmem_read_inst:  %08h", $time, pmem_read_inst);
      $display("[Time=%05t] [IFU] output Reg  cur_inst:        %08h", $time, cur_inst);
    end
  end
`endif

  // import DPI-C function to sync pc to CPU_state cpu
  import "DPI-C" function void sync_pc_data(input int unsigned pc);
  always @(posedge clk) begin
    if (~rst_l) begin
      sync_pc_data(32'h80000000);
    end else begin
      sync_pc_data(pmem_read_addr);
    end
  end

  //  itrace to trace instruction and sync state to Decode inst_decode
  import "DPI-C" function void trace_instruction(
    input int unsigned inst,
    input int unsigned pc,
    input int unsigned dnpc,
    input int unsigned snpc
  );
  always @(posedge clk) begin
    if (rst_l) begin
      trace_instruction(cur_inst, pc_addr - 32'd4, j_or_b_target_addr, pc_addr);
    end
  end

endmodule
