#ifndef TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD

#include <httpd.h>

#include "sapi/request.h"

namespace tempearly
{
    class ApacheRequest : public Request
    {
    public:
        explicit ApacheRequest(request_rec* request);

        ~ApacheRequest();

        HttpMethod::Kind GetMethod() const;

        String GetPath() const;

        bool IsSecure() const;

        bool IsAjax() const;

        String GetContentType() const;

        std::size_t GetContentLength() const;

        ByteString GetBody();

        ByteString GetQueryString();

    private:
        request_rec* m_request;
        const HttpMethod::Kind m_method;
        ByteString* m_body;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ApacheRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD */
