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

#include "common.h"
#include "debug.h"
#include <stdio.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DIGIT, TK_PARTH

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
	{"-", '-'},					  // sub
	{"\\*", '*'},					// mul
	{"/", '/'},						// div
  {"==", TK_EQ},        // equal
	{"[[:digit:]]+", TK_DIGIT},		// digit in POSIX regex
	{"\\(", '('},
	{"\\)", ')'},						//	parenthese
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
			Assert((nr_token <= 65536), "tokens[65536] is full");// prevent the length of digit is too long
																										 
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NOTYPE: break;//	for blank, do nothing
																
					case TK_EQ: 
					case '+': 
					case '*': 
					case '-': 
					case '/':
					case ')':
					case '(':							//	for sign, record in tokens
						tokens[nr_token].type = rules[i].token_type; 
						Info("tokens[%d].type: %d, str: %c",
								nr_token,tokens[nr_token].type,(char)tokens[nr_token].type);
						nr_token++; 
						break;

					case TK_DIGIT://	for digit, record and turn into str 
						tokens[nr_token].type = rules[i].token_type;
						Assert(substr_len < 31 ,"the digit's len is to long");
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';//	really important
						Info("tokens[%d].type: %d, str: %s",
								nr_token,tokens[nr_token].type,tokens[nr_token].str);
						nr_token++;
						break;

          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

	Log("the length of tokens:%d",nr_token);
  return true;
}

bool check_parentheses(int p, int q) {
	int count = 0;
	//	to deal with ")..."
	Assert(tokens[p].type != ')', "wrong position of parenthese");

	if(tokens[p].type == '(') {
		//	in case: "()"
		if(tokens[q].type == ')' && q == p + 1) return true;
		//	for other cases
		for(int i = p + 1; i < q; i++) {
			if (tokens[i].type == '(') count++;
			if (tokens[i].type == ')') count--;
			if (count < 0) return false;
		}
		if (count == 0) return true;
		else return false;
	} else {
		//	not starting with an parenthese
		return false;
	}
}

word_t eval(int p, int q) {
	if(p > q) {
		/* bad expr*/
		Assert(0, "bad expr: p:%d, q:%d", p, q);

	} else if(p == q) {
		/*should be a single number*/
		if(tokens[p].type == TK_DIGIT) {
			char* endptr;
			unsigned long value = strtoul(tokens[p].str, &endptr, 10);
			if(*endptr != '\0') Assert(0, "not a number");	
			return value;
		} else {
			//Assert(0, "should be a number");
			return 0;
		}

	} else if (check_parentheses(p, q) == true) {
			//the expr surrounded by a matched parentheses,
			//remove them and eval
			if (q == p + 1) return 0;//in case "()"
			return eval(p+1, q-1);	

	} else {
			//find the main op and eval, consider "1 + (2 + 3) / 4"

			//find main op, for case:"(val) op (val)"
			int surrounded = 0; int op = 0;
			for(int i = p; i < q; i++) {

				if (tokens[i].type == '(') surrounded++;
				if (tokens[i].type == ')') surrounded--;

				if (surrounded == 0) {
					//	find find main op '+' or '-'
					if (tokens[i].type == '+') op = i;
					if (tokens[i].type == '-') {
						// to differ "a-b" and "-a"
						if (tokens[i-1].type == '+' || 
							  tokens[i-1].type == '-' ||
							  tokens[i-1].type == '*' ||
							  tokens[i-1].type == '/') {
							if (i == p) { // when deal with "-1"
								op = i; // op == i == p
							} else { // when deal with "+-1"
								op = i-1;
							}
						} else {
							op = i;
						}
					}

					//	find main op '*' or '/'
					if (tokens[i].type == '*' || tokens[i].type == '/') {
							if (tokens[op].type != '+' && tokens[op].type != '-') {
							op = i;	
						}
					}
				}
			}	

			//do compute
			//in case "-1", which is actually 0-1, but p=0, op=0,
			//so val1 should be 0 instand of eval(0, 0-1), which Assert program
			word_t val1 = (p == op) ? 0 : eval(p, op - 1);
			word_t val2 = eval(op + 1, q);
			switch (tokens[op].type) {
				case '+': return val1 + val2;
				case '-': return val1 - val2;
				case '*': return val1 * val2;
				case '/': Assert(val2 != 0, "can't div by 0"); return val1 / val2;	
				default: Assert(0, "wrong in compute");
			}
	}
	return 0;
}

word_t expr(char *e, bool *success) {
	*success = true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token-1);
	// for debug
	//printf("the result:"FMT_WORD"\n", eval(0, nr_token-1));
	//TODO();
}
