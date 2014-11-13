#include <cctype>

#include "core/bytestring.h"
#include "core/string.h"
#include "core/vector.h"

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

            m_runes = Memory::Allocate<rune>(m_length);
            m_counter = Memory::Allocate<unsigned int>(1);
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
        , m_runes(Memory::Allocate<rune>(m_length))
        , m_counter(m_length ? Memory::Allocate<unsigned int>(1) : 0)
        , m_hash_code(0)
    {
        if (m_counter)
        {
            m_counter[0] = 1;
            Memory::Copy<rune>(m_runes, c, n);
        }
    }

    String::String(rune c, std::size_t n)
        : m_offset(0)
        , m_length(n)
        , m_runes(Memory::Allocate<rune>(m_length))
        , m_counter(m_length ? Memory::Allocate<unsigned int>(1) : 0)
        , m_hash_code(0)
    {
        if (m_counter)
        {
            m_counter[0] = 1;
            for (std::size_t i = 0; i < m_length; ++i)
            {
                m_runes[i] = c;
            }
        }
    }

    String::~String()
    {
        if (m_counter && --m_counter[0] == 0)
        {
            Memory::Unallocate<rune>(m_runes);
            Memory::Unallocate<unsigned int>(m_counter);
        }
    }

    String& String::Assign(const String& that)
    {
        if (m_runes != that.m_runes)
        {
            if (m_counter && --m_counter[0] == 0)
            {
                Memory::Unallocate<rune>(m_runes);
                Memory::Unallocate<unsigned int>(m_counter);
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

    String& String::Assign(const char* input)
    {
        const char* p = input;

        if (m_counter && --m_counter[0] == 0)
        {
            Memory::Unallocate<rune>(m_runes);
            Memory::Unallocate<unsigned int>(m_counter);
        }
        m_offset = m_length = m_hash_code = 0;
        if (!p)
        {
            return *this;
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

            m_runes = Memory::Allocate<rune>(m_length);
            m_counter = Memory::Allocate<unsigned int>(1);
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

        return *this;
    }

    ByteString String::Encode() const
    {
        Vector<byte> bytes;

        bytes.Reserve(m_length);
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
                bytes.PushBack(static_cast<byte>(r));
            }
            else if (r <= 0x07ff)
            {
                bytes.PushBack(static_cast<byte>(0xc0 | ((r & 0x7c0) >> 6)));
                bytes.PushBack(static_cast<byte>(0x80 | (r & 0x3f)));
            }
            else if (r <= 0xffff)
            {
                bytes.PushBack(static_cast<byte>(0xe0 | ((r & 0xf000)) >> 12));
                bytes.PushBack(static_cast<byte>(0x80 | ((r & 0xfc0)) >> 6));
                bytes.PushBack(static_cast<byte>(0x80 | (r & 0x3f)));
            } else {
                bytes.PushBack(static_cast<byte>(0xf0 | ((r & 0x1c0000) >> 18)));
                bytes.PushBack(static_cast<byte>(0x80 | ((r & 0x3f000) >> 12)));
                bytes.PushBack(static_cast<byte>(0x80 | ((r & 0xfc0) >> 6)));
                bytes.PushBack(static_cast<byte>(0x80 | (r & 0x3f)));
            }
        }

        return ByteString(bytes.GetData(), bytes.GetSize());
    }

#if defined(_WIN32)
    std::wstring String::Widen() const
    {
        std::wstring result;

        result.reserve(m_length);
        for (std::size_t i = 0; i < m_length; ++i)
        {
            rune r = m_runes[m_offset + i];

            if (r > 0xffff)
            {
                result.append(1, static_cast<wchar_t>(r >> 16));
                result.append(1, static_cast<wchar_t>((r & 0xff00) >> 8));
            } else {
                result.append(1, static_cast<wchar_t>(r));
            }
        }

        return result;
    }

    String String::Narrow(const wchar_t* input)
    {
        std::size_t length = 0;
        String result;

        for (const wchar_t* p = input; *p; ++p)
        {
            if (*p < 0xd800 || *p > 0xdfff)
            {
                ++length;
            }
            else if (p[1] >= 0xdc00 || p[1] <= 0xdfff)
            {
                ++length;
                ++p;
            }
        }
        if (length)
        {
            std::size_t offset = 0;

            result.m_length = length;
            result.m_runes = Memory::Allocate<rune>(length);
            result.m_counter = Memory::Allocate<unsigned int>(1);
            result.m_counter[0] = 1;
            for (const wchar_t* p = input; *p; ++p)
            {
                if (*p < 0xd800 || *p > 0xdfff)
                {
                    result.m_runes[offset++] = *p;
                }
                else if (p[1] >= 0xdc00 || p[1] <= 0xdfff)
                {
                    result.m_runes[offset++] = ((p[0] - 0xd7c0) << 10) + (p[1] - 0xdc00);
                    ++p;
                }
            }
        }

        return result;
    }
#endif

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

    std::size_t String::LastIndexOf(rune r, std::size_t pos) const
    {
        if (pos > m_length)
        {
            pos = m_length;
        }
        for (std::size_t i = pos; i > 0; --i)
        {
            if (m_runes[m_offset + i - 1] == r)
            {
                return i - 1;
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

    String String::Trim() const
    {
        std::size_t i, j;

        for (i = 0; i < m_length; ++i)
        {
            if (!std::isspace(m_runes[m_offset + i]))
            {
                break;
            }
        }
        for (j = m_length; j != 0; --j)
        {
            if (!std::isspace(m_runes[m_offset + j - 1]))
            {
                break;
            }
        }
        if (i == 0 && j == m_length)
        {
            return *this;
        } else {
            String result;

            result.m_offset = m_offset + i;
            result.m_length = j - i;
            result.m_runes = m_runes;
            if ((result.m_counter = m_counter))
            {
                ++m_counter[0];
            }

            return result;
        }
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
            Memory::Unallocate<rune>(m_runes);
            Memory::Unallocate<unsigned int>(m_counter);
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
            result.m_runes = Memory::Allocate<rune>(result.m_length);
            result.m_counter = Memory::Allocate<unsigned int>(1);
            result.m_counter[0] = 1;
            Memory::Copy<rune>(result.m_runes, m_runes + m_offset, m_length);
            Memory::Copy<rune>(result.m_runes + m_length, that.m_runes + that.m_offset, that.m_length);

            return result;
        }
    }

    bool String::Matches(bool (*callback)(rune)) const
    {
        if (!m_length)
        {
            return false;
        }
        for (std::size_t i = 0; i < m_length; ++i)
        {
            if (!callback(m_runes[m_offset + i]))
            {
                return false;
            }
        }

        return true;
    }

    String String::Map(rune (*callback)(rune)) const
    {
        String result;

        if (m_length)
        {
            result.m_length = m_length;
            result.m_runes = Memory::Allocate<rune>(m_length);
            result.m_counter = Memory::Allocate<unsigned int>(1);
            result.m_counter[0] = 1;
            for (std::size_t i = 0; i < m_length; ++i)
            {
                result.m_runes[i] = callback(m_runes[m_offset + i]);
            }
        }

        return result;
    }

    bool String::IsLower(rune c)
    {
        static const rune lower_table[480][2] =
        {
            { 0x0061, 0x007a }, { 0x00aa, 0x00aa }, { 0x00b5, 0x00b5 },
            { 0x00ba, 0x00ba }, { 0x00df, 0x00f6 }, { 0x00f8, 0x00ff },
            { 0x0101, 0x0101 }, { 0x0103, 0x0103 }, { 0x0105, 0x0105 },
            { 0x0107, 0x0107 }, { 0x0109, 0x0109 }, { 0x010b, 0x010b },
            { 0x010d, 0x010d }, { 0x010f, 0x010f }, { 0x0111, 0x0111 },
            { 0x0113, 0x0113 }, { 0x0115, 0x0115 }, { 0x0117, 0x0117 },
            { 0x0119, 0x0119 }, { 0x011b, 0x011b }, { 0x011d, 0x011d },
            { 0x011f, 0x011f }, { 0x0121, 0x0121 }, { 0x0123, 0x0123 },
            { 0x0125, 0x0125 }, { 0x0127, 0x0127 }, { 0x0129, 0x0129 },
            { 0x012b, 0x012b }, { 0x012d, 0x012d }, { 0x012f, 0x012f },
            { 0x0131, 0x0131 }, { 0x0133, 0x0133 }, { 0x0135, 0x0135 },
            { 0x0137, 0x0138 }, { 0x013a, 0x013a }, { 0x013c, 0x013c },
            { 0x013e, 0x013e }, { 0x0140, 0x0140 }, { 0x0142, 0x0142 },
            { 0x0144, 0x0144 }, { 0x0146, 0x0146 }, { 0x0148, 0x0149 },
            { 0x014b, 0x014b }, { 0x014d, 0x014d }, { 0x014f, 0x014f },
            { 0x0151, 0x0151 }, { 0x0153, 0x0153 }, { 0x0155, 0x0155 },
            { 0x0157, 0x0157 }, { 0x0159, 0x0159 }, { 0x015b, 0x015b },
            { 0x015d, 0x015d }, { 0x015f, 0x015f }, { 0x0161, 0x0161 },
            { 0x0163, 0x0163 }, { 0x0165, 0x0165 }, { 0x0167, 0x0167 },
            { 0x0169, 0x0169 }, { 0x016b, 0x016b }, { 0x016d, 0x016d },
            { 0x016f, 0x016f }, { 0x0171, 0x0171 }, { 0x0173, 0x0173 },
            { 0x0175, 0x0175 }, { 0x0177, 0x0177 }, { 0x017a, 0x017a },
            { 0x017c, 0x017c }, { 0x017e, 0x0180 }, { 0x0183, 0x0183 },
            { 0x0185, 0x0185 }, { 0x0188, 0x0188 }, { 0x018c, 0x018d },
            { 0x0192, 0x0192 }, { 0x0195, 0x0195 }, { 0x0199, 0x019b },
            { 0x019e, 0x019e }, { 0x01a1, 0x01a1 }, { 0x01a3, 0x01a3 },
            { 0x01a5, 0x01a5 }, { 0x01a8, 0x01a8 }, { 0x01aa, 0x01ab },
            { 0x01ad, 0x01ad }, { 0x01b0, 0x01b0 }, { 0x01b4, 0x01b4 },
            { 0x01b6, 0x01b6 }, { 0x01b9, 0x01ba }, { 0x01bd, 0x01bf },
            { 0x01c6, 0x01c6 }, { 0x01c9, 0x01c9 }, { 0x01cc, 0x01cc },
            { 0x01ce, 0x01ce }, { 0x01d0, 0x01d0 }, { 0x01d2, 0x01d2 },
            { 0x01d4, 0x01d4 }, { 0x01d6, 0x01d6 }, { 0x01d8, 0x01d8 },
            { 0x01da, 0x01da }, { 0x01dc, 0x01dd }, { 0x01df, 0x01df },
            { 0x01e1, 0x01e1 }, { 0x01e3, 0x01e3 }, { 0x01e5, 0x01e5 },
            { 0x01e7, 0x01e7 }, { 0x01e9, 0x01e9 }, { 0x01eb, 0x01eb },
            { 0x01ed, 0x01ed }, { 0x01ef, 0x01f0 }, { 0x01f3, 0x01f3 },
            { 0x01f5, 0x01f5 }, { 0x01f9, 0x01f9 }, { 0x01fb, 0x01fb },
            { 0x01fd, 0x01fd }, { 0x01ff, 0x01ff }, { 0x0201, 0x0201 },
            { 0x0203, 0x0203 }, { 0x0205, 0x0205 }, { 0x0207, 0x0207 },
            { 0x0209, 0x0209 }, { 0x020b, 0x020b }, { 0x020d, 0x020d },
            { 0x020f, 0x020f }, { 0x0211, 0x0211 }, { 0x0213, 0x0213 },
            { 0x0215, 0x0215 }, { 0x0217, 0x0217 }, { 0x0219, 0x0219 },
            { 0x021b, 0x021b }, { 0x021d, 0x021d }, { 0x021f, 0x021f },
            { 0x0221, 0x0221 }, { 0x0223, 0x0223 }, { 0x0225, 0x0225 },
            { 0x0227, 0x0227 }, { 0x0229, 0x0229 }, { 0x022b, 0x022b },
            { 0x022d, 0x022d }, { 0x022f, 0x022f }, { 0x0231, 0x0231 },
            { 0x0233, 0x0239 }, { 0x023c, 0x023c }, { 0x023f, 0x0240 },
            { 0x0250, 0x02af }, { 0x0390, 0x0390 }, { 0x03ac, 0x03ce },
            { 0x03d0, 0x03d1 }, { 0x03d5, 0x03d7 }, { 0x03d9, 0x03d9 },
            { 0x03db, 0x03db }, { 0x03dd, 0x03dd }, { 0x03df, 0x03df },
            { 0x03e1, 0x03e1 }, { 0x03e3, 0x03e3 }, { 0x03e5, 0x03e5 },
            { 0x03e7, 0x03e7 }, { 0x03e9, 0x03e9 }, { 0x03eb, 0x03eb },
            { 0x03ed, 0x03ed }, { 0x03ef, 0x03f3 }, { 0x03f5, 0x03f5 },
            { 0x03f8, 0x03f8 }, { 0x03fb, 0x03fc }, { 0x0430, 0x045f },
            { 0x0461, 0x0461 }, { 0x0463, 0x0463 }, { 0x0465, 0x0465 },
            { 0x0467, 0x0467 }, { 0x0469, 0x0469 }, { 0x046b, 0x046b },
            { 0x046d, 0x046d }, { 0x046f, 0x046f }, { 0x0471, 0x0471 },
            { 0x0473, 0x0473 }, { 0x0475, 0x0475 }, { 0x0477, 0x0477 },
            { 0x0479, 0x0479 }, { 0x047b, 0x047b }, { 0x047d, 0x047d },
            { 0x047f, 0x047f }, { 0x0481, 0x0481 }, { 0x048b, 0x048b },
            { 0x048d, 0x048d }, { 0x048f, 0x048f }, { 0x0491, 0x0491 },
            { 0x0493, 0x0493 }, { 0x0495, 0x0495 }, { 0x0497, 0x0497 },
            { 0x0499, 0x0499 }, { 0x049b, 0x049b }, { 0x049d, 0x049d },
            { 0x049f, 0x049f }, { 0x04a1, 0x04a1 }, { 0x04a3, 0x04a3 },
            { 0x04a5, 0x04a5 }, { 0x04a7, 0x04a7 }, { 0x04a9, 0x04a9 },
            { 0x04ab, 0x04ab }, { 0x04ad, 0x04ad }, { 0x04af, 0x04af },
            { 0x04b1, 0x04b1 }, { 0x04b3, 0x04b3 }, { 0x04b5, 0x04b5 },
            { 0x04b7, 0x04b7 }, { 0x04b9, 0x04b9 }, { 0x04bb, 0x04bb },
            { 0x04bd, 0x04bd }, { 0x04bf, 0x04bf }, { 0x04c2, 0x04c2 },
            { 0x04c4, 0x04c4 }, { 0x04c6, 0x04c6 }, { 0x04c8, 0x04c8 },
            { 0x04ca, 0x04ca }, { 0x04cc, 0x04cc }, { 0x04ce, 0x04ce },
            { 0x04d1, 0x04d1 }, { 0x04d3, 0x04d3 }, { 0x04d5, 0x04d5 },
            { 0x04d7, 0x04d7 }, { 0x04d9, 0x04d9 }, { 0x04db, 0x04db },
            { 0x04dd, 0x04dd }, { 0x04df, 0x04df }, { 0x04e1, 0x04e1 },
            { 0x04e3, 0x04e3 }, { 0x04e5, 0x04e5 }, { 0x04e7, 0x04e7 },
            { 0x04e9, 0x04e9 }, { 0x04eb, 0x04eb }, { 0x04ed, 0x04ed },
            { 0x04ef, 0x04ef }, { 0x04f1, 0x04f1 }, { 0x04f3, 0x04f3 },
            { 0x04f5, 0x04f5 }, { 0x04f7, 0x04f7 }, { 0x04f9, 0x04f9 },
            { 0x0501, 0x0501 }, { 0x0503, 0x0503 }, { 0x0505, 0x0505 },
            { 0x0507, 0x0507 }, { 0x0509, 0x0509 }, { 0x050b, 0x050b },
            { 0x050d, 0x050d }, { 0x050f, 0x050f }, { 0x0561, 0x0587 },
            { 0x1d00, 0x1d2b }, { 0x1d62, 0x1d77 }, { 0x1d79, 0x1d9a },
            { 0x1e01, 0x1e01 }, { 0x1e03, 0x1e03 }, { 0x1e05, 0x1e05 },
            { 0x1e07, 0x1e07 }, { 0x1e09, 0x1e09 }, { 0x1e0b, 0x1e0b },
            { 0x1e0d, 0x1e0d }, { 0x1e0f, 0x1e0f }, { 0x1e11, 0x1e11 },
            { 0x1e13, 0x1e13 }, { 0x1e15, 0x1e15 }, { 0x1e17, 0x1e17 },
            { 0x1e19, 0x1e19 }, { 0x1e1b, 0x1e1b }, { 0x1e1d, 0x1e1d },
            { 0x1e1f, 0x1e1f }, { 0x1e21, 0x1e21 }, { 0x1e23, 0x1e23 },
            { 0x1e25, 0x1e25 }, { 0x1e27, 0x1e27 }, { 0x1e29, 0x1e29 },
            { 0x1e2b, 0x1e2b }, { 0x1e2d, 0x1e2d }, { 0x1e2f, 0x1e2f },
            { 0x1e31, 0x1e31 }, { 0x1e33, 0x1e33 }, { 0x1e35, 0x1e35 },
            { 0x1e37, 0x1e37 }, { 0x1e39, 0x1e39 }, { 0x1e3b, 0x1e3b },
            { 0x1e3d, 0x1e3d }, { 0x1e3f, 0x1e3f }, { 0x1e41, 0x1e41 },
            { 0x1e43, 0x1e43 }, { 0x1e45, 0x1e45 }, { 0x1e47, 0x1e47 },
            { 0x1e49, 0x1e49 }, { 0x1e4b, 0x1e4b }, { 0x1e4d, 0x1e4d },
            { 0x1e4f, 0x1e4f }, { 0x1e51, 0x1e51 }, { 0x1e53, 0x1e53 },
            { 0x1e55, 0x1e55 }, { 0x1e57, 0x1e57 }, { 0x1e59, 0x1e59 },
            { 0x1e5b, 0x1e5b }, { 0x1e5d, 0x1e5d }, { 0x1e5f, 0x1e5f },
            { 0x1e61, 0x1e61 }, { 0x1e63, 0x1e63 }, { 0x1e65, 0x1e65 },
            { 0x1e67, 0x1e67 }, { 0x1e69, 0x1e69 }, { 0x1e6b, 0x1e6b },
            { 0x1e6d, 0x1e6d }, { 0x1e6f, 0x1e6f }, { 0x1e71, 0x1e71 },
            { 0x1e73, 0x1e73 }, { 0x1e75, 0x1e75 }, { 0x1e77, 0x1e77 },
            { 0x1e79, 0x1e79 }, { 0x1e7b, 0x1e7b }, { 0x1e7d, 0x1e7d },
            { 0x1e7f, 0x1e7f }, { 0x1e81, 0x1e81 }, { 0x1e83, 0x1e83 },
            { 0x1e85, 0x1e85 }, { 0x1e87, 0x1e87 }, { 0x1e89, 0x1e89 },
            { 0x1e8b, 0x1e8b }, { 0x1e8d, 0x1e8d }, { 0x1e8f, 0x1e8f },
            { 0x1e91, 0x1e91 }, { 0x1e93, 0x1e93 }, { 0x1e95, 0x1e9b },
            { 0x1ea1, 0x1ea1 }, { 0x1ea3, 0x1ea3 }, { 0x1ea5, 0x1ea5 },
            { 0x1ea7, 0x1ea7 }, { 0x1ea9, 0x1ea9 }, { 0x1eab, 0x1eab },
            { 0x1ead, 0x1ead }, { 0x1eaf, 0x1eaf }, { 0x1eb1, 0x1eb1 },
            { 0x1eb3, 0x1eb3 }, { 0x1eb5, 0x1eb5 }, { 0x1eb7, 0x1eb7 },
            { 0x1eb9, 0x1eb9 }, { 0x1ebb, 0x1ebb }, { 0x1ebd, 0x1ebd },
            { 0x1ebf, 0x1ebf }, { 0x1ec1, 0x1ec1 }, { 0x1ec3, 0x1ec3 },
            { 0x1ec5, 0x1ec5 }, { 0x1ec7, 0x1ec7 }, { 0x1ec9, 0x1ec9 },
            { 0x1ecb, 0x1ecb }, { 0x1ecd, 0x1ecd }, { 0x1ecf, 0x1ecf },
            { 0x1ed1, 0x1ed1 }, { 0x1ed3, 0x1ed3 }, { 0x1ed5, 0x1ed5 },
            { 0x1ed7, 0x1ed7 }, { 0x1ed9, 0x1ed9 }, { 0x1edb, 0x1edb },
            { 0x1edd, 0x1edd }, { 0x1edf, 0x1edf }, { 0x1ee1, 0x1ee1 },
            { 0x1ee3, 0x1ee3 }, { 0x1ee5, 0x1ee5 }, { 0x1ee7, 0x1ee7 },
            { 0x1ee9, 0x1ee9 }, { 0x1eeb, 0x1eeb }, { 0x1eed, 0x1eed },
            { 0x1eef, 0x1eef }, { 0x1ef1, 0x1ef1 }, { 0x1ef3, 0x1ef3 },
            { 0x1ef5, 0x1ef5 }, { 0x1ef7, 0x1ef7 }, { 0x1ef9, 0x1ef9 },
            { 0x1f00, 0x1f07 }, { 0x1f10, 0x1f15 }, { 0x1f20, 0x1f27 },
            { 0x1f30, 0x1f37 }, { 0x1f40, 0x1f45 }, { 0x1f50, 0x1f57 },
            { 0x1f60, 0x1f67 }, { 0x1f70, 0x1f7d }, { 0x1f80, 0x1f87 },
            { 0x1f90, 0x1f97 }, { 0x1fa0, 0x1fa7 }, { 0x1fb0, 0x1fb4 },
            { 0x1fb6, 0x1fb7 }, { 0x1fbe, 0x1fbe }, { 0x1fc2, 0x1fc4 },
            { 0x1fc6, 0x1fc7 }, { 0x1fd0, 0x1fd3 }, { 0x1fd6, 0x1fd7 },
            { 0x1fe0, 0x1fe7 }, { 0x1ff2, 0x1ff4 }, { 0x1ff6, 0x1ff7 },
            { 0x2071, 0x2071 }, { 0x207f, 0x207f }, { 0x210a, 0x210a },
            { 0x210e, 0x210f }, { 0x2113, 0x2113 }, { 0x212f, 0x212f },
            { 0x2134, 0x2134 }, { 0x2139, 0x2139 }, { 0x213c, 0x213d },
            { 0x2146, 0x2149 }, { 0x2c30, 0x2c5e }, { 0x2c81, 0x2c81 },
            { 0x2c83, 0x2c83 }, { 0x2c85, 0x2c85 }, { 0x2c87, 0x2c87 },
            { 0x2c89, 0x2c89 }, { 0x2c8b, 0x2c8b }, { 0x2c8d, 0x2c8d },
            { 0x2c8f, 0x2c8f }, { 0x2c91, 0x2c91 }, { 0x2c93, 0x2c93 },
            { 0x2c95, 0x2c95 }, { 0x2c97, 0x2c97 }, { 0x2c99, 0x2c99 },
            { 0x2c9b, 0x2c9b }, { 0x2c9d, 0x2c9d }, { 0x2c9f, 0x2c9f },
            { 0x2ca1, 0x2ca1 }, { 0x2ca3, 0x2ca3 }, { 0x2ca5, 0x2ca5 },
            { 0x2ca7, 0x2ca7 }, { 0x2ca9, 0x2ca9 }, { 0x2cab, 0x2cab },
            { 0x2cad, 0x2cad }, { 0x2caf, 0x2caf }, { 0x2cb1, 0x2cb1 },
            { 0x2cb3, 0x2cb3 }, { 0x2cb5, 0x2cb5 }, { 0x2cb7, 0x2cb7 },
            { 0x2cb9, 0x2cb9 }, { 0x2cbb, 0x2cbb }, { 0x2cbd, 0x2cbd },
            { 0x2cbf, 0x2cbf }, { 0x2cc1, 0x2cc1 }, { 0x2cc3, 0x2cc3 },
            { 0x2cc5, 0x2cc5 }, { 0x2cc7, 0x2cc7 }, { 0x2cc9, 0x2cc9 },
            { 0x2ccb, 0x2ccb }, { 0x2ccd, 0x2ccd }, { 0x2ccf, 0x2ccf },
            { 0x2cd1, 0x2cd1 }, { 0x2cd3, 0x2cd3 }, { 0x2cd5, 0x2cd5 },
            { 0x2cd7, 0x2cd7 }, { 0x2cd9, 0x2cd9 }, { 0x2cdb, 0x2cdb },
            { 0x2cdd, 0x2cdd }, { 0x2cdf, 0x2cdf }, { 0x2ce1, 0x2ce1 },
            { 0x2ce3, 0x2ce4 }, { 0x2d00, 0x2d25 }, { 0xfb00, 0xfb06 },
            { 0xfb13, 0xfb17 }, { 0xff41, 0xff5a }, { 0x10428, 0x1044f },
            { 0x1d41a, 0x1d433 }, { 0x1d44e, 0x1d454 }, { 0x1d456, 0x1d467 },
            { 0x1d482, 0x1d49b }, { 0x1d4b6, 0x1d4b9 }, { 0x1d4bb, 0x1d4bb },
            { 0x1d4bd, 0x1d4c3 }, { 0x1d4c5, 0x1d4cf }, { 0x1d4ea, 0x1d503 },
            { 0x1d51e, 0x1d537 }, { 0x1d552, 0x1d56b }, { 0x1d586, 0x1d59f },
            { 0x1d5ba, 0x1d5d3 }, { 0x1d5ee, 0x1d607 }, { 0x1d622, 0x1d63b },
            { 0x1d656, 0x1d66f }, { 0x1d68a, 0x1d6a5 }, { 0x1d6c2, 0x1d6da },
            { 0x1d6dc, 0x1d6e1 }, { 0x1d6fc, 0x1d714 }, { 0x1d716, 0x1d71b },
            { 0x1d736, 0x1d74e }, { 0x1d750, 0x1d755 }, { 0x1d770, 0x1d788 },
            { 0x1d78a, 0x1d78f }, { 0x1d7aa, 0x1d7c2 }, { 0x1d7c4, 0x1d7c9 }
        };

        for (int i = 0; i < 480; ++i)
        {
            if (c >= lower_table[i][0] && c <= lower_table[i][1])
            {
                return true;
            }
        }

        return false;
    }

    bool String::IsUpper(rune c)
    {
        static const rune upper_table[476][2] =
        {
            { 0x0041, 0x005a }, { 0x00c0, 0x00d6 }, { 0x00d8, 0x00de },
            { 0x0100, 0x0100 }, { 0x0102, 0x0102 }, { 0x0104, 0x0104 },
            { 0x0106, 0x0106 }, { 0x0108, 0x0108 }, { 0x010a, 0x010a },
            { 0x010c, 0x010c }, { 0x010e, 0x010e }, { 0x0110, 0x0110 },
            { 0x0112, 0x0112 }, { 0x0114, 0x0114 }, { 0x0116, 0x0116 },
            { 0x0118, 0x0118 }, { 0x011a, 0x011a }, { 0x011c, 0x011c },
            { 0x011e, 0x011e }, { 0x0120, 0x0120 }, { 0x0122, 0x0122 },
            { 0x0124, 0x0124 }, { 0x0126, 0x0126 }, { 0x0128, 0x0128 },
            { 0x012a, 0x012a }, { 0x012c, 0x012c }, { 0x012e, 0x012e },
            { 0x0130, 0x0130 }, { 0x0132, 0x0132 }, { 0x0134, 0x0134 },
            { 0x0136, 0x0136 }, { 0x0139, 0x0139 }, { 0x013b, 0x013b },
            { 0x013d, 0x013d }, { 0x013f, 0x013f }, { 0x0141, 0x0141 },
            { 0x0143, 0x0143 }, { 0x0145, 0x0145 }, { 0x0147, 0x0147 },
            { 0x014a, 0x014a }, { 0x014c, 0x014c }, { 0x014e, 0x014e },
            { 0x0150, 0x0150 }, { 0x0152, 0x0152 }, { 0x0154, 0x0154 },
            { 0x0156, 0x0156 }, { 0x0158, 0x0158 }, { 0x015a, 0x015a },
            { 0x015c, 0x015c }, { 0x015e, 0x015e }, { 0x0160, 0x0160 },
            { 0x0162, 0x0162 }, { 0x0164, 0x0164 }, { 0x0166, 0x0166 },
            { 0x0168, 0x0168 }, { 0x016a, 0x016a }, { 0x016c, 0x016c },
            { 0x016e, 0x016e }, { 0x0170, 0x0170 }, { 0x0172, 0x0172 },
            { 0x0174, 0x0174 }, { 0x0176, 0x0176 }, { 0x0178, 0x0179 },
            { 0x017b, 0x017b }, { 0x017d, 0x017d }, { 0x0181, 0x0182 },
            { 0x0184, 0x0184 }, { 0x0186, 0x0187 }, { 0x0189, 0x018b },
            { 0x018e, 0x0191 }, { 0x0193, 0x0194 }, { 0x0196, 0x0198 },
            { 0x019c, 0x019d }, { 0x019f, 0x01a0 }, { 0x01a2, 0x01a2 },
            { 0x01a4, 0x01a4 }, { 0x01a6, 0x01a7 }, { 0x01a9, 0x01a9 },
            { 0x01ac, 0x01ac }, { 0x01ae, 0x01af }, { 0x01b1, 0x01b3 },
            { 0x01b5, 0x01b5 }, { 0x01b7, 0x01b8 }, { 0x01bc, 0x01bc },
            { 0x01c4, 0x01c4 }, { 0x01c7, 0x01c7 }, { 0x01ca, 0x01ca },
            { 0x01cd, 0x01cd }, { 0x01cf, 0x01cf }, { 0x01d1, 0x01d1 },
            { 0x01d3, 0x01d3 }, { 0x01d5, 0x01d5 }, { 0x01d7, 0x01d7 },
            { 0x01d9, 0x01d9 }, { 0x01db, 0x01db }, { 0x01de, 0x01de },
            { 0x01e0, 0x01e0 }, { 0x01e2, 0x01e2 }, { 0x01e4, 0x01e4 },
            { 0x01e6, 0x01e6 }, { 0x01e8, 0x01e8 }, { 0x01ea, 0x01ea },
            { 0x01ec, 0x01ec }, { 0x01ee, 0x01ee }, { 0x01f1, 0x01f1 },
            { 0x01f4, 0x01f4 }, { 0x01f6, 0x01f8 }, { 0x01fa, 0x01fa },
            { 0x01fc, 0x01fc }, { 0x01fe, 0x01fe }, { 0x0200, 0x0200 },
            { 0x0202, 0x0202 }, { 0x0204, 0x0204 }, { 0x0206, 0x0206 },
            { 0x0208, 0x0208 }, { 0x020a, 0x020a }, { 0x020c, 0x020c },
            { 0x020e, 0x020e }, { 0x0210, 0x0210 }, { 0x0212, 0x0212 },
            { 0x0214, 0x0214 }, { 0x0216, 0x0216 }, { 0x0218, 0x0218 },
            { 0x021a, 0x021a }, { 0x021c, 0x021c }, { 0x021e, 0x021e },
            { 0x0220, 0x0220 }, { 0x0222, 0x0222 }, { 0x0224, 0x0224 },
            { 0x0226, 0x0226 }, { 0x0228, 0x0228 }, { 0x022a, 0x022a },
            { 0x022c, 0x022c }, { 0x022e, 0x022e }, { 0x0230, 0x0230 },
            { 0x0232, 0x0232 }, { 0x023a, 0x023b }, { 0x023d, 0x023e },
            { 0x0241, 0x0241 }, { 0x0386, 0x0386 }, { 0x0388, 0x038a },
            { 0x038c, 0x038c }, { 0x038e, 0x038f }, { 0x0391, 0x03a1 },
            { 0x03a3, 0x03ab }, { 0x03d2, 0x03d4 }, { 0x03d8, 0x03d8 },
            { 0x03da, 0x03da }, { 0x03dc, 0x03dc }, { 0x03de, 0x03de },
            { 0x03e0, 0x03e0 }, { 0x03e2, 0x03e2 }, { 0x03e4, 0x03e4 },
            { 0x03e6, 0x03e6 }, { 0x03e8, 0x03e8 }, { 0x03ea, 0x03ea },
            { 0x03ec, 0x03ec }, { 0x03ee, 0x03ee }, { 0x03f4, 0x03f4 },
            { 0x03f7, 0x03f7 }, { 0x03f9, 0x03fa }, { 0x03fd, 0x042f },
            { 0x0460, 0x0460 }, { 0x0462, 0x0462 }, { 0x0464, 0x0464 },
            { 0x0466, 0x0466 }, { 0x0468, 0x0468 }, { 0x046a, 0x046a },
            { 0x046c, 0x046c }, { 0x046e, 0x046e }, { 0x0470, 0x0470 },
            { 0x0472, 0x0472 }, { 0x0474, 0x0474 }, { 0x0476, 0x0476 },
            { 0x0478, 0x0478 }, { 0x047a, 0x047a }, { 0x047c, 0x047c },
            { 0x047e, 0x047e }, { 0x0480, 0x0480 }, { 0x048a, 0x048a },
            { 0x048c, 0x048c }, { 0x048e, 0x048e }, { 0x0490, 0x0490 },
            { 0x0492, 0x0492 }, { 0x0494, 0x0494 }, { 0x0496, 0x0496 },
            { 0x0498, 0x0498 }, { 0x049a, 0x049a }, { 0x049c, 0x049c },
            { 0x049e, 0x049e }, { 0x04a0, 0x04a0 }, { 0x04a2, 0x04a2 },
            { 0x04a4, 0x04a4 }, { 0x04a6, 0x04a6 }, { 0x04a8, 0x04a8 },
            { 0x04aa, 0x04aa }, { 0x04ac, 0x04ac }, { 0x04ae, 0x04ae },
            { 0x04b0, 0x04b0 }, { 0x04b2, 0x04b2 }, { 0x04b4, 0x04b4 },
            { 0x04b6, 0x04b6 }, { 0x04b8, 0x04b8 }, { 0x04ba, 0x04ba },
            { 0x04bc, 0x04bc }, { 0x04be, 0x04be }, { 0x04c0, 0x04c1 },
            { 0x04c3, 0x04c3 }, { 0x04c5, 0x04c5 }, { 0x04c7, 0x04c7 },
            { 0x04c9, 0x04c9 }, { 0x04cb, 0x04cb }, { 0x04cd, 0x04cd },
            { 0x04d0, 0x04d0 }, { 0x04d2, 0x04d2 }, { 0x04d4, 0x04d4 },
            { 0x04d6, 0x04d6 }, { 0x04d8, 0x04d8 }, { 0x04da, 0x04da },
            { 0x04dc, 0x04dc }, { 0x04de, 0x04de }, { 0x04e0, 0x04e0 },
            { 0x04e2, 0x04e2 }, { 0x04e4, 0x04e4 }, { 0x04e6, 0x04e6 },
            { 0x04e8, 0x04e8 }, { 0x04ea, 0x04ea }, { 0x04ec, 0x04ec },
            { 0x04ee, 0x04ee }, { 0x04f0, 0x04f0 }, { 0x04f2, 0x04f2 },
            { 0x04f4, 0x04f4 }, { 0x04f6, 0x04f6 }, { 0x04f8, 0x04f8 },
            { 0x0500, 0x0500 }, { 0x0502, 0x0502 }, { 0x0504, 0x0504 },
            { 0x0506, 0x0506 }, { 0x0508, 0x0508 }, { 0x050a, 0x050a },
            { 0x050c, 0x050c }, { 0x050e, 0x050e }, { 0x0531, 0x0556 },
            { 0x10a0, 0x10c5 }, { 0x1e00, 0x1e00 }, { 0x1e02, 0x1e02 },
            { 0x1e04, 0x1e04 }, { 0x1e06, 0x1e06 }, { 0x1e08, 0x1e08 },
            { 0x1e0a, 0x1e0a }, { 0x1e0c, 0x1e0c }, { 0x1e0e, 0x1e0e },
            { 0x1e10, 0x1e10 }, { 0x1e12, 0x1e12 }, { 0x1e14, 0x1e14 },
            { 0x1e16, 0x1e16 }, { 0x1e18, 0x1e18 }, { 0x1e1a, 0x1e1a },
            { 0x1e1c, 0x1e1c }, { 0x1e1e, 0x1e1e }, { 0x1e20, 0x1e20 },
            { 0x1e22, 0x1e22 }, { 0x1e24, 0x1e24 }, { 0x1e26, 0x1e26 },
            { 0x1e28, 0x1e28 }, { 0x1e2a, 0x1e2a }, { 0x1e2c, 0x1e2c },
            { 0x1e2e, 0x1e2e }, { 0x1e30, 0x1e30 }, { 0x1e32, 0x1e32 },
            { 0x1e34, 0x1e34 }, { 0x1e36, 0x1e36 }, { 0x1e38, 0x1e38 },
            { 0x1e3a, 0x1e3a }, { 0x1e3c, 0x1e3c }, { 0x1e3e, 0x1e3e },
            { 0x1e40, 0x1e40 }, { 0x1e42, 0x1e42 }, { 0x1e44, 0x1e44 },
            { 0x1e46, 0x1e46 }, { 0x1e48, 0x1e48 }, { 0x1e4a, 0x1e4a },
            { 0x1e4c, 0x1e4c }, { 0x1e4e, 0x1e4e }, { 0x1e50, 0x1e50 },
            { 0x1e52, 0x1e52 }, { 0x1e54, 0x1e54 }, { 0x1e56, 0x1e56 },
            { 0x1e58, 0x1e58 }, { 0x1e5a, 0x1e5a }, { 0x1e5c, 0x1e5c },
            { 0x1e5e, 0x1e5e }, { 0x1e60, 0x1e60 }, { 0x1e62, 0x1e62 },
            { 0x1e64, 0x1e64 }, { 0x1e66, 0x1e66 }, { 0x1e68, 0x1e68 },
            { 0x1e6a, 0x1e6a }, { 0x1e6c, 0x1e6c }, { 0x1e6e, 0x1e6e },
            { 0x1e70, 0x1e70 }, { 0x1e72, 0x1e72 }, { 0x1e74, 0x1e74 },
            { 0x1e76, 0x1e76 }, { 0x1e78, 0x1e78 }, { 0x1e7a, 0x1e7a },
            { 0x1e7c, 0x1e7c }, { 0x1e7e, 0x1e7e }, { 0x1e80, 0x1e80 },
            { 0x1e82, 0x1e82 }, { 0x1e84, 0x1e84 }, { 0x1e86, 0x1e86 },
            { 0x1e88, 0x1e88 }, { 0x1e8a, 0x1e8a }, { 0x1e8c, 0x1e8c },
            { 0x1e8e, 0x1e8e }, { 0x1e90, 0x1e90 }, { 0x1e92, 0x1e92 },
            { 0x1e94, 0x1e94 }, { 0x1ea0, 0x1ea0 }, { 0x1ea2, 0x1ea2 },
            { 0x1ea4, 0x1ea4 }, { 0x1ea6, 0x1ea6 }, { 0x1ea8, 0x1ea8 },
            { 0x1eaa, 0x1eaa }, { 0x1eac, 0x1eac }, { 0x1eae, 0x1eae },
            { 0x1eb0, 0x1eb0 }, { 0x1eb2, 0x1eb2 }, { 0x1eb4, 0x1eb4 },
            { 0x1eb6, 0x1eb6 }, { 0x1eb8, 0x1eb8 }, { 0x1eba, 0x1eba },
            { 0x1ebc, 0x1ebc }, { 0x1ebe, 0x1ebe }, { 0x1ec0, 0x1ec0 },
            { 0x1ec2, 0x1ec2 }, { 0x1ec4, 0x1ec4 }, { 0x1ec6, 0x1ec6 },
            { 0x1ec8, 0x1ec8 }, { 0x1eca, 0x1eca }, { 0x1ecc, 0x1ecc },
            { 0x1ece, 0x1ece }, { 0x1ed0, 0x1ed0 }, { 0x1ed2, 0x1ed2 },
            { 0x1ed4, 0x1ed4 }, { 0x1ed6, 0x1ed6 }, { 0x1ed8, 0x1ed8 },
            { 0x1eda, 0x1eda }, { 0x1edc, 0x1edc }, { 0x1ede, 0x1ede },
            { 0x1ee0, 0x1ee0 }, { 0x1ee2, 0x1ee2 }, { 0x1ee4, 0x1ee4 },
            { 0x1ee6, 0x1ee6 }, { 0x1ee8, 0x1ee8 }, { 0x1eea, 0x1eea },
            { 0x1eec, 0x1eec }, { 0x1eee, 0x1eee }, { 0x1ef0, 0x1ef0 },
            { 0x1ef2, 0x1ef2 }, { 0x1ef4, 0x1ef4 }, { 0x1ef6, 0x1ef6 },
            { 0x1ef8, 0x1ef8 }, { 0x1f08, 0x1f0f }, { 0x1f18, 0x1f1d },
            { 0x1f28, 0x1f2f }, { 0x1f38, 0x1f3f }, { 0x1f48, 0x1f4d },
            { 0x1f59, 0x1f59 }, { 0x1f5b, 0x1f5b }, { 0x1f5d, 0x1f5d },
            { 0x1f5f, 0x1f5f }, { 0x1f68, 0x1f6f }, { 0x1fb8, 0x1fbb },
            { 0x1fc8, 0x1fcb }, { 0x1fd8, 0x1fdb }, { 0x1fe8, 0x1fec },
            { 0x1ff8, 0x1ffb }, { 0x2102, 0x2102 }, { 0x2107, 0x2107 },
            { 0x210b, 0x210d }, { 0x2110, 0x2112 }, { 0x2115, 0x2115 },
            { 0x2119, 0x211d }, { 0x2124, 0x2124 }, { 0x2126, 0x2126 },
            { 0x2128, 0x2128 }, { 0x212a, 0x212d }, { 0x2130, 0x2131 },
            { 0x2133, 0x2133 }, { 0x213e, 0x213f }, { 0x2145, 0x2145 },
            { 0x2c00, 0x2c2e }, { 0x2c80, 0x2c80 }, { 0x2c82, 0x2c82 },
            { 0x2c84, 0x2c84 }, { 0x2c86, 0x2c86 }, { 0x2c88, 0x2c88 },
            { 0x2c8a, 0x2c8a }, { 0x2c8c, 0x2c8c }, { 0x2c8e, 0x2c8e },
            { 0x2c90, 0x2c90 }, { 0x2c92, 0x2c92 }, { 0x2c94, 0x2c94 },
            { 0x2c96, 0x2c96 }, { 0x2c98, 0x2c98 }, { 0x2c9a, 0x2c9a },
            { 0x2c9c, 0x2c9c }, { 0x2c9e, 0x2c9e }, { 0x2ca0, 0x2ca0 },
            { 0x2ca2, 0x2ca2 }, { 0x2ca4, 0x2ca4 }, { 0x2ca6, 0x2ca6 },
            { 0x2ca8, 0x2ca8 }, { 0x2caa, 0x2caa }, { 0x2cac, 0x2cac },
            { 0x2cae, 0x2cae }, { 0x2cb0, 0x2cb0 }, { 0x2cb2, 0x2cb2 },
            { 0x2cb4, 0x2cb4 }, { 0x2cb6, 0x2cb6 }, { 0x2cb8, 0x2cb8 },
            { 0x2cba, 0x2cba }, { 0x2cbc, 0x2cbc }, { 0x2cbe, 0x2cbe },
            { 0x2cc0, 0x2cc0 }, { 0x2cc2, 0x2cc2 }, { 0x2cc4, 0x2cc4 },
            { 0x2cc6, 0x2cc6 }, { 0x2cc8, 0x2cc8 }, { 0x2cca, 0x2cca },
            { 0x2ccc, 0x2ccc }, { 0x2cce, 0x2cce }, { 0x2cd0, 0x2cd0 },
            { 0x2cd2, 0x2cd2 }, { 0x2cd4, 0x2cd4 }, { 0x2cd6, 0x2cd6 },
            { 0x2cd8, 0x2cd8 }, { 0x2cda, 0x2cda }, { 0x2cdc, 0x2cdc },
            { 0x2cde, 0x2cde }, { 0x2ce0, 0x2ce0 }, { 0x2ce2, 0x2ce2 },
            { 0xff21, 0xff3a }, { 0x10400, 0x10427 }, { 0x1d400, 0x1d419 },
            { 0x1d434, 0x1d44d }, { 0x1d468, 0x1d481 }, { 0x1d49c, 0x1d49c },
            { 0x1d49e, 0x1d49f }, { 0x1d4a2, 0x1d4a2 }, { 0x1d4a5, 0x1d4a6 },
            { 0x1d4a9, 0x1d4ac }, { 0x1d4ae, 0x1d4b5 }, { 0x1d4d0, 0x1d4e9 },
            { 0x1d504, 0x1d505 }, { 0x1d507, 0x1d50a }, { 0x1d50d, 0x1d514 },
            { 0x1d516, 0x1d51c }, { 0x1d538, 0x1d539 }, { 0x1d53b, 0x1d53e },
            { 0x1d540, 0x1d544 }, { 0x1d546, 0x1d546 }, { 0x1d54a, 0x1d550 },
            { 0x1d56c, 0x1d585 }, { 0x1d5a0, 0x1d5b9 }, { 0x1d5d4, 0x1d5ed },
            { 0x1d608, 0x1d621 }, { 0x1d63c, 0x1d655 }, { 0x1d670, 0x1d689 },
            { 0x1d6a8, 0x1d6c0 }, { 0x1d6e2, 0x1d6fa }, { 0x1d71c, 0x1d734 },
            { 0x1d756, 0x1d76e }, { 0x1d790, 0x1d7a8 }
        };

        for (int i = 0; i < 476; ++i)
        {
            if (c >= upper_table[i][0] && c <= upper_table[i][1])
            {
                return true;
            }
        }

        return false;
    }

    bool String::IsControl(rune c)
    {
        static const rune cntrl_table[19][2] =
        {
            { 0x0000, 0x001f }, { 0x007f, 0x009f }, { 0x00ad, 0x00ad },
            { 0x0600, 0x0603 }, { 0x06dd, 0x06dd }, { 0x070f, 0x070f },
            { 0x17b4, 0x17b5 }, { 0x200b, 0x200f }, { 0x202a, 0x202e },
            { 0x2060, 0x2063 }, { 0x206a, 0x206f }, { 0xd800, 0xf8ff },
            { 0xfeff, 0xfeff }, { 0xfff9, 0xfffb }, { 0x1d173, 0x1d17a },
            { 0xe0001, 0xe0001 }, { 0xe0020, 0xe007f }, { 0xf0000, 0xffffd },
            { 0x100000, 0x10fffd }
        };

        for (int i = 0; i < 19; ++i)
        {
            if (c >= cntrl_table[i][0] && c <= cntrl_table[i][1])
            {
                return true;
            }
        }

        return false;
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
