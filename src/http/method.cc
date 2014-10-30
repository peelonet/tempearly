#include "core/dictionary.h"
#include "http/method.h"

namespace tempearly
{
    typedef Dictionary<HttpMethod::Kind> HttpMethodMap;

    static HttpMethodMap method_map;

    static void fill_method_map()
    {
        method_map.Insert("GET", HttpMethod::GET);
        method_map.Insert("HEAD", HttpMethod::HEAD);
        method_map.Insert("POST", HttpMethod::POST);
        method_map.Insert("PUT", HttpMethod::PUT);
        method_map.Insert("DELETE", HttpMethod::DELETE);
        method_map.Insert("TRACE", HttpMethod::TRACE);
        method_map.Insert("OPTIONS", HttpMethod::OPTIONS);
        method_map.Insert("CONNECT", HttpMethod::CONNECT);
        method_map.Insert("PATCH", HttpMethod::PATCH);
    }

    bool HttpMethod::Parse(const String& string, Kind& slot)
    {
        const Dictionary<Kind>::Entry* entry;

        if (method_map.IsEmpty())
        {
            fill_method_map();
        }
        if (string.Matches(String::IsUpper))
        {
            entry = method_map.Find(string);
        } else {
            entry = method_map.Find(string.Map(String::ToUpper));
        }
        if (entry)
        {
            slot = entry->GetValue();

            return true;
        }

        return false;
    }

    String HttpMethod::ToString(Kind kind)
    {
        if (method_map.IsEmpty())
        {
            fill_method_map();
        }
        for (const HttpMethodMap::Entry* entry = method_map.GetFront(); entry; entry = entry->GetNext())
        {
            if (entry->GetValue() == kind)
            {
                return entry->GetName();
            }
        }

        return "UNKNOWN";
    }
}
