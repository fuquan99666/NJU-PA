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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);

#define NR_WP 32

typedef struct watchpoint {
  int NO; // the index of this watchpoint in the watchpoint pool
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

  char expr[32]; // the expression to watch
  word_t last_value; // the last value of the expression, used to detect changes faster

} WP;

__attribute__((unused)) static WP wp_pool[NR_WP] = {};
__attribute__((unused)) static WP *head = NULL, *free_ = NULL;

WP* new_wp();
void free_wp(WP* wp);
void init_wp_pool();
void watchpoint_display();
void check_watchpoints();
WP* get_wp(int number);

#endif
