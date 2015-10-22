#include "api/exception.h"
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
            slot = entry->GetValue();

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
            m_headers.Insert(name, entry->GetValue() + ", " + value);
        } else {
            m_headers.Insert(name, value);
        }
    }

    void Response::RemoveHeader(const String& name)
    {
        m_headers.Erase(name);
    }

    void Response::Write(const String& text)
    {
        Write(text.Encode());
    }

    void Response::SendException(const Handle<Object>& exception)
    {
        if (exception->IsException())
        {
            String message = exception.As<ExceptionObject>()->GetMessage();

            if (IsCommitted())
            {
                Write("\n<p><strong>ERROR:</strong> " + message.EscapeXml() + "</p>\n");
            } else {
                m_headers.Clear();
                SetHeader("Content-Type", "text/plain; charset=utf-8");
                m_status = 500;
                Write("ERROR:\n" + message + "\n");
            }
        }
    }
}
