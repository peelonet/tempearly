#include "api/exception.h"
#include "core/bytestring.h"
#include "net/socket.h"
#include "sapi/httpd/response.h"

namespace tempearly
{
    HttpServerResponse::HttpServerResponse(const Handle<Socket>& socket)
        : m_socket(socket.Get())
        , m_committed(false) {}

    bool HttpServerResponse::IsCommitted() const
    {
        return m_committed;
    }

    void HttpServerResponse::Commit()
    {
        if (m_committed)
        {
            return;
        }
        m_committed = true;
        m_socket->Printf("HTTP/1.0 %d\r\n", GetStatus()); // TODO: status message
        for (const Dictionary<String>::Entry* entry = GetHeaders().GetFront(); entry; entry = entry->GetNext())
        {
            m_socket->Printf("%s: %s\r\n",
                             entry->GetName().Encode().c_str(),
                             entry->GetValue().Encode().c_str());
        }
        m_socket->Send("\r\n", 2);
    }

    void HttpServerResponse::Write(std::size_t size, const char* data)
    {
        if (!m_committed)
        {
            Commit();
        }
        m_socket->Send(data, size);
    }

    void HttpServerResponse::SendException(const Handle<ExceptionObject>& exception)
    {
        if (!exception)
        {
            return; // Just in case.
        }
        if (m_committed)
        {
            m_socket->Printf(
                "<p><strong>ERROR:</strong> %s</p>",
                exception->GetMessage().Encode().c_str()
            );
        } else {
            m_committed = true;
            m_socket->Printf("HTTP/1.0 500 Internal Server Error\r\n");
            m_socket->Printf("Content-Type: text/plain; charset=utf-8\r\n\r\n");
            m_socket->Printf("ERROR:\n%s\n",
                             exception->GetMessage().Encode().c_str());
            // TODO: flush socket
        }
    }

    void HttpServerResponse::Mark()
    {
        Response::Mark();
        if (m_socket && !m_socket->IsMarked())
        {
            m_socket->Mark();
        }
    }
}
