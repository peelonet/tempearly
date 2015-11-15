#ifndef TEMPEARLY_SAPI_REPL_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_REPL_REQUEST_H_GUARD

#include "sapi/request.h"

namespace tempearly
{
    class ReplRequest : public Request
    {
    public:
        explicit ReplRequest();

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

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ReplRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_REPL_REQUEST_H_GUARD */
