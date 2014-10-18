#include <cfloat>
#include <cmath>

#include "utils.h"
#include "core/bytestring.h"
#include "core/stringbuilder.h"

#if defined(TEMPEARLY_HAVE_CLIMITS)
# include <climits>
#elif defined(TEMPEARLY_HAVE_LIMITS_H)
# include <limits.h>
#endif

#if !defined(INT64_MAX)
# if defined(LLONG_MAX)
#  define INT64_MAX LLONG_MAX
# else
#  define INT64_MAX 0x7fffffffffffffff
# endif
#endif

namespace tempearly
{
    bool Utils::ParseInt(const String& source, i64& slot, int radix)
    {
        const rune* ptr = source.GetRunes();
        std::size_t remain = source.GetLength();
        u64 number = 0;
        bool sign = true;

        // Eat whitespace.
        while (remain && std::isspace(*ptr))
        {
            ++ptr; --remain;
        }

        // Get the sign.
        if (remain)
        {
            if (*ptr == '-')
            {
                sign = false;
                ++ptr; --remain;
            }
            else if (*ptr == '+')
            {
                ++ptr; --remain;
            }
        }

        // Try to decipher the prefix.
        if (radix <= 0 && remain > 1 && *ptr == '0')
        {
            ++ptr; --remain;
            switch (*ptr)
            {
                // Hexadecimal
                case 'x': case 'X':
                    radix = 16;
                    ++ptr; --remain;
                    break;

                // Binary
                case 'b': case 'B':
                    radix = 2;
                    ++ptr; --remain;
                    break;

                // Decimal
                case 'd': case 'D':
                    radix = 10;
                    ++ptr; --remain;
                    break;

                // Octal
                case 'o': case 'O':
                    ++ptr; --remain;

                // Default to octal
                default:
                    radix = 8;
            }
        }

        if (radix < 2 || radix > 36)
        {
            radix = 10;
        }

        const u64 div = INT64_MAX / radix;
        const u64 rem = INT64_MAX % radix;

        while (remain)
        {
            unsigned int digit;
            char c = *ptr++;

            --remain;
            if (c >= '0' && c < static_cast<char>('0' + radix))
            {
                digit = c - '0';
            }
            else if (radix > 10)
            {
                if (c >= 'a' && c < static_cast<char>('a' + radix - 10))
                {
                    digit = c - 'a' + 10;
                }
                else if (c >= 'A' && c < static_cast<char>('A' + radix - 10))
                {
                    digit = c - 'A' + 10;
                } else {
                    continue;
                }
            } else {
                continue;
            }
            if (number > div || (number == div && digit > rem))
            {
                return false; // Integer underflow / overflow
            }
            number = (number * radix) + digit;
        }
        slot = sign ? number : -number;

        return true;
    }

    bool Utils::ParseFloat(const String& source, double& slot)
    {
        const rune* ptr = source.GetRunes();
        std::size_t remain = source.GetLength();
        double number = 0.0;
        bool sign;
        i64 exponent = 0;
        bool has_digits = false;
        bool has_dot = false;

        // Eat whitespace.
        while (remain && std::isspace(*ptr))
        {
            ++ptr; --remain;
        }

        // Get the sign.
        sign = remain && *ptr == '-' ? false : true;
        if (!sign || (remain && *ptr == '+'))
        {
            ++ptr; --remain;
        }

        for (; remain; ++ptr, --remain)
        {
            if (*ptr >= '0' && *ptr <= '9')
            {
                has_digits = true;
                if (number > DBL_MAX * 0.1)
                {
                    ++exponent;
                } else {
                    number = (number * 10.0) + (*ptr - '0');
                }
                if (has_dot)
                {
                    --exponent;
                }
            }
            else if (!has_dot && *ptr == '.')
            {
                has_dot = true;
            } else {
                break;
            }
        }

        if (!has_digits)
        {
            slot = 0.0;

            return true;
        }

        // Parse exponent (this is kinda shitty way to it though)
        if (remain && (*ptr == 'e' || *ptr == 'E'))
        {
            if (!ParseInt(String(++ptr, --remain), exponent, 10))
            {
                return false;
            }
        }
        if (number == 0.0)
        {
            slot = 0.0;

            return true;
        }
        if (exponent < 0)
        {
            if (number < DBL_MIN * std::pow(10.0, static_cast<double>(-exponent)))
            {
                return false; // Float underflow
            }
        }
        else if (exponent > 0)
        {
            if (number > DBL_MAX * std::pow(10.0, static_cast<double>(exponent)))
            {
                return false; // Float overflow
            }
        }
        number *= std::pow(10.0, static_cast<double>(exponent));
        slot = number * sign;

        return true;
    }

