#include "common.h"
#include "cpu/sim_set.h"
#include "monitor/monitor.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void print_ftrace_log();

int main(int argc, char *argv[]) {
  init_monitor(argc, argv);

  //  initial cpu
  //  read in inst and run
  // top->sys_rst_l = 1;
  // for (int i = 0; i < 15; i++) {
  //   step_times(1);
  //   if (ebreak_flag == 1)
  //     break;
  // }
  sim_init();
  INFO("CPU INITIAL COMPLETED");

  sdb_mainloop();

  sim_exit();

  print_ftrace_log();
  Log("%s at pc = %08x", (halt_ret) ? "HIT BAD TRAP" : "HIT GOOD TRAP",
      top->fetch_inst_addr - 4);
  return halt_ret;
}
