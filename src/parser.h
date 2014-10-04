#ifndef TEMPEARLY_PARSER_H_GUARD
#define TEMPEARLY_PARSER_H_GUARD

#include <queue>
#include <vector>

#include "node.h"
#include "token.h"

namespace tempearly
{
    class Parser : public CountedObject
    {
    public:
        struct SourcePosition
        {
            int line;
            int column;
        };

        struct TokenDescriptor
        {
            Token::Kind kind;
            /** Position of the token in source code. */
            SourcePosition position;
            /** Identifier, number or string literal. */
            String text;
        };

        explicit Parser(FILE* stream);

        virtual ~Parser();

        /**
         * Closes the underlying stream.
         */
        void Close();

        int ReadChar();

        bool ReadChar(int c);

        void UnreadChar(int expected);

        const TokenDescriptor& PeekToken();

        bool PeekToken(Token::Kind kind);

        TokenDescriptor ReadToken();

        bool ReadToken(Token::Kind expected);

        void SkipToken();

        bool Compile(const Handle<Interpreter>& interpreter,
                     std::vector<Handle<Node> >& script);

    private:
        /** Keyword lookup map. */
        Dictionary<Token::Kind> m_keywords;
        /** Stream where the input is read from. */
        FILE* m_stream;
        std::queue<int> m_pushback_chars;
        std::queue<TokenDescriptor> m_pushback_tokens;
        bool m_seen_cr;
        /** Current token scanned from the stream. */
        TokenDescriptor m_token;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Parser);
    };
}

#endif /* !TEMPEARLY_PARSER_H_GUARD */
