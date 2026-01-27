#include "Vantpc.h"
#include "Vantpc__Dpi.h"
#include "pmem.h"
#include "svdpi.h"
#include <verilated.h>
#include <verilated_fst_c.h>

VerilatedContext *contextp = NULL;
VerilatedFstC *tfp = NULL;

//  TOP_NAME = "Vantpc"
TOP_NAME *top;

#define step_and_dump_wave()                                                   \
  top->eval();                                                                 \
  if (top->fetch_inst_addr >= 0x80000000)                                      \
    top->pmem_read = pmem_read(top->fetch_inst_addr);                          \
  top->eval();                                                                 \
  contextp->timeInc(1);                                                        \
  tfp->dump(contextp->time());

#define sim_init()                                                             \
  contextp = new VerilatedContext;                                             \
  tfp = new VerilatedFstC;                                                     \
  top = new TOP_NAME;                                                          \
  contextp->traceEverOn(true);                                                 \
  top->trace(tfp, 0);                                                          \
  tfp->open("top.fst");

#define sim_exit()                                                             \
  step_and_dump_wave();                                                        \
  tfp->close();                                                                \
  delete tfp;                                                                  \
  delete top;                                                                  \
  delete contextp;

#define step_times(n)                                                          \
  for (int i = 0; i <= n; i++) {                                               \
    top->clk = 1;                                                              \
    step_and_dump_wave();                                                      \
    top->clk = 0;                                                              \
    step_and_dump_wave();                                                      \
  }
