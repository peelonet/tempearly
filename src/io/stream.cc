#include <cstdarg>

#include "core/bytestring.h"
#include "core/string.h"
#include "io/stream.h"

#if !defined(BUFSIZ)
# define BUFSIZ 1024
#endif

namespace tempearly
{
    const std::size_t Stream::kBufferSize = BUFSIZ;

    Stream::Stream(std::size_t buffer_size)
        : m_buffer_size(buffer_size)
        , m_buffer(m_buffer_size ? static_cast<byte*>(std::malloc(m_buffer_size)) : 0)
        , m_offset(0)
        , m_remain(0)
        , m_line(1) {}

    Stream::~Stream()
    {
        if (m_buffer)
        {
            std::free(static_cast<void*>(m_buffer));
        }
    }

    void Stream::Flush()
    {
        m_offset = m_remain = 0;
    }

    bool Stream::Read(byte* buffer, std::size_t size, std::size_t& read)
    {
        read = 0;
        if (m_buffer)
        {
            while (size > 0)
            {
                std::size_t n = size;

                if (!m_remain)
                {
                    m_offset = 0;
                    if (!DirectRead(m_buffer, m_buffer_size, m_remain))
                    {
                        return false;
                    }
                    else if (!m_remain)
                    {
                        break;
                    }
                }
                if (n > m_remain)
                {
                    n = m_remain;
                }
                Memory::Copy<byte>(buffer + read, m_buffer + m_offset, n);
                m_offset += n;
                m_remain -= n;
                read += n;
                size -= n;
            }
        }
        else if (!DirectRead(buffer, size, read))
        {
            return false;
        }
        for (std::size_t i = 0; i < read; ++i)
        {
            if (i + 1 < read && buffer[i] == '\r' && buffer[i + 1] == '\n')
            {
                ++m_line;
                ++i;
            }
            else if (buffer[i] == '\n' || buffer[i] == '\r')
            {
                ++m_line;
            }
        }

        return true;
    }

    static inline std::size_t utf8_size(unsigned char input)
    {
        if ((input & 0x80) == 0x00)
        {
            return 1;
        }
        else if ((input & 0xc0) == 0x80)
        {
            return 0;
        }
        else if ((input & 0xe0) == 0xc0)
        {
            return 2;
        }
        else if ((input & 0xf0) == 0xe0)
        {
            return 3;
        }
        else if ((input & 0xf8) == 0xf0)
        {
            return 4;
        }
        else if ((input & 0xfc) == 0xf8)
        {
            return 5;
        }
        else if ((input & 0xfe) == 0xfc)
        {
            return 6;
        } else {
            return 0;
        }
    }

    bool Stream::ReadRune(rune& slot)
    {
        byte buffer[6];
        std::size_t read;
        std::size_t size;
        
        if (!Read(buffer, 1, read) || read < 1)
        {
            return false;
        }
        switch (size = utf8_size(buffer[0]))
        {
            case 1:
                slot = static_cast<rune>(buffer[0]);
                break;

            case 2:
                slot = static_cast<rune>(buffer[0] & 0x1f);
                break;

            case 3:
                slot = static_cast<rune>(buffer[0] & 0x0f);
                break;

            case 4:
                slot = static_cast<rune>(buffer[0] & 0x07);
                break;

            case 5:
                slot = static_cast<rune>(buffer[0] & 0x03);
                break;

            case 6:
                slot = static_cast<rune>(buffer[0] & 0x01);
                break;

            default:
                slot = 0xfffd; // Invalid code point
                return true;
        }
        if (size > 1 && (!Read(buffer + 1, size - 1, read) || read < size - 1))
        {
            slot = 0xfffd;

            return true;
        }
        for (std::size_t i = 1; i < size; ++i)
        {
            if ((buffer[i] & 0xc0) == 0x80)
            {
                slot = (slot << 6) | (buffer[i] & 0x3f);
            } else {
                slot = 0xfffd;

                return true;
            }
        }

        return true;
    }

    bool Stream::Write(const byte* data, std::size_t size)
    {
        if (!DirectWrite(data, size))
        {
            return false;
        }
        for (std::size_t i = 0; i < size; ++i)
        {
            if (i + 1 < size && data[i] == '\r' && data[i + 1] == '\n')
            {
                ++m_line;
                ++i;
            }
            else if (data[i] == '\n' || data[i] == '\r')
            {
                ++m_line;
            }
        }

        return true;
    }

    bool Stream::Write(const ByteString& data)
    {
        return Write(data.GetBytes(), data.GetLength());
    }

    bool Stream::Write(const String& text)
    {
        return Write(text.Encode());
    }

    bool Stream::Printf(const char* format, ...)
    {
        char buffer[1024];
        int length;
        va_list ap;

        va_start(ap, format);
        length = vsnprintf(buffer, sizeof(buffer), format, ap);
        va_end(ap);

        return Write(reinterpret_cast<byte*>(buffer), length);
    }

    bool Stream::Pipe(const Handle<Stream>& that)
    {
        if (m_remain && !that->Write(m_buffer, m_remain))
        {
            return false;
        }
        for (;;)
        {
            if (!DirectRead(m_buffer, m_buffer_size, m_remain))
            {
                return false;
            }
            else if (!m_remain)
            {
                return true;
            }
            else if (!that->Write(m_buffer, m_remain))
            {
                return false;
            }
        }
    }
}
