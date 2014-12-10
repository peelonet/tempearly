#include "utils.h"
#include "core/bytestring.h"
#include "core/stringbuilder.h"
#include "net/url.h"

namespace tempearly
{

    void Utils::ParseQueryString(const ByteString& input, Dictionary<Vector<String> >& dictionary)
    {
        ParseQueryString(input.GetBytes(), input.GetLength(), dictionary);
    }

    void Utils::ParseQueryString(const byte* input, std::size_t length, Dictionary<Vector<String> >& dictionary)
    {
        ByteString name_in;
        ByteString value_in;
        String name_out;
        String value_out;

        while (length > 0)
        {
            const byte* begin = input;
            const byte* end = static_cast<const byte*>(std::memchr(begin, '=', length));

            if (!end)
            {
                return;
            }
            name_in = ByteString(begin, end - begin);
            length -= end - begin + 1;
            begin = end + 1;
            if ((end = static_cast<const byte*>(std::memchr(begin, '&', length))))
            {
                value_in = ByteString(begin, end - begin);
                length -= end - begin + 1;
                input = end + 1;
            } else {
                value_in = ByteString(begin, length);
                length = 0;
            }
            if (Url::Decode(name_in, name_out) && Url::Decode(value_in, value_out))
            {
                Dictionary<Vector<String> >::Entry* entry = dictionary.Find(name_out);

                if (entry)
                {
                    entry->GetValue().PushBack(value_out);
                } else {
                    dictionary.Insert(name_out, Vector<String>(1, value_out));
                }
            }
        }
    }
}
