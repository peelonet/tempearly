#ifndef TEMPEARLY_SAPI_RESPONSE_H_GUARD
#define TEMPEARLY_SAPI_RESPONSE_H_GUARD

#include "core/dictionary.h"

namespace tempearly
{
    class Response : public CountedObject
    {
    public:
        explicit Response();

        virtual ~Response();

        /**
         * Returns true if the response has been committed.
         */
        virtual bool IsCommitted() const = 0;

        /**
         * Sends all the headers. If the headers have already been sent, does
         * nothing.
         */
        virtual void Commit() = 0;

        /**
         * Returns the current HTTP status of the response.
         */
        inline int GetStatus() const
        {
            return m_status;
        }

        /**
         * Sets the HTTP status of the response. If headers have already been
         * sent, this method does nothing.
         */
        inline void SetStatus(int status)
        {
            m_status = status;
        }

        inline const Dictionary<String>& GetHeaders() const
        {
            return m_headers;
        }

        /**
         * Returns true if response has header with given name.
         *
         * \param name Name of the response header to test existance of
         */
        bool HasHeader(const String& name) const;

        /**
         * Retrieves an response header with the given name and assigns it's
         * value to the given slot.
         *
         * \param name Name of the response header to retrieve
         * \param slot Where value of the header is assigned to
         * \return     A boolean flag indicating whether header with given
         *             name exists in the response or not
         */
        bool GetHeader(const String& name, String& slot) const;

        /**
         * Sets value of an response header. Existing headers with same name
         * are overridden.
         *
         * \param name  Name of the header
         * \param value Value of the header
         */
        void SetHeader(const String& name, const String& value);

        void AddHeader(const String& name, const String& value);

        /**
         * Removes an response header from the response.
         *
         * \param name Name of the header to remove
         */
        void RemoveHeader(const String& name);

        /**
         * Sends binary data to the client. Must be overridden by derived
         * classes. If response headers haven't been sent before this method
         * is called, they must be sent to the client before any data is
         * being sent.
         *
         * \param data Binary data which is going to be sent to the client
         */
        virtual void Write(const ByteString& data) = 0;

        /**
         * Sends text data to the client. It will be encoded with UTF-8
         * character encoding and sent to the client. Response headers are
         * also sent to the client before any text will be sent, if the
         * headers haven't already been sent.
         *
         * \param text Text which is going to be sent to the client
         */
        virtual void Write(const String& text);

        void SendException(const Handle<Object>& exception);

    private:
        /** Status code of the response. */
        int m_status;
        Dictionary<String> m_headers;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Response);
    };
}

#endif /* !TEMPEARLY_SAPI_RESPONSE_H_GUARD */
