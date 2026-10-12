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

extern uint64_t g_nr_guest_inst;

// define a ring buffer to store the log of the last 16 instructions executed 
#define IRINGBUF_SIZE 16
static char iringbuf[IRINGBUF_SIZE][128]; // each instruction log is at most 128 bytes 
static int iringbuf_index = 0;

#ifndef CONFIG_TARGET_AM
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}

void iringbuf_write(char *str) {
  // if the ring buffer is not full, write the log directly to the next position
  if (iringbuf_index < IRINGBUF_SIZE) {
    strcpy(iringbuf[iringbuf_index], str);
    iringbuf_index ++;
  } else {
    // if the ring buffer is full, move the first log and insert the new log to the last position 
    for (int i = 0; i < IRINGBUF_SIZE - 1; i ++) {
      strcpy(iringbuf[i], iringbuf[i + 1]);
    }
    strcpy(iringbuf[IRINGBUF_SIZE - 1], str);
  }
}

void iringbuf_display() {
  printf("The last %d instructions executed are:\n", IRINGBUF_SIZE);
  for (int i = 0; i < iringbuf_index; i++) {
    printf("%s\n", iringbuf[i]);
  }
}


#endif
