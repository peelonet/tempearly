#include <http_protocol.h>
#include <apr_strings.h>

#include "api/exception.h"
#include "core/bytestring.h"
#include "sapi/apache/response.h"

namespace tempearly
{
    ApacheResponse::ApacheResponse(request_rec* request)
        : m_request(request)
        , m_committed(false) {}

    bool ApacheResponse::IsCommitted() const
    {
        return m_committed;
    }

    void ApacheResponse::Commit()
    {
        bool content_type_set = false;

        if (m_committed)
        {
            return;
        }
        m_committed = true;
        m_request->status = GetStatus();
        for (const Dictionary<String>::Entry* e = GetHeaders().GetFront(); e; e = e->next)
        {
            if (e->id == "Content-Type")
            {
                ap_set_content_type(m_request, e->value.Encode().c_str());
                content_type_set = true;
            } else {
                apr_table_add(m_request->headers_out, e->id.Encode().c_str(), e->value.Encode().c_str());
            }
        }
        if (!content_type_set)
        {
            ap_set_content_type(m_request, "text/html; charset=utf-8");
        }
    }

    void ApacheResponse::Write(std::size_t size, const char* data)
    {
        if (!m_committed)
        {
            Commit();
        }
        ap_rwrite(data, size, m_request);
    }

    void ApacheResponse::SendException(const Handle<ExceptionObject>& exception)
    {
        if (!exception)
        {
            return; // Just in case
        }
        if (m_committed)
        {
            ap_rprintf(
                m_request,
                "<p><strong>ERROR:</strong> %s</p>",
                exception->GetMessage().Encode().c_str()
            );
        } else {
            m_committed = true;
            ap_set_content_type(m_request, "text/plain");
            ap_rprintf(
                m_request,
                "ERROR:\n%s\n",
                exception->GetMessage().Encode().c_str()
            );
        }
    }
}
