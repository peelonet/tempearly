#ifndef TEMPEARLY_CORE_PAIR_H_GUARD
#define TEMPEARLY_CORE_PAIR_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    /**
     * Container type which holds two values of same type.
     */
    template< class T >
    class Pair
    {
    public:
        /**
         * Constructs empty pair.
         */
        Pair()
            : m_key(T())
            , m_value(T()) {}

        /**
         * Copy constructor.
         */
        template< class U >
        Pair(const Pair<U>& that)
            : m_key(that.GetKey())
            , m_value(that.GetValue()) {}

        /**
         * Constructs pair from predefined pair of values.
         *
         * \param key   First value of the pair
         * \parma value Second value of the pair
         */
        template< class U1, class U2 >
        Pair(const U1& key, const U2& value)
            : m_key(key)
            , m_value(value) {}

        /**
         * Returns reference of first value.
         */
        inline const T& GetKey() const
        {
            return m_key;
        }

        /**
         * Returns reference of second value.
         */
        inline const T& GetValue() const
        {
            return m_value;
        }

        /**
         * Replaces values of the pair with values of another pair.
         *
         * \param that Other pair to copy values from
         */
        template< class U >
        Pair& operator=(const Pair<U>& that)
        {
            m_key = that.m_key;
            m_value = that.m_value;

            return *this;
        }

    private:
        /** First value of the pair. */
        T m_key;
        /** Second value of the pair. */
        T m_value;
    };
}

#endif /* !TEMPEARLY_CORE_PAIR_H_GUARD */
