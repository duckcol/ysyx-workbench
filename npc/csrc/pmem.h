#include <stdint.h>
#include <stdio.h>

#define word_t uint32_t
#define paddr_t uint32_t

#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000

void pmem_initial();
void pmem_write(paddr_t pc, word_t data);
word_t pmem_read(paddr_t pc);

#define Assert(cond, format, ...)                                              \
  if (cond) {                                                                  \
    printf("Assert:[%s:%d %s]: " format "\n", __FILE__, __LINE__, __func__,    \
           ##__VA_ARGS__);                                                     \
    assert(0);                                                                 \
  }

#define INFO(format, ...)                                                      \
  printf("INFO:[%s:%d %s]: " format "\n", __FILE__, __LINE__, __func__,        \
         ##__VA_ARGS__);
