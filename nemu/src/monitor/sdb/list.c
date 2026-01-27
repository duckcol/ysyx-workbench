#include "list.h"

List *List_create() {
  //	the memory will be set to 0
  return calloc(1, sizeof(List));
}

void List_clear_destroy(List *list) {
  LIST_FOREACH(list, first, next, cur) {
    if (cur->prev) {
      free(cur->prev->value);
      free(cur->prev);
    }
  }

  free(list->last->value);
  free(list->last);
  free(list);
}

//	push in a node to last of the list
void List_push(List *list, void *value) {
  ListNode *node = calloc(1, sizeof(ListNode));
  Assert(node, "push listnode error");

  node->value = value;

  if (list->last == NULL) {
    list->first = node;
    list->last = node;
  } else {
    list->last->next = node;
    node->prev = list->last;
    list->last = node;
  }

  list->count++;
}

//	pop out the last node of the list
void *List_pop(List *list) {
  ListNode *node = list->last;
  return node != NULL ? List_remove(list, node) : NULL;
}

//	add a node to the first of the list
void List_unshift(List *list, void *value) {
  ListNode *node = calloc(1, sizeof(ListNode));
  Assert(node, "push listnode error");

  node->value = value;

  if (list->first == NULL) {
    list->first = node;
    list->last = node;
  } else {
    node->next = list->first;
    list->first->prev = node;
    list->first = node;
  }

  list->count++;
}

//	remove the first node of the list
//	and return the value of the remove node
void *List_shift(List *list) {
  ListNode *node = list->first;
  return node != NULL ? List_remove(list, node) : NULL;
}

void *List_remove(List *list, ListNode *node) {
  void *value = NULL;

  Assert(list->first && list->last, "List is empty.");
  Assert(node, "node can't be NULL");

  if (node == list->first && node == list->last) {
    //	when node is the only node of the list
    list->first = NULL;
    list->last = NULL;
  } else if (node == list->first) {
    list->first = node->next;
    Assert(list->first != NULL,
           "Invalid list, somehow got a first that is NULL.");
    list->first->prev = NULL;
  } else if (node == list->last) {
    list->last = node->prev;
    Assert(list->last != NULL,
           "Invalid list, somehow got a next that is NULL.");
    list->last->next = NULL;
  } else {
    //	when node is in the middle of the list
    ListNode *after = node->next;
    ListNode *before = node->prev;
    after->prev = before;
    before->next = after;
  }

  list->count--;
  value = node->value;
  free(node);

  return value;
}
