#include "common.h"
#include "sim_set.h"

//  ebreak triggering function embedding into verilog
int ebreak_flag = 0;
extern int ebreak_flag;
void trigger_ebreak() {
  INFO("triggering inst ebreak");
  ebreak_flag = 1;

  // check $a0 or R(10) to see if it is 0
}

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
  top->sys_rst_l = 0;
  step_times(4);

  //  read in inst and run
  top->sys_rst_l = 1;
  for (int i = 0; i < 15; i++) {
    step_times(1);
    if (ebreak_flag == 1)
      break;
  }

  sim_exit();

  return 0;
}
