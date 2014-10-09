#include "utils.h"
#include "core/datetime.h"
#include "core/stringbuilder.h"

#if defined(_WIN32)
# include <windows.h>
#else
# include <ctime>
#endif

namespace tempearly
{
    DateTime::DateTime() {}

    DateTime::DateTime(const DateTime& that)
        : m_date(that.m_date)
        , m_time(that.m_time) {}

    DateTime::DateTime(const Date& date, const Time& time)
        : m_date(date)
        , m_time(time) {}

    DateTime::DateTime(int year,
                       Date::Month month,
                       int day,
                       int hour,
                       int minute,
                       int second)
        : m_date(year, month, day)
        , m_time(hour, minute, second) {}

    DateTime::DateTime(i64 timestamp)
    {
#if defined(_WIN32)
        FILETIME ft;
        SYSTEMTIME st;
        LONGLONG ll = Int32x32To64(timestamp, 10000000) + 116444736000000000;

        ft.dwLowDateTime = static_cast<DWORD>(ll);
        ft.dwHighDateTime = ll >> 32;
        ::FileTimeToSystemTime(&ft, &st);
        m_date.Assign(st.wYear, static_cast<Date::Month>(st.wMonth), st.wDay);
        m_time.Assign(st.wHour, st.wMinute, st.wSecond);
#else
        std::time_t ts = static_cast<std::time_t>(timestamp);
        std::tm* tm = std::localtime(&ts);

        if (tm)
        {
            m_date.Assign(tm->tm_year + 1900, static_cast<Date::Month>(tm->tm_mon + 1), tm->tm_mday);
            m_time.Assign(tm->tm_hour, tm->tm_min, tm->tm_sec);
        }
#endif
    }

    DateTime DateTime::Now()
    {
#if defined(_WIN32)
        SYSTEMTIME local_time;

        ::GetLocalTime(&local_time);

        return DateTime(
            local_time.wYear,
            static_cast<Date::Month>(local_time.wMonth),
            local_time.wDay,
            local_time.wHour,
            local_time.wMinute,
            local_time.wSecond
        );
#else
        std::time_t ts = std::time(0);
        std::tm* tm = std::localtime(&ts);

        if (!tm)
        {
            return DateTime();
        }

        return DateTime(
            tm->tm_year + 1900,
            static_cast<Date::Month>(tm->tm_mon + 1),
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec
        );
#endif
    }

