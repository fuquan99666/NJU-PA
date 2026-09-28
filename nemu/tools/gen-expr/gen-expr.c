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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};

// to make sure the expression is evaluated as unsigned, we add 'u' to the end of the number
static char buf_unsigned[65536] = {};
int pos = 0;
int pos_unsigned = 0;

static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int choose(int n) {
  return rand() % n; // [0, ..., n-1]
}

void gen_num() {
  uint32_t num = choose(500); // 先小一点
  pos += sprintf(buf + pos, "%u", num);
  pos_unsigned += sprintf(buf_unsigned + pos_unsigned, "%uu", num);
}

void gen(char c) {
  buf[pos++] = c;
  buf_unsigned[pos_unsigned++] = c;
}

void gen_rand_op() {
  switch (choose(4))
  {
  case 0: gen('+'); break;
  case 1: gen('-'); break;
  case 2: gen('*'); break;
  case 3: gen('/'); break;
  default: assert(0);
    break;
  }
}

static void gen_rand_expr() {
  int flag = choose(3);
  if (pos > 32) {
    // too long, just generate a number
    flag = 0;
  }
  switch (flag)
  {
  case 0:
    gen_num();
    break;
  case 1:
    gen('('); gen_rand_expr(); gen(')');
    break;
  case 2:
    gen_rand_expr(); gen_rand_op(); gen_rand_expr();
    break;
  default:
    assert(0);
    break;
  }
  // add '\0' 
  buf[pos] = '\0';
  buf_unsigned[pos_unsigned] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    pos = 0;
    pos_unsigned = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf_unsigned);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    
    if (ret != 1) {
      // normally this happens when divide by zero in the expression ...
      fprintf(stderr, "No result is obtained from the expression: %s\n", buf);
      continue;
    }

    printf("%u %s\n", result, buf);
  }
  return 0;
}
