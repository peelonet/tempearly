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
