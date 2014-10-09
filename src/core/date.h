#ifndef TEMPEARLY_CORE_DATE_H_GUARD
#define TEMPEARLY_CORE_DATE_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class Date
    {
    public:
        enum Month
        {
            JANUARY = 1,
            FEBRUARY = 2,
            MARCH = 3,
            APRIL = 4,
            MAY = 5,
            JUNE = 6,
            JULY = 7,
            AUGUST = 8,
            SEPTEMBER = 9,
            OCTOBER = 10,
            NOVEMBER = 11,
            DECEMBER = 12
        };

        enum Weekday
        {
            MONDAY = 1,
            TUESDAY = 2,
            WEDNESDAY = 3,
            THURSDAY = 4,
            FRIDAY = 5,
            SATURDAY = 6,
            SUNDAY = 7
        };

        Date();

        Date(const Date& that);

        Date(int year, Month month, int day);

        /**
         * Returns date value based on system clock.
         */
        static Date Today();

        /**
         * Returns yesterdays date value based on system clock.
         */
        static Date Yesterday();

        /**
         * Returns tomorrows date value based on system clock.
         */
        static Date Tomorrow();

        /**
         * Tests whether given date is valid.
         */
        static bool IsValid(int year, Month month, int day);

        /**
         * Returns year of the date.
         */
        inline int GetYear() const
        {
            return m_year;
        }

        /**
         * Returns month of the year.
         */
        inline Month GetMonth() const
        {
            return m_month;
        }

        /**
         * Returns day of the month.
         */
        inline int GetDay() const
        {
            return m_day;
        }

        /**
         * Returns weekday for this date.
         */
        Weekday GetWeekday() const;

        /**
         * Returns the day of the year (1 to 365 or 366 on a leap year) for
         * this date.
         */
        int GetDayOfYear() const;

        /**
         * Returns the number of days in the month (28 to 31) for this date.
         */
        int GetDaysInMonth() const;

        /**
         * Returns the number of days in the year (365 or 366) for this date.
         */
        int GetDaysInYear() const;

        /**
         * Returns true if the year represented by the date is a leap year.
         */
        bool IsLeapYear() const;

        Date& Assign(const Date& that);

        Date& Assign(int year, Month month, int day);

        inline Date& operator=(const Date& that)
        {
            return Assign(that);
        }

        bool Equals(const Date& that) const;

        inline bool operator==(const Date& that) const
        {
            return Equals(that);
        }

        inline bool operator!=(const Date& that) const
        {
            return !Equals(that);
        }

        int Compare(const Date& that) const;

        inline bool operator<(const Date& that) const
        {
            return Compare(that) < 0;
        }

        inline bool operator>(const Date& that) const
        {
            return Compare(that) > 0;
        }

        inline bool operator<=(const Date& that) const
        {
            return Compare(that) <= 0;
        }

        inline bool operator>=(const Date& that) const
        {
            return Compare(that) >= 0;
        }

        Date& operator++();
        Date& operator--();
        Date operator++(int) const;
        Date operator--(int) const;

    private:
        int m_year;
        Month m_month;
        int m_day;
    };
}

#endif /* !TEMPEARLY_CORE_DATE_H_GUARD */
