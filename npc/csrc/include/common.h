#ifndef __COMMON_H__
#define __COMMON_H__

#include "conf.h"
#include "macro.h"
#include "utils.h"

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* data type definition*/
typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t) sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

typedef word_t vaddr_t;
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)

/* debug log function definition */
#define Assert(cond, format, ...)                                              \
  do {                                                                         \
    if (!(cond)) {                                                             \
      MUXDEF(                                                                  \
          CONFIG_TARGET_AM,                                                    \
          printf(ANSI_FMT(format, ANSI_FG_RED) "\n", ##__VA_ARGS__),           \
          (fflush(stdout), fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", \
                                   ##__VA_ARGS__)));                           \
      IFNDEF(CONFIG_TARGET_AM, extern FILE * log_fp; fflush(log_fp));          \
      assert(cond);                                                            \
    }                                                                          \
  } while (0)

#define INFO(format, ...)                                                      \
  printf(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_CYAN) "\n", __FILE__,          \
         __LINE__, __func__, ##__VA_ARGS__)

#define Log(format, ...)                                                       \
  _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", __FILE__, __LINE__,  \
       __func__, ##__VA_ARGS__)

#define WARN(format, ...)                                                      \
  printf(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_YELLOW) "\n", __FILE__,        \
         __LINE__, __func__, ##__VA_ARGS__)

#define panic(format, ...) Assert(0, format, ##__VA_ARGS__)
#define TODO() panic("please implement me")

// strlen() for string constant
#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

// calculate the length of an array
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

#endif
