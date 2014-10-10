#ifndef TEMPERALY_NET_SOCKET_H_GUARD
#define TEMPEARLY_NET_SOCKET_H_GUARD

#include "core/string.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace tempearly
{
    /**
     * Unix socket wrapper base class.
     */
    class Socket : public CountedObject
    {
    public:
        /**
         * Constructs uninitialized socket.
         */
        explicit Socket();

        virtual ~Socket();

        inline bool HasError() const
        {
            return !m_error_message.IsEmpty();
        }

        inline const String& GetErrorMessage() const
        {
            return m_error_message;
        }

        bool Create(int port, int type, const String& host);

        bool Bind();

        bool Close();

        bool Listen(int max_connections);

        Handle<Socket> Accept();

        bool Receive(byte* buffer, std::size_t size, std::size_t& read);

        bool Send(const ByteString& data);

        bool Send(const byte* data, std::size_t size);

        bool Send(const char* data, std::size_t size);

        void Printf(const char* format, ...);

    private:
        int m_handle;
        sockaddr_in m_address;
        String m_error_message;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Socket);
    };
}

#endif /* !TEMPEARLY_NET_SOCKET_H_GUARD */
