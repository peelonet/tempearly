#include "core/dictionary.h"
#include "http/version.h"

namespace tempearly
{
    typedef Dictionary<HttpVersion::Kind> HttpVersionMap;

    static HttpVersionMap version_map;

    static void fill_version_map()
    {
        version_map.Insert("HTTP/0.9", HttpVersion::VERSION_09);
        version_map.Insert("HTTP/1.0", HttpVersion::VERSION_10);
        version_map.Insert("HTTP/1.1", HttpVersion::VERSION_11);
    }

    bool HttpVersion::Parse(const String& string, Kind& slot)
    {
        const HttpVersionMap::Entry* entry;

        if (version_map.IsEmpty())
        {
            fill_version_map();
        }
        if ((entry = version_map.Find(string)))
        {
            slot = entry->value;

            return true;
        }

        return false;
    }

    String HttpVersion::ToString(Kind kind)
    {
        if (version_map.IsEmpty())
        {
            fill_version_map();
        }
        for (const HttpVersionMap::Entry* entry = version_map.GetFront();
             entry;
             entry = entry->next)
        {
            if (entry->value == kind)
            {
                return entry->id;
            }
        }

        return "UNKNOWN";
    }
}
