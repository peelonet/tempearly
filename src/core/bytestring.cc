#include <cstring>

#include "core/bytestring.h"

namespace tempearly
{
    ByteString::ByteString()
        : m_length(0)
        , m_bytes(new byte[1])
        , m_counter(new unsigned int[1])
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
        , m_bytes(new byte[m_length + 1])
        , m_counter(new unsigned int[1])
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
        , m_bytes(new byte[m_length + 1])
        , m_counter(new unsigned int[1])
    {
        std::memcpy(static_cast<void*>(m_bytes),
                    static_cast<const void*>(b),
                    sizeof(byte) * m_length);
        m_bytes[m_length] = 0;
        m_counter[0] = 1;
    }

    ByteString::~ByteString()
    {
        if (--m_counter[0] == 0)
        {
            delete[] m_bytes;
            delete[] m_counter;
        }
    }

    ByteString& ByteString::operator=(const ByteString& that)
    {
        if (m_bytes != that.m_bytes)
        {
            if (--m_counter[0] == 0)
            {
                delete[] m_bytes;
                delete[] m_counter;
            }
            m_bytes = that.m_bytes;
            m_counter = that.m_counter;
            m_length = that.m_length;
            ++m_counter[0];
        }

        return *this;
    }
}
