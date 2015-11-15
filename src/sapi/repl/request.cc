#include "core/bytestring.h"
#include "sapi/repl/request.h"

namespace tempearly
{
    ReplRequest::ReplRequest() {}

    HttpMethod::Kind ReplRequest::GetMethod() const
    {
        return HttpMethod::GET;
    }

    String ReplRequest::GetPath() const
    {
        return "/";
    }

    bool ReplRequest::IsSecure() const
    {
        return false;
    }

    bool ReplRequest::IsAjax() const
    {
        return false;
    }

    String ReplRequest::GetContentType() const
    {
        return String();
    }

    std::size_t ReplRequest::GetContentLength() const
    {
        return 0;
    }

    ByteString ReplRequest::GetBody()
    {
        return ByteString();
    }

    ByteString ReplRequest::GetQueryString()
    {
        return ByteString();
    }
}
