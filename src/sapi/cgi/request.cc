#if defined(_WIN32)
# include <windows.h>
#else
# include <strings.h>
#endif

#include "utils.h"
#include "sapi/cgi/request.h"

#if !defined(BUFSIZ)
# define BUFSIZ 1024
#endif

namespace tempearly
{
    static bool cgi_getenv_bin(const char*, ByteString&);
    static bool cgi_getenv_str(const char*, String&);
    template< typename T >
    static void cgi_getenv_int(const char*, T&);
    static void cgi_getenv_bool(const char*, bool&);

    CgiRequest::CgiRequest()
        : m_server_port(-1)
        , m_content_length(0)
        , m_using_https(false)
        , m_body(nullptr)
    {
        ReadEnvironmentVariables();
        ReadBody();
    }

    CgiRequest::~CgiRequest()
    {
        if (m_body)
        {
            delete m_body;
        }
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

    bool CgiRequest::IsSecure() const
    {
        char* value = std::getenv("HTTPS");

        if (value && *value)
        {
            return !std::strcmp(value, "on");
        } else {
            return false;
        }
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

    String CgiRequest::GetContentType() const
    {
        return m_content_type;
    }

    std::size_t CgiRequest::GetContentLength() const
    {
        return m_content_length;
    }

    ByteString CgiRequest::GetBody()
    {
        return m_body ? *m_body : ByteString();
    }

    ByteString CgiRequest::GetQueryString()
    {
        return m_query_string;
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
        cgi_getenv_bin("QUERY_STRING", m_query_string);
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

    void CgiRequest::ReadBody()
    {
        if (m_content_length > 0)
        {
            Vector<byte> body;
            byte buffer[BUFSIZ];
            std::size_t remain = m_content_length;

            body.Reserve(remain);
            // On Win32, use binary read to avoid CRLF conversion.
#if defined(_WIN32)
# if defined(__BORLANDC__)
            setmode(_fileno(stdin), O_BINARY);
# else
            setmode(_fileno(stdin), _O_BINARY);
# endif
#endif
            while (remain > 0)
            {
                std::size_t read = std::fread(static_cast<void*>(buffer), sizeof(byte), BUFSIZ, stdin);

                if (!read)
                {
                    break;
                }
                body.PushBack(buffer, read);
                remain -= read;
            }
            if (!body.IsEmpty())
            {
                m_body = new ByteString(body.GetData(), body.GetSize());
            }
        }
    }

    static bool cgi_getenv_bin(const char* name, ByteString& slot)
    {
        char* value = std::getenv(name);

        if (value && *value)
        {
            slot = value;

            return true;
        }

        return false;
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

            if (String(value).ParseInt(number, 10))
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
