#include "core/bytestring.h"
#include "sapi/response.h"

namespace tempearly
{
    Response::Response()
        : m_status(200)
    {
        m_headers.Insert("Content-Type", "text/html; charset=utf-8");
    }

    Response::~Response() {}

    bool Response::HasHeader(const String& name) const
    {
        return m_headers.Find(name);
    }

    bool Response::GetHeader(const String& name, String& slot) const
    {
        const Dictionary<String>::Entry* entry = m_headers.Find(name);

        if (entry)
        {
            slot = entry->value;

            return true;
        }
        
        return false;
    }

    void Response::SetHeader(const String& name, const String& value)
    {
        m_headers.Insert(name, value);
    }

    void Response::AddHeader(const String& name, const String& value)
    {
        const Dictionary<String>::Entry* entry = m_headers.Find(name);

        if (entry)
        {
            m_headers.Insert(name, entry->value + ", " + value);
        } else {
            m_headers.Insert(name, value);
        }
    }

    void Response::Write(const String& string)
    {
        ByteString bytes = string.Encode();

        Write(bytes.GetLength(), bytes.c_str());
    }
}
