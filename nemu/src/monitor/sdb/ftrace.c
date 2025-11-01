#include "sdb.h"

void init_ftrace(const char *elf_file) {
  if (elf_file == NULL) {
    Log("no elf file input, ftrace malfunction");
    return;
  }
  Log("The elf file is %s", elf_file);
}
