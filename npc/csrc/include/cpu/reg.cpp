#include "reg.h"
#include "common.h"

CPU_STATE cpu = {};

void isa_reg_display() {
  int length = sizeof(regs) / sizeof(regs[0]);
  printf("Name\tHex\t\tData\n");
  for (int i = 0; i < length; i++) {
    // some regs should show as hex some should be in decimal
    if (i == 0 //	tN is temp reg and aN is Argu/Return reg
        || (i >= 5 && i <= 7) || (i >= 10 && i <= 17) || (i >= 28 && i <= 31)) {
      printf("%s\t0x%-8x\t%-8u\n", regs[i], gpr(i), gpr(i));
    } else {
      printf("%s\t0x%-8x\t0x%-8x\n", regs[i], gpr(i), gpr(i));
    }
  }
  printf("pc\t0x%-8x\t0x%-8x\n", cpu.pc, cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int length = sizeof(regs) / sizeof(regs[0]);
  word_t value;

  // search for reg name except $0 and pc
  for (int i = 1; i < length; i++) {
    *success = (strncmp(s + 1, regs[i], 11) == 0) ? true : false;
    if (*success == true) {
      value = gpr(i);
      INFO("reg found!");
      return value;
      break; //  maybe don't needed
    }
  }

  // search for reg $0
  *success = (strncmp(s, regs[0], 11) == 0) ? true : *success;
  value = *success ? gpr(0) : 0;
  if (*success) {
    INFO("reg found!");
    return value;
  }

  // search for reg pc
  *success = (strncmp(s + 1, "pc", 11) == 0) ? true : *success;
  value = *success ? cpu.pc : 0;
  if (*success) {
    INFO("reg found!");
    return value;
  } else {
    INFO("reg not found!");
  }
  TODO();
  return 0;
}
