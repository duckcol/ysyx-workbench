module IFU #(
    parameter integer ADDR_LEN = 32,
    parameter integer INST_LEN = 32
) (
    input rst_l,
    input clk,
    input inst_j_or_s,
    input [ADDR_LEN-1:0] j_or_s_target_addr,
    input [ADDR_LEN-1:0] pc_addr,
    input [INST_LEN-1:0] pmem_read_inst,
    output [ADDR_LEN-1:0] pmem_read_addr,
    output [INST_LEN-1:0] cur_inst
);
  Reg #(
      .WIDTH(INST_LEN),
      .RESET_VAL(32'd0)
  ) IR (
      .clk (clk),
      .rst (~rst_l),
      .din (pmem_read_inst),
      .dout(cur_inst),
      .wen (1'b1)
  );
  assign pmem_read_addr = inst_j_or_s ? j_or_s_target_addr : pc_addr;
  import "DPI-C" function int pmem_read(input int raddr);
  always @(*) begin
    if (rst_l) begin
      $display("time:%03t DPI-C  func pmem_read:       %08h", $time, pmem_read(pmem_read_addr));
      $display("time:%03t input  wire pmem_read_inst:  %08h", $time, pmem_read_inst);
      $display("time:%03t output Reg  cur_inst:        %08h", $time, cur_inst);
    end
  end

  // 导入DPI-C函数，来同步pc
  import "DPI-C" function void sync_pc_data(input int unsigned pc);

  // 写操作时同步到C侧
  always @(posedge clk) begin
    if (~rst_l) begin
      sync_pc_data(32'h80000000);
    end else begin
      sync_pc_data(pmem_read_addr);
    end
  end

  //  itrace
  import "DPI-C" function void trace_instruction(
    input int unsigned inst,
    input int unsigned pc,
    input int unsigned dnpc,
    input int unsigned snpc
  );
  always @(posedge clk) begin
    if (rst_l) begin
      trace_instruction(cur_inst, pc_addr - 32'd4, j_or_s_target_addr, pc_addr);
    end
  end

endmodule
