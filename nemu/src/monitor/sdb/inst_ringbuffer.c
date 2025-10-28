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

void log_iringbuff() {
  for (int i = 0; i < NR_INST; i++) {
    log_write("%s\n", iringbuff.insts[i]);
  }
}
