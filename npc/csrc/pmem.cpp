#include "pmem.h"
#include <cstdint>

uint8_t *guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

word_t pmem_read(paddr_t pc) {
  Assert(pc >= CONFIG_MBASE, "current pc %08x < BASE ADDR %08x", pc,
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

//  instructions defined
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

word_t inst_U(uint32_t imm, uint32_t regd, uint32_t opcode) {
  imm &= 0xfffff; // the imm you type is the high 20 bits
  regd &= 0x1f;
  opcode &= 0x7f;

  uint32_t ret = (imm << 12) | (regd << 7) | (opcode);
  return ret;
}

word_t inst_J(uint32_t imm, uint32_t regd, uint32_t opcode) {
  // 处理符号扩展和21位截断（保留低21位，最高位作为符号位）
  int32_t simm = (int32_t)((int32_t)imm << 11) >> 11; // 保留21位有符号值
  simm &= 0x1fffff;                                   // 确保21位

  // 提取各段立即数
  uint32_t imm20 = (simm >> 20) & 0x1;     // bit 20 -> bit 31
  uint32_t imm10_1 = (simm >> 1) & 0x3ff;  // bits 10:1 -> bits 30:21
  uint32_t imm11 = (simm >> 11) & 0x1;     // bit 11 -> bit 20
  uint32_t imm19_12 = (simm >> 12) & 0xff; // bits 19:12 -> bits 19:12

  regd &= 0x1f;
  opcode &= 0x7f;

  uint32_t ret = (imm20 << 31) | (imm10_1 << 21) | (imm11 << 20) |
                 (imm19_12 << 12) | (regd << 7) | opcode;

  return ret;
}

word_t inst_S(uint32_t imm, uint32_t reg1, uint32_t reg2, uint32_t funct3,
              uint32_t opcode) {
  // 处理12位有符号立即数
  int32_t simm = (int32_t)((int32_t)imm << 20) >> 20; // 保留12位有符号值
  simm &= 0xfff;

  uint32_t imm11_5 = (simm >> 5) & 0x7f; // 高7位 -> bits 31:25
  uint32_t imm4_0 = simm & 0x1f;         // 低5位 -> bits 11:7

  reg1 &= 0x1f;
  reg2 &= 0x1f;
  funct3 &= 0x07;
  opcode &= 0x7f;

  uint32_t ret = (imm11_5 << 25) | (reg2 << 20) | (reg1 << 15) |
                 (funct3 << 12) | (imm4_0 << 7) | opcode;
  return ret;
}

word_t addi(uint32_t imm, uint32_t reg1, uint32_t regd) {
  return inst_I(imm, reg1, 0b000, regd, 0b0010011);
}

word_t auipc(uint32_t imm, uint32_t regd) {
  return inst_U(imm, regd, 0b0010111);
}

word_t lui(uint32_t imm, uint32_t regd) { return inst_U(imm, regd, 0b0110111); }

word_t jal(uint32_t imm, uint32_t regd) { return inst_J(imm, regd, 0b1101111); }

word_t jalr(uint32_t imm, uint32_t reg1, uint32_t regd) {
  return inst_I(imm, reg1, 0b000, regd, 0b1100111);
}

word_t sw(uint32_t imm, uint32_t reg1, uint32_t reg2) {
  return inst_S(imm, reg1, reg2, 0b010, 0b0100011);
}

word_t ebreak() { return inst_I(0b1, 0, 0b000, 0, 0b1110011); }

//  testing instructions write in pmem
#define inst_write(inst)                                                       \
  pmem_write(CONFIG_MBASE + i * 4, inst);                                      \
  i++;
void pmem_initial() {
  int i = 0;
  inst_write(addi(0xf, 0, 7));
  inst_write(jal(12, 1));
  inst_write(addi(0x1, 0, 7));
  inst_write(ebreak());
  inst_write(addi(0xf, 0, 7)); // should jump to here
  inst_write(addi(0x1, 7, 7));
  inst_write(auipc(0x1, 6));
  inst_write(addi(0x101, 0, 7));
  inst_write(lui(0x1010, 7));
  inst_write(sw(0x101, 7, 8));
  inst_write(addi(0x1, 7, 7));
  inst_write(addi(0x1, 7, 7));
  inst_write(ebreak());
  inst_write(addi(0x1, 7, 9));
  inst_write(addi(0x1, 7, 9));
  inst_write(ebreak());
  inst_write(addi(0x1, 7, 9));
  inst_write(addi(0x1, 7, 9));
}
