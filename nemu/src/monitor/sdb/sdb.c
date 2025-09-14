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

#include "sdb.h"
#include "common.h"
#include "debug.h"
#include <complex.h>
#include <cpu/cpu.h>
#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdint.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  if (nemu_state.state != NEMU_END || nemu_state.halt_ret != 0) {
    nemu_state.state = NEMU_QUIT;
  }
  // added to solve the problem
  // when ``make run`` and then type ``q``
  // will result in error 1 in make
  // view nemu-main.c to see is_good_status_bad()
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "si N:execute the N commands and stop", cmd_si},
    {"info", "info r: print all regs;info w: print all watchpoint", cmd_info},
    {"x",
     "x N EXPR: print 4*N bytes starting from EXPR(paddr, but will auto "
     "convert invalid paddr)",
     cmd_x},
    {"p", "p $EXPR: print the compute result of $EXPR", cmd_p},
    {"w", "w $EXPR: stop program if $EXPR changes", cmd_w},
    {"d", "d N: delete the watchpoint N which has the NO == N", cmd_d},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  Info("the args: %s", args); //	for dbg

  //	check if the args is NULL
  if (args == NULL) {
    Log("You must type one args,plz try again");
    return 0;
  }

  //	check if the args is multiple
  char *arg = strtok(args, " ");
  Info("the arg: %s", arg);
  char *exceed = strtok(NULL, " "); // continue to split the args
  if (exceed != NULL) {
    Log("exceed args input, the exceeded part will be ignored");
    Log("the exceed starting at %s", exceed);
  }

  //	check if the arg's number is valid
  char *endptr;
  long time = strtol(arg, &endptr, 10);
  if (arg == endptr) {
    Log("no number, try again");
    return 0;
  }

  //	real to step in execution
  cpu_exec((int)time);

  return 0;
}

static int cmd_info(char *args) {
  // check args:
  // 1.NULL or not
  // 2.only "r" and "w" is valid
  Info("the args: %s", args);

  if (args == NULL) {
    Info("need an arg, plz try again");
    return 0;
  }

  if (strcmp(args, "r") == 0) {
    isa_reg_display();
    return 0;
  }

  if (strcmp(args, "w") == 0) {
    //	for test
    info_w();
    //	end test
    return 0;
  } else {
    Log("invalid arg, plz try again");
    return 0;
  }
}

static int cmd_x(char *args) {
  //	first, split the args to be N and EXPR

  //	the N part
  char *token = strtok(args, " ");
  char *endptr;
  int N = (int)strtol(token, &endptr, 10);
  Log("the N: %d", N);
  //	here are some checks to N
  //	check if there's a number
  if (token == endptr) {
    WARN("no number, try again");
    return 0;
  }
  //	check if the number > 0
  if (N <= 0) {
    WARN("N should be > 0, plz try again");
    return 0;
  }

  //	the EXPR part
  token = strtok(NULL, " ");
  if (token == NULL) {
    WARN("no EXPR found !");
    return 0;
  }
  char *EXPR = malloc(sizeof(char) * 20);
  strcpy(EXPR, token);

  //	second, compute the EXPR and turn into address
  // the address
  bool success = false;
  paddr_t address = (paddr_t)expr(EXPR, &success);
  Assert(success == true, "$EXPR failed!");
  Info("the expr: " FMT_WORD "", address);
  if (in_pmem(address)) {
    Log("the paddr: " FMT_PADDR "", address);
  } else {
    Log("not a valid paddr, consider in pmem");
    address = CONFIG_MBASE + address;
    Log("converted to paddr:" FMT_PADDR "", address);
  }

  //	then, print the memory around
  printf("address: content\n");
  printf("" FMT_PADDR ":", address);
  for (int i = 0; i < N; i++) {
    printf(" 0x%08x", vaddr_read(address + i * 4, 4));
  }
  printf("\n");

  free(EXPR);
  return 0;
}

static int cmd_p(char *args) {
  Info("the args: %s", args);
  if (args == NULL) {
    WARN("no expr input !");
    return 0;
  }

  bool *success = malloc(sizeof(bool));
  word_t result = success ? expr(args, success) : -1;
  if (success)
    Info("expr success");
  else
    WARN("expr failed");
  printf("the result: " FMT_WORD "\n", result);
  // printf("the result: %u\n", (uint32_t)result);

  return 0;
}

static int cmd_w(char *args) {
#ifdef CONFIG_WATCHPOINT
  //	check to args
  Info("the args: %s", args);
  if (args == NULL) {
    WARN("no expr input !");
    return 0;
  }

  //	check if the expr is valid or not
  bool *success = malloc(sizeof(bool));
  word_t result = success ? expr(args, success) : -1;
  if (success)
    Info("expr success, the first result: " FMT_WORD "", result);
  else {
    WARN("expr failed! try again.");
    return 0;
  }
  //	apply an new WP and set the content
  if (apply_and_set_WP(args, result))
    return 0;
  else {
    WARN("watchpoint apply failed!");
    return -1;
  }
#else
  Info("CONFIG_WATCHPOINT false, watchpoint disabled");
  return 0;
#endif
}

static int cmd_d(char *args) {
#ifdef CONFIG_WATCHPOINT
  //	check to args
  Info("the args: %s", args);
  if (args == NULL) {
    WARN("no N input !");
    return 0;
  }

  //	turn args into N
  char *token = strtok(args, " ");
  char *endptr;
  int N = (int)strtol(token, &endptr, 10);
  Log("the N: %d", N);
  //	here are some checks to N
  //	check if there's a number
  if (token == endptr) {
    WARN("no number, try again");
    return 0;
  }
  //	check if the number > 0
  if (N < 0) {
    WARN("N should be >= 0, plz try again");
    return 0;
  }

  //	delete the watchpoint N
  if (delete_WP(N)) {
    Info("delete the watchpoint %d. ", N);
    return 0;
  } else {
    WARN("watchpoint %d delete failed! ", N);
    return -1;
  }
#else
  Info("CONFIG_WATCHPOINT false, watchpoint disabled");
  return 0;
#endif
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  // Log("the is_batch_mode: %s", (is_batch_mode ? "True":"False"));
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
