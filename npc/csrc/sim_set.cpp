#include "sim_set.h"

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
void trigger_ebreak() {
  INFO("triggering inst ebreak");
  ebreak_flag = 1;

  // check $a0 or R(10) to see if it is 0
  top->debug_reg_addr = 10;
  word_t reg10_data = top->debug_reg_data;
  INFO("current reg10 data: %08x", reg10_data);
  halt_ret = reg10_data;
}
