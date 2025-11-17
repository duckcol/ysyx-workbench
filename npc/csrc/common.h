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
