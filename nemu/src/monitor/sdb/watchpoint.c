/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "debug.h"
#include "sdb.h"
#include <string.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
	// free_ is wp_pool[0] after init
	WP* new_ = free_;
	//	free_ give wp_pool[0] to new_, then became wp_pool[1]
	free_ = free_->next;
	//	the last one, wp_pool[NR_WP - 1].next = NULL
	Assert(free_ != NULL, "no free watchpoint !");
	return new_;
}

void free_wp(WP *wp) {
	if(wp->NO < free_->NO) {
		wp->next = free_;
		free_ = wp;
	} else { // free_->NO < wp->NO
		WP* small = free_;
		while (small->next->NO <= wp->NO) {
			Assert(small->next != NULL, \
			"inserting error when free_wp(): couldn't find the position");
			small = small->next;
		}
		wp->next = small->next;
		small->next = wp;
	}
}

void info_w() {
	head = new_wp();
	new_wp();
	for(WP *each = head; each != free_; each = each->next) {
		printf("watchpoint %d's NO: %d\n", each->NO, each->NO);
	}
	free_wp(&wp_pool[1]);
	for(WP *each = head; each != free_; each = each->next) {
		printf("watchpoint %d's NO: %d\n", each->NO, each->NO);
	}
}
