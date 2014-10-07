#include <cerrno>
#include <cstring>
#include <cstdarg>

#include "core/bytestring.h"
#include "socket.h"

namespace tempearly
{
    Socket::Socket()
        : m_fd(-1) {}

    Socket::~Socket()
    {
        Close();
    }

    bool Socket::Create(int port, int type, const String& host)
    {
        Close();
        if ((m_fd = ::socket(AF_INET, type, 0)) < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }

        // If bindable socket is being created, set the address:port reusable.
        if (port || type == SOCK_STREAM)
        {
            int value = 1;

            ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&value), sizeof(value));
        }

        std::memset(static_cast<void*>(&m_address), 0, sizeof(m_address));
        m_address.sin_family = AF_INET;
        if (host.IsEmpty())
        {
            m_address.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            m_address.sin_addr.s_addr = inet_addr(host.Encode().c_str());
        }
        m_address.sin_port = htons(port);
        if (port || type == SOCK_DGRAM || !host.IsEmpty())
        {
            return Bind();
        }

        return true;
    }

    bool Socket::Bind()
    {
        if (::bind(m_fd, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address)) < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }

        return true;
    }

    bool Socket::Close()
    {
        int retval = 0;

        if (m_fd >= 0)
        {
            retval = ::close(m_fd);
        }
        m_fd = -1;

        return retval >= 0;
    }

    bool Socket::Listen(int max_connections)
    {
        if (::listen(m_fd, max_connections) < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }

        return true;
    }

    Handle<Socket> Socket::Accept()
    {
        Handle<Socket> socket;
        sockaddr_in address;
        socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
        int fd;

ACCEPT_AGAIN:
        if ((fd = ::accept(m_fd, reinterpret_cast<sockaddr*>(&address), &len)) < 0)
        {
            if (errno == EINTR)
            {
                goto ACCEPT_AGAIN;
            }

            return Handle<Socket>();
        }
        socket = new Socket();
        socket->m_fd = fd;

        return socket;
    }

    bool Socket::Receive(byte* buffer, std::size_t size, std::size_t& read)
    {
        int length;

        std::memset(buffer, 0, size);
        if ((length = ::recv(m_fd, buffer, sizeof(char) * size, 0)) < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }
        read = static_cast<std::size_t>(length);

        return true;
    }

    bool Socket::Send(const char* data, std::size_t size)
    {
        int length = ::send(m_fd, data, size, 0);

        if (length < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }

        return true;
    }

    void Socket::Printf(const char* format, ...)
    {
        char buffer[1024];
        int length;
        va_list ap;

        va_start(ap, format);
        length = vsnprintf(buffer, sizeof(buffer), format, ap);
        va_end(ap);
        Send(buffer, length);
    }
}
