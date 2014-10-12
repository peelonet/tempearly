#ifndef TEMPEARLY_SAPI_HTTPD_RESPONSE_H_GUARD
#define TEMPEARLY_SAPI_HTTPD_RESPONSE_H_GUARD

#include "sapi/response.h"

namespace tempearly
{
    class Socket;

    class HttpServerResponse : public Response
    {
    public:
        explicit HttpServerResponse(const Handle<Socket>& socket);

        bool IsCommitted() const;

        void Commit();

        void Write(std::size_t size, const char* data);

        void SendException(const Handle<ExceptionObject>& exception);

        void Mark();

    private:
        Socket* m_socket;
        bool m_committed;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(HttpServerResponse);
    };
}

#endif /* !TEMPEARLY_SAPI_HTTPD_RESPONSE_H_GUARD */
