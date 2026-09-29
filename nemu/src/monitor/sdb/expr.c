/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

extern word_t vaddr_read(vaddr_t addr, int len);

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_NUMBER, TK_NEG, TK_NEQ, AND, DERER,
  TK_HEX, TK_REG

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
  {"==", TK_EQ},        // equal
	{"!=", TK_NEQ},       // not equal
  {"&&", AND},          // and
	{"-", '-'},      		// sub
	{"\\*", '*'},					// mul
	{"/", '/'},					// divide
	{"\\(", '('}, 				// (
	{",", ','},						// ,
	{"\\)", ')'},					// )
	{"[0-9]+", TK_NUMBER}, // a decimal number 
  {"0[xX][0-9a-fA-F]+", TK_HEX}, // a hex number 
  {"\\$[a-zA-Z0-9]*", TK_REG}, // a register name
	
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

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
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
					case TK_EQ:  
            tokens[nr_token].type = TK_EQ;
            tokens[nr_token].str[0] = '=';
            nr_token ++;
            break;
          case '+':
            tokens[nr_token].type = '+';
            tokens[nr_token].str[0] = '+';
            nr_token ++;
            break;
          case '-':
            tokens[nr_token].type = '-';
            tokens[nr_token].str[0] = '-';
            nr_token ++;
            break;
          case '*':
            tokens[nr_token].type = '*';
            tokens[nr_token].str[0] = '*';
            nr_token ++;
            break;
          case '/':
            tokens[nr_token].type = '/';
            tokens[nr_token].str[0] = '/';
            nr_token ++;
            break;
          case '(':
            tokens[nr_token].type = '(';
            tokens[nr_token].str[0] = '(';
            nr_token ++;
            break;
          case ')':
            tokens[nr_token].type = ')';
            tokens[nr_token].str[0] = ')';
            nr_token ++;
            break;
          case TK_NUMBER:
            tokens[nr_token].type = TK_NUMBER;
            // here we need to make sure the length of the number is shorter than 32
            Assert(substr_len < 32, "Number token is too long than 32");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;
          case TK_HEX:
            tokens[nr_token].type = TK_HEX;
            Assert(substr_len < 32, "Hex token is too long than 32");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;
          case TK_NEQ:
            tokens[nr_token].type = TK_NEQ;
            tokens[nr_token].str[0] = '!';
            tokens[nr_token].str[1] = '=';
            tokens[nr_token].str[2] = '\0';
            nr_token ++;
            break;
          case AND:
            tokens[nr_token].type = AND;
            tokens[nr_token].str[0] = '&';
            tokens[nr_token].str[1] = '&';
            tokens[nr_token].str[2] = '\0';
            nr_token ++;
            break;
          case TK_REG:
            tokens[nr_token].type = TK_REG;
            Assert(substr_len < 32, "Register token is too long than 32");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;
          default: 
            fprintf(stderr, "Unknown token type: %d\n", rules[i].token_type);
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// check whether the expression is surrounded by a matched pair of parentheses 
// for false, it may be bad expression or just the left and right parentheses are not matched, e.g. (1+2) * (3+4)
// we need to provide this info to the caller ... (if it is a bad expression, we can directly stop the evaluation)
// hhh, for simplicity, now we just use Assert to record the bad expression ...
bool check_parentheses(int p, int q) {
  // 如果 为了安全，最好是把tokens[p..q]整体的括号匹配情况检查一遍，确保没有不匹配的括号
  int left = 0; // the count of left parentheses
  int left_parens[64] = {0}; // store the index of left parentheses

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      left_parens[left++] = i;
    } else if (tokens[i].type == ')') {
      // this ')' should match the last '('
      if (left == 0) {
        Assert(0, "Unmatched right parentheses at index %d", i);
      }
      left--; // pop the last '('
      if (i == q && left == 0 && left_parens[left] == p) {
        return true;
      }
    } 
  }
  
  return false;
}

// find the main operator in the expression tokens[p..q]
// note that because of we have judged the parentheses matching situation, so
// we can include that the main operator is not in the parentheses ...

int find_main_operator(int p, int q) {
  int main_op = -1;
  int left = 0; // the count of left parentheses , used for to judge whether the operator is in the parentheses
  int main_op_adv = -1; // the level of the main operator, '+' and '-' are 1, '*' and '/' are 2 , OP_EQ is 0

  for (int i = p; i <= q; i++) {
    int op = tokens[i].type;
    switch (op)
    {
    case '(':
      left++;
      break;
    case ')':
      left--;
      break;
    case TK_NEQ:
    case TK_EQ:
      if (left == 0 && main_op_adv <= 0) {
        main_op = i;
        main_op_adv = 0;
        break;
      }
    case '+':
    case '-':
      if (left == 0 && main_op_adv != 0) {
        main_op = i;
        main_op_adv = 1;
        break;
      }
    case '*':
    case '/':
      if (left == 0 && (main_op_adv == -1 || main_op_adv == 2)) {
        main_op = i;
        main_op_adv = 2;
        break;
      }
    case AND:
      if (left == 0 && (main_op_adv == -1 || main_op_adv == 3)) {
        main_op = i;
        main_op_adv = 3;
        break;
      }

    default:
      // e.g. number, we just ignore it 
      break;
    }
  }

  return main_op;
}

bool is_single_operator(int p) {
  int op = tokens[p].type;
  if (op == TK_NEG || op == DERER) {
    return true;
  }
  return false;
}

// eval the child expression of tokens[p..q] and return its value 
word_t eval(int p, int q) {
  if (p > q) {
    // a bad expression
    Assert(0, "Bad expression");
  } else if (p == q) {
    // a single token, e.g. a number or a register
    switch (tokens[p].type)
    {
    case TK_NUMBER:
      return (word_t)atoi(tokens[p].str);
      break;
    case TK_REG:
      bool success;
      word_t val = isa_reg_str2val(tokens[p].str + 1, &success);
      Assert(success, "Invalid register name: %s", tokens[p].str);
      return val;
      break;
    case TK_HEX:
      return (word_t)strtoul(tokens[p].str, NULL, 16);
      break;
    
    default:
      Assert(0, "Unknown token type: %c", tokens[p].type);
    }
  } else if (check_parentheses(p, q) == true) {
    // the expression is surrounded by a matched pair of parentheses
    return eval(p + 1, q - 1);
  } else if (is_single_operator(p)) {
    // p is a single operator, e.g. -1, *0x1000
    word_t val = eval(p + 1, q);
    switch (tokens[p].type)
    {
    case TK_NEG:
      return -val;
    case DERER:
      return vaddr_read(val, 4);
    default:
      Assert(0, "Unknown single operator: %c", tokens[p].type);
    }
  } else {
    // we should find the main operator in the expression 
    int op = find_main_operator(p, q);
    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);

    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      Assert(val2 != 0, "Divide by zero");
      return val1 / val2;
    case TK_EQ:
      return val1 == val2;
    default:
      Assert(0, "Unknown operator: %c", tokens[op].type);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  // Well, before we evaluate the expression, we need to fix some token's type 
  // for example, '-' can be a negative sign or a subtraction operator, we need to distinguish them
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_NUMBER && tokens[i - 1].type != ')'))) {
      // this '-' is a negative sign 
      tokens[i].type = TK_NEG;
    }

    // for more operators, e.g. '*' 
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_NUMBER && tokens[i - 1].type != ')'))) {
      // this '*' is a dereference operator 
      tokens[i].type = DERER;
    }
  }

  /* TODO: Insert codes to evaluate the expression. */

  return eval(0, nr_token - 1);
}
