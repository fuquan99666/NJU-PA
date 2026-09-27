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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  // PA1 stage 2 : read the expression and result from nemu/tools/gen-expr/input 
  // use expr() to evaluate the expression and compare the result with the expected result 

  char *input_file = "/home/zz/Code/ics2026/nemu/tools/gen-expr/input";

  FILE *fp = fopen(input_file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open input file: %s\n", input_file);
    return 1;
  }

  // read line by line 
  char line[256];
  uint32_t expected_result;
  char expression[64];
  while (fgets(line, sizeof(line), fp) != NULL) {
    // format : result expression 
    sscanf(line, "%u %[^\n]", &expected_result, expression);

    bool success = true;
    uint32_t eval_result = expr(expression, &success);
    if (!success) {
      fprintf(stderr, "Failed to evaluate expression: %s\n", expression);
      return 1;
    }
    if (eval_result != expected_result) {
      fprintf(stderr, "Mismatch: expected %u, got %u for expression: %s\n", expected_result, eval_result, expression);
      return 1;
    } else {
      printf("GG!\n");
    }
  }

  fclose(fp);
  return 0;


  /* Start engine. */
  // engine_start();

  return is_exit_status_bad();
}
