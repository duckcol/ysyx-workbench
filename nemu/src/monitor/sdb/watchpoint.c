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

//	move WP from free_ to head
WP* new_wp() {
	//	spcify the WP
	WP* new = free_; 
	free_ = free_->next;
	Assert(free_ != NULL, "no more free WP !");

	//	rebind the WP
	free_->prev = NULL; 
	new->next = NULL; new->prev = NULL;
	if(head == NULL) head = new;
	else { 
		//	insert the new into head list in order
		if (new->NO < head->NO ) {
			new->prev = NULL; new->next = head;
			head->prev = new; head->next = head->next;
			head = new;
		} else {
			//	head->NO < new->NO
			//	to find the position where:
			//	tmp.NO < new.NO < tmp.next.NO
			WP* tmp = head;
			while (tmp->next != NULL) {
				Assert(tmp != NULL, "couldn't find the position in head");
				if (tmp->next->NO < new->NO) 
					tmp = tmp->next;
				else break;
			}
			//	rebind tmp and new
			new->prev = tmp; new->next = tmp->next;
			tmp->next = new; 
			if (new->next != NULL) {
				new->next->prev = new;
			}
		}
	}

	return new;
}

//	move back WP from head to free_
//	while free the content in WP
void free_wp(WP *wp) {
	WP* tmp = wp;
	if (tmp == head) head = head->next;
	if (tmp == NULL) {
		WARN("WP* to be freed is NULL !");
		return;
	}

	//	seperate the wp from head list
	if (tmp->prev == NULL && tmp->next == NULL)
		;//do nothing
	if (tmp->prev == NULL && tmp->next != NULL) {
		tmp->next->prev = NULL;
		tmp->next = NULL;
	}
	if (tmp->prev != NULL && tmp->next == NULL) {
		tmp->prev->next = NULL;
		tmp->prev = NULL;
	}
	if (tmp->prev != NULL && tmp->next != NULL) {
		tmp->prev->next = tmp->next;
		tmp->next->prev = tmp->prev;
		tmp->prev = tmp->next = NULL;
	}

	//	clear the content
	
	//	join the tmp back to free_ list
	if(free_ == NULL) free_ = tmp;
	else { 
		if (tmp->NO < free_->NO ) {
			tmp->prev = NULL; tmp->next = free_;
			free_->prev = tmp; free_->next = free_->next;
			free_ = tmp;
		} else {
			//	free_ < tmp
			//	find a position
			//	near < tmp < near.next
			WP* near = free_;
			while (near->next != NULL) {
				Assert(near != NULL, "couldn't find the position in free_");
				if (near->next->NO < tmp->NO) 
					near = near->next;
				else break;
			}
			tmp->prev = near; tmp->next = near->next;
			near->next = tmp; 
			if (tmp->next != NULL) {
				tmp->next->prev = tmp;
			}
		}
	}
	
	
}

void printf_the_free_WP_list() {
	WP* ptr = free_;
	printf("free_ = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		if (ptr->next == NULL) break;
		else ptr = ptr->next;
	}
	printf("NULL\n");

	printf("free_ (revserse) = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->prev;
	}
	printf("NULL\n");
}

void printf_the_used_WP_list() {
	WP* ptr = head;
	printf("head = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		if (ptr->next == NULL) break;
		else ptr = ptr->next;
	}
	printf("NULL\n");

	printf("head (revserse) = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->prev;
	}
	printf("NULL\n");
}

void test_new_and_free_WP(){
	printf("the primitive free and used WP List:\n");
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("use 5 WP, the list:\n");
	for(int i = 0; i < 5; i++) {
		new_wp();
	}
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("free WP 1, the list:\n");
	free_wp(head + 1);
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("free WP 0, the list:\n");
	free_wp(head);
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("free WP 2, the list:\n");
	free_wp(head);
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	printf("use 5 WP, the list:\n");
	for(int i = 0; i < 5; i++) new_wp();
	printf_the_free_WP_list();
	printf_the_used_WP_list();

	//printf("used till the end \n");for(int i = 30; i > 0; i--) new_wp();
	free_wp(NULL);
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
