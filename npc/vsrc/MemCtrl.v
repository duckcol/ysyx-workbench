module MemCtrl #(
    parameter integer DATA_LEN = 32
) (
    input  [DATA_LEN-1:0] alu_result_addr,
    input  [DATA_LEN-1:0] rs2,
    input                 inst_L,
    input                 inst_S,
    input  [         2:0] funct3,
    output [DATA_LEN-1:0] read_result
);
  import "DPI-C" function void pmem_write(
    input int  waddr,
    input int  wdata,
    input byte wmask
  );
  import "DPI-C" function int pmem_read(input int raddr);

  wire [7:0] pmem_write_mask;
  MuxKeyWithDefault #(
      .NR_KEY  (3),
      .KEY_LEN (1 + 3),
      .DATA_LEN(8)
  ) pmem_write_mask_mux (
      .out(pmem_write_mask),
      .key({inst_S, funct3}),
      .default_out({(8) {1'b0}}),
      .lut({
        // format:
        // inst
        // inst_S
        // funct3
        // mask
        // sb
        1'b1,
        3'h0,
        8'h01,
        // sw
        1'b1,
        3'h1,
        8'h03,
        // sw
        1'b1,
        3'h2,
        8'h0F
      })
  );

  reg [DATA_LEN-1:0] read_data;
  always @(*) begin
    if (inst_L | inst_S) begin  // 有读写请求时
      $display("pmem_addr:%x", alu_result_addr);
      read_data = pmem_read(alu_result_addr);
      if (inst_S) begin  // 有写请求时
        pmem_write(alu_result_addr, rs2, pmem_write_mask);
      end
    end else begin
      read_data = 0;
    end
  end

  MuxKeyWithDefault #(
      .NR_KEY  (5),
      .KEY_LEN (1 + 3),
      .DATA_LEN(DATA_LEN)
  ) pmem_read_mask_mux (
      .out(read_result),
      .key({inst_L, funct3}),
      .default_out({(DATA_LEN) {1'b0}}),
      .lut({
        // format:
        // // inst
        // inst_L
        // funct3
        // read_data_masked
        // lb
        1'b1,
        3'h0,
        {{(DATA_LEN - 7) {read_data[7]}}, read_data[6:0]},
        // lh
        1'b1,
        3'h1,
        {{(DATA_LEN - 15) {read_data[15]}}, read_data[14:0]},
        // lw
        1'b1,
        3'h2,
        read_data,
        // lbu
        1'b1,
        3'h4,
        {{(DATA_LEN - 8) {1'b0}}, read_data[7:0]},
        // lhu
        1'b1,
        3'h5,
        {{(DATA_LEN - 16) {1'b0}}, read_data[15:0]}
      })
  );
endmodule