    static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    String Utils::ToString(u64 number, int radix)
    {
        if (radix < 2 || radix > 36)
        {
            radix = 10;
        }
        if (number != 0)
        {
            StringBuilder result;

            result.Reserve(20);
            do
            {
                result.Prepend(digitmap[number % radix]);
                number /= radix;
            }
            while (number);

            return result.ToString();
        }

        return "0";
    }

    String Utils::ToString(i64 number, int radix)
    {
        if (radix < 2 || radix > 36)
        {
            radix = 10;
        }
        if (number != 0)
        {
            StringBuilder result;
            const bool negative = number < 0;
            u64 mag = static_cast<u64>(negative ? -number: number);

            result.Reserve(21);
            do
            {
                result.Prepend(digitmap[mag % radix]);
                mag /= radix;
            }
            while (mag);
            if (negative)
            {
                result.Prepend('-');
            }

            return result.ToString();
        }

        return "0";
    }

    String Utils::ToString(double number)
    {
        if (std::isinf(number))
        {
            return number < 0.0 ? "-Inf" : "Inf";
        }
        else if (std::isnan(number))
        {
            return "NaN";
        } else {
            char buffer[20];

            std::snprintf(buffer, sizeof(buffer), "%g", number);

            return buffer;
        }
    }

    String Utils::XmlEscape(const String& string)
    {
        if (string.IndexOf('&') != String::npos
            || string.IndexOf('<') != String::npos
            || string.IndexOf('>') != String::npos
            || string.IndexOf('"') != String::npos
            || string.IndexOf('\'') != String::npos)
        {
            StringBuilder result;

            result.Reserve(string.GetLength() + 16);
            for (std::size_t i = 0; i < string.GetLength(); ++i)
            {
                const rune r = string[i];

                switch (r)
                {
                    case '&':
                        result << "&amp;";
                        break;

                    case '<':
                        result << "&lt;";
                        break;

                    case '>':
                        result << "&gt;";
                        break;

                    case '"':
                        result << "&quot;";
                        break;

                    case '\'':
                        result << "&#39;";
                        break;

                    case '\r':
                        break;

                    default:
                        result << r;
                }
            }

            return result.ToString();
        }

        return string;
    }

    bool Utils::UrlDecode(const String& string, String& slot)
    {
        // First check if the string has any encoding.
        if (string.IndexOf('+') != String::npos
            || string.IndexOf('%') != String::npos)
        {
            ByteString bytes = string.Encode();
            std::vector<char> result;

            result.reserve(bytes.GetLength());
            for (std::size_t i = 0; i < bytes.GetLength(); ++i)
            {
                const byte b = bytes[i];

                if (b == '+')
                {
                    result.push_back(' ');
                }
                else if (b == '%')
                {
                    char byte = 0;

                    if (bytes.GetLength() - i < 2)
                    {
                        return false; // Malformed query string
                    }
                    for (std::size_t j = 0; j < 2; ++j)
                    {
                        const char code = bytes[i + j + 1];

                        if (0x30 <= code && code <= 0x39)
                        {
                            byte = byte * 16 + code - 0x30;
                        }
                        else if (0x41 <= code && code <= 0x46)
                        {
                            byte = byte * 16 + code - 0x37;
                        }
                        else if (0x61 <= code && code <= 0x66)
                        {
                            byte = byte * 16 + code - 0x57;
                        } else {
                            return false;
                        }
                    }
                    result.push_back(byte);
                    i += 2;
                } else {
                    result.push_back(b);
                }
            }
            result.push_back(0);
            slot = String(result.data());
        } else {
            slot = string;
        }

        return true;
    }

    void Utils::ParseQueryString(const String& string,
                                 Dictionary<std::vector<String> >& dictionary)
    {
        std::size_t current_index = 0;
        String name;
        String value;

        while (current_index < string.GetLength())
        {
            std::size_t index = string.IndexOf('=', current_index);

            if (index == String::npos)
            {
                return;
            }
            name = string.SubString(current_index, index - current_index);
            current_index = index + 1;
            index = string.IndexOf('&', current_index);
            if (index == String::npos)
            {
                value = string.SubString(current_index);
                current_index = string.GetLength();
            } else {
                value = string.SubString(current_index, index - current_index);
                current_index = index + 1;
            }
            if (UrlDecode(name, name) && UrlDecode(value, value))
            {
                Dictionary<std::vector<String> >::Entry* entry = dictionary.Find(name);

                if (entry)
                {
                    entry->GetValue().push_back(value);
                } else {
                    dictionary.Insert(name, std::vector<String>(1, value));
                }
            }
        }
    }
}
