#ifndef TEMPEARLY_NET_SOCKET_H_GUARD
#define TEMPEARLY_NET_SOCKET_H_GUARD

#include <io/stream.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace tempearly
{
    /**
     * Wraps socket into a stream.
     */
    class Socket : public Stream
    {
    public:
        /**
         * Constructs uninitialized socket.
         */
        explicit Socket();

        ~Socket();

        bool IsOpen() const;

        bool IsReadable() const;

        bool IsWritable() const;

        void Close();

        bool DirectRead(byte* buffer, std::size_t size, std::size_t& read);

        bool DirectWrite(const byte* data, std::size_t size);

        bool Create(int port, int type, const String& host);

        bool Bind();

        bool Listen(int max_connections);

        Handle<Socket> Accept();

    private:
        int m_handle;
        sockaddr_in m_address;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Socket);
    };
}

#endif /* !TEMPEARLY_NET_SOCKET_H_GUARD */
