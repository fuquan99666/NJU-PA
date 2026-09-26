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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

// For PA1 , we need to add step , x , info r 

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */

  { "si", "Step through the program for N instructions", cmd_si },
  { "info", "Print register state or watchpoint information", cmd_info },
  { "x", "Print the value of an address in memory for N 4 bytes (the address is given by an expression)", cmd_x },

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

// si [N] : Step through the program for N instructions (default 1)
static int cmd_si(char *args) {
  int n = 1;
  
  // use strtok to parse the N
  char *arg = strtok(args, " ");

  if (arg != NULL) {
    int ret = sscanf(arg, "%d", &n);
    if (ret != 1) {
      fprintf(stderr, "Invalid argument for si command: %s\n", arg);
      return 0;
    }
  } 
  cpu_exec(n);
  return 0;
}

// info r : Print register state 
static int cmd_info(char *args) {
  char *arg = strtok(args, " ");
  if (arg == NULL) {
    fprintf(stderr, "Missing argument for info command\n");
    return 0;
  } else if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0) {
    return 0;
  } else {
    fprintf(stderr, "Unknown argument for info command: %s\n", arg);
  }
  return 0;
}

// x N EXPR : Print the value of an address in memory for N 4 bytes (the address is given by an expression)
static int cmd_x(char *args) {
  // now we assume the EXP is just a hex number, we will implement the expression parser later
  char *arg = strtok(args, " ");

  if (arg == NULL) {
    fprintf(stderr, "Missing N argument for x command\n");
    return 0;
  }

  int n ;
  int ret = sscanf(arg, "%d", &n);

  if (ret != 1) {
    fprintf(stderr, "Invalid N argument for x command: %s\n", arg);
    return 0;
  }

  char *expr = strtok(NULL, " ");
  if (expr == NULL) {
    fprintf(stderr, "Missing expression argument for x command\n");
    return 0;
  }

  // we assume the expression is just a hex number 
  int addr;
  ret = sscanf(expr, "%x", &addr);
  if (ret != 1) {
    fprintf(stderr, "Invalid expression argument for x command: %s\n", expr);
    return 0;
  }

  int left = n % 4;

  for (int i = 0; i < n/4; i+=1) {
    // read 4 bytes from memory 
    word_t val_0 = vaddr_read(addr + i * 16, 4);
    printf("0x%08x: 0x%08x  ", addr + i * 16, val_0);
    word_t val_1 = vaddr_read(addr + i * 16 + 4, 4);
    printf("0x%08x: 0x%08x  ", addr + i * 16 + 4, val_1);
    word_t val_2 = vaddr_read(addr + i * 16 + 8, 4);
    printf("0x%08x: 0x%08x  ", addr + i * 16 + 8, val_2);
    word_t val_3 = vaddr_read(addr + i * 16 + 12, 4);
    printf("0x%08x: 0x%08x\n", addr + i * 16 + 12, val_3);
  }

  // print the left bytes if n is not a multiple of 4
  if (left > 0) {
    for (int i = 0; i < left; i++) {
      word_t val = vaddr_read(addr + (n - left + i) * 4, 4);
      printf("0x%08x: 0x%08x  ", addr + (n - left + i) * 4, val);
    }
    printf("\n");
  }

  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
