#include "core/bytestring.h"
#include "sapi/repl/response.h"

namespace tempearly
{
    ReplResponse::ReplResponse() {}

    bool ReplResponse::IsCommitted() const
    {
        return false;
    }

    void ReplResponse::Commit() {}

    void ReplResponse::Write(const ByteString& data)
    {
        std::fwrite(
            static_cast<const void*>(data.GetBytes()),
            sizeof(byte),
            data.GetLength(),
            stdout
        );
    }
}
