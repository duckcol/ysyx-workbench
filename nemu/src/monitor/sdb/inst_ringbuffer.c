#include "sdb.h"

#define NR_INST 20

typedef struct {
  char insts[NR_INST][128];
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

#define rp_update                                                              \
  iringbuff.rp++;                                                              \
  iringbuff.rp = (iringbuff.rp >= 20) ? 0 : iringbuff.rp;

int push_iringbuff(char *inst) {
  strncpy(iringbuff.insts[iringbuff.rp], inst, 128);
  if (is_exit_status_bad()) {
    strncat(iringbuff.insts[iringbuff.rp], "<-- ERROR!", 15);
  }
  rp_update;
  return 0;
}

void log_iringbuff() {
  for (int i = 0; i < NR_INST; i++) {
    log_write("%s\n", iringbuff.insts[i]);
    puts(iringbuff.insts[i]);
  }
}
