#include "pmem.h"
#include <assert.h>
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

word_t pmem_read(paddr_t pc) {
  Assert(pc < CONFIG_MBASE, "current pc %08x < BASE ADDR %08x", pc,
         CONFIG_MBASE);
  uint8_t *addr_in_pmem = pmem + pc - CONFIG_MBASE;
  word_t ret = *(word_t *)addr_in_pmem;
  return ret;
}

void pmem_write(paddr_t pc, word_t data) {
  uint8_t *addr_in_pmem = pmem + pc - CONFIG_MBASE;
  *(word_t *)addr_in_pmem = data;
  INFO("write data %08x into %p:%08x", data, addr_in_pmem,
       *(word_t *)addr_in_pmem);
}

word_t inst_I(uint32_t imm, uint32_t reg1, uint32_t funct3, uint32_t regd,
              uint32_t opcode) {
  imm &= 0xfff;
  reg1 &= 0x1f;
  funct3 &= 0x07;
  regd &= 0x1f;
  opcode &= 0x7f;

  uint32_t ret =
      (imm << 20) | (reg1 << 15) | (funct3 << 12) | (regd << 7) | (opcode);
  return ret;
}

word_t addi(uint32_t imm, uint32_t reg1, uint32_t regd) {
  return inst_I(imm, reg1, 0b000, regd, 0b0010011);
}

word_t ebreak() { return inst_I(0b1, 0, 0b000, 0, 0b1110011); }

#define inst_write(inst)                                                       \
  pmem_write(CONFIG_MBASE + i * 4, inst);                                      \
  i++;
void pmem_initial() {
  int i = 0;
  inst_write(addi(0xf, 0, 0));
  inst_write(addi(0xf, 0, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(ebreak());
  inst_write(addi(0x1, 7, 9));
  inst_write(addi(0x1, 7, 9));
  inst_write(ebreak());
  inst_write(addi(0x1, 7, 9));
  inst_write(addi(0x1, 7, 9));
}
