#include "common.h"
#define NR_INST 20

#ifdef CONFIG_ITRACE
typedef struct {
  char insts[NR_INST][256];
  int rp;
  int wp;
} inst_ring_buffer;

static inst_ring_buffer iringbuff = {};

void init_iringbuff() {
  for (int i = 0; i < NR_INST; i++) {
    strncpy(iringbuff.insts[i], "0000_0000_0000", 20 * sizeof(char));
  }
  iringbuff.rp = 0;
  iringbuff.wp = 0;
}

int push_iringbuff(char *inst, bool bad_ending) {
  strncpy(iringbuff.insts[iringbuff.rp], inst, 256);
  if (bad_ending) {
    strncat(iringbuff.insts[iringbuff.rp], "<-- ERROR!", 15);
  }
  iringbuff.rp++;
  iringbuff.rp = (iringbuff.rp >= NR_INST) ? 0 : iringbuff.rp;
  return 0;
}

void log_iringbuff() {
  int last_idx = iringbuff.rp == 0 ? NR_INST - 1 : iringbuff.rp - 1;
  strncat(iringbuff.insts[last_idx], " <-- END HERE!", 20);
  Log("the instruction ringbuffer are as following");
  _Log("========instructions ringbuffer========\n");
  for (int i = 0; i < NR_INST; i++) {
    log_write("%s\n", iringbuff.insts[i]);
    puts(iringbuff.insts[i]);
  }
  _Log("================end====================\n");
}
#else
void init_iringbuff() { return; }
int push_iringbuff(char *inst, bool bad_ending) { return 0; };
void log_iringbuff() { return; };
#endif
