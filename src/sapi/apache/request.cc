#include <cstring>

#include "utils.h"
#include "sapi/apache/request.h"

namespace tempearly
{
    static HttpMethod::Kind parse_method(request_rec*);
    static void parse_post(request_rec*, Dictionary<Vector<String> >&);

    ApacheRequest::ApacheRequest(request_rec* request)
        : m_request(request)
        , m_method(parse_method(request))
        , m_parameters_read(false)
    {
        ReadParameters();
    }

    HttpMethod::Kind ApacheRequest::GetMethod() const
    {
        return m_method;
    }

    String ApacheRequest::GetPath() const
    {
        return m_request->uri;
    }

    bool ApacheRequest::IsAjax() const
    {
        const char* value = apr_table_get(m_request->headers_in, "X-Requested-With");

        if (value)
        {
            return String(value).EqualsIgnoreCase("xmlhttprequest");
        } else {
            return false;
        }
    }

    bool ApacheRequest::HasParameter(const String& id) const
    {
        const Dictionary<Vector<String> >::Entry* e = m_parameters.Find(id);

        return e && !e->GetValue().IsEmpty();
    }

    bool ApacheRequest::GetParameter(const String& id, String& value) const
    {
        const Dictionary<Vector<String> >::Entry* entry = m_parameters.Find(id);

        if (entry)
        {
            const Vector<String>& values = entry->GetValue();

            if (!values.IsEmpty())
            {
                value = values.GetFront();

                return true;
            }
        }

        return false;
    }

    void ApacheRequest::ReadParameters()
    {
        if (m_parameters_read)
        {
            return;
        }
        m_parameters_read = true;
        if (m_request->args && std::strlen(m_request->args) > 0)
        {
            Utils::ParseQueryString(m_request->args, m_parameters);
        }
        if (m_method == HttpMethod::POST)
        {
            parse_post(m_request, m_parameters);
        }
    }

    static HttpMethod::Kind parse_method(request_rec* request)
    {
        if (request->header_only)
        {
            return HttpMethod::HEAD;
        }
        switch (request->method_number)
        {
            case M_GET:
                return HttpMethod::GET;
            case M_PUT:
                return HttpMethod::PUT;
            case M_POST:
                return HttpMethod::POST;
            case M_DELETE:
                return HttpMethod::DELETE;
            case M_CONNECT:
                return HttpMethod::CONNECT;
            case M_OPTIONS:
                return HttpMethod::OPTIONS;
            case M_TRACE:
                return HttpMethod::TRACE;
            case M_PATCH:
                return HttpMethod::PATCH;
            default:
                return HttpMethod::GET;
        }
    }

    static inline void add_param(const String& key, const String& value, Dictionary<Vector<String> >& parameters)
    {
        Dictionary<Vector<String> >::Entry* entry = parameters.Find(key);

        if (entry)
        {
            entry->GetValue().PushBack(value);
        } else {
            parameters.Insert(key, Vector<String>(1, value));
        }
    }

    static void parse_post(request_rec* request, Dictionary<Vector<String> >& parameters)
    {
        const char* ct = apr_table_get(request->headers_in, "Content-Type");

        if (ct && !std::strncmp("application/x-www-form-urlencoded", ct, 33))
        {
            apr_array_header_t* pairs = 0;
            int result = ap_parse_form_data(request, 0, &pairs, -1, HUGE_STRING_LEN);

            if (result != OK || !pairs)
            {
                return;
            }
            while (pairs && !apr_is_empty_array(pairs))
            {
                ap_form_pair_t* pair = static_cast<ap_form_pair_t*>(apr_array_pop(pairs));
                apr_off_t length;
                apr_size_t size;
                char* buffer;

                apr_brigade_length(pair->value, 1, &length);
                buffer = static_cast<char*>(apr_palloc(request->pool, length + 1));
                apr_brigade_flatten(pair->value, buffer, &size);
                buffer[size] = 0;
                add_param(pair->name, buffer, parameters);
            }
        }
        // TODO: parse multipart request
    }
}
