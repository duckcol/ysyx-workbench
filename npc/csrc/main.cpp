#include "common.h"
#include "sim_set.h"

//  ebreak
int ebreak_flag = 0;
extern int ebreak_flag;
void trigger_ebreak() {
  INFO("triggering inst ebreak");
  ebreak_flag = 1;
}

int main(int argc, char *argv[]) {
  //  initial pmem
  parse_args(argc, argv);
  load_img();
  return 0;
  // pmem_initial();

  //  initial cpu pc counter
  sim_init();
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
