#include "common.h"
#include "cpu/sim_set.h"
#include "monitor/monitor.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void print_ftrace_log();
void log_iringbuff();

int main(int argc, char *argv[]) {
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

  init_monitor(argc, argv);

  sdb_mainloop();

  sim_exit();

  if (halt_ret == 0) {
    print_ftrace_log();
    log_iringbuff();
  }
  Log("%s at pc = %08x", (halt_ret) ? "HIT BAD TRAP" : "HIT GOOD TRAP",
      top->pmem_read_addr - 4);
  return halt_ret;
}
