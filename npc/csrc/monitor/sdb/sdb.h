#ifndef __SDB_H__
#define __SDB_H__

#include "common.h"
#include <readline/history.h>
#include <readline/readline.h>

void sdb_mainloop();

word_t expr(char *e, bool *success);

void init_regex();

#endif
