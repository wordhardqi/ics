#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_PLUS,TK_MINUS,TK_MULT,TK_DIV,TK_LP,TK_RP,TK_DIGITS

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ} ,        // equal
  {"\\+", TK_PLUS},         // plus
  {"-",TK_MINUS},
  {"\\*",TK_MULT},
  {"/",TK_DIV},
  {"\\(",TK_LP},
  {"\\)",TK_RP},

  {"[0-9]+",'d'},
};
int precedence(int token_type){
  if(token_type == TK_PLUS || token_type ==TK_MINUS)
  return 1;
  if(token_type == TK_MULT || token_type == TK_DIV)
  return 2;
  return 0; 
}

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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

int applyOp(int val1, int val2, Token* token){
  switch (token->type){
    case TK_PLUS: return val1 + val2;
    case TK_MINUS: return val1 - val2;
    case TK_MULT : return val1 * val2;
    case TK_DIV : return val1 / val2;
  }
  return 0;
  // Log_write("error: unkonwn token %s", token->str);  
}
Token tokens[32];
int nr_token;

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
        if(substr_len > 31){
          return false;
          // panic("Substr to long");
        }

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_EQ:
          case TK_PLUS:
          case TK_MINUS:
          case TK_MULT:
          case TK_DIV:
          case TK_LP:
          case TK_RP:
              tokens[nr_token].type = rules[i].token_type;
              strncpy(tokens[nr_token].str,substr_start,substr_len);
              tokens[nr_token].str[substr_len]='\0';
              ++nr_token;
              break;
          default:
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  Token* opStack[32];
  uint32_t valStack[32];
  int opCur = 0;
  int valCur = 0;
  int i = 0;
           int val;

  for(i=0; i<nr_token;++i){
    Token* curToken = &tokens[i];
    switch (curToken->type){
      case TK_NOTYPE: break;
      case TK_LP:
          opStack[opCur]=curToken;
          ++opCur;
          break;
      case TK_DIGITS:
         sscanf(tokens[i].str,"%d", &val);
         valStack[valCur] = val;
         ++valCur;
         break;
      case TK_PLUS:
      case TK_MINUS:
      case TK_DIV:
      case TK_MULT:
          while(opCur !=0 
            && precedence(opStack[opCur]->type)>= precedence(curToken->type) ){
              int val2 = valStack[valCur];
              --valCur;
              int val1 = valStack[valCur];
              --valCur;
              Token* op = opStack[opCur];
              --opCur;
              valStack[valCur] = applyOp(val1,val2,op);
              ++valCur;

          }
          opStack[opCur] = curToken;
          ++opCur;
          break;
      case TK_RP:
          while(opCur !=0 && opStack[opCur]->type != TK_LP){
              int val2 = valStack[valCur];
              --valCur;
              int val1 = valStack[valCur];
              --valCur;
              Token* op = opStack[opCur];
              --opCur;
              valStack[valCur] = applyOp(val1,val2,op);
              ++valCur;
          }
          --opCur;
      default: break; 



    }
   


  }
   while(opCur !=0){
       int val2 = valStack[valCur];
              --valCur;
              int val1 = valStack[valCur];
              --valCur;
              Token* op = opStack[opCur];
              --opCur;
              valStack[valCur] = applyOp(val1,val2,op);
              ++valCur;
    }
    *success =  true;
  return valStack[0];

}
