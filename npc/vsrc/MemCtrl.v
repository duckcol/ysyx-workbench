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

  // 1. 计算对齐地址 (去掉低2位)
  wire [DATA_LEN-1:0] aligned_addr = {alu_result_addr[DATA_LEN-1:2], 2'b00};
  // 2. 提取地址偏移 (0, 1, 2, 3)
  wire [1:0] addr_offset = alu_result_addr[1:0];

  // NOTE:
  // addr misaligned handle logic is related to CSR,
  // which is not implemented yet,
  // so I just leave a note here.
  // RV32 require:
  // lw/sw is 4-byte alignment
  // lh/lhu/sh is 2-byte alignment
  // lb/lbu/sb is 1-byte alignment
  // So far, I only calculate shift situation
  // for lb/lbu/sb

  // 3. 计算写逻辑掩码与写入数据
  wire [7:0] pmem_write_mask;
  MuxKeyWithDefault #(
      .NR_KEY  (12),
      .KEY_LEN (1 + 3 + 2),
      .DATA_LEN(8)
  ) pmem_write_mask_mux (
      .out(pmem_write_mask),
      .key({inst_S, funct3, addr_offset}),
      .default_out({(8) {1'b0}}),
      .lut({
        // format:
        // // inst, offset
        // inst_S
        // funct3
        // addr_offset
        // mask
        // sb, 0
        1'b1,
        3'h0,
        2'd0,
        8'h01,
        // sb, 1
        1'b1,
        3'h0,
        2'd1,
        8'h02,
        // sb, 2
        1'b1,
        3'h0,
        2'd2,
        8'h04,
        // sb, 3
        1'b1,
        3'h0,
        2'd3,
        8'h08,
        // sh: 根据 offset[1] 选择 2 字节
        1'b1,
        3'h1,
        2'd0,
        8'h03,
        1'b1,
        3'h1,
        2'd1,
        8'h03,
        1'b1,
        3'h1,
        2'd2,
        8'h0C,
        1'b1,
        3'h1,
        2'd3,
        8'h0C,
        // sw: 总是 4 字节
        1'b1,
        3'h2,
        2'd0,
        8'h0F,
        1'b1,
        3'h2,
        2'd1,
        8'h0F,
        1'b1,
        3'h2,
        2'd2,
        8'h0F,
        1'b1,
        3'h2,
        2'd3,
        8'h0F
      })
  );

  reg [DATA_LEN-1:0] read_data;
  always @(*) begin
    if (inst_L | inst_S) begin  // 有读写请求时
      $display("pmem_addr: real=%x, aligned=%x, offset=%d", alu_result_addr, aligned_addr,
               addr_offset);
      read_data = pmem_read(alu_result_addr);
      if (inst_S) begin  // 有写请求时
        pmem_write(alu_result_addr, rs2 << (addr_offset * 8), pmem_write_mask);
      end
    end else begin
      read_data = 0;
    end
  end

  MuxKeyWithDefault #(
      .NR_KEY  (20),         // 有效条目数
      .KEY_LEN (1 + 3 + 2),  // inst_L + funct3 + addr_offset
      .DATA_LEN(DATA_LEN)
  ) pmem_read_result_mux (
      .out(read_result),
      .key({inst_L, funct3, addr_offset}),
      .default_out({(DATA_LEN) {1'b0}}),
      .lut({
        // ==================== lb (funct3=0) ====================
        // inst_L=1, funct3=0, offset=0
        1'b1,
        3'h0,
        2'd0,
        {{(DATA_LEN - 8) {read_data[7]}}, read_data[7:0]},
        // inst_L=1, funct3=0, offset=1
        1'b1,
        3'h0,
        2'd1,
        {{(DATA_LEN - 8) {read_data[15]}}, read_data[15:8]},
        // inst_L=1, funct3=0, offset=2
        1'b1,
        3'h0,
        2'd2,
        {{(DATA_LEN - 8) {read_data[23]}}, read_data[23:16]},
        // inst_L=1, funct3=0, offset=3
        1'b1,
        3'h0,
        2'd3,
        {{(DATA_LEN - 8) {read_data[31]}}, read_data[31:24]},

        // ==================== lh (funct3=1) ====================
        // inst_L=1, funct3=1, offset[1]=0 (offset=0,1)
        1'b1,
        3'h1,
        2'd0,
        {{(DATA_LEN - 16) {read_data[15]}}, read_data[15:0]},
        1'b1,
        3'h1,
        2'd1,
        {{(DATA_LEN - 16) {read_data[15]}}, read_data[15:0]},
        // inst_L=1, funct3=1, offset[1]=1 (offset=2,3)
        1'b1,
        3'h1,
        2'd2,
        {{(DATA_LEN - 16) {read_data[31]}}, read_data[31:16]},
        1'b1,
        3'h1,
        2'd3,
        {{(DATA_LEN - 16) {read_data[31]}}, read_data[31:16]},

        // ==================== lw (funct3=2) ====================
        // inst_L=1, funct3=2, offset=0 (理论上应该只有这一种)
        1'b1,
        3'h2,
        2'd0,
        read_data,
        // 为安全起见，其他 offset 也返回完整数据（实际应该产生异常）
        1'b1,
        3'h2,
        2'd1,
        read_data,
        1'b1,
        3'h2,
        2'd2,
        read_data,
        1'b1,
        3'h2,
        2'd3,
        read_data,

        // ==================== lbu (funct3=4) ====================
        // inst_L=1, funct3=4, offset=0
        1'b1,
        3'h4,
        2'd0,
        {{(DATA_LEN - 8) {1'b0}}, read_data[7:0]},
        // inst_L=1, funct3=4, offset=1
        1'b1,
        3'h4,
        2'd1,
        {{(DATA_LEN - 8) {1'b0}}, read_data[15:8]},
        // inst_L=1, funct3=4, offset=2
        1'b1,
        3'h4,
        2'd2,
        {{(DATA_LEN - 8) {1'b0}}, read_data[23:16]},
        // inst_L=1, funct3=4, offset=3
        1'b1,
        3'h4,
        2'd3,
        {{(DATA_LEN - 8) {1'b0}}, read_data[31:24]},

        // ==================== lhu (funct3=5) ====================
        // inst_L=1, funct3=5, offset[1]=0 (offset=0,1)
        1'b1,
        3'h5,
        2'd0,
        {{(DATA_LEN - 16) {1'b0}}, read_data[15:0]},
        1'b1,
        3'h5,
        2'd1,
        {{(DATA_LEN - 16) {1'b0}}, read_data[15:0]},
        // inst_L=1, funct3=5, offset[1]=1 (offset=2,3)
        1'b1,
        3'h5,
        2'd2,
        {{(DATA_LEN - 16) {1'b0}}, read_data[31:16]},
        1'b1,
        3'h5,
        2'd3,
        {{(DATA_LEN - 16) {1'b0}}, read_data[31:16]}
      })
  );
endmodule
