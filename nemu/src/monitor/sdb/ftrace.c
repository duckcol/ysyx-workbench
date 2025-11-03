#include "sdb.h"
#include <elf.h>

void init_ftrace(const char *elf_file) {
  //  check if there is an elf file
  if (elf_file == NULL) {
    Log("no elf file input, ftrace malfunction");
    return;
  }

  //  read in elf file
  Log("The elf file is %s", elf_file);
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can't open elf_file '%s'", elf_file);

  //  read elf header
  Elf32_Ehdr elf_header;
  int ret;
  ret = fread(&elf_header, 1, sizeof(Elf32_Ehdr), fp);
  Assert(ret != 0, "read elf header error");

  //  find .symtab section
  fseek(fp, elf_header.e_shoff, SEEK_SET);
  // Elf32_Shdr symtab_shdr, strtab_shdr;
  for (int i = 0; i < elf_header.e_shnum; i++) {
    Elf32_Shdr section_header;
    ret = fread(&section_header, 1, sizeof(Elf32_Shdr), fp);
    Assert(ret != 0, "read section error");
    if (section_header.sh_type == SHT_SYMTAB) {
      Log("find symtab when shnum == %d", i);
      // symtab_shdr = section_header;
    } else if (section_header.sh_type == SHT_STRTAB) {
      Log("find strtab when shnum == %d", i);
      // strtab_shdr = section_header;
    }
  }
}
