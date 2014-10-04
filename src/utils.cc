#include <climits>
#include <cfloat>
#include <cmath>

#include "utils.h"

namespace tempearly
{
    bool Utils::ParseInt(const String& source, int& slot, int radix)
    {
        const char* ptr = source.c_str();
        std::size_t remain = source.length();
        unsigned int number = 0;
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

        const unsigned int div = INT_MAX / radix;
        const unsigned int rem = INT_MAX % radix;

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
        const char* ptr = source.c_str();
        std::size_t remain = source.length();
        double number = 0.0;
        bool sign;
        int exponent = 0;
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
}
