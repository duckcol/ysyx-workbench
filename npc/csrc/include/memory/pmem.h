#ifndef __PMEM_H__
#define __PMEM_H__
#include "common.h"

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

void pmem_initial();
void pmem_write(paddr_t pc, word_t data);
word_t pmem_read(paddr_t pc);

uint8_t *guest_to_host(paddr_t paddr);

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

word_t vaddr_read(vaddr_t addr);
#endif
