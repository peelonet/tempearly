#include <cstring>

#include "core/stringbuilder.h"

namespace tempearly
{
    StringBuilder::StringBuilder(std::size_t capacity)
        : m_capacity(capacity)
        , m_length(0)
        , m_runes(new rune[m_capacity]) {}

    StringBuilder::StringBuilder(const StringBuilder& that)
        : m_capacity(that.m_length)
        , m_length(that.m_length)
        , m_runes(new rune[m_capacity])
    {
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(that.m_runes),
                    sizeof(rune) * m_length);
    }

    StringBuilder::StringBuilder(const String& string)
        : m_capacity(string.GetLength())
        , m_length(m_capacity)
        , m_runes(new rune[m_capacity])
    {
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(string.GetRunes()),
                    sizeof(rune) * m_length);
    }

    StringBuilder::~StringBuilder()
    {
        delete[] m_runes;
    }

    StringBuilder& StringBuilder::operator=(const StringBuilder& that)
    {
        if (m_runes == that.m_runes)
        {
            return *this;
        }
        if (m_capacity < that.m_length)
        {
            delete[] m_runes;
            m_runes = new rune[m_capacity = that.m_length];
        }
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(that.m_runes),
                    sizeof(rune) * (m_length = that.m_length));

        return *this;
    }

    StringBuilder& StringBuilder::operator=(const String& s)
    {
        if (m_capacity < s.GetLength())
        {
            delete[] m_runes;
            m_runes = new rune[m_capacity = s.GetLength()];
        }
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(s.GetRunes()),
                    sizeof(rune) * (m_length = s.GetLength()));

        return *this;
    }

    StringBuilder& StringBuilder::operator=(rune c)
    {
        if (m_capacity < 1)
        {
            delete[] m_runes;
            m_runes = new rune[m_capacity = 16];
        }
        m_runes[0] = c;
        m_length = 1;

        return *this;
    }

    void StringBuilder::Clear()
    {
        m_length = 0;
    }

    void StringBuilder::Reserve(std::size_t n)
    {
        rune* runes;

        if (m_capacity >= n)
        {
            return;
        }
        runes = new rune[m_capacity = n];
        std::memcpy(static_cast<void*>(runes),
                    static_cast<const void*>(m_runes),
                    sizeof(rune) * m_length);
        delete[] m_runes;
        m_runes = runes;
    }

    StringBuilder& StringBuilder::Assign(std::size_t n, rune r)
    {
        if (m_capacity < n)
        {
            delete[] m_runes;
            m_runes = new rune[m_capacity = n];
        }
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(&r),
                    sizeof(rune) * (m_length = n));

        return *this;
    }

    void StringBuilder::Append(rune c)
    {
        if (m_capacity < m_length + 1)
        {
            rune* runes = new rune[m_capacity += 16];

            std::memcpy(static_cast<void*>(runes),
                        static_cast<const void*>(m_runes),
                        sizeof(rune) * m_length);
            delete[] m_runes;
            m_runes = runes;
        }
        m_runes[m_length++] = c;
    }

    void StringBuilder::Append(const rune* c, std::size_t n)
    {
        if (!n)
        {
            return;
        }
        Reserve(m_length + n);
        std::memcpy(static_cast<void*>(m_runes + m_length),
                    static_cast<const void*>(c),
                    sizeof(rune) * n);
        m_length += n;
    }

    void StringBuilder::Append(const String& s)
    {
        Append(s.GetRunes(), s.GetLength());
    }

    void StringBuilder::Prepend(rune c)
    {
        if (m_length + 1 <= m_capacity)
        {
            std::memmove(static_cast<void*>(m_runes + 1),
                         static_cast<const void*>(m_runes),
                         sizeof(rune) * m_length);
        } else {
            rune* runes = new rune[m_capacity += 16];

            std::memcpy(static_cast<void*>(runes + 1),
                        static_cast<const void*>(m_runes),
                        sizeof(rune) * m_length);
            delete[] m_runes;
            m_runes = runes;
        }
        m_runes[0] = c;
        ++m_length;
    }

    void StringBuilder::Prepend(const rune* c, std::size_t n)
    {
        if (!n)
        {
            return;
        }
        if (m_capacity >= m_length + n)
        {
            std::memmove(static_cast<void*>(m_runes + n),
                         static_cast<const void*>(m_runes),
                         sizeof(rune) * m_length);
        } else {
            rune* runes = new rune[m_capacity += n];

            std::memcpy(static_cast<void*>(runes + n),
                        static_cast<const void*>(m_runes),
                        sizeof(rune) * m_length);
            delete[] m_runes;
            m_runes = runes;
        }
        std::memcpy(static_cast<void*>(m_runes),
                    static_cast<const void*>(c),
                    sizeof(rune) * n);
        m_length += n;
    }

    void StringBuilder::Prepend(const String& string)
    {
        Prepend(string.GetRunes(), string.GetLength());
    }

    rune StringBuilder::PopFront()
    {
        rune c = m_runes[0];

        std::memmove(static_cast<void*>(m_runes),
                     static_cast<const void*>(m_runes + 1),
                     sizeof(rune) * --m_length);

        return c;
    }

    rune StringBuilder::PopBack()
    {
        return m_runes[--m_length];
    }

    void StringBuilder::Erase(std::size_t i)
    {
        std::memmove(static_cast<void*>(m_runes + i),
                     static_cast<const void*>(m_runes + i + 1),
                     sizeof(rune) * (--m_length - i));
    }
}
