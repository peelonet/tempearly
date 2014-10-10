#include "core/date.h"

#if defined(_WIN32)
# include <windows.h>
#else
# include <ctime>
#endif

namespace tempearly
{
    static inline int days_in_month(Date::Month month, bool leap_year)
    {
        switch (month)
        {
            case Date::JANUARY: return 31;
            case Date::FEBRUARY: return leap_year ? 29 : 28;
            case Date::MARCH: return 31;
            case Date::APRIL: return 30;
            case Date::MAY: return 31;
            case Date::JUNE: return 30;
            case Date::JULY: return 31;
            case Date::AUGUST: return 31;
            case Date::SEPTEMBER: return 30;
            case Date::OCTOBER: return 31;
            case Date::NOVEMBER: return 30;
            default: return 31;
        }
    }

    static inline bool is_leap_year(int year)
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    Date::Date()
        : m_year(1900)
        , m_month(JANUARY)
        , m_day(1) {}

    Date::Date(const Date& that)
        : m_year(that.m_year)
        , m_month(that.m_month)
        , m_day(that.m_day) {}

    Date::Date(int year, Month month, int day)
        : m_year(1900)
        , m_month(JANUARY)
        , m_day(1)
    {
        if (IsValid(year, month, day))
        {
            m_year = year;
            m_month = month;
            m_day = day;
        }
    }

    Date Date::Today()
    {
#if defined(_WIN32)
        SYSTEMTIME lt;

        ::GetLocalTime(&lt);

        return Date(lt.wYear, static_cast<Month>(lt.wMonth), lt.wDay);
#else
        std::time_t ts = std::time(0);
        std::tm* tm = std::localtime(&ts);

        if (tm)
        {
            return Date(tm->tm_year + 1900, static_cast<Month>(tm->tm_mon + 1), tm->tm_mday);
        } else {
            return Date();
        }
#endif
    }

    Date Date::Yesterday()
    {
#if defined(_WIN32)
        SYSTEMTIME lt;

        ::GetLocalTime(&lt);
        if (lt.wDay > 1)
        {
            return Date(lt.wYear, static_cast<Month>(lt.wMonth), lt.wDay - 1);
        }
        else if (lt.wMonth == 1)
        {
            return Date(lt.wYear - 1, DECEMBER, 31);
        } else {
            const Month month = static_cast<Month>(lt.wMonth - 1);

            return Date(lt.wYear, month, days_in_month(month, is_leap_year(lt.wYear)));
        }
#else
        std::time_t ts = std::time(0);
        std::tm* tm = std::localtime(&ts);

        if (!tm)
        {
            return Date();
        }
        --tm->tm_mday;
        if ((ts = std::mktime(tm)) == -1)
        {
            return Date();
        }

        return Date(tm->tm_year + 1900, static_cast<Month>(tm->tm_mon + 1), tm->tm_mday);
#endif
    }

    Date Date::Tomorrow()
    {
#if defined(_WIN32)
        SYSTEMTIME lt;

        ::GetLocalTime(&lt);
        if (lt.wDay == days_in_month(static_cast<Month>(lt.wMonth), is_leap_year(lt.wYear)))
        {
            if (lt.wMonth == DECEMBER)
            {
                return Date(lt.wYear + 1, JANUARY, 1);
            } else {
                return Date(lt.wYear, static_cast<Month>(lt.wMonth + 1), 1);
            }
        } else {
            return Date(lt.wYear, static_cast<Month>(lt.wMonth), lt.wDay + 1);
        }
#else
        std::time_t ts = std::time(0);
        std::tm* tm = std::localtime(&ts);

        if (!tm)
        {
            return Date();
        }
        ++tm->tm_mday;
        if ((ts = std::mktime(tm)) == -1)
        {
            return Date();
        }

        return Date(tm->tm_year + 1900, static_cast<Month>(tm->tm_mon + 1), tm->tm_mday);
#endif
    }

    bool Date::IsValid(int year, Month month, int day)
    {
        return day > 0 && day <= days_in_month(month, is_leap_year(year));
    }

