#include <cctype>

#include "utils.h"
#include "core/bytestring.h"
#include "core/stringbuilder.h"
#include "net/url.h"

namespace tempearly
{
    static bool is_reserved(rune);
    static bool is_valid(rune);

    Url::Url(const String& scheme,
             const String& username,
             const String& password,
             const String& hostname,
             int port,
             const String& path,
             const String& query,
             const String& fragment)
        : m_scheme(scheme)
        , m_username(username)
        , m_password(password)
        , m_hostname(hostname)
        , m_port(port)
        , m_path(path)
        , m_query(query)
        , m_fragment(fragment) {}

    Url::Url(const Url& that)
        : m_scheme(that.m_scheme)
        , m_username(that.m_username)
        , m_password(that.m_password)
        , m_hostname(that.m_hostname)
        , m_port(that.m_port)
        , m_path(that.m_path)
        , m_query(that.m_query)
        , m_fragment(that.m_fragment) {}

    String Url::GetAuthority() const
    {
        if (m_username.IsEmpty() && m_password.IsEmpty())
        {
            return String();
        } else {
            return m_username + ":" + m_password;
        }
    }

    bool Url::Decode(const ByteString& input, String& output)
    {
        return Decode(input.GetBytes(), input.GetLength(), output);
    }

    bool Url::Decode(const byte* input, std::size_t length, String& output)
    {
        // First check if the string actually contains any encoded input.
        if (std::memchr(input, '+', length) || std::memchr(input, '%', length))
        {
            Vector<char> result;

            result.Reserve(length);
            for (std::size_t i = 0; i < length; ++i)
            {
                byte b = input[i];

                if (b == '+')
                {
                    result.PushBack(' ');
                    continue;
                }
                if (b == '%')
                {
                    if (length - i < 2)
                    {
                        return false; // Malformed query string
                    }
                    for (std::size_t j = 0; j < 2; ++j)
                    {
                        const byte code = input[i + j + 1];

                        if (0x30 <= code && code <= 0x39)
                        {
                            b = b * 16 + code - 0x30;
                        }
                        else if (0x41 <= code && code <= 0x46)
                        {
                            b = b * 16 + code - 0x37;
                        }
                        else if (0x61 <= code && code <= 0x66)
                        {
                            b = b * 16 + code - 0x57;
                        } else {
                            return false;
                        }
                    }
                    i += 2;
                }
                result.PushBack(static_cast<char>(b));
            }
            result.PushBack(0);
            output.Assign(result.GetData());
        } else {
            output.Assign(String::DecodeAscii(input, length));
        }

        return true;
    }

    static inline void percent_encode(StringBuilder& result, char* buffer, byte b)
    {
        std::sprintf(buffer, "%%%02x", b);
        result << buffer;
    }

    String Url::Encode(const String& input)
    {
        if (!input.Matches(is_valid))
        {
            const std::size_t length = input.GetLength();
            StringBuilder result(length);
            char buffer[4];

            for (std::size_t i = 0; i < length; ++i)
            {
                const rune r = input[i];

                if (is_valid(r))
                {
                    result << r;
                }
                else if (r > 0x10ffff
                        || (r & 0xfffe) == 0xfffe
                        || (r >= 0xd800 && r <= 0xdfff)
                        || (r >= 0xfdd0 && r <= 0xfdef))
                {
                    continue;
                }
                else if (r <= 0x7f)
                {
                    percent_encode(result, buffer, static_cast<byte>(r));
                }
                else if (r <= 0x07ff)
                {
                    percent_encode(result, buffer, static_cast<byte>(0xc0 | ((r & 0x7c0) >> 6)));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | (r & 0x3f)));
                }
                else if (r <= 0xffff)
                {
                    percent_encode(result, buffer, static_cast<byte>(0xe0 | ((r & 0xf000)) >> 12));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | ((r & 0xfc0)) >> 6));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | (r & 0x3f)));
                } else {
                    percent_encode(result, buffer, static_cast<byte>(0xf0 | ((r & 0x1c0000) >> 18)));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | ((r & 0x3f000) >> 12)));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | ((r & 0xfc0) >> 6)));
                    percent_encode(result, buffer, static_cast<byte>(0x80 | (r & 0x3f)));
                }
            }

            return result.ToString();
        }

        return input;
    }

    Url& Url::Assign(const Url& that)
    {
        m_scheme.Assign(that.m_scheme);
        m_username.Assign(that.m_username);
        m_password.Assign(that.m_password);
        m_hostname.Assign(that.m_hostname);
        m_port = that.m_port;
        m_path.Assign(that.m_path);
        m_query.Assign(that.m_query);
        m_fragment.Assign(that.m_fragment);

        return *this;
    }

    String Url::ToString() const
    {
        StringBuilder result;

        if (!m_scheme.IsEmpty())
        {
            result << m_scheme << ':' << '/' << '/';
        }
        if (!m_hostname.IsEmpty())
        {
            if (!m_username.IsEmpty() || !m_password.IsEmpty())
            {
                result << m_username << ':' << m_password << '@';
            }
            result << m_hostname;
            if (m_port >= 0)
            {
                result << ':' << Utils::ToString(static_cast<i64>(m_port));
            }
            if (!m_path.IsEmpty() && m_path.GetFront() != '/')
            {
                result << '/';
            }
        }
        if (!m_path.IsEmpty())
        {
            result << m_path;
        }
        if (!m_query.IsEmpty())
        {
            result << '?' << m_query;
        }
        if (!m_fragment.IsEmpty())
        {
            result << '#' << m_fragment;
        }

        return result.ToString();
    }

    static bool is_reserved(rune r)
    {
        return r == '!' || r == '*' || r == '\'' || r == '(' || r == ')' || r == ';' || r == ':' || r == '@'
            || r == '&' || r == '=' || r == '+' || r == '$' || r == ',' || r == '/' || r == '?' || r == '#'
            || r == '[' || r == ']';
    }

    static bool is_valid(rune r)
    {
        return !is_reserved(r) && (r == '-' || r == '_' || r == '.' || r == '~' || std::isalnum(r));
    }
}
