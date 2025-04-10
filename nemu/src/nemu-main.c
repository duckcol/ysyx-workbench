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

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
	
	/* read expr test samples and check. */
	FILE *file = fopen("/home/coladuck/ysyx-workbench/nemu/tools/gen-expr/input", "r");
	Assert(file != NULL, "file open failed");

	char line[65536 + 128];
	while (fgets(line, sizeof(line), file) != NULL) {
		//	deal with line change
		long len = strlen(line);	
		if (len > 0 && line[len - 1] == '\n') {
			line[len - 1] = '\0';	
		}

		//	deal each line 
		word_t num; char *endptr;
		num = strtoull(line , &endptr, 10);
		Assert(endptr != line, "first character is not digit");
		printf("Num: "FMT_WORD"; Line: %s\n",num ,endptr);
	}

	//	some check
	Assert(ferror(file) == 0, "file reading error");
	fclose(file);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
