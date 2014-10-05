#include <cstring>

#include "core/bytestring.h"
#include "core/string.h"

namespace tempearly
{
    const std::size_t String::npos = -1;

    String::String()
        : m_offset(0)
        , m_length(0)
        , m_runes(0)
        , m_counter(0)
        , m_hash_code(0) {}

    String::String(const String& that)
        : m_offset(that.m_offset)
        , m_length(that.m_length)
        , m_runes(that.m_runes)
        , m_counter(that.m_counter)
        , m_hash_code(that.m_hash_code)
    {
        if (m_counter)
        {
            ++(*m_counter);
        }
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

    String::String(const char* input)
        : m_offset(0)
        , m_length(0)
        , m_runes(0)
        , m_counter(0)
        , m_hash_code(0)
    {
        const char* p = input;

        if (!p)
        {
            return;
        }
        while (*p)
        {
            std::size_t n = utf8_size(*p);

            if (n)
            {
                ++m_length;
                p += n;
            } else {
                break;
            }
        }
        if (m_length)
        {
            bool valid = true;

            m_runes = new rune[m_length];
            m_counter = new unsigned int[1];
            m_counter[0] = 1;
            for (std::size_t i = 0; i < m_length; ++i)
            {
                std::size_t n = utf8_size(input[0]);
                rune result;

                switch (n)
                {
                    case 1:
                        result = static_cast<rune>(input[0]);
                        break;

                    case 2:
                        result = static_cast<rune>(input[0] & 0x1f);
                        break;

                    case 3:
                        result = static_cast<rune>(input[0] & 0x0f);
                        break;

                    case 4:
                        result = static_cast<rune>(input[0] & 0x07);
                        break;

                    case 5:
                        result = static_cast<rune>(input[0] & 0x03);
                        break;

                    case 6:
                        result = static_cast<rune>(input[0] & 0x01);
                        break;

                    default:
                        valid = false;
                }
                if (valid)
                {
                    for (std::size_t j = 1; j < n; ++j)
                    {
                        if ((input[j] & 0xc0) == 0x80)
                        {
                            result = (result << 6) | (input[j] & 0x3f);
                        } else {
                            valid = false;
                            break;
                        }
                    }
                    if (valid)
                    {
                        m_runes[i] = result;
                        input += n;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        }
    }

    String::String(const rune* c, std::size_t n)
        : m_offset(0)
        , m_length(n)
        , m_runes(m_length ? new rune[m_length] : 0)
        , m_counter(m_length ? new unsigned int[1] : 0)
        , m_hash_code(0)
    {
        if (m_counter)
        {
            m_counter[0] = 1;
            std::memcpy(static_cast<void*>(m_runes),
                        static_cast<const void*>(c),
                        sizeof(rune) * n);
        }
    }

    String::String(rune c, std::size_t n)
        : m_offset(0)
        , m_length(n)
        , m_runes(m_length ? new rune[m_length] : 0)
        , m_counter(m_length ? new unsigned int[1] : 0)
        , m_hash_code(0)
    {
        if (m_counter)
        {
            std::memcpy(static_cast<void*>(m_runes),
                        static_cast<const void*>(&c),
                        sizeof(rune) * m_length);
            m_counter[0] = 1;
        }
    }

    String::~String()
    {
        if (m_counter && --m_counter[0] == 0)
        {
            delete[] m_runes;
            delete[] m_counter;
        }
    }

    String& String::operator=(const String& that)
    {
        if (m_runes != that.m_runes)
        {
            if (m_counter && --m_counter[0] == 0)
            {
                delete[] m_runes;
                delete[] m_counter;
            }
            m_runes = that.m_runes;
            if ((m_counter = that.m_counter))
            {
                ++m_counter[0];
            }
        }
        m_offset = that.m_offset;
        m_length = that.m_length;
        m_hash_code = that.m_hash_code;

        return *this;
    }

    ByteString String::Encode() const
    {
        std::vector<byte> bytes;

        bytes.reserve(m_length);
        for (std::size_t i = 0; i < m_length; ++i)
        {
            const rune r = m_runes[m_offset + i];

            if (r > 0x10ffff
                || (r & 0xfffe) == 0xfffe
                || (r >= 0xd800 && r <= 0xdfff)
                || (r >= 0xfdd0 && r <= 0xfdef))
            {
                continue;
            }
            else if (r <= 0x7f)
            {
                bytes.push_back(static_cast<byte>(r));
            }
            else if (r <= 0x07ff)
            {
                bytes.push_back(static_cast<byte>(0xc0 | ((r & 0x7c0) >> 6)));
                bytes.push_back(static_cast<byte>(0x80 | (r & 0x3f)));
            }
            else if (r <= 0xffff)
            {
                bytes.push_back(static_cast<byte>(0xe0 | ((r & 0xf000)) >> 12));
                bytes.push_back(static_cast<byte>(0x80 | ((r & 0xfc0)) >> 6));
                bytes.push_back(static_cast<byte>(0x80 | (r & 0x3f)));
            } else {
                bytes.push_back(static_cast<byte>(0xf0 | ((r & 0x1c0000) >> 18)));
                bytes.push_back(static_cast<byte>(0x80 | ((r & 0x3f000) >> 12)));
                bytes.push_back(static_cast<byte>(0x80 | ((r & 0xfc0) >> 6)));
                bytes.push_back(static_cast<byte>(0x80 | (r & 0x3f)));
            }
        }

        return ByteString(bytes.data(), bytes.size());
    }

    std::size_t String::HashCode() const
    {
        std::size_t h = m_hash_code;

        if (h == 0)
        {
            h = 5381;
            for (std::size_t i = 0; i < m_length; ++i)
            {
                h = ((h << 5) + h) + m_runes[m_offset + i];
            }
            m_hash_code = h;
        }

        return h;
    }

    std::size_t String::IndexOf(rune r, std::size_t pos) const
    {
        for (std::size_t i = pos; i < m_length; ++i)
        {
            if (m_runes[m_offset + i] == r)
            {
                return i;
            }
        }

        return npos;
    }

    String String::SubString(std::size_t pos, std::size_t count) const
    {
        String result;

        if (pos >= m_length)
        {
            return result;
        }
        else if (count == npos)
        {
            count = m_length - pos;
        }
        else if (count > m_length)
        {
            count = m_length;
        }
        result.m_offset = m_offset + pos;
        result.m_length = count;
        result.m_runes = m_runes;
        if ((result.m_counter = m_counter))
        {
            ++m_counter[0];
        }

        return result;
    }

    bool String::Equals(const String& that) const
    {
        if (m_runes == that.m_runes)
        {
            return m_offset == that.m_offset && m_length == that.m_length;
        }
        else if (m_length != that.m_length)
        {
            return false;
        }
        for (std::size_t i = 0; i < m_length; ++i)
        {
            if (m_runes[m_offset + i] != that.m_runes[that.m_offset + i])
            {
                return false;
            }
        }

        return true;
    }

    bool String::EqualsIgnoreCase(const String& that) const
    {
        if (m_runes == that.m_runes)
        {
            return m_offset == that.m_offset && m_length == that.m_length;
        }
        else if (m_length != that.m_length)
        {
            return false;
        }
        for (std::size_t i = 0; i < m_length; ++i)
        {
            const rune a = ToLower(m_runes[m_offset + i]);
            const rune b = ToLower(that.m_runes[that.m_offset + i]);

            if (a != b)
            {
                return false;
            }
        }

        return true;
    }

    int String::Compare(const String& that) const
    {
        if (m_runes != that.m_runes || m_offset != that.m_offset)
        {
            std::size_t n = m_length;

            if (that.m_length < n)
            {
                n = that.m_length;
            }
            for (std::size_t i = 0; i < n; ++i)
            {
                const rune a = m_runes[m_offset + i];
                const rune b = that.m_runes[that.m_offset + i];

                if (a != b)
                {
                    return a > b ? 1 : -1;
                }
            }
        }
        if (m_length == that.m_length)
        {
            return 0;
        } else {
            return m_length > that.m_length ? 1 : -1;
        }
    }

    int String::CompareIgnoreCase(const String& that) const
    {
        if (m_runes != that.m_runes || m_offset != that.m_offset)
        {
            std::size_t n = m_length;

            if (that.m_length < n)
            {
                n = that.m_length;
            }
            for (std::size_t i = 0; i < n; ++i)
            {
                const rune a = ToLower(m_runes[m_offset + i]);
                const rune b = ToLower(that.m_runes[that.m_offset + i]);

                if (a != b)
                {
                    return a > b ? 1 : -1;
                }
            }
        }
        if (m_length == that.m_length)
        {
            return 0;
        } else {
            return m_length > that.m_length ? 1 : -1;
        }
    }

    void String::Clear()
    {
        if (m_counter && --m_counter[0] == 0)
        {
            delete[] m_runes;
            delete[] m_counter;
        }
        m_offset = m_length = m_hash_code = 0;
        m_runes = 0;
        m_counter = 0;
    }

    String String::Concat(const String& that) const
    {
        if (!m_length)
        {
            return that;
        }
        else if (!that.m_length)
        {
            return *this;
        } else {
            String result;

            result.m_length = m_length + that.m_length;
            result.m_runes = new rune[result.m_length];
            result.m_counter = new unsigned int[1];
            result.m_counter[0] = 1;
            std::memcpy(static_cast<void*>(result.m_runes),
                        static_cast<const void*>(m_runes + m_offset),
                        sizeof(rune) * m_length);
            std::memcpy(static_cast<void*>(result.m_runes + m_length),
                        static_cast<const void*>(that.m_runes + that.m_offset),
                        sizeof(rune) * that.m_length);

            return result;
        }
    }

    rune String::ToLower(rune c)
    {
        if (c >= 'A' && c <= 'Z')
        {
            return c + 32;
        }
        if (c >= 0x00C0)
        {
            if ((c >= 0x00C0 && c <= 0x00D6) || (c >= 0x00D8 && c <= 0x00DE))
            {
                return c + 32;
            }
            else if ((c >= 0x0100 && c < 0x0138) || (c > 0x0149 && c < 0x0178))
            {
                if (c == 0x0130)
                {
                    return 0x0069;
                }
                else if ((c & 1) == 0)
                {
                    return c + 1;
                }
            }
            else if (c == 0x0178)
            {
                return 0x00FF;
            }
            else if ((c >= 0x0139 && c < 0x0149) || (c > 0x0178 && c < 0x017F))
            {
                if (c & 1)
                {
                    return c + 1;
                }
            }
            else if (c >= 0x0200 && c <= 0x0217)
            {
                if ((c & 1) == 0)
                {
                    return c + 1;
                }
            }
            else if ((c >= 0x0401 && c <= 0x040C) || (c >= 0x040E && c <= 0x040F))
            {
                return c + 80;
            }
            else if (c >= 0x410 && c <= 0x042F)
            {
                return c + 32;
            }
            else if (c >= 0x0460 && c <= 0x047F)
            {
                if ((c & 1) == 0)
                {
                    return c + 1;
                }
            }
            else if (c >= 0x0531 && c <= 0x0556)
            {
                return c + 48;
            }
            else if (c >= 0x10A0 && c <= 0x10C5)
            {
                return c + 48;
            }
            else if (c >= 0xFF21 && c <= 0xFF3A)
            {
                return c + 32;
            }
        }

        return c;
    }

    rune String::ToUpper(rune c)
    {
        if (c >= 'a' && c <= 'z')
        {
            return c - 32;
        }
        if (c >= 0x00E0)
        {
            if ((c >= 0x00E0 && c <= 0x00F6) || (c >= 0x00F8 && c <= 0x00FE))
            {
                return c - 32;
            }
            else if (c == 0x00FF)
            {
                return 0x0178;
            }
            else if ((c >= 0x0100 && c < 0x0138) || (c > 0x0149 && c < 0x0178))
            {
                if (c == 0x0131)
                {
                    return 0x0049;
                }
                else if (c & 1)
                {
                    return c - 1;
                }
            }
            else if ((c >= 0x0139 && c < 0x0149) || (c > 0x0178 && c < 0x017F))
            {
                if ((c & 1) == 0)
                {
                    return c - 1;
                }
            }
            else if (c == 0x017F)
            {
                return 0x0053;
            }
            else if (c >= 0x0200 && c <= 0x0217)
            {
                if (c & 1)
                {
                    return c - 1;
                }
            }
            else if (c >= 0x0430 && c <= 0x044F)
            {
                return c - 32;
            }
            else if ((c >= 0x0451 && c <= 0x045C) || (c >= 0x045E && c <= 0x045F))
            {
                return c - 80;
            }
            else if (c >= 0x0460 && c <= 0x047F)
            {
                if (c & 1)
                {
                    return c - 1;
                }
            }
            else if (c >= 0x0561 && c < 0x0587)
            {
                return c - 48;
            }
            else if (c >= 0xFF41 && c <= 0xFF5A)
            {
                return c - 32;
            }
        }

        return c;
    }

    String operator+(const char* a, const String& b)
    {
        return String(a).Concat(b);
    }
}
