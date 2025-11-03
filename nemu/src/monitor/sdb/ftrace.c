#include "debug.h"
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

  Log("the strtabndx: %d", elf_header.e_shstrndx);

  //  find .symtab .strtab and shstrtab section header
  fseek(fp, elf_header.e_shoff, SEEK_SET);
  Elf32_Shdr symtab_shdr, strtab_shdr, shstrtab_shdr;
  for (int i = 0; i < elf_header.e_shnum; i++) {
    Elf32_Shdr section_header;
    ret = fread(&section_header, 1, sizeof(Elf32_Shdr), fp);
    Assert(ret != 0, "read section error");

    //  to find section which name == 'symtab' and 'strtab'
    if (section_header.sh_type == SHT_SYMTAB) {
      Log("find symtab when shnum == %d", i);
      symtab_shdr = section_header;
    } else if (section_header.sh_type == SHT_STRTAB &&
               i != elf_header.e_shstrndx) {
      Log("find strtab when shnum == %d", i);
      strtab_shdr = section_header;
    } else if (i == elf_header.e_shstrndx) {
      Log("find shstrtab when shnum == %d", i);
      shstrtab_shdr = section_header;
    }
  }

  //  read in .symtab .strtab and .shstrtab data
  fseek(fp, symtab_shdr.sh_offset, SEEK_SET);
  Elf32_Sym *symtab = malloc(symtab_shdr.sh_size);
  ret = fread(symtab, 1, symtab_shdr.sh_size, fp);
  Assert(ret, "symtab readin error");

  fseek(fp, strtab_shdr.sh_offset, SEEK_SET);
  char *strtab = malloc(strtab_shdr.sh_size);
  ret = fread(strtab, 1, strtab_shdr.sh_size, fp);
  Assert(ret, "strtab readin error");

  fseek(fp, shstrtab_shdr.sh_offset, SEEK_SET);
  char *shstrtab = malloc(shstrtab_shdr.sh_size);
  ret = fread(shstrtab, 1, shstrtab_shdr.sh_size, fp);
  Assert(ret, "shstrtab readin error");

  Log("first str in shstrtab:%s", shstrtab + 1);
  Log("the symtab name:%s", shstrtab + symtab_shdr.sh_name);
  Log("the strtab name:%s", shstrtab + strtab_shdr.sh_name);
}
