#ifndef TEMPEARLY_CORE_PARSER_H_GUARD
#define TEMPEARLY_CORE_PARSER_H_GUARD

#include "core/string.h"
#include "core/vector.h"

namespace tempearly
{
    class Parser : public CountedObject
    {
    public:
        struct Position
        {
            int line;
            int column;
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

        /**
         * Returns current source position.
         */
        inline const Position& GetPosition() const
        {
            return m_position;
        }

        /**
         * Closes the underlying stream.
         */
        void Close();

        /**
         * Returns next rune from the input stream without advancing forwards.
         *
         * \return Next rune from input stream or -1 if no more runes are
         *         available
         */
        int PeekRune();

        bool PeekRune(rune r);

        int ReadRune();

        bool ReadRune(rune r);

        void UnreadRune(rune r);

        void SkipRune();

        /**
         * Consumes runes until first non-whitespace rune is found.
         */
        void SkipWhitespace();

        virtual void Mark();

    private:
        /** Stream where the input is read from. */
        Stream* m_stream;
        Vector<rune> m_pushback_runes;
        /** Current position in source code. */
        Position m_position;
        bool m_seen_cr;
        String m_error_message;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Parser);
    };
}

#endif /* !TEMPEARLY_CORE_PARSER_H_GUARD */
