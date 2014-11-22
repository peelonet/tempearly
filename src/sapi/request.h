#ifndef TEMPERALY_SAPI_REQUEST_H_GUARD
#define TEMPERALY_SAPI_REQUEST_H_GUARD

#include "core/dictionary.h"
#include "core/vector.h"
#include "http/method.h"

namespace tempearly
{
    class Request : public CountedObject
    {
    public:
        typedef Dictionary<Vector<String> > ParameterMap;

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

        /**
         * Returns value of the Content-Type header sent by the client or an
         * empty string if the client did not send that specific header.
         */
        virtual String GetContentType() const = 0;

        /**
         * Returns value of the Content-Length header sent by the client or 0
         * if the client did not send that specific header or it's value is
         * indeed a zero.
         */
        virtual std::size_t GetContentLength() const = 0;

        /**
         * Returns body of the request as binary string or empty binary string
         * if the client did not send anything with the request.
         */
        virtual ByteString GetBody() = 0;

        /**
         * Returns the query string extracted from the URL or empty byte string
         * if the client did not send a request containing query string.
         */
        virtual ByteString GetQueryString() = 0;

        /**
         * Returns names of all request parameters included in this request.
         */
        Vector<String> GetParameterNames();

        bool HasParameter(const String& id);

        bool GetParameter(const String& id, String& value);

        bool GetAllParameters(const String& id, Vector<String>& values);

    private:
        void ParseParameters();

    private:
        ParameterMap m_parameters;
        bool m_parameters_parsed;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Request);
    };
}

#endif /* !TEMPERALY_SAPI_REQUEST_H_GUARD */
