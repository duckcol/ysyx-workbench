#ifndef __COMMON_H__
#define __COMMON_H__

#include "macro.h"
#include "utils.h"
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define word_t uint32_t
#define paddr_t uint32_t

#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000

#define RESET_VECTOR CONFIG_MBASE
typedef word_t vaddr_t;
#define FMT_WORD "0x%08" PRIx32
#define FMT_PADDR "0x%08" PRIx32

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

int parse_args(int argc, char *argv[]);
long load_img();
void init_log(/*const char *log_file*/);

// strlen() for string constant
#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

// calculate the length of an array
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

#endif
