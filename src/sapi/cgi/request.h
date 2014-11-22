#ifndef TEMPEARLY_SAPI_CGI_REQUEST_H_GUARD
#define TEMPEARLY_SAPI_CGI_REQUEST_H_GUARD

#include "core/bytestring.h"
#include "sapi/request.h"

namespace tempearly
{
    class CgiRequest : public Request
    {
    public:
        explicit CgiRequest();

        ~CgiRequest();

        HttpMethod::Kind GetMethod() const;

        String GetPath() const;

        bool IsSecure() const;

        bool IsAjax() const;

        String GetContentType() const;

        std::size_t GetContentLength() const;

        ByteString GetBody();

        ByteString GetQueryString();

    private:
        void ReadEnvironmentVariables();
        void ReadBody();

    private:
        /** Request method ("GET", "POST", etc.). */
        String m_method;
        /** Requested URI. */
        String m_path;
        String m_server_software;
        String m_server_name;
        String m_gateway_interface;
        String m_server_protocol;
        int m_server_port;
        String m_path_info;
        String m_path_translated;
        String m_script_name;
        ByteString m_query_string;
        String m_remote_host;
        String m_remote_addr;
        String m_auth_type;
        String m_remote_user;
        String m_remote_ident;
        String m_content_type;
        std::size_t m_content_length;
        String m_accept;
        String m_user_agent;
        String m_redirect_request;
        String m_redirect_url;
        String m_redirect_status;
        String m_referrer;
        String m_cookie;
        bool m_using_https;
        ByteString* m_body;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CgiRequest);
    };
}

#endif /* !TEMPEARLY_SAPI_CGI_REQUEST_H_GUARD */
