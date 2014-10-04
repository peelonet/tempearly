#ifndef TEMPEARLY_UTILS_H_GUARD
#define TEMPEARLY_UTILS_H_GUARD

#include "tempearly.h"

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

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Utils);
    };
}

#endif /* !TEMPEARLY_UTILS_H_GUARD */
