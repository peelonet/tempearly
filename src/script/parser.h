#ifndef TEMPEARLY_PARSER_H_GUARD
#define TEMPEARLY_PARSER_H_GUARD

#include <queue>

#include "script.h"
#include "token.h"
#include "core/stringbuilder.h"

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

        explicit Parser(const Handle<Stream>& stream);

        virtual ~Parser();

        inline const String& GetErrorMessage() const
        {
            return m_error_message;
        }

        inline void SetErrorMessage(const String& error_message)
        {
            m_error_message = error_message;
        }

        Handle<Script> Compile();

        /**
         * Closes the underlying stream.
         */
        void Close();

        int PeekChar();

        bool PeekChar(int c);

        int ReadChar();

        bool ReadChar(int c);

        void UnreadChar(int expected);

        void SkipChar();

        const TokenDescriptor& PeekToken();

        bool PeekToken(Token::Kind kind);

        TokenDescriptor ReadToken();

        bool ReadToken(Token::Kind expected);

        void SkipToken();

        void Mark();

    private:
        /** Keyword lookup map. */
        Dictionary<Token::Kind> m_keywords;
        /** Stream where the input is read from. */
        Stream* m_stream;
        std::queue<int> m_pushback_chars;
        std::queue<TokenDescriptor> m_pushback_tokens;
        bool m_seen_cr;
        /** Current position in source code. */
        SourcePosition m_position;
        StringBuilder m_buffer;
        String m_error_message;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Parser);
    };
}

#endif /* !TEMPEARLY_PARSER_H_GUARD */
