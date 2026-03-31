#ifndef __FTRACE_H__
#define __FTRACE_H__
#include "common.h"
#include "list.h"
#include <elf.h>

void add_ftrace(word_t pc, word_t target, bool is_ret);
#endif // !__FTRACE_H__
