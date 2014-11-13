#ifndef TEMPEARLY_NET_URL_H_GUARD
#define TEMPEARLY_NET_URL_H_GUARD

#include "core/string.h"

namespace tempearly
{
    /**
     * Object representation of an URL.
     */
    class Url
    {
    public:
        /**
         * Default constructor for URL.
         */
        Url(const String& scheme = String(),
            const String& username = String(),
            const String& password = String(),
            const String& hostname = String(),
            int port = 0,
            const String& path = String(),
            const String& query = String(),
            const String& fragment = String());

        /**
         * Copy constructor.
         *
         * \param that Other URL to construct copy of
         */
        Url(const Url& that);

        /**
         * Attempts to decode URL entities from given string.
         *
         * \param input  String to decode
         * \param output Where the resulting decoded string is assigned to
         * \return       A boolean flag indicating whether decoding was
         *               successfull or not
         */
        static bool Decode(const String& input, String& output);

        /**
         * Encodes URL entities from the given string.
         */
        static String Encode(const String& input);

        /**
         * Returns scheme of the URL or empty string if not specified.
         */
        inline const String& GetScheme() const
        {
            return m_scheme;
        }

        /**
         * Returns the authority part of the URL in form of
         * "username:password", or empty string if it's not defined in the
         * URL.
         */
        String GetAuthority() const;

        /**
         * Returns username part of the authority, or empty string if it's not
         * specified.
         */
        inline const String& GetUsername() const
        {
            return m_username;
        }

        /**
         * Returns password part of the authority, or empty string if it's not
         * specified.
         */
        inline const String& GetPassword() const
        {
            return m_password;
        }

        /**
         * Returns hostname part of the URL, or empty string if it's not
         * specified.
         */
        inline const String& GetHostname() const
        {
            return m_hostname;
        }

        /**
         * Returns port number, or 0 if it's not specified in the URL.
         */
        inline int GetPort() const
        {
            return m_port;
        }

        /**
         * Returns path of the URL, or empty string if it's not specified.
         */
        inline const String& GetPath() const
        {
            return m_path;
        }

        /**
         * Returns query string of the URL, or empty string if it's not
         * specified.
         */
        inline const String& GetQuery() const
        {
            return m_query;
        }

        /**
         * Returns fragment part of the URL, or empty string if it's not
         * specified.
         */
        inline const String& GetFragment() const
        {
            return m_fragment;
        }

        /**
         * Copies contents of another URL into this one.
         *
         * \param that Other URL to copy content from
         */
        Url& Assign(const Url& that);

        /**
         * Assignment operator.
         */
        inline Url& operator=(const Url& that)
        {
            return Assign(that);
        }

        /**
         * Builds a string from components of the URL.
         */
        String ToString() const;

    private:
        String m_scheme;
        String m_username;
        String m_password;
        String m_hostname;
        int m_port;
        String m_path;
        String m_query;
        String m_fragment;
    };
}

#endif /* !TEMPEARLY_NET_URL_H_GUARD */
