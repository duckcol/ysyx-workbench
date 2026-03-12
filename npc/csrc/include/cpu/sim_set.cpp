#include "sim_set.h"
#include "common.h"
#include "cpu/decode.h"
#include "ftrace.h"
#include "reg.h"

VerilatedContext *contextp = NULL;
VerilatedFstC *tfp = NULL;

//  TOP_NAME is ``Vantpc``, which is a macro defined by makefile
TOP_NAME *top;

int sim_init() {
  contextp = new VerilatedContext;
  tfp = new VerilatedFstC;
  top = new TOP_NAME;
  contextp->traceEverOn(true);
  top->trace(tfp, 0);
  tfp->open("top.fst");

  top->sys_rst_l = 0;
  top->clk = 0;
  step_times(4);
  top->sys_rst_l = 1;

  return 0;
}

int sim_exit() {
  step_and_dump_wave();
  tfp->close();
  delete tfp;
  delete top;
  delete contextp;

  return 0;
}

int step_and_dump_wave() {
  top->eval();
  // if (top->pmem_read_addr >= 0x80000000)
  //   top->pmem_read_result = paddr_read(top->pmem_read_addr);
  // top->eval();
  contextp->timeInc(1);
  tfp->dump(contextp->time());

  return 0;
};

int step_times(int n) {
  for (int i = 0; i <= n; i++) {
    // top->clk = 1;
    // step_and_dump_wave();
    // top->clk = 0;
    // step_and_dump_wave();
    top->clk = ~top->clk;
    step_and_dump_wave();
  }
  return n;
}

void difftest_step(vaddr_t pc, vaddr_t npc);
void cpu_exec(uint64_t n) {
  for (; n > 0; n--) {
    if (ebreak_flag == 1) {
      printf(
          "Program execution has ended. To restart the program, exit NPC and "
          "run again.\n");
      return;
    }
    step_times(1);
    if (inst_decode.pc >= RESET_VECTOR)
      IFDEF(CONFIG_DIFFTEST,
            difftest_step(
                inst_decode.pc,
                (inst_decode.dnpc ? inst_decode.snpc : inst_decode.dnpc)));
  }
}

int halt_ret = 1;
int ebreak_flag = 0;
extern "C" void trigger_ebreak() {
  Log("triggering inst ebreak");
  ebreak_flag = 1;

  // check $a0 or R(10) to see if it is 0
  top->debug_reg_addr = 10;
  word_t reg10_data = top->debug_reg_data;
  INFO("current reg10 data: %08x", reg10_data);
  halt_ret = reg10_data;
}

extern "C" void sync_rf_data(uint32_t addr, uint32_t data) {
  INFO("sync reg data");
  gpr(addr) = data;
  INFO("sync reg ends");
  return;
}

extern "C" void sync_pc_data(uint32_t pc) {
  INFO("sync pc data");
  cpu.pc = pc;
  INFO("sync pc ends");
  return;
}

extern "C" void disassemble(char *str, int size, uint64_t pc, uint8_t *code,
                            int nbyte);
std::string get_asm_mnemonic(uint32_t inst, uint32_t pc) {
  char buf[128];
  uint8_t bytes[4];

  // 注意端序！RISC-V 是小端序 (Little Endian)
  // inst 是 uint32_t，内存中低位在前
  bytes[0] = inst & 0xFF;
  bytes[1] = (inst >> 8) & 0xFF;
  bytes[2] = (inst >> 16) & 0xFF;
  bytes[3] = (inst >> 24) & 0xFF;

  disassemble(buf, sizeof(buf), pc, bytes, 4);

  return std::string(buf);
}

Decode inst_decode;
#ifdef CONFIG_ITRACE
int push_iringbuff(char *inst, bool bad_ending);
void itrace(word_t inst, word_t pc) {
  //  disassemle the inst and print it out
  std::string asm_str = get_asm_mnemonic((uint32_t)inst, (uint32_t)pc);
  _Log("" FMT_WORD ":\t" FMT_WORD "\t%s\n", pc, inst, asm_str.c_str());
  snprintf(inst_decode.logbuf, 128, "" FMT_WORD ":\t" FMT_WORD "\t%s", pc, inst,
           asm_str.c_str());
  push_iringbuff(inst_decode.logbuf, 0);
}
#else
void itrace(word_t inst, word_t pc);
#endif

#ifdef CONFIG_FTRACE
void ftrace(word_t inst, word_t pc, word_t dnpc, word_t snpc) {
  uint8_t opcode, funct3;
  opcode = inst & 0x7F;
  funct3 = (inst >> 12) & 0x03;

  if (opcode == 0b01101111) {
    _Log("jal  pc: " FMT_WORD " dnpc: " FMT_WORD " snpc: " FMT_WORD "\n", pc,
         dnpc, snpc);
    add_ftrace(dnpc, 0);
  } else if (funct3 == 0x00 && opcode == 0b01100111) {
    _Log("jalr pc: " FMT_WORD " dnpc: " FMT_WORD " snpc: " FMT_WORD "\n", pc,
         dnpc, snpc);
    uint8_t rs1 = (inst >> 15) & 0x1F;
    word_t offset = (inst >> 20) & 0xFFFF;
    if (rs1 == 1 && offset == 0)
      _Log("this inst is a ret\n");

    add_ftrace(dnpc, (rs1 == 1 && offset == 0));
  }
}
#else
void ftrace(word_t inst, word_t pc, word_t dnpc, word_t snpc);
#endif

extern "C" void trace_instruction(word_t inst, word_t pc, word_t dnpc,
                                  word_t snpc) {
  inst_decode.pc = pc;
  inst_decode.snpc = snpc;
  inst_decode.dnpc = dnpc;
  inst_decode.isa.inst.val = inst;
  itrace(inst, pc);
  INFO("snpc: " FMT_WORD " dnpc: " FMT_WORD "", snpc, dnpc);
  ftrace(inst, pc, dnpc, snpc);
}

extern "C" int pmem_read(int raddr) {
  // 总是读取地址为`raddr & ~0x3u`的4字节返回
  // raddr & 0x3u is for 4 byte alignment
  word_t ret;
  INFO("raddr & ~0x3u =" FMT_PADDR "", (paddr_t)raddr & ~0x3u);
  if ((raddr & ~0x3u) < CONFIG_MBASE) {
    WARN("raddr " FMT_PADDR " < CONFIG_MBASE " FMT_PADDR
         " read in 0x8000000 data",
         raddr & ~0x3u, CONFIG_MBASE);
    ret = paddr_read(CONFIG_MBASE);
  } else {
    ret = paddr_read((paddr_t)raddr & ~0x3u);
  }
  // word_t ret = paddr_read(raddr & ~0x3u);
  Log("read data " FMT_WORD " from addr " FMT_PADDR "", ret,
      (paddr_t)raddr & ~0x3u);
  return ret;
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  // 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
  // `wmask`中每比特表示`wdata`中1个字节的掩码,
  // 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
  word_t wdata_after_mask;
  switch (wmask) {
  case (0x1):
    wdata_after_mask = wdata & 0x000F;
    break;
  case (0x3):
    wdata_after_mask = wdata & 0x00FF;
    break;
  case (0xF):
    wdata_after_mask = wdata & 0xFFFF;
    break;
  default:
    Assert(0, "wmask is %X not supported", (uint8_t)wmask);
  }
  Log("write data " FMT_WORD " into addr " FMT_PADDR "", wdata_after_mask,
      waddr & ~0x3u);
  paddr_write(waddr & ~0x3u, wdata & wmask);
}
