#include "response.h"

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

    String Response::GetHeader(const String& name) const
    {
        const Dictionary<String>::Entry* entry = m_headers.Find(name);

        if (entry)
        {
            return entry->value;
        } else {
            return String();
        }
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

    void Response::Write(const std::string& string)
    {
        Write(string.length(), string.c_str());
    }
}
