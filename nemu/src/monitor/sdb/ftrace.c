#include "list.h"
#include "sdb.h"
#include <elf.h>

List *ftrace_log = NULL;

void init_ftrace(const char *elf_file) {
  //  check if there is an elf file
  if (elf_file == NULL) {
    Log("no elf file input, ftrace malfunction");
    return;
  }

  //  init ftrace log list
  ftrace_log = List_create();
  Assert(ftrace_log, "ftrace log init error");

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

  //  go through symtab for FUNC and push in ftrace_log list
  int num_symbols = symtab_shdr.sh_size / symtab_shdr.sh_entsize;
  for (int i = 0; i < num_symbols; i++) {
    Elf32_Sym *sym_entry = &symtab[i];
    if (ELF32_ST_TYPE(sym_entry->st_info) == STT_FUNC &&
        sym_entry->st_size != 0) {
      char *func_name = &strtab[sym_entry->st_name];
      Log("Found function: %s begin " FMT_PADDR " end " FMT_PADDR " bytes",
          func_name, sym_entry->st_value,
          sym_entry->st_value + sym_entry->st_size);

      func_log *a_log = calloc(1, sizeof(func_log));
      a_log->start = sym_entry->st_value;
      a_log->end = sym_entry->st_value + sym_entry->st_size;
      strncpy(a_log->name, func_name, 50 * sizeof(char));
      List_push(ftrace_log, a_log);
    }
  }

  free(strtab);
  free(symtab);
  free(shstrtab);
  fclose(fp);
}

void search_func_name(paddr_t pc, char *name) {
  LIST_FOREACH(ftrace_log, first, next, cur) {
    func_log log = *(func_log *)cur->value;
    if (log.start <= pc && pc < log.end) {
      strncpy(name, log.name, 50 * sizeof(char));
      return;
    }
  }
  Assert(0, "NOT found func name");
}

int level = 0;
void add_ftrace(word_t target, bool is_ret) {
  char name[50];
  // char blank[100] = "";
  search_func_name(target, name);
  // for (int i = level; i > 0; i--)
  //   strncat(blank, " ", 100);
  if (is_ret == 1) {
    Log("layer %d:ret to %s", level, name);
    level--;
  } else {
    Log("layer %d:jmp to %s", level, name);
    level++;
  }
}

void print_ftrace_log() {
  LIST_FOREACH(ftrace_log, first, next, cur) {
    func_log a_log = *(func_log *)cur->value;
    Log("Fn %s start at " FMT_PADDR " end at " FMT_PADDR "", a_log.name,
        a_log.start, a_log.end);
  }
  List_clear_destroy(ftrace_log);
}
