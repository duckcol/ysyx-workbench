/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>

// functions for csr read and write
word_t csr_read(word_t csr_num);
void csr_write(word_t csr_num, word_t wdata);
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  csr_write(0x341, epc); // set mepc
  csr_write(0x342, NO);  // set mcause
  word_t mtvec_addr = csr_read(0x305);
#ifdef CONFIG_ETRACE
  _Log("\n[etrace] mepc = " FMT_WORD "; mcause = " FMT_WORD
       "; mtvec = " FMT_WORD "\n",
       epc, NO, mtvec_addr);
#endif
  return mtvec_addr; // return mtvec for JMP
}

word_t isa_query_intr() { return INTR_EMPTY; }
