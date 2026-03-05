#include "Vantpc.h"
#include "Vantpc__Dpi.h"
#include "common.h"
#include "memory/pmem.h"
#include "svdpi.h"
#include <verilated.h>
#include <verilated_fst_c.h>

extern VerilatedContext *contextp;
extern VerilatedFstC *tfp;
extern TOP_NAME *top;

int step_times(int n);
int sim_exit();
int sim_init();
int step_and_dump_wave();

extern int ebreak_flag;
extern int halt_ret;
extern "C" void trigger_ebreak();
extern "C" void sync_rf_data(uint32_t addr, uint32_t data);
extern "C" void sync_pc_data(uint32_t pc);

#ifdef CONFIG_ITRACE
extern "C" void trace_instruction(word_t inst, vaddr_t pc);
#endif // CONFIG_ITRACE

void cpu_exec(uint64_t n);
