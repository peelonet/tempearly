#include "cookie.h"

namespace tempearly
{
    Cookie::Cookie(const String& name,
                   const String& value,
                   const String& comment,
                   const String& domain,
                   unsigned long max_age,
                   const String& path,
                   bool secure)
        : m_name(name)
        , m_value(value)
        , m_comment(comment)
        , m_domain(domain)
        , m_max_age(max_age)
        , m_path(path)
        , m_secure(secure) {}

    bool Cookie::Parse(const String& source, Cookie& cookie)
    {
        // Find the '=' separating the name and value.
        std::size_t pos = source.IndexOf('=');

        // If no '=' was found, it's an invalid cookie.
        if (pos == String::npos)
        {
            return false;
        }

        // Skip leading whitespace.
        std::size_t wscount = 0;

        for (std::size_t i = 0; i < source.GetLength(); ++i)
        {
            if (source[i] == ' '
                || source[i] == '\f'
                || source[i] == '\n'
                || source[i] == '\r'
                || source[i] == '\t'
                || source[i] == '\v')
            {
                ++wscount;
            } else {
                break;
            }
        }

        cookie.m_name = source.SubString(wscount, pos - wscount);
        cookie.m_value = source.SubString(pos + 1);

        return true;
    }

    Cookie& Cookie::assign(const Cookie& that)
    {
        m_name = that.m_name;
        m_value = that.m_value;
        m_comment = that.m_comment;
        m_domain = that.m_domain;
        m_max_age = that.m_max_age;
        m_path = that.m_path;
        m_secure = that.m_secure;

        return *this;
    }
}
