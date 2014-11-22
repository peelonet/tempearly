#ifndef TEMPEARLY_SAPI_HTTPD_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_HTTPD_REQUEST_H_GUARD

#include "sapi/request.h"

namespace tempearly
{
    class Socket;

    class HttpServerRequest : public Request
    {
    public:
        explicit HttpServerRequest(const Handle<Socket>& socket,
                                   HttpMethod::Kind method,
                                   const String& path,
                                   const ByteString& query_string,
                                   const Dictionary<String>& headers,
                                   const byte* body,
                                   std::size_t body_size);

        ~HttpServerRequest();

        HttpMethod::Kind GetMethod() const;

        String GetPath() const;

        bool IsSecure() const;

        bool IsAjax() const;

        String GetContentType() const;

        std::size_t GetContentLength() const;

        ByteString GetBody();

        ByteString GetQueryString();

        bool HasHeader(const String& id) const;

        bool GetHeader(const String& id, String& slot) const;

        void Mark();

    private:
        Socket* m_socket;
        const HttpMethod::Kind m_method;
        const String m_path;
        const ByteString m_query_string;
        const Dictionary<String> m_headers;
        ByteString* m_body;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(HttpServerRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_HTTPD_REQUEST_H_GUARD */
