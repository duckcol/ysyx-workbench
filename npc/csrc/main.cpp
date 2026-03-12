#include "common.h"
#include "cpu/decode.h"
#include "cpu/sim_set.h"
#include "monitor/monitor.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void print_ftrace_log();
void log_iringbuff();

int main(int argc, char *argv[]) {
  init_monitor(argc, argv);

  sdb_mainloop();

  sim_exit();

  if (halt_ret == 0) {
    print_ftrace_log();
    log_iringbuff();
  }
  Log("%s at pc = %08x", (halt_ret) ? "HIT BAD TRAP" : "HIT GOOD TRAP",
      inst_decode.pc);
  return halt_ret;
}
