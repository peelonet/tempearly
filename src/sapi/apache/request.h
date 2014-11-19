#ifndef TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD

#include <httpd.h>

#include "core/dictionary.h"
#include "sapi/request.h"

namespace tempearly
{
    class ApacheRequest : public Request
    {
    public:
        explicit ApacheRequest(request_rec* request);

        HttpMethod::Kind GetMethod() const;

        String GetPath() const;

        bool IsSecure() const;

        bool IsAjax() const;

        bool HasParameter(const String& id) const;

        bool GetParameter(const String& id, String& value) const;

    private:
        void ReadParameters();

    private:
        request_rec* m_request;
        const HttpMethod::Kind m_method;
        Dictionary<Vector<String> > m_parameters;
        bool m_parameters_read;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ApacheRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_APACHE_REQUEST_H_GUARD */
