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
        m_socket->Write(reinterpret_cast<const byte*>("\r\n"), 2);
    }

    void HttpServerResponse::Write(const ByteString& data)
    {
        if (data.IsEmpty())
        {
            return;
        }
        if (!m_committed)
        {
            Commit();
        }
        m_socket->Write(data);
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
