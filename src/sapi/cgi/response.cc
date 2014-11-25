#include "core/bytestring.h"
#include "sapi/cgi/response.h"

namespace tempearly
{
    CgiResponse::CgiResponse()
        : m_committed(false) {}

    bool CgiResponse::IsCommitted() const
    {
        return m_committed;
    }

    void CgiResponse::Commit()
    {
        if (m_committed)
        {
            return;
        }
        m_committed = true;
        if (GetStatus() != 200)
        {
            std::fprintf(stdout, "Status: %d\r\n", GetStatus());
        }
        for (const Dictionary<String>::Entry* entry = GetHeaders().GetFront(); entry; entry = entry->GetNext())
        {
            std::fprintf(stdout,
                         "%s: %s\r\n",
                         entry->GetName().Encode().c_str(),
                         entry->GetValue().Encode().c_str());
        }
        std::fprintf(stdout, "\r\n");
        std::fflush(stdout);
    }

    void CgiResponse::Write(const ByteString& data)
    {
        const std::size_t length = data.GetLength();

        if (!length)
        {
            return;
        }
        if (!m_committed)
        {
            Commit();
        }
        std::fwrite(static_cast<const void*>(data.GetBytes()), sizeof(byte), length, stdout);
    }
}
