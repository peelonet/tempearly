#ifndef TEMPEARLY_CORE_DATETIME_H_GUARD
#define TEMPEARLY_CORE_DATETIME_H_GUARD

#include "core/date.h"
#include "core/time.h"

namespace tempearly
{
    class DateTime
    {
    public:
        DateTime();

        DateTime(const DateTime& that);

        DateTime(const Date& date, const Time& time);

        DateTime(int year,
                 Date::Month month,
                 int day,
                 int hour,
                 int minute,
                 int second);

        /**
         * Calculates date and time from UNIX timestamp.
         */
        DateTime(i64 timestamp);

        /**
         * Returns date and time based on system clock.
         */
        static DateTime Now();

        inline const Date& GetDate() const
        {
            return m_date;
        }

        inline const Time& GetTime() const
        {
            return m_time;
        }

        String Format(const String& format) const;

        DateTime& Assign(const DateTime& that);

        inline DateTime& operator=(const DateTime& that)
        {
            return Assign(that);
        }

        bool Equals(const DateTime& that) const;

        inline bool operator==(const DateTime& that) const
        {
            return Equals(that);
        }

        inline bool operator!=(const DateTime& that) const
        {
            return !Equals(that);
        }

        int Compare(const DateTime& that) const;

        inline bool operator<(const DateTime& that) const
        {
            return Compare(that) < 0;
        }

        inline bool operator>(const DateTime& that) const
        {
            return Compare(that) > 0;
        }

        inline bool operator<=(const DateTime& that) const
        {
            return Compare(that) <= 0;
        }

        inline bool operator>=(const DateTime& that) const
        {
            return Compare(that) >= 0;
        }

    private:
        Date m_date;
        Time m_time;
    };
}

#endif /* !TEMPEARLY_CORE_DATETIME_H_GUARD */
