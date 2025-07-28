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

#include "common.h"
#include "debug.h"
#include "sdb.h"
#include <stdio.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
	char *expr;
	word_t old_expr_value;

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
	new_->next = NULL;
	//	the last one, wp_pool[NR_WP - 1].next = NULL
	Assert(free_ != NULL, "no free watchpoint !");

	//	WP* head is 1st WP in used WP list
	//	and the new_ WP will be the last of the WP in used list
	//	new_->next == NULL
	if(head == NULL) head = new_;
	else {
		WP* last = head;
		while (last->next != NULL) {
			last = last->next;
			Assert(last != NULL, "WP* last point to NULL!");
		}
		last->next = new_;
	}
	return new_;
}

void free_wp(WP *wp) {
	if(wp->NO < free_->NO) {
		wp->next = free_;
		free_ = wp;
	} else { // free_->NO < wp->NO
		WP* small = free_;
		//	insert the wp in right position
		while (small->next->NO <= wp->NO) {
			Assert(small->next != NULL, \
			"inserting error when free_wp(): couldn't find the position");
			small = small->next;
		}
		wp->next = small->next;
		small->next = wp;
	}
}

void test_new_and_free_WP(){
	WP* ptr = free_;
	printf("free_ -> ");
	while (ptr != NULL) {
		printf("%d -> ",ptr->NO);
		ptr = ptr->next;
	}
	printf("NULL\n");
}

void info_w() {
	/*
	head = new_wp();
	new_wp();
	for(WP *each = head; each != free_; each = each->next) {
		printf("watchpoint %d's NO: %d\n", each->NO, each->NO);
	}
	free_wp(&wp_pool[1]);
	for(WP *each = head; each != free_; each = each->next) {
		printf("watchpoint %d's NO: %d\n", each->NO, each->NO);
	}
	*/
	test_new_and_free_WP();
}
