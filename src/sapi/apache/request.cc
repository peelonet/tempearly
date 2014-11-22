#include <cstring>
#include <apache2/http_protocol.h>

#include "utils.h"
#include "core/bytestring.h"
#include "sapi/apache/request.h"

#if !defined(BUFSIZ)
# define BUFSIZ 1024
#endif

namespace tempearly
{
    static HttpMethod::Kind parse_method(request_rec*);

    ApacheRequest::ApacheRequest(request_rec* request)
        : m_request(request)
        , m_method(parse_method(request))
        , m_body(0) {}

    ApacheRequest::~ApacheRequest()
    {
        if (m_body)
        {
            delete m_body;
        }
    }

    HttpMethod::Kind ApacheRequest::GetMethod() const
    {
        return m_method;
    }

    String ApacheRequest::GetPath() const
    {
        return m_request->uri;
    }

    bool ApacheRequest::IsSecure() const
    {
        if (m_request->server && m_request->server->server_scheme)
        {
            return !std::strcmp(m_request->server->server_scheme, "https");
        } else {
            return false;
        }
    }

    bool ApacheRequest::IsAjax() const
    {
        const char* value = apr_table_get(m_request->headers_in, "X-Requested-With");

        if (value)
        {
            return String(value).EqualsIgnoreCase("xmlhttprequest");
        } else {
            return false;
        }
    }

    String ApacheRequest::GetContentType() const
    {
        const char* value = apr_table_get(m_request->subprocess_env, "CONTENT_TYPE");

        return value ? value : String();
    }

    std::size_t ApacheRequest::GetContentLength() const
    {
        const char* value = apr_table_get(m_request->subprocess_env, "CONTENT_LENGTH");

        if (value)
        {
            i64 number;

            if (Utils::ParseInt(value, number, 10))
            {
                return static_cast<std::size_t>(number);
            }
        }

        return 0;
    }

    ByteString ApacheRequest::GetBody()
    {
        if (!m_body && ap_should_client_block(m_request))
        {
            std::size_t remain = GetContentLength();
            Vector<byte> body;
            char buffer[BUFSIZ];

            body.Reserve(remain);
            ap_setup_client_block(m_request, REQUEST_CHUNKED_DECHUNK);
            while (remain > 0)
            {
                long read = ap_get_client_block(m_request, buffer, BUFSIZ);

                if (read <= 0)
                {
                    break;
                }
                body.PushBack(reinterpret_cast<byte*>(buffer), read);
            }
            if (!body.IsEmpty())
            {
                m_body = new ByteString(body.GetData(), body.GetSize());
            }
        }

        return m_body ? *m_body : ByteString();
    }

    ByteString ApacheRequest::GetQueryString()
    {
        if (m_request->args && std::strlen(m_request->args) > 0)
        {
            return m_request->args;
        } else {
            return ByteString();
        }
    }

    static HttpMethod::Kind parse_method(request_rec* request)
    {
        if (request->header_only)
        {
            return HttpMethod::HEAD;
        }
        switch (request->method_number)
        {
            case M_GET:
                return HttpMethod::GET;
            case M_PUT:
                return HttpMethod::PUT;
            case M_POST:
                return HttpMethod::POST;
            case M_DELETE:
                return HttpMethod::DELETE;
            case M_CONNECT:
                return HttpMethod::CONNECT;
            case M_OPTIONS:
                return HttpMethod::OPTIONS;
            case M_TRACE:
                return HttpMethod::TRACE;
            case M_PATCH:
                return HttpMethod::PATCH;
            default:
                return HttpMethod::GET;
        }
    }
}
