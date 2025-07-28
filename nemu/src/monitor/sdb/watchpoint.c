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

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
	struct watchpoint *prev;

  /* TODO: Add more members if necessary */
	char *expr;
	word_t old_value;
	word_t new_value;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
		wp_pool[i].prev = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
	//	spcify the WP
	WP* new = free_; 
	free_ = free_->next;

	//	rebind the WP
	free_->prev = NULL; 
	new->next = NULL; new->prev = NULL;
	if(head == NULL) head = new;
	else { 
		//	insert the new into head list in order
		if (new->NO < head->NO ) {
			new->next = head;
			head->prev = new;
			head = new;
		} else {
			//	head->NO < new->NO
			//	to find the excat tmp where:
			//	tmp.NO < new.NO < tmp.next.NO
			WP* tmp = head;
			while (tmp->next->NO < new->NO) {
				tmp = tmp->next;
			}
			Assert(tmp != NULL, \
			"couldn't find the right order for WP* new");
			new->prev = tmp; new->next = tmp->next;
			tmp->next = new; tmp->next->prev = new;
		}
	}

	return new;
}

void free_wp(WP *wp) {
}

void printf_the_free_WP_list() {
	WP* ptr = free_;
	printf("free_ = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->next;
	}
	printf("NULL\n");
}

void printf_the_used_WP_list() {
	WP* ptr = head;
	printf("head = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->next;
	}
	printf("NULL\n");
}

void test_new_and_free_WP(){
	printf("the primitive free and used WP List:\n");
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("used 5 WP, the list:\n");
	for(int i = 0; i < 2; i++) {
		new_wp();
	}
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	/*
	printf("free 1 WP, the list:\n");
	free(head);
	printf_the_free_WP_list();
	printf_the_used_WP_list();
	*/

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
