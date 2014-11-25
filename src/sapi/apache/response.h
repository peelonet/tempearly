#ifndef TEMPEARLY_SAPI_APACHE_RESPONSE_H_GUARD
#define TEMPEARLY_SAPI_APACHE_RESPONSE_H_GUARD

#include <httpd.h>

#include "sapi/response.h"

namespace tempearly
{
    class ApacheResponse : public Response
    {
    public:
        explicit ApacheResponse(request_rec* request);

        bool IsCommitted() const;

        void Commit();

        void Write(const ByteString& data);

    private:
        request_rec* m_request;
        bool m_committed;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ApacheResponse);
    };
}

#endif /* !TEMPEARLY_SAPI_APACHE_RESPONSE_H_GUARD */
