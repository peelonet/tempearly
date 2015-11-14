#ifndef TEMPEARLY_SAPI_REPL_RESPONSE_H_GUARD
#define TEMPEARLY_SAPI_REPL_RESPONSE_H_GUARD

#include "sapi/response.h"

namespace tempearly
{
    class ReplResponse : public Response
    {
    public:
        explicit ReplResponse();

        bool IsCommitted() const;

        void Commit();

        void Write(const ByteString& data);

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ReplResponse);
    };
}

#endif /* !TEMPEARLY_SAPI_REPL_RESPONSE_H_GUARD */
