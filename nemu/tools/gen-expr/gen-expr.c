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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format = "#include <stdio.h>\n"
                           "#include <stdint.h>\n"
                           "int main() { "
                           "  uint32_t result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

#define choose(max) rand() % max

#define BUFEND buf + strlen(buf)
#define INSERT_BLANK                                                           \
  for (int i = 0; i < (rand() % 10); i++) {                                    \
    sprintf(BUFEND, " ");                                                      \
  }
static void gen_rand_expr() {
  // prevent buf from overflowing
  if (BUFEND - buf >= 60000) {
    sprintf(BUFEND, "1");
    return;
  }
  switch (choose(3)) {
  case 0:
    // gen_num
    INSERT_BLANK;
    sprintf(BUFEND, "%uul", (uint32_t)choose(1000));
    INSERT_BLANK;
    break;
  case 1:
    // gen_parenthese :=
    // gen('('); gen_rand_expr(); gen(')');
    sprintf(BUFEND, "(");
    INSERT_BLANK;
    gen_rand_expr();
    INSERT_BLANK;
    sprintf(BUFEND, ")");
    break;
  default:
    // actually it's for case 2
    // gen_whole_expr :=
    // gen_rand_expr(); gen_rand_op(); gen_rand_expr();
    gen_rand_expr();
    switch (choose(4)) {
    case 0:
      sprintf(BUFEND, "+");
      break;
    case 1:
      sprintf(BUFEND, "-");
      break;
    case 2:
      sprintf(BUFEND, "*");
      break;
    default:
      sprintf(BUFEND, "/");
      break;
    }
    gen_rand_expr();
    break;
  }
}

int main(int argc, char *argv[]) {
  //	some initail
  int seed = time(0);
  srand(seed);

  //	if loop == 1; it will only run test expr
  //	if loop != 1; then it will gen random expr
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++) {
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();

    if (loop == 1) {
      sprintf(buf, "%s", "(    (  156u      )-  423u  )/132u     ");
      sprintf(code_buf, code_format, buf);
    } else {
      sprintf(code_buf, code_format, buf);
    }

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -Wall -Werror -o /tmp/.expr");
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result;
    ret = fscanf(fp, "%u", &result);
    int status = pclose(fp);
    if (WEXITSTATUS(status) == 136)
      printf("0 0\n");
    else
      printf("%u %s\n", result, buf);
  }
  return 0;
}
