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
#include <memory/vaddr.h>
#include <memory/paddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ,
	TK_DIGIT, TK_PARTH, TK_AND,
	TK_DEREF,
	TK_HEX, TK_REG

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
  {"!=", TK_NEQ},        // not equal
	{"&&", TK_AND},				// AND
	{"\\$[a-z0-9]+",TK_REG},//	reg name
	{"0[xX][a-fA-F0-9]+", TK_HEX},	//	hex number
	{"[0-9]+", TK_DIGIT},		// digit in POSIX regex
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
																
					case TK_NEQ:
					case TK_EQ: 
					case '+': 
					case '*': 
					case '-': 
					case '/':
					case TK_AND:
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

					case TK_HEX://  for hex numbers, record and turn into str
						tokens[nr_token].type = rules[i].token_type;
						Assert(substr_len < 11 ,"the hex digit's len is to long");
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';//	really important
						Info("tokens[%d].type: %d, str: %s",
								nr_token,tokens[nr_token].type,tokens[nr_token].str);
						nr_token++;
						break;

					case TK_REG://	for register, record the name of the register and turn into str
						tokens[nr_token].type = rules[i].token_type;
						Assert(substr_len < 10, "too long for a reg name");
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
	Assert(tokens[p].type != ')', "wrong parenthese, starting with ')', instead of '('");

	if(tokens[p].type == '(') {
		//	in case: "()"
		if(tokens[q].type == ')' && q == p + 1) return true;
		//	for other cases like "( expr )"
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
		Info("expr p > q");
		Assert(0, "bad expr: p:%d, q:%d", p, q);

	} else if(p == q) {
		Info("expr p == q");
		/*should be a single number*/
		if(tokens[p].type == TK_DIGIT) {
			char* endptr;
			word_t value = strtoul(tokens[p].str, &endptr, 10);
			if(*endptr != '\0') Assert(0, "not a number");
			return value;
		} else if (tokens[p].type == TK_HEX) {
			char* endptr;
			word_t value = strtoul(tokens[p].str, &endptr, 16);
			if(*endptr != '\0') Assert(0, "not a number");
			return value;
		} else if (tokens[p].type == TK_REG) {
			bool success = false;
			Info("the reg name:%s\n", tokens[p].str);
			word_t value = isa_reg_str2val(tokens[p].str, &success);
			return (success) ? value : -1;
		} else {
			//Assert(0, "should be a number");
			return 0;
		}

	} else if (check_parentheses(p, q) == true) {
			//the expr surrounded by a matched parentheses,
			//remove them and eval
			Info("expr in parentheses, remove the parentheses");
			if (q == p + 1) return 0;//in case "()"
			else return eval(p+1, q-1);	

	} else {
			//find the main op and eval, for example "1 + (2 + 3) / 4"

			//find main op, for example "(val) op (val)"
			Info("expr not in parentheses");
			int surrounded = 0; int op = 0;
			for(int i = p; i <= q; i++) {

				if (tokens[i].type == '(') surrounded++;
				if (tokens[i].type == ')') surrounded--;

				if (surrounded == 0) {// skip the expr surrounded by parentheses
					//	find mian op "==" and "!="
					if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) op = i;

					//	find main op "&&"
					if (tokens[i].type == TK_AND) op = i;

					//	find main op '+' 
					if (tokens[i].type == '+') op = i;

					//	find main op '-'
					//	differ "a-b" and "-a"
					if (tokens[i].type == '-') {
						// differ case "p (x + -y)" and "p -x"
						if (tokens[i-1].type == '+' || 
							  tokens[i-1].type == '-' ||
							  tokens[i-1].type == '*' ||
							  tokens[i-1].type == '/') {
							// in case p (x op -y)
							// when eval the "-y" in "x op -y"
							// op == i == p 
							if (i == p) op = i;  
							// when eval the "op" in "x op -y"
							// op == position of "op"
							else op = i-1; 
						} else {
							//	in case "x - y"
							op = i;
						}
					}

					//	find main op '/'
					if (tokens[i].type == '/') {
							if (tokens[op].type != '+' && tokens[op].type != '-') op = i;		
					}

					//	find main op '*'
					//	differ "muliple" and "DEREF"
					if (tokens[i].type == '*') {
						if (tokens[i-1].type == '+' || 
							  tokens[i-1].type == '-' ||
							  tokens[i-1].type == '*' ||
							  tokens[i-1].type == '/' ||
								i == 0) {
							if (i == p) {op = i; tokens[op].type = TK_DEREF;}
						} else if (tokens[op].type != '+' && tokens[op].type != '-') 
							op = i;
					}

				}
			}	

			Info("the main op found, type:%c , position:%d", tokens[op].type, op);

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
				case TK_DEREF:
					if(in_pmem(val2)) {
						Info("the paddr: "FMT_PADDR"", val2);
					} else {
						WARN("not a valid paddr, consider in pmem");
						val2 = CONFIG_MBASE + val2;
						WARN("converted to paddr:"FMT_PADDR"", val2);
					}
					return vaddr_read(val2, 4);
				case TK_EQ: return (val1 == val2) ? 1 : 0;
				case TK_NEQ: return (val1 != val2) ? 1 : 0;
				case TK_AND: return (val1 && val2) ? 1 : 0;
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
