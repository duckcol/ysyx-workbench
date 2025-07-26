/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "local-include/reg.h"
#include "debug.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
	int length = sizeof(regs)/sizeof(regs[0]);
	printf("Name\tHex\t\tData\n");
	for(int i = 0; i < length; i++) {
	//some regs should show as hex some should be in decimal
		if(i==0 //	tN is temp reg and aN is Argu/Return reg
				|| (i>=5 && i<=7) 
				|| (i>=10 && i<= 17) 
				|| (i>=28 && i<=31)) 
		{
			printf("%s\t0x%-8x\t%-8u\n", regs[i], gpr(i), gpr(i));
		} else {
			printf("%s\t0x%-8x\t0x%-8x\n", regs[i], gpr(i), gpr(i));
		}
	}
	printf("pc\t0x%x\t0x%x\n", cpu.pc, cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
	int length = sizeof(regs)/sizeof(regs[0]);
	for (int i = 1; i < length; i++) {
		*success = (strncmp(s+1, regs[i], 11) == 0) ? true : false;
		if (*success == true) break;
	}
	*success = (strncmp(s, regs[0], 11) == 0) ? true : *success;
	*success = (strncmp(s+1, "pc", 11) == 0) ? true : *success;
	if (*success) Info("reg found!\n"); else Info("reg not found!\n");
  return 0;
}
