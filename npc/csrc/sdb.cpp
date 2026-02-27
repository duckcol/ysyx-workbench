#include "sdb.h"
#include "sim_set.h"

static int cmd_help(char *args);
static int cmd_si(char *args);
// static int cmd_info(char *args);
// static int cmd_x(char *args);
// static int cmd_p(char *args);
// static int cmd_w(char *args);
// static int cmd_d(char *args);
// static int cmd_q(char *args);

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
    // {"info", "info r: print all regs;info w: print all watchpoint",
    // cmd_info},
    // {"x",
    //  "x N EXPR: print 4*N bytes starting from EXPR(paddr, but will auto "
    //  "convert invalid paddr)",
    //  cmd_x},
    // {"p", "p $EXPR: print the compute result of $EXPR", cmd_p},
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
