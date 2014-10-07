#include "utils.h"
#include "core/bytestring.h"
#include "sapi/httpd/httpd-request.h"
#include "sapi/httpd/socket.h"

namespace tempearly
{
    HttpServerRequest::HttpServerRequest(const Handle<Socket>& socket,
                                         const String& method,
                                         const String& query_string,
                                         const Dictionary<String>& headers,
                                         const byte* data,
                                         std::size_t data_size)
        : m_socket(socket.Get())
        , m_method(method)
        , m_headers(headers)
    {
        if (!query_string.IsEmpty())
        {
            Utils::ParseQueryString(query_string, m_parameters);
        }
        if (m_method == "POST"
            && GetContentType() == "application/x-www-form-urlencoded"
            && GetContentLength() > 0)
        {
            Utils::ParseQueryString(ByteString(data, data_size).c_str(), m_parameters);
        }
    }

    String HttpServerRequest::GetMethod() const
    {
        return m_method;
    }

    bool HttpServerRequest::HasParameter(const String& id) const
    {
        const Dictionary<std::vector<String> >::Entry* e = m_parameters.Find(id);

        return e && !e->value.empty();
    }

    bool HttpServerRequest::GetParameter(const String& id, String& slot) const
    {
        const Dictionary<std::vector<String> >::Entry* e = m_parameters.Find(id);

        if (e && !e->value.empty())
        {
            slot = e->value.front();

            return true;
        }

        return false;
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
            slot = e->value;

            return true;
        }

        return false;
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

            if (Utils::ParseInt(value, slot, 10))
            {
                return static_cast<std::size_t>(slot);
            }
        }

        return 0;
    }

    void HttpServerRequest::Mark()
    {
        Request::Mark();
        if (m_socket && !m_socket->IsMarked())
        {
            m_socket->Mark();
        }
    }
}
