// In this log.h file, we define some data structures and functions for logging the execution of instructions in NEMU.

#include <common.h>

// define a ring buffer to store the log of the last 16 instructions executed 
#define IRINGBUF_SIZE 16
extern char iringbuf[IRINGBUF_SIZE][128]; // each instruction log is at most 128 bytes 
extern int iringbuf_index;

// define a function table to store the function name, address and size for function tracing 
#define MAX_FUNC_NUM 512
typedef struct func_info
{
  char name[64]; // function name
  vaddr_t addr; // function address
  size_t size; // function size
} func_info;
extern func_info func_table[MAX_FUNC_NUM];
extern int func_num; // the number of functions in the function table 


typedef struct f_OP
{
  vaddr_t pc; // the program counter of the instruction being executed
  char name[32]; // the function name
  vaddr_t addr; // the function address 
  bool is_call; // whether the instruction is a call instruction or a ret instruction
} f_OP;

extern int f_op_num; // the number of function operations in the f_op_list
extern f_OP f_op_list[MAX_FUNC_NUM]; // the list of function operations e.g. call, ret

void ftrace_write(vaddr_t pc, vaddr_t target_addr, bool is_call);

void ftrace_display(); // display the function trace information after the end of the program execution 