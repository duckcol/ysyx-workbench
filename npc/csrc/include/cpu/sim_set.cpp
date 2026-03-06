#include "sim_set.h"
#include "common.h"
#include "reg.h"

VerilatedContext *contextp = NULL;
VerilatedFstC *tfp = NULL;

//  TOP_NAME is ``Vantpc``, which is a macro defined by makefile
TOP_NAME *top;

int sim_init() {
  contextp = new VerilatedContext;
  tfp = new VerilatedFstC;
  top = new TOP_NAME;
  contextp->traceEverOn(true);
  top->trace(tfp, 0);
  tfp->open("top.fst");

  top->sys_rst_l = 0;
  step_times(4);
  top->sys_rst_l = 1;

  return 0;
}

int sim_exit() {
  step_and_dump_wave();
  tfp->close();
  delete tfp;
  delete top;
  delete contextp;

  return 0;
}

int step_and_dump_wave() {
  top->eval();
  if (top->fetch_inst_addr >= 0x80000000)
    top->pmem_read = pmem_read(top->fetch_inst_addr);
  top->eval();
  contextp->timeInc(1);
  tfp->dump(contextp->time());

  return 0;
};

int step_times(int n) {
  for (int i = 0; i <= n; i++) {
    // top->clk = 1;
    // step_and_dump_wave();
    // top->clk = 0;
    // step_and_dump_wave();
    top->clk = ~top->clk;
    step_and_dump_wave();
  }
  return n;
}

void cpu_exec(uint64_t n) {
  for (; n > 0; n--) {
    if (ebreak_flag == 1) {
      printf(
          "Program execution has ended. To restart the program, exit NPC and "
          "run again.\n");
      return;
    }
    step_times(1);
  }
}

int halt_ret = 1;
int ebreak_flag = 0;
extern "C" void trigger_ebreak() {
  INFO("triggering inst ebreak");
  ebreak_flag = 1;

  // check $a0 or R(10) to see if it is 0
  top->debug_reg_addr = 10;
  word_t reg10_data = top->debug_reg_data;
  INFO("current reg10 data: %08x", reg10_data);
  halt_ret = reg10_data;
}

extern "C" void sync_rf_data(uint32_t addr, uint32_t data) {
  INFO("sync reg data");
  gpr(addr) = data;
  INFO("sync reg ends");
  return;
}

extern "C" void sync_pc_data(uint32_t pc) {
  INFO("sync pc data");
  cpu.pc = pc;
  INFO("sync reg ends");
  return;
}

extern "C" void disassemble(char *str, int size, uint64_t pc, uint8_t *code,
                            int nbyte);
std::string get_asm_mnemonic(uint32_t inst, uint32_t pc) {
  char buf[128];
  uint8_t bytes[4];

  // 注意端序！RISC-V 是小端序 (Little Endian)
  // inst 是 uint32_t，内存中低位在前
  bytes[0] = inst & 0xFF;
  bytes[1] = (inst >> 8) & 0xFF;
  bytes[2] = (inst >> 16) & 0xFF;
  bytes[3] = (inst >> 24) & 0xFF;

  disassemble(buf, sizeof(buf), pc, bytes, 4);

  return std::string(buf);
}

extern "C" void trace_instruction(word_t inst, word_t pc) {
  //  disassemle the inst and print it out
  std::string asm_str = get_asm_mnemonic((uint32_t)inst, (uint32_t)pc);
  _Log("" FMT_WORD ":\t" FMT_WORD "\t%s\n", pc, inst, asm_str.c_str());
}
