#ifndef TEMPEARLY_SAPI_HTTPD_HTTPD_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_HTTPD_HTTPD_REQUEST_H_GUARD

#include "core/dictionary.h"
#include "sapi/request.h"

namespace tempearly
{
    class Socket;

    class HttpServerRequest : public Request
    {
    public:
        explicit HttpServerRequest(const Handle<Socket>& socket,
                                   const String& method,
                                   const String& query_string,
                                   const Dictionary<String>& headers,
                                   const byte* data,
                                   std::size_t data_size);

        String GetMethod() const;

        bool HasParameter(const String& id) const;

        bool GetParameter(const String& id, String& slot) const;

        bool HasHeader(const String& id) const;

        bool GetHeader(const String& id, String& slot) const;

        String GetContentType() const;

        std::size_t GetContentLength() const;

        void Mark();

    private:
        Socket* m_socket;
        String m_method;
        Dictionary<String> m_headers;
        Dictionary<std::vector<String> > m_parameters;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(HttpServerRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_HTTPD_HTTPD_REQUEST_H_GUARD */
