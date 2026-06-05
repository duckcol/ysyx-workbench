// vsrc/vsrc_conf.h
`ifndef _VSRC_CONF_H_
`define _VSRC_CONF_H_
`endif

`define NPC_MEM_BASE 32'h80000000
`define NPC_IR_RST_VAL 32'h000000013

`define NPC_MSTATUS_RST_VAL 32'h1800

// some data len setting
`define NPC_ADDR_LEN 32
`define NPC_INST_LEN 32
`define NPC_REG_ADDR_LEN 5
`define NPC_CSR_ADDR_LEN 12
`define OPCODE_LEN 7

// debug print info
// `define DEBUG_IFU
// `define DEBUG_IDU
// `define DEBUG_CSR
// `define DEBUG_ALUOPCTRL
// `define DEBUG_ALUINCTRL
// `define DEBUG_ALU
// `define DEBUG_inst_j_or_b
// `define DEBUG_TARGET_ADDR_CTRL
// `define DEBUG_MEMCTRL
