#ifndef TEMPEARLY_TOKEN_H_GUARD
#define TEMPEARLY_TOKEN_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class Token
    {
    public:
        enum Kind
        {
            // Punctuators
            LPAREN,
            RPAREN,
            LBRACK,
            RBRACK,
            LBRACE,
            RBRACE,
            COLON,
            SEMICOLON,
            DOT,
            DOT_DOT,
            DOT_DOT_DOT,
            DOT_CONDITIONAL,
            CONDITIONAL,
            INCREMENT,
            DECREMENT,
            ARROW,

            // Assignment operators
            ASSIGN,
            ASSIGN_AND,
            ASSIGN_OR,
            ASSIGN_BIT_AND,
            ASSIGN_BIT_OR,
            ASSIGN_BIT_XOR,
            ASSIGN_LSH,
            ASSIGN_RSH,
            ASSIGN_ADD,
            ASSIGN_SUB,
            ASSIGN_MUL,
            ASSIGN_DIV,
            ASSIGN_MOD,
            
            // Operators
            COMMA,
            AND,
            OR,
            BIT_AND,
            BIT_OR,
            BIT_XOR,
            LSH,
            RSH,
            ADD,
            SUB,
            MUL,
            DIV,
            MOD,

            // Comparison operators
            EQ,
            NE,
            LT,
            GT,
            LTE,
            GTE,
            CMP,
            MATCH,
            NO_MATCH,

            // Unary operators
            NOT,
            BIT_NOT,

            // Keywords
            KW_BREAK,
            KW_CONTINUE,
            KW_DO,
            KW_FALSE,
            KW_FOR,
            KW_IF,
            KW_NULL,
            KW_RETURN,
            KW_THROW,
            KW_TRUE,
            KW_WHILE,
            
            IDENTIFIER,
            STRING,
            INT,
            FLOAT,
            CLOSE_TAG,
            ERROR,
            END_OF_INPUT
        };

        static const char* What(Kind kind);

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Token);
    };
}

#endif /* !TEMPEARLY_TOKEN_H_GUARD */
