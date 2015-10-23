#include "core/bytestring.h"
#include "sapi/httpd/request.h"

namespace tempearly
{
    HttpServerRequest::HttpServerRequest(HttpMethod::Kind method,
                                         const String& path,
                                         const ByteString& query_string,
                                         const Dictionary<String>& headers,
                                         const byte* body,
                                         std::size_t body_size)
        : m_method(method)
        , m_path(path)
        , m_query_string(query_string)
        , m_headers(headers)
        , m_body(nullptr)
    {
        if (body_size)
        {
            m_body = new ByteString(body, body_size);
        }
    }

    HttpServerRequest::~HttpServerRequest()
    {
        if (m_body)
        {
            delete m_body;
        }
    }

    HttpMethod::Kind HttpServerRequest::GetMethod() const
    {
        return m_method;
    }

    String HttpServerRequest::GetPath() const
    {
        return m_path;
    }

    bool HttpServerRequest::IsSecure() const
    {
        return false;
    }

    bool HttpServerRequest::IsAjax() const
    {
        String value;

        if (GetHeader("X-Requested-With", value))
        {
            return value.EqualsIgnoreCase("xmlhttprequest");
        } else {
            return false;
        }
    }

    String HttpServerRequest::GetContentType() const
    {
        String value;

        GetHeader("Content-Type", value);

        return value;
    }

    std::size_t HttpServerRequest::GetContentLength() const
    {
        String value;

        if (GetHeader("Content-Length", value))
        {
            i64 slot;

            if (value.ParseInt(slot, 10))
            {
                return static_cast<std::size_t>(slot);
            }
        }

        return 0;
    }

    ByteString HttpServerRequest::GetBody()
    {
        return m_body ? *m_body : ByteString();
    }

    ByteString HttpServerRequest::GetQueryString()
    {
        return m_query_string;
    }

    bool HttpServerRequest::HasHeader(const String& id) const
    {
        return m_headers.Find(id);
    }

    bool HttpServerRequest::GetHeader(const String& id, String& slot) const
    {
        const Dictionary<String>::Entry* e = m_headers.Find(id);

        if (e)
        {
            slot = e->GetValue();

            return true;
        }

        return false;
    }
}
