#ifndef TEMPEARLY_UTILS_H_GUARD
#define TEMPEARLY_UTILS_H_GUARD

#include "core/dictionary.h"
#include "core/vector.h"

namespace tempearly
{
    class Utils
    {
    public:
        /**
         * Parses an 64-bit integer value from contents of the string.
         *
         * \param slot  Variable where the result is stored
         * \param radix The radix to use, for example if you want to parse
         *              a hexadecimal number, set this to 16. If you want
         *              this method to determine the readix, leave it to -1
         * \return      True if the conversion was successfull, false if
         *              underflow or overflow error occurred
         */
        static bool ParseInt(const String& source, i64& slot, int radix = -1);

        /**
         * Parses an double precision decimal value from contents of the
         * string.
         *
         * \param slot Variable where the result is stored
         * \return     True if the conversion was successfull, false if
         *             underflow or overflow error occurred
         */
        static bool ParseFloat(const String& source, double& slot);

        static String ToString(u64 number, int radix = 10);

        static String ToString(i64 number, int radix = 10);

        static String ToString(double number);

        /**
         * Parses given byte string as query string and inserts named values
         * extracted from it to the given dictionary.
         */
        static void ParseQueryString(const ByteString& input, Dictionary<Vector<String> >& dictionary);

        /**
         * Parses given byte array as query string and inserts named values
         * extracted from it to the given dictionary.
         */
        static void ParseQueryString(const byte* input, std::size_t length, Dictionary<Vector<String> >& dictionary);

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Utils);
    };
}

#endif /* !TEMPEARLY_UTILS_H_GUARD */
