#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ, TK_NOTEQ, TK_AND, TK_OR, TK_PLUS, TK_MINUS, TK_MULT, TK_DIV, TK_LP, TK_RP, TK_DIGITS, TK_REG, TK_HEX,
    TK_NEG, TK_POS, TK_DREF, TK_NOT,

    /* TODO: Add more token types */

};
enum {
    ASS_LEFT = 0, ASS_RIGHT
};

static struct rule {
    char *regex;
    int token_type;
} rules[] = {

        /* TODO: Add more rules.
         * Pay attention to the precedence level of different rules.
         */

        {" +",     TK_NOTYPE},    // spaces
        {"\\$\w+", TK_REG},
//        {"P",      TK_POS},
//        {"N",      TK_NEG},
//         {"D", TK_DREF},
        {"==",     TK_EQ},        // equal
        {"!=",     TK_NOTEQ},
        {"!",      TK_NOT},
        {"&&",     TK_AND},
        {"||",     TK_OR},
        {"\\+",    TK_PLUS},         // plus
        {"-",      TK_MINUS},
        {"\\*",    TK_MULT},
        {"/",      TK_DIV},
        {"\\(",    TK_LP},
        {"\\)",    TK_RP},
        {"0x\d+",  TK_HEX},
        {"[0-9]+", TK_DIGITS},


};

int precedence(int token_type) {
    switch (token_type) {
        case TK_OR:
            return 1;
        case TK_AND:
            return 2;
        case TK_EQ:
        case TK_NOTEQ:
            return 3;
        case TK_PLUS:
        case TK_MINUS:
            return 4;
        case TK_MULT:
        case TK_DIV:
            return 5;
        case TK_NEG:
        case TK_POS:
        case TK_NOT:
        case TK_DREF:
            return 6;
        default:
            return 7;
    }
}

bool isUnary(int lastTokenType, int curTokenType) {
    switch (curTokenType) {
        case TK_NOT:
            return true;
        default:
            break;
    }

    switch (lastTokenType) {
        case TK_EQ:
        case TK_NOTEQ:
        case TK_PLUS:
        case TK_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_LP:
        case TK_NEG:
        case TK_POS:
        case TK_DREF:
        case TK_NOT:
        case TK_AND:
        case TK_OR:

            switch (curTokenType) {
                case TK_NEG:
                case TK_POS:
                case TK_DREF:

                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

bool unary(int tokenType) {
    switch (tokenType) {
        case TK_POS:
        case TK_NEG:
        case TK_DREF:
            return true;
        default:
            return false;
    }
}

int associty(int token_type) {
    switch (token_type) {
        case TK_PLUS:
        case TK_MINUS:
        case TK_MULT:
        case TK_DIV:
            return ASS_LEFT;
        case TK_POS:
        case TK_NEG:
            return ASS_RIGHT;
        default:
            return ASS_LEFT;
    }
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

    for (i = 0; i < NR_REGEX; i++) {
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

long applyBinaryOp(long  val1, long val2, Token *token) {
    switch (token->type) {
        case TK_PLUS:
            return val1 + val2;
        case TK_MINUS:
            return val1 - val2;
        case TK_MULT :
            return val1 * val2;
        case TK_DIV :
            return val1 / val2;
        case TK_AND :
            return val1 && val2;
        case TK_OR:
            return val1 || val2;
        case TK_EQ:
            return val1 == val2;
        case TK_NOTEQ:
            return val1 != val2;

    }
    return 0;
    // Log_write("error: unkonwn token %s", token->str);
}
long applyUnaryOp(long val, Token* token){
    switch (token->type){
        case TK_POS:
            return val;
        case TK_NEG:
            return -val;
        case TK_DREF:
            return vaddr_read(val,4);
    }
}

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
    int position = 0;
    int i;
    int lastTokenType = TK_LP;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len, substr_start);
                position += substr_len;
                if (substr_len > 31) {
                    return false;
                    // panic("Substr to long");
                }

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                    case TK_EQ:
                    case TK_NOTEQ:
                    case TK_NOT:
                    case TK_PLUS:
                    case TK_OR:
                    case TK_AND:
                    case TK_MINUS:
                    case TK_MULT:
                    case TK_DIV:
                    case TK_DIGITS:
                    case TK_LP:
                    case TK_RP:
                    case TK_POS:
                    case TK_NEG:
                    case TK_DREF:
                        tokens[nr_token].type = rules[i].token_type;
                        if (isUnary(lastTokenType, tokens[nr_token].type)) {
                            switch (tokens[nr_token].type) {
                                case TK_PLUS:
                                    tokens[nr_token].type = TK_POS;
                                case TK_MINUS:
                                    tokens[nr_token].type = TK_NEG;
                                case TK_MULT:
                                    tokens[nr_token].type = TK_DREF;
                                default:
                                    break;
                            }
                        }
                        Log("change +-* to unary");

                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        ++nr_token;
                        break;


                    default:
                        break;


                }
                lastTokenType = tokens[nr_token].type;

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
    Log("Enter");
    if (!make_token(e)) {

        *success = false;
        Log("Fail");
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    Token *opStack[32];
    uint32_t valStack[32];
    int opCur = -1;
    int valCur = -1;
    int i = 0;
    uint32_t val;
    bool reg_read_success = false;

    for (i = 0; i < nr_token; ++i) {
        Token *curToken = &tokens[i];
        switch (curToken->type) {
            case TK_NOTYPE:
                break;
            case TK_LP:
                opStack[++opCur] = curToken;
                break;
            case TK_DIGITS:
                sscanf(tokens[i].str, "%u", &val);
                Log("get val %u\n", val);
                valStack[++valCur] = val;

                break;
            case TK_HEX:
                sscanf(tokens[i].str, "%x", &val);
                Log("get val 0x%x\n", val);
                valStack[++valCur] = val;
                break;
            case TK_REG:
                reg_read(tokens[i].str + 1, &reg_read_success);
                if(reg_read_success){
                    Log("get val %u\n", val);
                }else{
                    printf("Unkown register name %s", tokens[i].str);
                    *success = false;
                    return 0 ;
                }


            case TK_PLUS:
            case TK_MINUS:
            case TK_DIV:
            case TK_MULT:
                while (opCur != -1
                       && precedence(opStack[opCur]->type) >= precedence(curToken->type)) {
                    int val2 = valStack[valCur--];
                    int val1 = valStack[valCur--];
                    Token *op = opStack[opCur--];
                    valStack[++valCur] = applyBinaryOp(val1, val2, op);


                }
                opStack[++opCur] = curToken;

                break;
            case TK_NEG:
            case TK_POS:
                while (opCur != -1 &&
                       precedence(opStack[opCur]->type) > precedence(curToken->type)
                       && unary(precedence(opStack[opCur]->type))){
                    int val = valStack[valCur--];
                    int
                }
                    case TK_RP:
                        while (opCur != -1 && opStack[opCur]->type != TK_LP) {

                            int val2 = valStack[valCur--];
                            int val1 = valStack[valCur--];
                            Token *op = opStack[opCur--];
                            valStack[++valCur] = applyBinaryOp(val1, val2, op);

                        }
                --opCur;
                break;
            default:
                break;


        }


    }
    while (opCur != -1) {
        int val2 = valStack[valCur--];
        int val1 = valStack[valCur--];
        Token *op = opStack[opCur--];
        valStack[++valCur] = applyBinaryOp(val1, val2, op);
    }
    *success = true;
    return valStack[0];

}

