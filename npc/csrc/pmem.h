#include "common.h"
#define word_t uint32_t
#define paddr_t uint32_t

#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000

#define RESET_VECTOR CONFIG_MBASE

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

void pmem_initial();
void pmem_write(paddr_t pc, word_t data);
word_t pmem_read(paddr_t pc);

uint8_t *guest_to_host(paddr_t paddr);
