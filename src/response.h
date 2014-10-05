#ifndef TEMPEARLY_RESPONSE_H_GUARD
#define TEMPEARLY_RESPONSE_H_GUARD

#include "dictionary.h"

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
         * Retrieves an response header with the given name and returns it's
         * value if such exists. Otherwise an empty string is returned.
         *
         * \param name Name of the response header to retrieve
         */
        String GetHeader(const String& name) const;

        /**
         * Sets value of an response value. Existing headers with same name are
         * overridden.
         *
         * \param name  Name of the header
         * \param value Value of the header
         */
        void SetHeader(const String& name, const String& value);

        void AddHeader(const String& name, const String& value);

        // TODO: void RemoveHeader(const String& name);

        virtual void Write(const String& string);

        virtual void Write(std::size_t size, const char* data) = 0;

        virtual void SendException(const Handle<ExceptionObject>& exception) = 0;

    private:
        /** Status code of the response. */
        int m_status;
        Dictionary<String> m_headers;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Response);
    };
}

#endif /* !TEMPEARLY_RESPONSE_H_GUARD */
