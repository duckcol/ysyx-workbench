#ifndef __REG_H__
#define __REG_H__

#include "common.h"

/* cpu state definition */
typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);
typedef riscv32_CPU_state CPU_state;

extern CPU_state cpu;

/* reg definition */
static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline const char *reg_name(int idx) {
  extern const char *regs[];
  return regs[check_reg_idx(idx)];
}

void isa_reg_display();

word_t isa_reg_str2val(const char *s, bool *success);
#endif
