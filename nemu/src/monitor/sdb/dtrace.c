#include "sdb.h"

#define NR_DTRACE_BUF 20
#define DTRACE_CHAR_BUF 256
typedef struct {
  char ringbuff[NR_DTRACE_BUF][DTRACE_CHAR_BUF];
  int rp;
  int wp;
} device_trace_t;
device_trace_t dt = {};

void init_dtrace() {
  Log("init device trace");
  for (int i = 0; i < NR_DTRACE_BUF; i++) {
    strncpy(dt.ringbuff[i], "2222_2222_2222_2222", 20 * sizeof(char));
  }
  dt.rp = 0;
  dt.wp = 0;
}

int push_device_trace(const char *device_name, paddr_t addr, int type,
                      word_t data) {
  if (type == 0) {
    sprintf(dt.ringbuff[dt.wp],
            "type:write name:%-15s addr:" FMT_PADDR " data:" FMT_WORD " ",
            device_name, addr, data);
    IFDEF(CONFIG_DTRACE_LOG, _Log("\n%s\n", dt.ringbuff[dt.wp]);)
  } else if (type == 1) {
    sprintf(dt.ringbuff[dt.wp],
            "type:read  name:%-15s addr:" FMT_PADDR " data:" FMT_WORD " ",
            device_name, addr, data);
    IFDEF(CONFIG_DTRACE_LOG, _Log("\n%s\n", dt.ringbuff[dt.wp]);)
  }

  dt.wp++;
  dt.wp = (dt.wp >= NR_DTRACE_BUF) ? 0 : dt.wp;
  return 0;
}

void log_device_trace() {
  _Log("===========device trace===========\n");
  for (int i = 0; i < NR_DTRACE_BUF; i++) {
    if ((dt.wp - 1) == i) {
      strncat(dt.ringbuff[i], "<--current position", 25);
    }

    _Log("%s\n", dt.ringbuff[i]);
  }
  _Log("===============end================\n");
}
