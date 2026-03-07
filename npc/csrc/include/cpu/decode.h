#ifndef __DECODE_H__
#define __DECODE_H__

#include "common.h"

// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

typedef riscv32_ISADecodeInfo ISADecodeInfo;

typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  // IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;

#endif
