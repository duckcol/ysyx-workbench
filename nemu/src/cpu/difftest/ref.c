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

#include "../local-include/reg.h"
#include "common.h"
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <isa.h>
#include <memory/paddr.h>

__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n,
                              bool direction) {
  // nemu is used as REF
  if (direction == DIFFTEST_TO_REF) {
    // set nemu's memory to be what passed in the function
    for (size_t i = 0; i < n; i++) {
      paddr_write(addr + i, 1, *((uint8_t *)buf + i));
    }
    for (size_t i = 0; i < n; i++) {
      if (i % 4 == 0)
        Info("addr: " FMT_PADDR " buf: " FMT_WORD "", (paddr_t)(addr + i),
             *(word_t *)((uint8_t *)buf + i));
    }
  } else if (direction == DIFFTEST_TO_DUT) {
    // set buf to be what nemu's memory is
    // *(word_t *)buf = paddr_read(addr, n);
    TODO();
  } else {
    assert(0);
  }
}

__EXPORT void difftest_regcpy(void *dut, bool direction) {
  // nemu is used as REF
  CPU_state *dut_cpu = (CPU_state *)dut;
  if (direction == DIFFTEST_TO_REF) {
    // set nemu's registerfile to be what passed in the function
    for (int i = 0; i < RISCV_GPR_NUM; i++) {
      gpr(i) = (word_t)(dut_cpu->gpr[i]);
      cpu.pc = (word_t)(dut_cpu->pc);
    }
  } else if (direction == DIFFTEST_TO_DUT) {
    // set dut to be what nemu's registerfile is
    for (int i = 0; i < RISCV_GPR_NUM; i++) {
      dut_cpu->gpr[i] = gpr(i);
      dut_cpu->pc = cpu.pc;
    }
  } else {
    assert(0);
  }
}

__EXPORT void difftest_exec(uint64_t n) { cpu_exec(n); }

__EXPORT void difftest_raise_intr(word_t NO) { assert(0); }

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
