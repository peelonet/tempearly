#ifndef TEMPEARLY_CORE_TIME_H_GUARD
#define TEMPEARLY_CORE_TIME_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class Time
    {
    public:
        Time();

        Time(const Time& that);

        Time(int hour, int minute, int second);

        /**
         * Returns time based on system clock.
         */
        static Time Now();

        /**
         * Tests whether given time information is valid.
         */
        static bool IsValid(int hour, int minute, int second);

        inline int GetHour() const
        {
            return m_hour;
        }

        inline int GetMinute() const
        {
            return m_minute;
        }

        inline int GetSecond() const
        {
            return m_second;
        }

        Time& Assign(const Time& that);

        Time& Assign(int hour, int minute, int second);

        inline Time& operator=(const Time& that)
        {
            return Assign(that);
        }

        bool Equals(const Time& that) const;

        inline bool operator==(const Time& that) const
        {
            return Equals(that);
        }

        inline bool operator!=(const Time& that) const
        {
            return !Equals(that);
        }

        int Compare(const Time& that) const;

        inline bool operator<(const Time& that) const
        {
            return Compare(that) < 0;
        }

        inline bool operator>(const Time& that) const
        {
            return Compare(that) > 0;
        }

        inline bool operator<=(const Time& that) const
        {
            return Compare(that) <= 0;
        }

        inline bool operator>=(const Time& that) const
        {
            return Compare(that) >= 0;
        }

    private:
        int m_hour;
        int m_minute;
        int m_second;
    };
}

#endif /* !TEMPEARLY_CORE_TIME_H_GUARD */
