#if defined(_WIN32)
# include <windows.h>
#else
# include <strings.h>
#endif

#include "utils.h"
#include "sapi/cgi/request.h"

namespace tempearly
{
    static bool cgi_getenv_str(const char*, String&);

    template< typename T >
    static void cgi_getenv_int(const char*, T&);

    static void cgi_getenv_bool(const char*, bool&);

    CgiRequest::CgiRequest()
        : m_parameters_read(false)
        , m_server_port(-1)
        , m_content_length(0)
        , m_using_https(false)
    {
        ReadEnvironmentVariables();
        ReadParameters();

        // On Win32, use binary read to avoid CRLF conversion.
#if defined(_WIN32)
# if defined(__BORLANDC__)
        setmode(_fileno(stdin), O_BINARY);
# else
        setmode(_fileno(stdin), _O_BINARY);
# endif
#endif
    }

    HttpMethod::Kind CgiRequest::GetMethod() const
    {
        HttpMethod::Kind method;

        if (HttpMethod::Parse(m_method, method))
        {
            return method;
        } else {
            return HttpMethod::GET;
        }
    }

    String CgiRequest::GetPath() const
    {
        return m_path;
    }

    bool CgiRequest::IsAjax() const
    {
        String value;

        if (cgi_getenv_str("HTTP_X_REQUESTED_WITH", value))
        {
            return value.EqualsIgnoreCase("xmlhttprequest");
        } else {
            return false;
        }
    }

    bool CgiRequest::HasParameter(const String& id) const
    {
        const Dictionary<Vector<String> >::Entry* e = m_parameters.Find(id);

        return e && !e->GetValue().IsEmpty();
    }

    bool CgiRequest::GetParameter(const String& id, String& value) const
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

    void CgiRequest::ReadEnvironmentVariables()
    {
        // It's propably not a good idea performance wise to read all the
        // environment variables during construction of the request. Perhaps
        // they should be read individually when needed and cached into the
        // request.
        cgi_getenv_str("REQUEST_METHOD", m_method);
        cgi_getenv_str("REQUEST_URI", m_path);
        cgi_getenv_str("SERVER_SOFTWARE", m_server_software);
        cgi_getenv_str("SERVER_NAME", m_server_name);
        cgi_getenv_str("GATEWAY_INTERFACE", m_gateway_interface);
        cgi_getenv_str("SERVER_PROTOCOL", m_server_protocol);
        cgi_getenv_int("SERVER_PORT", m_server_port);
        cgi_getenv_str("PATH_INFO", m_path_info);
        cgi_getenv_str("PATH_TRANSLATED", m_path_translated);
        cgi_getenv_str("SCRIPT_NAME", m_script_name);
        cgi_getenv_str("QUERY_STRING", m_query_string);
        cgi_getenv_str("REMOTE_HOST", m_remote_host);
        cgi_getenv_str("REMOTE_ADDR", m_remote_addr);
        cgi_getenv_str("AUTH_TYPE", m_auth_type);
        cgi_getenv_str("REMOTE_USER", m_remote_user);
        cgi_getenv_str("REMOTE_IDENT", m_remote_ident);
        cgi_getenv_str("CONTENT_TYPE", m_content_type);
        cgi_getenv_int("CONTENT_LENGTH", m_content_length);
        cgi_getenv_str("HTTP_ACCEPT", m_accept);
        cgi_getenv_str("HTTP_USER_AGENT", m_user_agent);
        cgi_getenv_str("REDIRECT_REQUEST", m_redirect_request);
        cgi_getenv_str("REDIRECT_URL", m_redirect_url);
        cgi_getenv_str("REDIRECT_STATUS", m_redirect_status);
        cgi_getenv_str("REFERRER", m_referrer);
        cgi_getenv_str("HTTP_COOKIE", m_cookie);
        cgi_getenv_bool("HTTPS", m_using_https);
    }

    void CgiRequest::ReadParameters()
    {
        if (m_parameters_read)
        {
            return;
        }
        m_parameters_read = true;
        if (!m_query_string.IsEmpty())
        {
            Utils::ParseQueryString(m_query_string, m_parameters);
        }
        // TODO: Process multipart requests
        if (m_method == "POST"
            && m_content_type == "application/x-www-form-urlencoded"
            && m_content_length > 0)
        {
            char* buffer = new char[m_content_length];
            std::size_t read = std::fread(static_cast<void*>(buffer),
                                          sizeof(char),
                                          m_content_length,
                                          stdin);

            if (read > 0)
            {
                Utils::ParseQueryString(buffer, m_parameters);
            }
            delete[] buffer;
        }
    }

    static bool cgi_getenv_str(const char* name, String& slot)
    {
        char* value = std::getenv(name);

        if (value && *value)
        {
            slot = value;

            return true;
        }

        return false;
    }

    template< typename T >
    static void cgi_getenv_int(const char* name, T& slot)
    {
        char* value = std::getenv(name);

        if (value && *value)
        {
            i64 number;

            if (Utils::ParseInt(value, number, 10))
            {
                slot = static_cast<T>(number);
            }
        }
    }

    static void cgi_getenv_bool(const char* name, bool& slot)
    {
        char* value = std::getenv(name);

        if (value && *value)
        {
#if defined(_WIN32)
            slot = !stricmp(value, "on")
#else
            slot = !strcasecmp(value, "on");
#endif
        }
    }
}
