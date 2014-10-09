#include "core/time.h"

#if defined(_WIN32)
# include <windows.h>
#else
# include <ctime>
#endif

namespace tempearly
{
    Time::Time()
        : m_hour(0)
        , m_minute(0)
        , m_second(0) {}

    Time::Time(const Time& that)
        : m_hour(that.m_hour)
        , m_minute(that.m_minute)
        , m_second(that.m_second) {}

    Time::Time(int hour, int minute, int second)
        : m_hour(0)
        , m_minute(0)
        , m_second(0)
    {
        if (IsValid(hour, minute, second))
        {
            m_hour = hour;
            m_minute = minute;
            m_second = second;
        }
    }

    bool Time::IsValid(int hour, int minute, int second)
    {
        return (hour >= 0 && hour < 23)
            && (minute >= 0 && minute < 59)
            && (second >= 0 && second < 59);
    }

    Time Time::Now()
    {
#if defined(_WIN32)
        SYSTEMTIME lt;

        ::GetLocalTime(&lt);

        return Time(lt.wHour, lt.wMinute, lt.wSecond);
#else
        std::time_t ts = std::time(0);
        std::tm* tm = std::localtime(&ts);

        if (tm)
        {
            return Time(tm->tm_hour, tm->tm_min, tm->tm_sec);
        } else {
            return Time();
        }
#endif
    }

    Time& Time::Assign(const Time& that)
    {
        m_hour = that.m_hour;
        m_minute = that.m_minute;
        m_second = that.m_second;

        return *this;
    }

    Time& Time::Assign(int hour, int minute, int second)
    {
        if (IsValid(hour, minute, second))
        {
            m_hour = hour;
            m_minute = minute;
            m_second = second;
        }

        return *this;
    }

    bool Time::Equals(const Time& that) const
    {
        return m_hour == that.m_hour
            && m_minute == that.m_minute
            && m_second == that.m_second;
    }

    int Time::Compare(const Time& that) const
    {
        if (m_hour != that.m_hour)
        {
            return m_hour > that.m_hour ? 1 : -1;
        }
        else if (m_minute != that.m_minute)
        {
            return m_minute > that.m_minute ? 1 : -1;
        }
        else if (m_second != that.m_second)
        {
            return m_second > that.m_second ? 1 : -1;
        } else {
            return 0;
        }
    }
}
