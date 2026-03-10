#include "sdb.h"
#include "common.h"
#include "cpu/sim_set.h"

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
// static int cmd_w(char *args);
// static int cmd_d(char *args);
static int cmd_q(char *args);

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
  Log("the args: %s", args); //	for dbg

  //	check if the args is NULL
  if (args == NULL) {
    INFO("You must type one args,plz try again");
    return 0;
  }

  //	check if the args is multiple
  char *arg = strtok(args, " ");
  Log("the arg: %s", arg);
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

static int cmd_q(char *args) { return -1; }

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
  char *EXPR = (char *)malloc(sizeof(char) * 20);
  strcpy(EXPR, token);

  //  second, compute the EXPR and turn into address
  //  the address
  bool success = false;
  vaddr_t address = (vaddr_t)expr(EXPR, &success);
  Assert(success == true, "$EXPR failed!");
  Log("the expr: " FMT_WORD "", address);
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
    printf(" " FMT_WORD "", vaddr_read(address + i * 4));
  }
  printf("\n");

  free(EXPR);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NPC", cmd_q},

    /* TODO: Add more commands */
    {"si", "si N:execute the N commands and stop", cmd_si},
    {"info", "info r: print all regs;info w: print all watchpoint", cmd_info},
    {"x",
     "x N EXPR: print 4*N bytes starting from EXPR(paddr, but will auto "
     "convert invalid paddr)",
     cmd_x},
    {"p", "p $EXPR: print the compute result of $EXPR", cmd_p},
    // {"w", "w $EXPR: stop program if $EXPR changes", cmd_w},
    // {"d", "d N: delete the watchpoint N which has the NO == N", cmd_d},

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

static int cmd_p(char *args) {
  Log("the args: %s", args);
  if (args == NULL) {
    WARN("no expr input !");
    return 0;
  }

  bool *success = (bool *)malloc(sizeof(bool));
  word_t result = success ? expr(args, success) : -1;
  if (success)
    INFO("expr success");
  else
    WARN("expr failed");
  printf("the result: " FMT_WORD "\n", result);
  // printf("the result: %u\n", (uint32_t)result);

  return 0;
}

void isa_reg_display();
static int cmd_info(char *args) {
  // check args:
  // 1.NULL or not
  // 2.only "r" and "w" is valid
  Log("the args: %s", args);

  if (args == NULL) {
    Log("need an arg, plz try again");
    return 0;
  }

  if (strcmp(args, "r") == 0) {
    isa_reg_display();
    return 0;
  }

  if (strcmp(args, "w") == 0) {
#ifdef CONFIG_WATCHPOINT
    info_w();
#else
    INFO("CONFIG_WATCHPOINT false, watchpoint disabled");
#endif
    return 0;
  } else {
    Log("invalid arg, plz try again");
    return 0;
  }
}

static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(npc) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

void sdb_mainloop() {

  // if (is_batch_mode) {
  //   cmd_c(NULL);
  //   return;
  // }

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

    // #ifdef CONFIG_DEVICE
    //     extern void sdl_clear_event_queue();
    //     sdl_clear_event_queue();
    // #endif

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
