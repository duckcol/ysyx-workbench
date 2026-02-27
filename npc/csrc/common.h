#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define Assert(cond, format, ...)                                              \
  if (!(cond)) {                                                               \
    printf("Assert:[%s:%d %s]: " format "\n", __FILE__, __LINE__, __func__,    \
           ##__VA_ARGS__);                                                     \
    assert(0);                                                                 \
  }

#define INFO(format, ...)                                                      \
  printf("INFO:[%s:%d %s]: " format "\n", __FILE__, __LINE__, __func__,        \
         ##__VA_ARGS__);

int parse_args(int argc, char *argv[]);
long load_img();

// strlen() for string constant
#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

// calculate the length of an array
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
