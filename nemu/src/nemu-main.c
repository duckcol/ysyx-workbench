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

#include "debug.h"
#include <common.h>
#include <stdio.h>
#include <string.h>
#include "monitor/sdb/sdb.h"

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

void readin_expr_test() {
	FILE *file = fopen("/home/coladuck/ysyx-workbench/nemu/tools/gen-expr/input", "r");
	Assert(file != NULL, "file open failed");

	char line[65536 + 128];
	while (fgets(line, sizeof(line), file) != NULL) {
		//	deal with line change
		long len = strlen(line);	
		if (len > 0 && line[len - 1] == '\n') {
			line[len - 1] = '\0';	
		}

		//	deal with each line 
		//	store and change the number
		word_t num; char *expression;
		num = strtoull(line , &expression, 10);
		Assert(expression != line, "first character is not digit");
		bool success;
		word_t result = expr(expression, &success);
		if (result == num) {
			CORRECT("Num:"FMT_WORD"; Result:"FMT_WORD"; Success:%d; Expression:%s;\n", num, result, success, expression);
		} else {
			WARN("Num:"FMT_WORD"; Result:"FMT_WORD"; Success:%d; Expression:%s;\n", num, result, success, expression);
		}
	}

	//	some check
	Assert(ferror(file) == 0, "file reading error");
	fclose(file);
};

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
	
	/* read expr test samples and check. */
	readin_expr_test();

  /* Start engine. */
  //engine_start();

  return is_exit_status_bad();
}
