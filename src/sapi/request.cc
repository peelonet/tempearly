#include "utils.h"
#include "core/bytestring.h"
#include "sapi/request.h"

namespace tempearly
{
    Request::Request()
        : m_parameters_parsed(false) {}

    Request::~Request() {}

    Vector<String> Request::GetParameterNames()
    {
        Vector<String> names;

        if (!m_parameters_parsed)
        {
            ParseParameters();
        }
        for (const ParameterMap::Entry* entry = m_parameters.GetFront(); entry; entry = entry->GetNext())
        {
            names.PushBack(entry->GetName());
        }

        return names;
    }

    bool Request::HasParameter(const String& id)
    {
        const ParameterMap::Entry* entry;

        if (!m_parameters_parsed)
        {
            ParseParameters();
        }
        
        return (entry = m_parameters.Find(id)) && !entry->GetValue().IsEmpty();
    }

    bool Request::GetParameter(const String& id, String& value)
    {
        const ParameterMap::Entry* entry;

        if (!m_parameters_parsed)
        {
            ParseParameters();
        }
        if ((entry = m_parameters.Find(id)))
        {
            const Vector<String>& values = entry->GetValue();

            if (!values.IsEmpty())
            {
                value = values.GetFront();

                return true;
            }
        }

        return false;
    }

    bool Request::GetAllParameters(const String& id, Vector<String>& values)
    {
        const ParameterMap::Entry* entry;

        if (!m_parameters_parsed)
        {
            ParseParameters();
        }
        if ((entry = m_parameters.Find(id)))
        {
            const Vector<String>& values2 = entry->GetValue();

            if (!values2.IsEmpty())
            {
                values = values2;

                return true;
            }
        }

        return false;
    }

    void Request::ParseParameters()
    {
        const ByteString query_string = GetQueryString();

        m_parameters_parsed = true;
        if (!query_string.IsEmpty())
        {
            Utils::ParseQueryString(query_string.GetBytes(), query_string.GetLength(), m_parameters);
        }
        // TODO: process multipart requests
        if (GetMethod() == HttpMethod::POST
            && GetContentLength() > 0
            && GetContentType().StartsWith("application/x-www-form-urlencoded"))
        {
            const ByteString body = GetBody();

            Utils::ParseQueryString(body.GetBytes(), body.GetLength(), m_parameters);
        }
    }
}
