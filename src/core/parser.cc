#include "core/parser.h"
#include "io/stream.h"

namespace tempearly
{
    Parser::Parser(const Handle<Stream>& stream)
        : m_stream(stream.Get())
        , m_seen_cr(false) {}

    Parser::~Parser()
    {
        if (m_stream)
        {
            m_stream->Close();
        }
    }

    void Parser::Close()
    {
        if (m_stream)
        {
            m_stream->Close();
            m_stream = nullptr;
        }
    }

    int Parser::PeekRune()
    {
        if (m_pushback_runes.IsEmpty())
        {
            int r = ReadRune();

            if (r < 0)
            {
                return -1;
            }
            m_pushback_runes.PushBack(r);

            return r;
        }

        return m_pushback_runes.GetFront();
    }

    bool Parser::PeekRune(rune r)
    {
        return PeekRune() == static_cast<int>(r);
    }

    int Parser::ReadRune()
    {
        if (!m_pushback_runes.IsEmpty())
        {
            int r = m_pushback_runes.GetFront();

            m_pushback_runes.Erase(0);

            return r;
        }
        if (m_stream)
        {
            rune slot;
            Stream::ReadResult result = m_stream->ReadRune(slot);

            if (result != Stream::SUCCESS && result != Stream::DECODING_ERROR)
            {
                m_stream->Close();
                m_stream = nullptr;

                return -1;
            }
            switch (slot)
            {
                case '\r':
                    ++m_position.line;
                    m_position.column = 0;
                    m_seen_cr = true;
                    break;

                case '\n':
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    } else {
                        ++m_position.line;
                        m_position.column = 0;
                    }
                    break;

                default:
                    ++m_position.column;
                    if (m_seen_cr)
                    {
                        m_seen_cr = false;
                    }
            }

            return slot;
        }

        return -1;
    }

    bool Parser::ReadRune(rune expected)
    {
        int r = ReadRune();

        if (r == static_cast<int>(expected))
        {
            return true;
        }
        else if (r > 0)
        {
            m_pushback_runes.PushBack(r);
        }

        return false;
    }

    void Parser::UnreadRune(rune r)
    {
        if (r > 0)
        {
            m_pushback_runes.PushBack(r);
        }
    }

    void Parser::SkipRune()
    {
        if (m_pushback_runes.IsEmpty())
        {
            ReadRune();
        } else {
            m_pushback_runes.Erase(0);
        }
    }

    void Parser::SkipWhitespace()
    {
        for (;;)
        {
            int r = PeekRune();

            if (r == ' ' || r == '\t' || r == '\n' || r == '\r')
            {
                SkipRune();
            } else {
                return;
            }
        }
    }

    void Parser::Mark()
    {
        CountedObject::Mark();
        if (m_stream && !m_stream->IsMarked())
        {
            m_stream->Mark();
        }
    }
}
