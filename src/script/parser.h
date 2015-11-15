#ifndef TEMPEARLY_SCRIPT_PARSER_H_GUARD
#define TEMPEARLY_SCRIPT_PARSER_H_GUARD

#include "core/parser.h"
#include "core/stringbuilder.h"
#include "script/script.h"
#include "script/token.h"

namespace tempearly
{
    class ScriptParser : public Parser
    {
    public:
        struct TokenDescriptor
        {
            Token::Kind kind;
            /** Position of the token in source code. */
            Position position;
            /** Identifier, number or string literal. */
            String text;
        };

        explicit ScriptParser(const Handle<Stream>& stream);

        Handle<Script> Compile();

        Handle<Script> CompileExpression();

        const TokenDescriptor& PeekToken();

        bool PeekToken(Token::Kind kind);

        TokenDescriptor ReadToken();

        bool ReadToken(Token::Kind expected);

        void SkipToken();

    private:
        /** Keyword lookup map. */
        Dictionary<Token::Kind> m_keywords;
        Vector<TokenDescriptor> m_pushback_tokens;
        StringBuilder m_buffer;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ScriptParser);
    };
}

#endif /* !TEMPEARLY_SCRIPT_PARSER_H_GUARD */
