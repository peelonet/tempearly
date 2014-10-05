#ifndef TEMPERALY_REQUEST_H_GUARD
#define TEMPERALY_REQUEST_H_GUARD

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

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Request);
    };
}

#endif /* !TEMPERALY_REQUEST_H_GUARD */
