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
#include <memory/paddr.h>
#include <elf.h>
#include <log.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static void load_and_parse_elf(const char *elf_file) {
  if (elf_file == NULL) {
    Log("No ELF file is given. ftrace can't work ...");
    return;
  }

  // read and parse this elf file 
  printf("\nLoading ELF file %s and parsing it ...\n", elf_file);

  // Actually, we can directly use readelf to get the symbol table and function name ...
  // But that not make your hand dirty ...

  // Firstly, get the position of the symbol table and string table in the elf file
  // Just use read to read the ELF Header to get the position of the section header table and then loop through the section header table to find the symbol table and string table's info.
  
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open %s", elf_file);

  // read the ELF header
  Elf32_Ehdr elf_header;
  int ret = fread(&elf_header, sizeof(Elf32_Ehdr), 1, fp);
  assert(ret == 1);

  // check the ELF magic number : 0x7f 'E' 'L' 'F'
  assert(elf_header.e_ident[EI_MAG0] == ELFMAG0);
  assert(elf_header.e_ident[EI_MAG1] == ELFMAG1);
  assert(elf_header.e_ident[EI_MAG2] == ELFMAG2);
  assert(elf_header.e_ident[EI_MAG3] == ELFMAG3);

  // check the ELF class : 32-bit
  assert(elf_header.e_ident[EI_CLASS] == ELFCLASS32);

  // check the ELF data encoding : Two's complement, little endian 
  assert(elf_header.e_ident[EI_DATA] == ELFDATA2LSB);

  // check the ELF version : current version
  assert(elf_header.e_ident[EI_VERSION] == EV_CURRENT);

  // check the ELF type : executable file 
  assert(elf_header.e_type == ET_EXEC);

  // check the ELF machine : RISC-V
  assert(elf_header.e_machine == EM_RISCV);

  // check the ELF version : current version
  assert(elf_header.e_version == EV_CURRENT);

  // print the entry point address
  printf("Entry point address: 0x%08x\n", elf_header.e_entry);

  // get the section header table offset, size of each entry, and number of entries
  uint32_t shoff = elf_header.e_shoff;
  uint16_t shentsize = elf_header.e_shentsize;
  uint16_t shnum = elf_header.e_shnum;
  printf("Section header table offset: 0x%08x, entry size: %d, number of entries: %d\n", shoff, shentsize, shnum);

  // get the section header string table index
  uint16_t shstrndx = elf_header.e_shstrndx;

  // read the section header table
  Elf32_Shdr *section_headers = (Elf32_Shdr *)malloc(shentsize * shnum);
  fseek(fp, shoff, SEEK_SET);

  ret = fread(section_headers, shentsize, shnum, fp);
  assert(ret == shnum);

  // find the symbol table and string table 
  UNUSED Elf32_Shdr *symtab_section = NULL;
  UNUSED Elf32_Shdr *strtab_section = NULL;

  //for (int i = 0; i < shnum; i++) {
    //if (section_headers[i].sh_type == SHT_SYMTAB) {
      //symtab_section = &section_headers[i];
      //printf("%d\n", i);
    //} else if (section_headers[i].sh_type == SHT_STRTAB) {
      //strtab_section = &section_headers[i];
      //printf("%d\n", i);
    //}
  //}

  // Hold on , hold on . You can't loop so rude !
  // Here we need to know that the number of STRTAB sections is more than one, such as .strtab , .shstrtab, .dynstr, etc.
  // You may be confused that how can we include which is the string table for the symbol table in case that we could only 
  // judge by the section name , but section name is stored in the string table that we just want to find ...
  // The fact is that the ELF header has a field e_shstrndx, which is the key !

  // So we firstly use the e_shstrndx to find the section header string table, and then loop through and select by its name !
  Elf32_Shdr *shstrtab_section = &section_headers[shstrndx];

  // get the section header string table content 
  char *shstrtab = (char *)malloc(shstrtab_section->sh_size);
  fseek(fp, shstrtab_section->sh_offset, SEEK_SET);

  ret = fread(shstrtab, shstrtab_section->sh_size, 1, fp);
  assert(ret == 1);

  // Now we get the section header string table content, so we can loop through the section headers and find the right symbol table and string table by their names.
  // We only want to get .symtab and .strtab 
  for (int i = 0; i < shnum; i++) {
    const char *section_name = shstrtab + section_headers[i].sh_name;
    if (strcmp(section_name, ".symtab") == 0) {
      symtab_section = &section_headers[i];
    } else if (strcmp(section_name, ".strtab") == 0) {
      strtab_section = &section_headers[i];
    }
  }

  assert(symtab_section != NULL && strtab_section != NULL);

  // Secondly, we can read the symbol table and string table content 
  // read the symbol table 
  Elf32_Sym *symtab = (Elf32_Sym *)malloc(symtab_section->sh_size);
  fseek(fp, symtab_section->sh_offset, SEEK_SET);
  ret = fread(symtab, symtab_section->sh_size, 1, fp);
  assert(ret == 1);

  // loop through the symbol table and find the function symbols
  int symtab_num = symtab_section->sh_size / sizeof(Elf32_Sym);
  for (int i = 0; i < symtab_num; i++) {
    if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
      // get the function name from the string table 
      fseek(fp, strtab_section->sh_offset + symtab[i].st_name, SEEK_SET);
      char func_name[32];
      ret = fread(func_name, sizeof(func_name), 1, fp); 
      assert(ret == 1);

      // store the function info in the function table 
      strcpy(func_table[func_num].name, func_name);
      func_table[func_num].addr = symtab[i].st_value;
      func_table[func_num].size = symtab[i].st_size;
      func_num++;
    }
  }

  // print the function table
  printf("\nFunction table:\n");
  for (int i = 0; i < func_num; i++) {
    printf("Function %d: name = %s, addr = 0x%08x, size = %zu\n", i, func_table[i].name, func_table[i].addr, func_table[i].size);
  }
  printf("\n");

  // Now, we get the whole function info in the symbol table.
  // And this info will help the implement of ftrace function.

  return ;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {"elf"      , required_argument, NULL, 'e'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'e': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-e,--elf=ELF_FILE       load ELF file for ftrace\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Load the ELF file. Read the ELF file and get its symbol table info. */
  IFDEF(CONFIG_FTRACE, load_and_parse_elf(elf_file));

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
