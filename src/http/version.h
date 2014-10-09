#ifndef TEMPEARLY_HTTP_VERSION_H_GUARD
#define TEMPEARLY_HTTP_VERSION_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class HttpVersion
    {
    public:
        enum Kind
        {
            VERSION_09 = 9,
            VERSION_10 = 10,
            VERSION_11 = 11
        };

        static bool Parse(const String& string, Kind& slot);

        static String ToString(Kind kind);

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(HttpVersion);
    };
}

#endif /* !TEMPEARLY_HTTP_VERSION_H_GUARD */
