#ifndef TEMPERALY_SAPI_REQUEST_H_GUARD
#define TEMPERALY_SAPI_REQUEST_H_GUARD

#include "memory.h"

namespace tempearly
{
    class Request : public CountedObject
    {
    public:
        explicit Request();

        virtual ~Request();

        /**
         * Returns request method (GET, POST, etc.).
         */
        virtual String GetMethod() const = 0;

        virtual bool HasParameter(const String& id) const = 0;

        virtual bool GetParameter(const String& id, String& value) const = 0;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Request);
    };
}

#endif /* !TEMPERALY_SAPI_REQUEST_H_GUARD */
