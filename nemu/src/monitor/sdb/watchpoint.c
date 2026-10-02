/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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

#include "sdb.h"


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

// return a free watchpoint from the free list
WP* new_wp() {
  if (free_ == NULL) {
    return NULL;
  }

  // we return the first free watchpoint from the free list 
  // and change the free_ to the next free watchpoint 
  WP* wp = free_;
  free_ = free_->next;

  // add the new watchpoint to the head of the watchpoint list 
  wp->next = head;
  head = wp;

  return wp;

}

// free a watchpoint and return it to the free list 
void free_wp(WP *wp) {

  if (wp == NULL) {
    fprintf(stderr, "free_wp failed: wp is NULL\n");
    return;
  }

  // loop through the watchpoint list to find the watchpoint to free
  if (head == NULL) {
    fprintf(stderr, "No watchpoints to free\n");
    return;
  }

  WP *cur = head;
  WP *prev = NULL;

  while (cur != NULL)
  {
    if (cur == wp) {
      // that is wp, remove it from the watchpoint list
      if (prev == NULL) {
        head = cur->next;
      } else {
        prev->next = cur->next;
      }
      // add it to the free list 
      cur->next = free_;
      free_ = cur;
      return;
    }
    prev = cur;
    cur = cur->next;
  }
  fprintf(stderr, "free_wp failed: wp is not in the watchpoint list\n");
  return;
}

void watchpoint_display() {
  // loop through the watchpoint list 
  if (head == NULL) {
    fprintf(stderr, "No watchpoints to display\n");
    return;
  }

  WP *cur = head;
  while(cur != NULL) {
    printf("Watchpoint %d: %s, last value = 0x%08x\n", cur->NO, cur->expr, cur->last_value);
    cur = cur->next;
  }
  return;
}

// after each instruction execution, we need to check whether the value of each watchpoint's expression has changed
void check_watchpoints() {
  if (head == NULL) {
    fprintf(stderr, "No watchpoints to check\n");
    return;
  }

  WP *wp = head;
  while (wp != NULL) {
    // compute the value of the expression 
    bool success;
    word_t cur_value = expr(wp->expr, &success);
    if (!success) {
      fprintf(stderr, "Failed to evaluate watchpoint expression: %s\n", wp->expr);
      break;
    }

    // check if the value has changed 
    if (cur_value != wp->last_value) {
      printf("Watchpoint %d triggered: %s\n", wp->NO, wp->expr);
      printf("Old value = 0x%08x\n", wp->last_value);
      printf("New value = 0x%08x\n", cur_value);
      wp->last_value = cur_value; // update the last value
      if (nemu_state.state == NEMU_RUNNING) {
        nemu_state.state = NEMU_STOP; // stop the CPU, only when the state is running 
      }
      break;
    }
    wp = wp->next;
  }
}

WP* get_wp(int number) {
  if (number < 0 || number >= NR_WP) {
    fprintf(stderr, "Invalid watchpoint number: %d\n", number);
    return NULL;
  }

  WP *cur = head;
  while (cur != NULL) {
    if (cur->NO == number) {
      return cur;
    }
    cur = cur->next;
  }

  fprintf(stderr, "Watchpoint %d not found\n", number);
  return NULL;
}