#include "token.h"

namespace tempearly
{
    const char* Token::What(Kind kind)
    {
        switch (kind)
        {
            case LPAREN: return "`('";
            case RPAREN: return "`)'";
            case LBRACK: return "`['";
            case RBRACK: return "`]'";
            case LBRACE: return "`{'";
            case RBRACE: return "`}'";
            case COLON: return "`:'";
            case SEMICOLON: return "`;'";
            case DOT: return "`.'";
            case DOT_DOT: return "`..'";
            case DOT_DOT_DOT: return "`...'";
            case DOT_CONDITIONAL: return "`?.'";
            case CONDITIONAL: return "`?'";
            case INCREMENT: return "`++'";
            case DECREMENT: return "`--'";
            case ARROW: return "`=>'";

            case ASSIGN: return "`='";
            case ASSIGN_AND: return "`&&='";
            case ASSIGN_OR: return "`||='";
            case ASSIGN_BIT_AND: return "`&='";
            case ASSIGN_BIT_OR: return "`|='";
            case ASSIGN_BIT_XOR: return "`^='";
            case ASSIGN_LSH: return "`<<='";
            case ASSIGN_RSH: return "`>>='";
            case ASSIGN_ADD: return "`+='";
            case ASSIGN_SUB: return "`-='";
            case ASSIGN_MUL: return "`*='";
            case ASSIGN_DIV: return "`/='";
            case ASSIGN_MOD: return "`%='";
            
            case COMMA: return "`,'";
            case AND: return "`&&'";
            case OR: return "`||'";
            case BIT_AND: return "`&'";
            case BIT_OR: return "`|'";
            case BIT_XOR: return "`^'";
            case LSH: return "`<<'";
            case RSH: return "`>>'";
            case ADD: return "`+'";
            case SUB: return "`-'";
            case MUL: return "`*'";
            case DIV: return "`/'";
            case MOD: return "`%'";

            case EQ: return "`=='";
            case NE: return "`!='";
            case LT: return "`<'";
            case GT: return "`>'";
            case LTE: return "`<='";
            case GTE: return "`>='";
            case CMP: return "`<=>'";
            case MATCH: return "`=~'";
            case NO_MATCH: return "`!~'";

            case NOT: return "`!'";
            case BIT_NOT: return "`~'";

            case KW_BREAK: return "`break'";
            case KW_CONTINUE: return "`continue'";
            case KW_DO: return "`do'";
            case KW_ELSE: return "`else'";
            case KW_END: return "`end'";
            case KW_FALSE: return "`false'";
            case KW_FOR: return "`for'";
            case KW_IF: return "`if'";
            case KW_NULL: return "`null'";
            case KW_RETURN: return "`return'";
            case KW_THROW: return "`throw'";
            case KW_TRUE: return "`true'";
            case KW_WHILE: return "`while'";
            
            case IDENTIFIER: return "identifier";
            case STRING: return "string literal";
            case INT:
            case FLOAT: return "number literal";
            case CLOSE_TAG: return "`%>'";
            case ERROR: return "error";
            case END_OF_INPUT: return "end of input";
        }

        return "unknown token";
    }
}
