#include "common.h"
#include "cpu/decode.h"
#include "cpu/sim_set.h"
#include "monitor/monitor.h"

void init_monitor(int argc, char *argv[]);
void sdb_mainloop();
void print_ftrace_log();
void log_iringbuff();
void log_mem_trace();

int main(int argc, char *argv[]) {
  init_monitor(argc, argv);

  sdb_mainloop();

  sim_exit();

  switch (halt_ret) {
  case (0):
    print_ftrace_log();
    log_iringbuff();
    log_mem_trace();
    Log("%s at pc = %08x",
        ANSI_FMT(ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN), ANSI_BG_BLACK),
        inst_decode.pc);
    break;
  case (1):
    print_ftrace_log();
    log_iringbuff();
    log_mem_trace();
    Log("%s at pc = %08x",
        ANSI_FMT(ANSI_FMT("HIT BAD  TRAP", ANSI_FG_RED), ANSI_BG_BLACK),
        inst_decode.pc);
    return 1;
    break;
  default:
    print_ftrace_log();
    log_iringbuff();
    log_mem_trace();
    Log("%s at pc = %08x",
        ANSI_FMT(ANSI_FMT("NOT YET DONE", ANSI_FG_YELLOW), ANSI_BG_CYAN),
        inst_decode.pc);
    break;
  }

  return 0;
}
