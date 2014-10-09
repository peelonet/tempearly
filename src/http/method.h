#ifndef TEMPEARLY_HTTP_METHOD_H_GUARD
#define TEMPEARLY_HTTP_METHOD_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class HttpMethod
    {
    public:
        enum Kind
        {
            GET,
            HEAD,
            POST,
            PUT,
            DELETE,
            TRACE,
            OPTIONS,
            CONNECT,
            PATCH
        };

        static bool Parse(const String& string, Kind& slot);

        static String ToString(Kind kind);

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(HttpMethod);
    };
}

#endif /* !TEMPEARLY_HTTP_METHOD_H_GUARD */
