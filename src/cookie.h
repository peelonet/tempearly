#ifndef TEMPEARLY_COOKIE_H_GUARD
#define TEMPEARLY_COOKIE_H_GUARD

#include "core/string.h"

namespace tempearly
{
    class Cookie
    {
    public:
        Cookie(const String& name = String(),
               const String& value = String(),
               const String& comment = String(),
               const String& domain = String(),
               unsigned long max_age = 0,
               const String& path = String(),
               bool secure = false);

        Cookie(const Cookie& that);

        static bool Parse(const String& source, Cookie& cookie);

        Cookie& assign(const Cookie& that);

        inline Cookie& operator=(const Cookie& that)
        {
            return assign(that);
        }

        inline const String& GetName() const
        {
            return m_name;
        }

        inline const String& GetValue() const
        {
            return m_value;
        }

        inline const String& GetComment() const
        {
            return m_comment;
        }

        inline const String& GetDomain() const
        {
            return m_domain;
        }

        inline unsigned long GetMaxAge() const
        {
            return m_max_age;
        }

        inline bool IsSecure() const
        {
            return m_secure;
        }

    private:
        String m_name;
        String m_value;
        String m_comment;
        String m_domain;
        unsigned long m_max_age;
        String m_path;
        bool m_secure;
    };
}

#endif /* !TEMPEARLY_COOKIE_H_GUARD */
