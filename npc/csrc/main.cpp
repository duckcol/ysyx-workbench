#include "Vantpc.h"
#include "Vantpc__Dpi.h"
#include "pmem.h"
#include "svdpi.h"
#include <verilated.h>
#include <verilated_fst_c.h>

VerilatedContext *contextp = NULL;
VerilatedFstC *tfp = NULL;

//  TOP_NAME = "Vantpc"
static TOP_NAME *top;

//  ebreak
int ebreak_flag = 0;
extern int ebreak_flag;
void trigger_ebreak() {
  INFO("triggering inst ebreak");
  ebreak_flag = 1;
}

void step_and_dump_wave() {
  top->eval();
  if (top->fetch_inst_addr >= 0x80000000)
    top->pmem_read = pmem_read(top->fetch_inst_addr);
  top->eval();
  contextp->timeInc(1);
  tfp->dump(contextp->time());
}

void sim_init() {
  contextp = new VerilatedContext;
  tfp = new VerilatedFstC;
  top = new TOP_NAME;
  contextp->traceEverOn(true);
  top->trace(tfp, 0);
  tfp->open("top.fst");
}

void sim_exit() {
  step_and_dump_wave();
  tfp->close();
  delete tfp;
  delete top;
  delete contextp;
}

void step_times(int n) {
  for (int i = 0; i <= n; i++) {
    top->clk = 1;
    step_and_dump_wave();
    top->clk = 0;
    step_and_dump_wave();
  }
}

int main() {
  //  initial cpu pc counter
  sim_init();
  pmem_initial();
  top->sys_rst_l = 0;
  step_times(4);

  //  read in inst and run
  top->sys_rst_l = 1;
  for (int i = 0; i < 10; i++) {
    step_times(1);
    if (ebreak_flag == 1)
      break;
  }

  sim_exit();

  return 0;
}
