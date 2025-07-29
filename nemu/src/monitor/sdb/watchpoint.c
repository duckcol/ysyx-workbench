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
#include <stdbool.h>
#include <stdio.h>

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
			//	near.NO < new.NO < near.next.NO
			WP* near = head;
			while (near->next != NULL) {
				if (near->next->NO < new->NO) 
					near = near->next;
				else break;
				Assert(near != NULL, \
				"couldn't find the near position in head list");
			}
			//	rebind near and new
			new->prev = near; new->next = near->next;
			near->next = new; 
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

	//	clear the content in WP* tmp
	free(tmp->expr);
	tmp->expr = NULL;
	tmp->new_value = tmp->old_value = 0;
	
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
			//	near.NO < tmp.NO < near.next.NO
			WP* near = free_;
			while (near->next != NULL) {
				if (near->next->NO < tmp->NO) 
					near = near->next;
				else break;
				Assert(near != NULL, "couldn't find the position in free_");
			}
			tmp->prev = near; tmp->next = near->next;
			near->next = tmp; 
			if (tmp->next != NULL) {
				tmp->next->prev = tmp;
			}
		}
	}
}

bool apply_and_set_WP(char *expr, word_t first_value){
	//	apply WP* new
	bool success = false;
	WP* new = new_wp();
	Assert(new != NULL, \
	"new_wp() in apply_and_set_WP() failed! ");

	//	set value
	int NO = new->NO;
	//	TODO: free the char* storage in free_wp()!
	char *storage = malloc(sizeof(char) * 512);
	strcpy(storage, expr);
	new->expr = storage;
	new->old_value = new->new_value = first_value;
	Assert(new->new_value != 0, \
	"WP init wrong: WP->new_value == 0");

	//	check if WP* new in head list
	for(WP* tmp = head; tmp != NULL; tmp = tmp->next) {
		if (tmp->NO == NO) {
			success = true;
		}
	}
	Assert(success, "could't find WP* new in head list");
	return success;
}

bool wp_list_change() {
	bool change = false;
	for (WP* tmp = head; tmp != NULL; tmp = tmp->next) {
		bool expr_success = false;
		tmp->new_value = expr(tmp->expr, &expr_success);
		Assert(expr_success, "expr failed while checking watchpoint changes");

		if (tmp->old_value != tmp->new_value) {
			change = true;
			printf("watchpoint %d: "
			"expr: %s | old_value: "FMT_WORD" | new_value: "FMT_WORD" \n",
			tmp->NO, tmp->expr, tmp->old_value, tmp->new_value);

			tmp->old_value = tmp->new_value;
		}
	}
	
	return change;
}

bool delete_WP(int N) {
	bool success = false;
	//	check if N is in head list 
	//	if found delete_WP
	//	else return false
	for(WP* tmp = head; tmp != NULL; tmp = tmp->next) {
		if (tmp->NO == N) {
			free_wp(tmp);
			Assert(
			(tmp->expr == NULL) &&\
			(tmp->old_value == 0) &&\
			(tmp->new_value == 0) ,\
			"watchpoint clear failed");
			return true;
		}
	}
	if (success == false) {
		WARN("can't find watchpoint %d", N);
		return false;
	}
	return success;
}

//	this two printf could printf list to NULL
void printf_the_free_WP_list(int opt) {
	WP* ptr = free_;
	printf("free_ = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		if (ptr->next == NULL) break;
		else ptr = ptr->next;
	}
	printf("NULL\n");

	if(opt == 0){
	printf("free_ (revserse) = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->prev;
	}
	printf("NULL\n");
	}
}

void printf_the_used_WP_list(int opt) {
	WP* ptr = head;
	printf("head = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		if (ptr->next == NULL) break;
		else ptr = ptr->next;
	}
	printf("NULL\n");

	if (opt == 0) {
	printf("head (revserse) = ");
	while (ptr != NULL) {
		printf("%d - ",ptr->NO);
		ptr = ptr->prev;
	}
	printf("NULL\n");
	}
}

void test_new_and_free_WP(){
	printf("the primitive free and used WP List:\n");
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	printf("use 5 WP, the list:\n");
	for(int i = 0; i < 5; i++) {
		new_wp();
	}
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	printf("free WP 1, the list:\n");
	free_wp(head + 1);
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	printf("free WP 0, the list:\n");
	free_wp(head);
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	printf("free WP 2, the list:\n");
	free_wp(head);
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	printf("use 5 WP, the list:\n");
	for(int i = 0; i < 5; i++) new_wp();
	printf_the_free_WP_list(0);
	printf_the_used_WP_list(0);

	free_wp(NULL);
	//printf("used till the end \n");for(int i = 30; i > 0; i--) new_wp();
}

void info_w() {
	printf("list all watchpoints in used:\n");
	printf_the_free_WP_list(1);
	printf_the_used_WP_list(1);
	for(WP* tmp = head; tmp != NULL; tmp = tmp->next) {
		Info("expr: %s",tmp->expr);
		printf("watchpoint %d: "
				"expr:%s | old_value: "FMT_WORD" | new_value: "FMT_WORD"\n", 
				tmp->NO, tmp->expr ,tmp->old_value, tmp->new_value);
	}

	//test_new_and_free_WP();
}
