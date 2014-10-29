#include <cstdarg>

#include "core/bytestring.h"
#include "core/string.h"
#include "io/stream.h"

namespace tempearly
{
    Stream::Stream() {}

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
        
        if (!ReadData(buffer, 1, read) || read < 1)
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
        if (size > 1 && (!ReadData(buffer + 1, size - 1, read) || read < size - 1))
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

    bool Stream::Write(const char* data, std::size_t size)
    {
        return WriteData(reinterpret_cast<const byte*>(data), size);
    }

    bool Stream::Write(const ByteString& data)
    {
        return WriteData(data.GetBytes(), data.GetLength());
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

        return WriteData(reinterpret_cast<byte*>(buffer), length);
    }
}