    Date::Weekday Date::GetWeekday() const
    {
#if defined(_WIN32)
        SYSTEMTIME st;
        FILETIME ft;

        st.wYear = m_year;
        st.wMonth = static_cast<int>(m_month);
        st.wDay = m_day;
        ::SystemTimeToFileTime(&st, &ft);
        ::FileTimeToSystemTime(&ft, &st);
        switch (st.wDayOfWeek)
        {
            case 1: return MONDAY;
            case 2: return TUESDAY;
            case 3: return WEDNESDAY;
            case 4: return THURSDAY;
            case 5: return FRIDAY;
            case 6: return SATURDAY;
            default: return SUNDAY;
        }
#else
        std::tm tm;

        tm.tm_year = m_year - 1900;
        tm.tm_mon = static_cast<int>(m_month) - 1;
        tm.tm_mday = m_day;
        if (std::mktime(&tm) == -1)
        {
            return SUNDAY;
        }
        switch (tm.tm_wday)
        {
            case 1: return MONDAY;
            case 2: return TUESDAY;
            case 3: return WEDNESDAY;
            case 4: return THURSDAY;
            case 5: return FRIDAY;
            case 6: return SATURDAY;
            default: return SUNDAY;
        }
#endif
    }

    int Date::GetDayOfYear() const
    {
        const bool leap_year = IsLeapYear();
        int result = 0;

        for (int i = 1; i < m_month; ++i)
        {
            result += days_in_month(static_cast<Month>(i), leap_year);
        }

        return result + m_day;
    }

    int Date::GetDaysInMonth() const
    {
        return days_in_month(m_month, IsLeapYear());
    }

    int Date::GetDaysInYear() const
    {
        return IsLeapYear() ? 366 : 365;
    }

    bool Date::IsLeapYear() const
    {
        return is_leap_year(m_year);
    }

    Date& Date::Assign(const Date& that)
    {
        m_year = that.m_year;
        m_month = that.m_month;
        m_day = that.m_day;

        return *this;
    }

    Date& Date::Assign(int year, Month month, int day)
    {
        if (IsValid(year, month, day))
        {
            m_year = year;
            m_month = month;
            m_day = day;
        }

        return *this;
    }

    bool Date::Equals(const Date& that) const
    {
        return m_year == that.m_year
            && m_month == that.m_month
            && m_day == that.m_day;
    }

    int Date::Compare(const Date& that) const
    {
        if (m_year != that.m_year)
        {
            return m_year > that.m_year ? 1 : -1;
        }
        else if (m_month != that.m_month)
        {
            return m_month > that.m_month ? 1 : -1;
        }
        else if (m_day != that.m_day)
        {
            return m_day > that.m_day ? 1 : -1;
        } else {
            return 0;
        }
    }

    Date& Date::operator++()
    {
        if (m_day == GetDaysInMonth())
        {
            m_day = 1;
            if (m_month == DECEMBER)
            {
                ++m_year;
                m_month = JANUARY;
            } else {
                m_month = static_cast<Month>(m_month + 1);
            }
        } else {
            ++m_day;
        }

        return *this;
    }

    Date& Date::operator--()
    {
        if (m_day > 1)
        {
            --m_day;
        }
        else if (m_month == JANUARY)
        {
            --m_year;
            m_month = DECEMBER;
            m_day = 31;
        } else {
            m_month = static_cast<Month>(m_month - 1);
            m_day = days_in_month(m_month, IsLeapYear());
        }

        return *this;
    }

    Date Date::operator++(int) const
    {
        Date clone(*this);

        if (clone.m_day == clone.GetDaysInMonth())
        {
            clone.m_day = 1;
            if (clone.m_month == DECEMBER)
            {
                ++clone.m_year;
                clone.m_month = JANUARY;
            } else {
                clone.m_month = static_cast<Month>(clone.m_month + 1);
            }
        } else {
            ++clone.m_day;
        }

        return clone;
    }

    Date Date::operator--(int) const
    {
        Date clone(*this);

        if (clone.m_day > 1)
        {
            --clone.m_day;
        }
        else if (clone.m_month == JANUARY)
        {
            --clone.m_year;
            clone.m_month = DECEMBER;
            clone.m_day = 31;
        } else {
            clone.m_month = static_cast<Month>(clone.m_month - 1);
            clone.m_day = days_in_month(clone.m_month, clone.IsLeapYear());
        }

        return clone;
    }
}
