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
        if (m_handle >= 0)
        {
            ::close(m_handle);
            m_handle = -1;
        }
    }

    bool Socket::IsOpen() const
    {
        return m_handle >= 0;
    }

    bool Socket::IsReadable() const
    {
        return IsOpen();
    }

    bool Socket::IsWritable() const
    {
        return IsOpen();
    }

    void Socket::Close()
    {
        if (m_handle >= 0)
        {
            ::close(m_handle);
            m_handle = -1;
        }
    }

    bool Socket::ReadData(byte* buffer, std::size_t size, std::size_t& read)
    {
        if (m_handle >= 0)
        {
            int length;

            std::memset(buffer, 0, size);
            if ((length = ::recv(m_handle, buffer, sizeof(byte) * size, 0)) < 0)
            {
                SetErrorMessage(std::strerror(errno));
                errno = 0;

                return false;
            }
            read = static_cast<std::size_t>(length);

            return true;
        }
        SetErrorMessage("Socket is not open");

        return false;
    }

    bool Socket::WriteData(const byte* data, std::size_t size)
    {
        if (m_handle >= 0)
        {
            int length = ::send(m_handle, data, size, 0);

            if (length < 0)
            {
                SetErrorMessage(std::strerror(errno));
                errno = 0;

                return false;
            }

            return true;
        }
        SetErrorMessage("Socket is not open");

        return false;
    }

    bool Socket::Create(int port, int type, const String& host)
    {
        Close();
        if ((m_handle = ::socket(AF_INET, type, 0)) < 0)
        {
            SetErrorMessage(std::strerror(errno));
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
                SetErrorMessage(std::strerror(errno));
                errno = 0;

                return false;
            }

            return true;
        }
        SetErrorMessage("Socket has not been initialized");

        return false;
    }

    bool Socket::Listen(int max_connections)
    {
        if (m_handle >= 0)
        {
            if (::listen(m_handle, max_connections) < 0)
            {
                SetErrorMessage(std::strerror(errno));
                errno = 0;

                return false;
            }

            return true;
        }
        SetErrorMessage("Socket has not been initialized");

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
                SetErrorMessage(std::strerror(errno));
                errno = 0;

                return Handle<Socket>();
            }
            socket = new Socket();
            socket->m_handle = client_handle;

            return socket;
        }
        SetErrorMessage("Socket is not open");

        return Handle<Socket>();
    }
}
