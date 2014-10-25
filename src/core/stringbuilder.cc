#include <cstring>

#include "core/stringbuilder.h"

namespace tempearly
{
    StringBuilder::StringBuilder(std::size_t capacity)
        : m_capacity(capacity)
        , m_length(0)
        , m_runes(Memory::Allocate<rune>(m_capacity)) {}

    StringBuilder::StringBuilder(const StringBuilder& that)
        : m_capacity(that.m_length)
        , m_length(that.m_length)
        , m_runes(Memory::Allocate<rune>(m_capacity))
    {
        Memory::Copy<rune>(m_runes, that.m_runes, m_length);
    }

    StringBuilder::StringBuilder(const String& string)
        : m_capacity(string.GetLength())
        , m_length(m_capacity)
        , m_runes(Memory::Allocate<rune>(m_capacity))
    {
        Memory::Copy<rune>(m_runes, string.GetRunes(), m_length);
    }

    StringBuilder::~StringBuilder()
    {
        Memory::Unallocate<rune>(m_runes);
    }

    StringBuilder& StringBuilder::operator=(const StringBuilder& that)
    {
        if (m_runes == that.m_runes)
        {
            return *this;
        }
        if (m_capacity < that.m_length)
        {
            Memory::Unallocate<rune>(m_runes);
            m_runes = Memory::Allocate<rune>(m_capacity = that.m_length);
        }
        Memory::Copy<rune>(m_runes, that.m_runes, m_length = that.m_length);

        return *this;
    }

    StringBuilder& StringBuilder::operator=(const String& s)
    {
        if (m_capacity < s.GetLength())
        {
            Memory::Unallocate<rune>(m_runes);
            m_runes = Memory::Allocate<rune>(m_capacity = s.GetLength());
        }
        Memory::Copy<rune>(m_runes, s.GetRunes(), m_length = s.GetLength());

        return *this;
    }

    StringBuilder& StringBuilder::operator=(rune c)
    {
        if (m_capacity < 1)
        {
            Memory::Unallocate<rune>(m_runes);
            m_runes = Memory::Allocate<rune>(m_capacity = 16);
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
        runes = Memory::Allocate<rune>(m_capacity = n);
        Memory::Copy<rune>(runes, m_runes, m_length);
        Memory::Unallocate<rune>(m_runes);
        m_runes = runes;
    }

    StringBuilder& StringBuilder::Assign(std::size_t n, rune r)
    {
        if (m_capacity < n)
        {
            Memory::Unallocate<rune>(m_runes);
            m_runes = Memory::Allocate<rune>(m_capacity = n);
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            m_runes[i] = r;
        }
        m_length = n;

        return *this;
    }

    void StringBuilder::Append(rune c)
    {
        if (m_capacity < m_length + 1)
        {
            rune* runes = Memory::Allocate<rune>(m_capacity += 16);

            Memory::Copy<rune>(runes, m_runes, m_length);
            Memory::Unallocate<rune>(m_runes);
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
        Memory::Copy<rune>(m_runes + m_length, c, n);
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
            Memory::Move<rune>(m_runes + 1, m_runes, m_length);
        } else {
            rune* runes = Memory::Allocate<rune>(m_capacity += 16);

            Memory::Copy<rune>(runes + 1, m_runes, m_length);
            Memory::Unallocate<rune>(m_runes);
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
            Memory::Move<rune>(m_runes + n, m_runes, m_length);
        } else {
            rune* runes = Memory::Allocate<rune>(m_capacity += n);

            Memory::Copy<rune>(runes + n, m_runes, m_length);
            Memory::Unallocate<rune>(m_runes);
            m_runes = runes;
        }
        Memory::Copy<rune>(m_runes, c, n);
        m_length += n;
    }

    void StringBuilder::Prepend(const String& string)
    {
        Prepend(string.GetRunes(), string.GetLength());
    }

    rune StringBuilder::PopFront()
    {
        rune c = m_runes[0];

        Memory::Move<rune>(m_runes, m_runes + 1, --m_length);

        return c;
    }

    rune StringBuilder::PopBack()
    {
        return m_runes[--m_length];
    }

    void StringBuilder::Erase(std::size_t i)
    {
        Memory::Move<rune>(m_runes + i, m_runes + i + 1, --m_length - i);
    }
}
