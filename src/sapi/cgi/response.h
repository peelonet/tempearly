#ifndef TEMPEARLY_SAPI_CGI_RESPONSE_H_GUARD
#define TEMPEARLY_SAPI_CGI_RESPONSE_H_GUARD

#include "sapi/response.h"

namespace tempearly
{
    class CgiResponse : public Response
    {
    public:
        explicit CgiResponse();

        bool IsCommitted() const;

        void Commit();

        void Write(const ByteString& data);

    private:
        /** Whether the response has been committed or not. */
        bool m_committed;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CgiResponse);
    };
}

#endif /* !TEMPEARLY_SAPI_CGI_RESPONSE_H_GUARD */
