#ifndef TEMPERALY_SAPI_REQUEST_H_GUARD
#define TEMPERALY_SAPI_REQUEST_H_GUARD

#include "memory.h"
#include "http/method.h"

namespace tempearly
{
    class Request : public CountedObject
    {
    public:
        explicit Request();

        virtual ~Request();

        /**
         * Returns request method (GET, POST, etc.).
         *
         * Same as the value of the CGI variable REQUEST_METHOD.
         */
        virtual HttpMethod::Kind GetMethod() const = 0;

        /**
         * Returns the requested path.
         *
         * Same as the value of the CGI variable REQUEST_URI.
         */
        virtual String GetPath() const = 0;

        /**
         * Attempts to find out whether the request was made through a secure
         * channel such as HTTPS.
         */
        virtual bool IsSecure() const = 0;

        /**
         * Attempts to find out whether the request was made with
         * XMLHttpRequest or not.
         */
        virtual bool IsAjax() const = 0;

        virtual bool HasParameter(const String& id) const = 0;

        virtual bool GetParameter(const String& id, String& value) const = 0;

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Request);
    };
}

#endif /* !TEMPERALY_SAPI_REQUEST_H_GUARD */
