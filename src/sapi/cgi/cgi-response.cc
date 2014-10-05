#include "api/exception.h"
#include "sapi/cgi/cgi-response.h"

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
        for (const Dictionary<String>::Entry* e = GetHeaders().GetFront();
             e;
             e = e->next)
        {
            std::fprintf(stdout,
                         "%s: %s\r\n",
                         e->id.c_str(),
                         e->value.c_str());
        }
        std::fprintf(stdout, "\r\n");
        std::fflush(stdout);
    }

    void CgiResponse::Write(std::size_t size, const char* data)
    {
        if (!m_committed)
        {
            Commit();
        }
        std::fwrite(static_cast<const void*>(data),
                    sizeof(char),
                    size,
                    stdout);
    }

    void CgiResponse::SendException(const Handle<ExceptionObject>& exception)
    {
        if (!exception)
        {
            return; // Just in case.
        }
        if (m_committed)
        {
            std::fprintf(
                stdout,
                "<p><strong>ERROR:</strong> %s</p>",
                exception->GetMessage().c_str()
            );
        } else {
            m_committed = true;
            std::fprintf(
                stdout,
                "Status: 500\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n"
            );
            std::fprintf(
                stdout,
                "ERROR:\n%s\n",
                exception->GetMessage().c_str()
            );
            std::fflush(stdout);
        }
    }
}
