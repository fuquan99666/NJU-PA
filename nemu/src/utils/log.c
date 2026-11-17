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
#include <log.h>

extern uint64_t g_nr_guest_inst;

#ifndef CONFIG_TARGET_AM

FILE *log_fp = NULL;

int iringbuf_index = 0;
char iringbuf[IRINGBUF_SIZE][128]; // each instruction log is at most 128 bytes

int func_num = 0;
func_info func_table[MAX_FUNC_NUM];

int f_op_num = 0; // the number of function operations in the f_op_list
f_OP f_op_list[MAX_FUNC_NUM]; // the list of function operations e.g. call, ret

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

/**
 * @brief ftrace_write is used to write the function trace information to the log file.
 * @param pc The program counter of the instruction being executed.
 * @return None
 * @note This function is only used when CONFIG_FTRACE is defined.
 */
void ftrace_write(vaddr_t pc, vaddr_t target_address, bool is_call) {
  // We have got the whole function info from symbol table of the elf file
  // let's finish the main logic of ftrace 

  // Well, we firstly should know that when the instruction is call or ret
  // Because we use riscv32 as the target ISA, so maybe we can use the instruction format to judge whether it is a call or ret instruction 
  // And after making sure, then we use its pc and its target address to find the function name from the function table we have got.
  // And at last, we can write this info to the f_op_list.

  // Interestingly, in riscv32, we have jxx and branch instructions which all can be used to jump to some address, but 
  // the call instruction is exactly the jal x1 offset or jalr x1 rs1 offset, and the ret instructio is exactly the jalr x0 x1 0
  // That's our clue to find the right call and ret instruction !

  // By inserting some checking code in the isa_exec_once() function, we now get the pc , target address and is_call info

  if (is_call) {
    // call
    // We have target address, it is actually the function start address, so find it in the function table !

    f_op_list[f_op_num].is_call = true;
    strcpy(f_op_list[f_op_num].name, "???"); // default name

    for (int i = 0; i < func_num; i++) {
      if (func_table[i].addr == target_address) {
        // record it into the f_op_list
        f_op_list[f_op_num].pc = pc;
        f_op_list[f_op_num].addr = target_address;
        strcpy(f_op_list[f_op_num].name, func_table[i].name);
        f_op_num++;
        break;
      }
    }
  } else {
    // ret 

    strcpy(f_op_list[f_op_num].name, "???"); // default name
    f_op_list[f_op_num].is_call = false; 
    

    // for ret instruction, its target address is the return address, in CSAPP, we know that 
    // the return is pair to the call, so maybe you think just use a stack to pop the last function call is ok.
    // But in some cases, the ret is not pair to the last call, e.g. tail call optimization -- so amazing! 
    // So that's why we use f_op_list to record both call and ret instruction info ...

    for (int i = 0; i < func_num; i++) {
      if (func_table[i].addr <= pc && pc < func_table[i].addr + func_table[i].size) {
        // notice that ret instruction is between the function start and end...
        // And each function has a unique address range ...
        f_op_list[f_op_num].pc = pc;
        f_op_list[f_op_num].addr = target_address;
        strcpy(f_op_list[f_op_num].name, func_table[i].name);
        f_op_num++;
        break;
      }
    }
  }

  // Well, we have recorded the function call and ret info into the f_op_list !
  // So we can display it after the end of the program execution to help us understand the program flow !

  return;
}

void ftrace_display() {
  printf("\nFunction trace information:\n");

  // For better visualization, if call , move right 2 spaces, if ret, move left 2 spaces
  int indent = 1;
  for (int i = 0; i < f_op_num; i++) {
    if (f_op_list[i].is_call) {
      printf("0x%08x:%*scall [%s@0x%08x]\n", f_op_list[i].pc, indent, "", f_op_list[i].name, f_op_list[i].addr);
      indent += 2;
    } else {
      indent -= 2;
      printf("0x%08x:%*sret  [%s]\n", f_op_list[i].pc, indent, "", f_op_list[i].name);
    }
  }
}

#endif
