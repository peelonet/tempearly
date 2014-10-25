#include <cstring>

#include "core/bytestring.h"

namespace tempearly
{
    ByteString::ByteString()
        : m_length(0)
        , m_bytes(Memory::Allocate<byte>(1))
        , m_counter(Memory::Allocate<unsigned int>(1))
    {
        m_bytes[0] = 0;
        m_counter[0] = 1;
    }

    ByteString::ByteString(const ByteString& that)
        : m_length(that.m_length)
        , m_bytes(that.m_bytes)
        , m_counter(that.m_counter)
    {
        ++m_counter[0];
    }

    ByteString::ByteString(const char* input)
        : m_length(std::strlen(input))
        , m_bytes(Memory::Allocate<byte>(m_length + 1))
        , m_counter(Memory::Allocate<unsigned int>(1))
    {
        for (std::size_t i = 0; i < m_length; ++i)
        {
            m_bytes[i] = input[i];
        }
        m_bytes[m_length] = 0;
        m_counter[0] = 1;
    }

    ByteString::ByteString(const byte* b, std::size_t n)
        : m_length(n)
        , m_bytes(Memory::Allocate<byte>(m_length + 1))
        , m_counter(Memory::Allocate<unsigned int>(1))
    {
        Memory::Copy<byte>(m_bytes, b, m_length);
        m_bytes[m_length] = 0;
        m_counter[0] = 1;
    }

    ByteString::~ByteString()
    {
        if (--m_counter[0] == 0)
        {
            Memory::Unallocate<byte>(m_bytes);
            Memory::Unallocate<unsigned int>(m_counter);
        }
    }

    ByteString& ByteString::Assign(const ByteString& that)
    {
        if (m_bytes != that.m_bytes)
        {
            if (--m_counter[0] == 0)
            {
                Memory::Unallocate<byte>(m_bytes);
                Memory::Unallocate<unsigned int>(m_counter);
            }
            m_bytes = that.m_bytes;
            m_counter = that.m_counter;
            m_length = that.m_length;
            ++m_counter[0];
        }

        return *this;
    }

    bool ByteString::Equals(const ByteString& that) const
    {
        if (m_bytes == that.m_bytes)
        {
            return true;
        }
        else if (m_length != that.m_length)
        {
            return false;
        }
        for (std::size_t i = 0; i < m_length; ++i)
        {
            if (m_bytes[i] != that.m_bytes[i])
            {
                return false;
            }
        }

        return true;
    }

    int ByteString::Compare(const ByteString& that) const
    {
        const std::size_t n = m_length < that.m_length ? m_length : that.m_length;

        if (m_bytes == that.m_bytes)
        {
            return 0;
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            const byte a = m_bytes[i];
            const byte b = that.m_bytes[i];

            if (a != b)
            {
                return a > b ? 1 : -1;
            }
        }
        if (m_length == that.m_length)
        {
            return 0;
        } else {
            return m_length > that.m_length ? 1 : -1;
        }
    }

    ByteString ByteString::Concat(const ByteString& that) const
    {
        if (m_length == 0)
        {
            return that;
        }
        else if (that.m_length == 0)
        {
            return *this;
        } else {
            ByteString result;

            result.m_length = m_length + that.m_length;
            result.m_bytes = Memory::Allocate<byte>(result.m_length + 1);
            result.m_counter = Memory::Allocate<unsigned int>(1);
            result.m_counter[0] = 1;
            Memory::Copy<byte>(result.m_bytes, m_bytes, m_length);
            Memory::Copy<byte>(result.m_bytes + m_length, that.m_bytes, that.m_length);

            return result;
        }
    }
}
