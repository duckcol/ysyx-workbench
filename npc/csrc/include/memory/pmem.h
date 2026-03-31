#ifndef __PMEM_H__
#define __PMEM_H__
#include "common.h"

#define CONFIG_SERIAL_MMIO 0xa00003f8
#define CONFIG_RTC_MMIO 0xa0000048

#define CONFIG_VGA_CTL_MMIO 0xa0000100
#define CONFIG_FB_ADDR 0xa1000000

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

void pmem_initial();
void paddr_write(paddr_t pc, word_t data);
word_t paddr_read(paddr_t pc);

uint8_t *guest_to_host(paddr_t paddr);

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

word_t vaddr_read(vaddr_t addr);
#endif
