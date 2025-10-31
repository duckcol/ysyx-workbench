#include "sdb.h"

#define NR_MEM_TRACE 20
typedef struct {
  char ringbuff[128][NR_MEM_TRACE];
  int rp;
  int wp;
} memtrace;
memtrace mt = {};

void init_mtrace() {
  Log("init memory trace");
  for (int i = 0; i < NR_MEM_TRACE; i++) {
    strncpy(mt.ringbuff[i], "1111_1111", 20 * sizeof(char));
  }
  mt.rp = 0;
  mt.wp = 0;
}

int push_mem_trace(paddr_t addr, int type, word_t data) {
  if (type == 0)
    sprintf(mt.ringbuff[mt.wp],
            "type: write, addr:" FMT_PADDR ", data:" FMT_WORD " ", addr, data);
  else if (type == 1)
    sprintf(mt.ringbuff[mt.wp],
            "type: read , addr:" FMT_PADDR ", data:" FMT_WORD " ", addr, data);
  // sprintf(mt.ringbuff[mt.wp], "test %d", mt.wp);

  mt.wp++;
  mt.wp = (mt.wp >= NR_MEM_TRACE) ? 0 : mt.wp;
  return 0;
}

void log_mem_trace() {
  for (int i = 0; i < NR_MEM_TRACE; i++) {
    log_write("%s\n", mt.ringbuff[i]);
    puts(mt.ringbuff[i]);
  }
}