    String DateTime::Format(const String& format) const
    {
        StringBuilder result(format.GetLength());

        for (std::size_t i = 0; i < format.GetLength(); ++i)
        {
            if (format[i] != '%' || i + 1 >= format.GetLength())
            {
                result << format[i];
                continue;
            }
            switch (format[++i])
            {
                case '%':
                    result << '%';
                    break;

                case 'n':
                    result << '\n';
                    break;

                case 't':
                    result << '\t';
                    break;

                // Year

                // Year as 4 digit decimal number
                case 'Y':
                    result << Utils::ToString(
                        static_cast<i64>(m_date.GetYear()),
                        10
                    );
                    break;

                case 'y': // TODO: last 2 digits of year

                case 'C': // TODO: first 2 digits of year

                case 'G': // TODO: ISO 8601 week-based year

                case 'g': // TODO: 2 last digits of ISO 8601 week-based year

                // Month

                // Abbreviated month name
                case 'h':
                case 'b':
                    switch (m_date.GetMonth())
                    {
                        case Date::JANUARY:
                            result << "Jan";
                            break;
                        case Date::FEBRUARY:
                            result << "Feb";
                            break;
                        case Date::MARCH:
                            result << "Mar";
                            break;
                        case Date::APRIL:
                            result << "Apr";
                            break;
                        case Date::MAY:
                            result << "May";
                            break;
                        case Date::JUNE:
                            result << "Jun";
                            break;
                        case Date::JULY:
                            result << "Jul";
                            break;
                        case Date::AUGUST:
                            result << "Aug";
                            break;
                        case Date::SEPTEMBER:
                            result << "Sep";
                            break;
                        case Date::OCTOBER:
                            result << "Oct";
                            break;
                        case Date::NOVEMBER:
                            result << "Nov";
                            break;
                        case Date::DECEMBER:
                            result << "Dec";
                            break;
                    }
                    break;

                // Full month name
                case 'B':
                    switch (m_date.GetMonth())
                    {
                        case Date::JANUARY:
                            result << "January";
                            break;
                        case Date::FEBRUARY:
                            result << "February";
                            break;
                        case Date::MARCH:
                            result << "March";
                            break;
                        case Date::APRIL:
                            result << "April";
                            break;
                        case Date::MAY:
                            result << "May";
                            break;
                        case Date::JUNE:
                            result << "June";
                            break;
                        case Date::JULY:
                            result << "July";
                            break;
                        case Date::AUGUST:
                            result << "August";
                            break;
                        case Date::SEPTEMBER:
                            result << "September";
                            break;
                        case Date::OCTOBER:
                            result << "October";
                            break;
                        case Date::NOVEMBER:
                            result << "November";
                            break;
                        case Date::DECEMBER:
                            result << "December";
                            break;
                    }
                    break;

                // Month as decimal number (01-12)
                case 'm':
                {
                    const i64 month = static_cast<i64>(m_date.GetMonth());

                    if (month < 10)
                    {
                        result << '0';
                    }
                    result << Utils::ToString(month, 10);
                    break;
                }

                // Week

                case 'U': // TODO: week of the year as decimal number (sunday as first day of the week)
                    break;

                case 'W': // TODO: week of the year as decimal number (monday as first day of the week)
                    break;

                case 'V': // TODO: iso 8601 week of the year (begins with monday)
                    break;

                // Day of the year/month

                // Day of the year as decimal number (001-366)
                case 'j':
                {
                    const i64 day = m_date.GetDayOfYear();

                    if (day < 100)
                    {
                        result << '0';
                        if (day < 10)
                        {
                            result << '0';
                        }
                    }
                    result << Utils::ToString(day, 10);
                    break;
                }

                // Day of the month as decimal number (01-31)
                case 'd':
                {
                    const i64 day = m_date.GetDay();

                    if (day < 10)
                    {
                        result << '0';
                    }
                    result << Utils::ToString(day, 10);
                    break;
                }

                // Day of the month as decimal number (1-31) (single digit
                // preceded by space)
                case 'e':
                {
                    const i64 day = m_date.GetDay();

                    if (day < 10)
                    {
                        result << ' ';
                    }
                    result << Utils::ToString(day, 10);
                    break;
                }

                // Day of the week

                // Abbreviated weekday name
                case 'a':
                    switch (m_date.GetWeekday())
                    {
                        case Date::SUNDAY:
                            result << "Sun";
                            break;
                        case Date::MONDAY:
                            result << "Mon";
                            break;
                        case Date::TUESDAY:
                            result << "Tue";
                            break;
                        case Date::WEDNESDAY:
                            result << "Wed";
                            break;
                        case Date::THURSDAY:
                            result << "Thu";
                            break;
                        case Date::FRIDAY:
                            result << "Fri";
                            break;
                        case Date::SATURDAY:
                            result << "Sat";
                            break;
                    }
                    break;

                // Full weekday name
                case 'A':
                    switch (m_date.GetWeekday())
                    {
                        case Date::SUNDAY:
                            result << "Sunday";
                            break;
                        case Date::MONDAY:
                            result << "Monday";
                            break;
                        case Date::TUESDAY:
                            result << "Tuesday";
                            break;
                        case Date::WEDNESDAY:
                            result << "Wednesday";
                            break;
                        case Date::THURSDAY:
                            result << "Thursday";
                            break;
                        case Date::FRIDAY:
                            result << "Friday";
                            break;
                        case Date::SATURDAY:
                            result << "Saturday";
                            break;
                    }
                    break;

                // Weekday as decimal, where Sunday is 0 (0-6)
                case 'w':
                    switch (m_date.GetWeekday())
                    {
                        case Date::SUNDAY:
                            result << '0';
                            break;
                        case Date::MONDAY:
                            result << '1';
                            break;
                        case Date::TUESDAY:
                            result << '2';
                            break;
                        case Date::WEDNESDAY:
                            result << '3';
                            break;
                        case Date::THURSDAY:
                            result << '4';
                            break;
                        case Date::FRIDAY:
                            result << '5';
                            break;
                        case Date::SATURDAY:
                            result << '6';
                            break;
                    }
                    break;

                // Weekday as decimal, Monday is 1 (1-7)
                case 'u':
                    result << Utils::ToString(
                        static_cast<i64>(m_date.GetWeekday()),
                        10
                    );
                    break;

                // Hour, minute, second

                // Hour as decimal, 24 hour clock (00-23)
                case 'H':
                {
                    const i64 hour = m_time.GetHour();

                    if (hour < 10)
                    {
                        result << '0';
                    }
                    result << Utils::ToString(hour, 10);
                    break;
                }

                case 'I': // TODO: hour as decimal, 12 hour clock (01-12)
                    break;

                // Minute as decimal (00-59)
                case 'M':
                {
                    const i64 minute = m_time.GetMinute();

                    if (minute < 10)
                    {
                        result << '0';
                    }
                    result << Utils::ToString(minute, 10);
                    break;
                }

                // Second as decimal (00-60)
                case 'S':
                {
                    const i64 second = m_time.GetSecond();

                    if (second < 10)
                    {
                        result << '0';
                    }
                    result << Utils::ToString(second, 10);
                    break;
                }

                // Other

                case 'c': // TODO: standard date and time string
                    break;

                case 'x': // TODO: localized date representation
                    break;

                case 'X': // TODO: localized time representation
                    break;

                // Equivalent to "%m/%d/%y"
                case 'D':
                    result << Format("%m/%d/%y");
                    break;

                // Equivalent to "%Y-%m-%d"
                case 'F':
                    result << Format("%Y-%m-%d");
                    break;

                case 'r': // TODO: localized 12-hour clock
                    break;

                // Equivalent to "%H:%M"
                case 'R':
                    result << Format("%H:%M");
                    break;

                // Equivalent to "%H:%M:%S"
                case 'T':
                    result << Format("%H:%M:%S");
                    break;

                case 'p': // TODO: localized a.m. or p.m.
                    break;

                // Offset from UTC, since we don't have timezone information,
                // this does nothing
                case 'z':
                case 'Z':
                    break;

                default:
                    result << '%' << format[i];
            }
        }

        return result.ToString();
    }

    DateTime& DateTime::Assign(const DateTime& that)
    {
        m_date.Assign(that.m_date);
        m_time.Assign(that.m_time);

        return *this;
    }

    bool DateTime::Equals(const DateTime& that) const
    {
        return m_date.Equals(that.m_date) && m_time.Equals(that.m_time);
    }

    int DateTime::Compare(const DateTime& that) const
    {
        int cmp = m_date.Compare(that.m_date);

        return cmp == 0 ? m_time.Compare(that.m_time) : cmp;
    }
}
