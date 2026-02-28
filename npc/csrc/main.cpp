#include "common.h"
#include "sdb.h"
#include "sim_set.h"

int main(int argc, char *argv[]) {
  //  initial pmem with image
  parse_args(argc, argv);
  load_img();
  // return 0;
  //  initial pmem with testing
  //  instructions in pmem_initial()
  //  in pmem.cpp
  // pmem_initial();

  //  initial cpu pc counter
  sim_init();
  init_regex();
  INFO("CPU INITIAL COMPLETED");

  //  read in inst and run
  // top->sys_rst_l = 1;
  // for (int i = 0; i < 15; i++) {
  //   step_times(1);
  //   if (ebreak_flag == 1)
  //     break;
  // }
  sdb_mainloop();

  sim_exit();

  INFO("%s at pc = %08x", (halt_ret) ? "HIT BAD TRAP" : "HIT GOOD TRAP",
       top->fetch_inst_addr - 4);
  return halt_ret;
}
