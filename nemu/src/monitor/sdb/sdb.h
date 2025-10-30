/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

#ifdef CONFIG_WATCHPOINT
bool apply_and_set_WP(char *expr, word_t first_value);
bool delete_WP(int N);
bool wp_list_change();
void test_new_and_free_WP();
void info_w();
#endif

#ifdef CONFIG_ITRACE
void init_iringbuff();
void log_iringbuff();
int push_iringbuff(char *inst);
#endif

int is_exit_status_bad();
#define bad_ending                                                             \
  is_exit_status_bad() &&                                                      \
      (nemu_state.state == NEMU_END || nemu_state.state == NEMU_QUIT ||        \
       nemu_state.state == NEMU_ABORT)

#ifdef CONFIG_MTRACE
void init_mtrace();
int push_mem_trace(paddr_t addr, int type, word_t data);
#endif

word_t expr(char *e, bool *success);

#endif
