#include "common.h"

#ifdef CONFIG_MTRACE
#define NR_MEM_TRACE 20
typedef struct {
  char ringbuff[NR_MEM_TRACE][128];
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

paddr_t prev_mt_read_addr = 0;
paddr_t prev_mt_write_addr = 0;
word_t prev_mt_read_data = 0;
word_t prev_mt_write_data = 0;
int push_mem_trace(paddr_t addr, int type, word_t data) {
  if (type == 0) {
    if (addr == prev_mt_write_addr && data == prev_mt_write_data) {
      return 0;
    }
    sprintf(mt.ringbuff[mt.wp],
            "type:write addr:" FMT_PADDR " data:" FMT_WORD " ", addr, data);
    prev_mt_write_addr = addr;
    prev_mt_write_data = data;
  } else if (type == 1) {
    if (addr == prev_mt_read_addr && data == prev_mt_read_data) {
      return 0;
    }
    sprintf(mt.ringbuff[mt.wp],
            "type:read  addr:" FMT_PADDR " data:" FMT_WORD " ", addr, data);
    prev_mt_read_addr = addr;
    prev_mt_read_data = data;
  }

  mt.wp++;
  mt.wp = (mt.wp >= NR_MEM_TRACE) ? 0 : mt.wp;
  return 0;
}

void log_mem_trace() {
  Log("the memory trace are as following");
  _Log("===========memory trace===========\n");
  for (int i = 0; i < NR_MEM_TRACE; i++) {
    if ((mt.wp - 1) == i) {
      strncat(mt.ringbuff[i], "<--current position", 50);
    }
    log_write("%s\n", mt.ringbuff[i]);
    puts(mt.ringbuff[i]);
  }
  _Log("===============end================\n");
}
#else
void init_mtrace() { WARN("CONFIG_MTRACE not defined, MTRACE disable"); };
int push_mem_trace(paddr_t addr, int type, word_t data) { return 0; }
void log_mem_trace() { WARN("CONFIG_MTRACE not defined, MTRACE disable"); };
#endif
