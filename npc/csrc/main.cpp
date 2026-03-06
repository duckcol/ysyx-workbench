#include "common.h"
#include "cpu/sim_set.h"
#include "monitor/sdb/sdb.h"

int parse_args(int argc, char *argv[]);
long load_img();
void init_log(/*const char *log_file*/);
extern "C" void init_disasm(const char *triple);

int main(int argc, char *argv[]) {
  //  initial pmem with image
  parse_args(argc, argv);

  //  initial cpu pc counter
  sim_init();
  INFO("CPU INITIAL COMPLETED");

  init_disasm("riscv32-pc-linux-gnu");
  INFO("disassemble INITIAL COMPLETED");

  //  initial regex expression
  init_regex();
  INFO("REGEX INITIAL COMPLETED");

  //  initial log
  init_log();
  INFO("LOGGING INITIAL COMPLETED");

  //  initial mem
  load_img();
  INFO("MEM INITIAL COMPLETED");
  //  initial pmem with testing
  //  instructions in pmem_initial()
  //  in pmem.cpp
  // pmem_initial();

  //  read in inst and run
  // top->sys_rst_l = 1;
  // for (int i = 0; i < 15; i++) {
  //   step_times(1);
  //   if (ebreak_flag == 1)
  //     break;
  // }
  sdb_mainloop();

  sim_exit();

  Log("%s at pc = %08x", (halt_ret) ? "HIT BAD TRAP" : "HIT GOOD TRAP",
      top->fetch_inst_addr - 4);
  return halt_ret;
}
