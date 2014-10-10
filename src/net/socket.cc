#include <cerrno>
#include <cstring>
#include <cstdarg>

#include "core/bytestring.h"
#include "net/socket.h"

namespace tempearly
{
    Socket::Socket()
        : m_handle(-1) {}

    Socket::~Socket()
    {
        Close();
    }

    bool Socket::Create(int port, int type, const String& host)
    {
        Close();
        if ((m_handle = ::socket(AF_INET, type, 0)) < 0)
        {
            m_error_message = std::strerror(errno);
            errno = 0;

            return false;
        }

        // If bindable socket is being created, set the address:port reusable.
        if (port || type == SOCK_STREAM)
        {
            int value = 1;

            ::setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&value), sizeof(value));
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
        if (m_handle >= 0)
        {
            if (::bind(m_handle, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address)) < 0)
            {
                m_error_message = std::strerror(errno);
                errno = 0;

                return false;
            }

            return true;
        }
        m_error_message = "Socket has not been initialized";

        return false;
    }

    bool Socket::Close()
    {
        if (m_handle >= 0)
        {
            int retval = 0;

            if (m_handle >= 0)
            {
                retval = ::close(m_handle);
            }
            m_handle = -1;

            return retval >= 0;
        }
        m_error_message = "Socket is not open";

        return false;
    }

    bool Socket::Listen(int max_connections)
    {
        if (m_handle >= 0)
        {
            if (::listen(m_handle, max_connections) < 0)
            {
                m_error_message = std::strerror(errno);
                errno = 0;

                return false;
            }

            return true;
        }
        m_error_message = "Socket has not been initialized";

        return false;
    }

    Handle<Socket> Socket::Accept()
    {
        if (m_handle >= 0)
        {
            Handle<Socket> socket;
            sockaddr_in address;
            socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_in));
            int client_handle;

    ACCEPT_AGAIN:
            if ((client_handle = ::accept(m_handle, reinterpret_cast<sockaddr*>(&address), &len)) < 0)
            {
                if (errno == EINTR)
                {
                    goto ACCEPT_AGAIN;
                }
                m_error_message = std::strerror(errno);
                errno = 0;

                return Handle<Socket>();
            }
            socket = new Socket();
            socket->m_handle = client_handle;

            return socket;
        }
        m_error_message = "Socket is not open";

        return Handle<Socket>();
    }

    bool Socket::Receive(byte* buffer, std::size_t size, std::size_t& read)
    {
        if (m_handle >= 0)
        {
            int length;

            std::memset(buffer, 0, size);
            if ((length = ::recv(m_handle, buffer, sizeof(char) * size, 0)) < 0)
            {
                m_error_message = std::strerror(errno);
                errno = 0;

                return false;
            }
            read = static_cast<std::size_t>(length);

            return true;
        }
        m_error_message = "Socket is not open";

        return false;
    }

    bool Socket::Send(const ByteString& data)
    {
        return Send(data.GetBytes(), data.GetLength());
    }

    bool Socket::Send(const byte* data, std::size_t size)
    {
        if (m_handle >= 0)
        {
            int length = ::send(m_handle, data, size, 0);

            if (length < 0)
            {
                m_error_message = std::strerror(errno);
                errno = 0;

                return false;
            }

            return true;
        }
        m_error_message = "Socket is not open";

        return false;
    }

    bool Socket::Send(const char* data, std::size_t size)
    {
        return Send(reinterpret_cast<const byte*>(data), size);
    }

    void Socket::Printf(const char* format, ...)
    {
        char buffer[1024];
        int length;
        va_list ap;

        va_start(ap, format);
        length = vsnprintf(buffer, sizeof(buffer), format, ap);
        va_end(ap);
        Send(reinterpret_cast<byte*>(buffer), length);
    }
}
