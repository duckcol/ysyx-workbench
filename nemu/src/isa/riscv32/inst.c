/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "common.h"
#include "debug.h"
#include "isa.h"
#include "local-include/reg.h"
#include "macro.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_R,
  TYPE_I,
  TYPE_U,
  TYPE_S,
  TYPE_B,
  TYPE_J,
  TYPE_N, // none
};

#define src1R()                                                                \
  do {                                                                         \
    *src1 = R(rs1);                                                            \
  } while (0)
#define src2R()                                                                \
  do {                                                                         \
    *src2 = R(rs2);                                                            \
  } while (0)
#define immI()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 20), 12);                                          \
  } while (0)
#define immU()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12;                                    \
  } while (0)
#define immS()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);                   \
  } while (0)
// TODO: immB() and immJ() untested
#define immB()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) |          \
           (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1);                     \
  } while (0)
#define immJ()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) |        \
           (BITS(i, 20, 20) << 11) | (BITS(i, 30, 25) << 5) |                  \
           (BITS(i, 24, 21) << 1);                                             \
  } while (0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  switch (type) {
  case TYPE_R:
    src1R();
    src2R();
    break;
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_U:
    immU();
    break;
  // TODO: TYPE_R, TYPE_B, TYPE_J untested
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;
  case TYPE_B:
    src1R();
    src2R();
    immB();
    break;
  case TYPE_J:
    immJ();
    break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)                   \
  {                                                                            \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type));           \
    __VA_ARGS__;                                                               \
  }

  INSTPAT_START(); //	decode and printf
  //	example instructions
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U,
          R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I,
          R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S,
          Mw(src1 + imm, 1, src2));

  // instructions to run dummy.c
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I,
          R(rd) = src1 + imm);

  INSTPAT(
      "??????? ????? ????? ??? ????? 11011 11", jal, J,
      Info("jal: rd = %d(%s)  imm = " FMT_WORD "", rd, isa_reg_name(rd), imm);
      Info("jal: pc = " FMT_WORD ", snpc = " FMT_WORD ", dnpc = " FMT_WORD "",
           s->pc, s->snpc, s->dnpc);
      Info("jal: target dnpc = " FMT_WORD "", s->pc + imm); R(rd) = s->snpc;
      s->dnpc = s->pc + imm // dynamic next pc point to pc + imm
  );

  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S,
          Mw(src1 + imm, 4, src2));

  INSTPAT(
      "??????? ????? ????? 000 ????? 11001 11", jalr, I,
      Info("jalr: rd = %d(%s)  imm = " FMT_WORD " ", rd, isa_reg_name(rd), imm);
      Info("jalr: target dnpc = " FMT_WORD "", (src1 + imm) & ~((word_t)1));
      R(rd) = s->snpc; s->dnpc = (src1 + imm) & ~((word_t)1));

  // instructions to run add.c
  // lw rd offset(rs1): TYPE_I, load word
  // add rd rs1 rs2: TYPE_R, rd = rs1 + rs2
  // sub rd rs1 rs2: TYPE_R, rd = rs1 - rs2
  // sltiu rd rs1 imm: TYPE_I, rd = ((uint) rs1 < (uint) imm)
  // beq rs1 rs2 offset: if(rs1 == rs2) pc += offset
  // bne rs1 rs2 offset: if(rs1 != rs2) pc += offset
  //
  //  more instructions:
  //  Integer Computational instructions
  //  Integer Register-Immediate Instructions
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I,
          WARN("exec inst \"slti\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"slti\": convert imm uint32_t " FMT_WORD
               " to int32_t %d",
               imm, (int32_t)imm);
          R(rd) = ((int32_t)src1 < (int32_t)imm));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I,
          R(rd) = (src1 < imm));
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I,
          R(rd) = (src1 & imm));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I,
          R(rd) = (src1 | imm));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I,
          R(rd) = (src1 ^ imm));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, I,
          R(rd) = (src1 << imm));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai, I,
          WARN("exec inst \"srai\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          R(rd) = (word_t)((int32_t)src1 >> imm));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli, I,
          R(rd) = (src1 >> imm));
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);

  //  Integer Register-Register Operations
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R,
          R(rd) = src1 + src2);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R,
          WARN("exec inst \"slt\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"slt\": convert src2 uint32_t " FMT_WORD
               " to int32_t %d",
               src2, (int32_t)src2);
          R(rd) = ((int32_t)src1 < (int32_t)src2));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R,
          R(rd) = (src1 < src2));
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R,
          R(rd) = (src1 & src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R,
          R(rd) = (src1 ^ src2));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R,
          R(rd) = (src1 | src2));
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R,
          WARN("exec inst \"sll\": src2 " FMT_WORD
               " only use lowwer 5 bits " FMT_WORD "",
               src2, (word_t)BITS(src2, 4, 0));
          R(rd) = (src1 << (word_t)BITS(src2, 4, 0)));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R,
          WARN("exec inst \"srl\": src2 " FMT_WORD
               " only use lowwer 5 bits " FMT_WORD "",
               src2, (word_t)BITS(src2, 4, 0));
          R(rd) = (src1 >> (word_t)BITS(src2, 4, 0)));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R,
          WARN("exec inst \"sra\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"sra\": src2 " FMT_WORD
               " only use lowwer 5 bits " FMT_WORD "",
               src2, (word_t)BITS(src2, 4, 0));
          R(rd) = (word_t)((int32_t)src1 >> (word_t)BITS(src2, 4, 0)));
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R,
          R(rd) = (src1 - src2));

  //  M Standard Extension for Multiplication and Division
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R,
          //  the result is the lowwer 32 bits of src1 * src2
          R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R,
          WARN("exec inst \"mulh\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"mulh\": convert src2 uint32_t " FMT_WORD
               " to int32_t %d",
               src2, (int32_t)src2);
          int64_t tmp = (int64_t)src1 * (int64_t)src2;
          Info("mulh: full result is "
               "0x%016" PRIx64 ", high 32 bits is " FMT_WORD "",
               tmp, (word_t)BITS(tmp, 63, 32));
          R(rd) = (word_t)BITS(tmp, 63, 32));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu, R,
          R(rd) = BITS((int64_t)((int32_t)src1 * (uint32_t)src2), 63, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R,
          R(rd) = BITS((uint64_t)((uint32_t)src1 * (uint32_t)src2), 63, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div, R,
          if (src2 == 0) R(rd) = -1;
          else R(rd) = (int32_t)src1 / (int32_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R,
          if (src2 == 0) R(rd) = -1;
          else R(rd) = (uint32_t)src1 / (uint32_t)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R,
          if (src2 == 0) R(rd) = src1;
          else R(rd) = (int32_t)src1 % (int32_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R,
          if (src2 == 0) R(rd) = src1;
          else R(rd) = (uint32_t)src1 % (uint32_t)src2);

  //  Conditional Branches
  INSTPAT("? ?????? ????? ????? 000 ???? ? 11000 11", beq, B,
          if (src1 == src2) s->dnpc = s->pc + imm);
  INSTPAT("? ?????? ????? ????? 001 ???? ? 11000 11", bne, B,
          if (src1 != src2) s->dnpc = s->pc + imm);
  INSTPAT("? ?????? ????? ????? 111 ???? ? 11000 11", bgeu, B,
          if (src1 >= src2) s->dnpc = s->pc + imm);
  INSTPAT("? ?????? ????? ????? 101 ???? ? 11000 11", bge, B,
          WARN("exec inst \"bge\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"bge\": convert src2 uint32_t " FMT_WORD
               " to int32_t %d",
               src2, (int32_t)src2);
          if ((int32_t)src1 >= (int32_t)src2) s->dnpc = s->pc + imm);
  INSTPAT("? ?????? ????? ????? 110 ???? ? 11000 11", bltu, B,
          if (src1 < src2) s->dnpc = s->pc + imm);
  INSTPAT("? ?????? ????? ????? 100 ???? ? 11000 11", blt, B,
          WARN("exec inst \"blt\": convert src1 uint32_t " FMT_WORD
               " to int32_t %d",
               src1, (int32_t)src1);
          WARN("exec inst \"blt\": convert src2 uint32_t " FMT_WORD
               " to int32_t %d",
               src2, (int32_t)src2);
          if ((int32_t)src1 < (int32_t)src2) s->dnpc = s->pc + imm);

  //  Load and Store instructions
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I,
          R(rd) = SEXT(BITS(Mr(src1 + imm, 1), 7, 0), 8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I,
          R(rd) = SEXT(BITS(Mr(src1 + imm, 2), 15, 0), 16));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I,
          R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I,
          R(rd) = Mr(src1 + imm, 2));

  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S,
          Mw(src1 + imm, 2, src2));

  //  ending instructions
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N,
          NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, N, INV(s->pc));
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  //  inst_fetch() will update snpc to pc + 4
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  //  decode_exec() will decode and execute current pc inst
  //  and it will let dnpc == snpc by default
  return decode_exec(s);
}
