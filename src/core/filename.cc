#include "core/bytestring.h"
#include "core/datetime.h"
#include "core/filename.h"
#include "core/stringbuilder.h"
#if defined(_WIN32)
# define UNICODE
# include <windows.h>
#endif

namespace tempearly
{
    static void parse(const String&, String&, String&, std::vector<String>&);

#if defined(_WIN32)
    const rune Filename::kSeparator = '\\';
#else
    const rune Filename::kSeparator = '/';
#endif

    Filename::Filename()
#if !defined(_WIN32)
        : m_state(STAT_NOT_DONE)
#endif
        {}

    Filename::Filename(const Filename& that)
        : m_filename(that.m_filename)
        , m_root(that.m_root)
        , m_path(that.m_path)
#if !defined(_WIN32)
        , m_stat(that.m_stat)
        , m_state(that.m_state)
#endif
        {}

    Filename::Filename(const String& source)
#if !defined(_WIN32)
        : m_state(STAT_NOT_DONE)
#endif
    {
        parse(source, m_filename, m_root, m_path);
    }

    Filename& Filename::operator=(const Filename& that)
    {
        m_filename = that.m_filename;
        m_root = that.m_root;
        m_path = that.m_path;
#if !defined(_WIN32)
        m_stat = that.m_stat;
        m_state = that.m_state;
#endif

        return *this;
    }

    Filename& Filename::operator=(const String& source)
    {
        m_filename.Clear();
        m_root.Clear();
        m_path.clear();
#if !defined(_WIN32)
        m_state = STAT_NOT_DONE;
#endif
        parse(source, m_filename, m_root, m_path);

        return *this;
    }

    String Filename::GetExtension() const
    {
        if (!m_path.empty())
        {
            const String& filename = m_path[m_path.size() - 1];
            std::size_t index = filename.IndexOf('.');

            if (index != String::npos && index > 0)
            {
                return filename.SubString(index + 1);
            }
        }

        return String();
    }

    std::size_t Filename::GetSize() const
    {
        if (m_filename.IsEmpty())
        {
            return 0;
        }
#if defined(_WIN32)
        return ::GetFileSize(m_filename.Widen().c_str(), 0);
#else
        return Stat() ? m_stat.st_size : 0;
#endif
    }

    DateTime Filename::GetLastModified() const
    {
#if defined(_WIN32)
        WIN32_FILE_ATTRIBUTE_DATA data = {0};

        if (::GetFileAttributesExW(m_filename.Widen().c_str(), GetFileExInfoStandard, &data))
        {
            SYSTEMTIME st;

            ::FileTimeToSystemTime(&data.ftLastWriteTime, &st);

            return DateTime(
                st.wYear,
                static_cast<Date::Month>(st.wMonth),
                st.wDay,
                st.wHour,
                st.wMinute,
                st.wSecond
            );
        }
#else
        if (Stat())
        {
            return DateTime(m_stat.st_mtime);
        }
#endif

        return DateTime();
    }

    bool Filename::IsSeparator(rune r)
    {
        return r == '/' || r == '\\';
    }

    bool Filename::IsEmpty() const
    {
        return m_filename.IsEmpty();
    }

    bool Filename::IsAbsolute() const
    {
        return !m_root.IsEmpty();
    }

    bool Filename::IsFile() const
    {
        if (IsEmpty())
        {
            return false;
        }
#if defined(_WIN32)
        return ::GetFileAttributesW(m_filename.Widen().c_str()) & FILE_ATTRIBUTE_NORMAL;
#else
        return Stat() && S_ISREG(m_stat.st_mode);
#endif
    }

    bool Filename::IsDir() const
    {
        if (IsEmpty())
        {
            return false;
        }
#if defined(_WIN32)
        return ::GetFileAttributesW(m_filename.Widen().c_str()) & FILE_ATTRIBUTE_DIRECTORY;
#else
        return Stat() && S_ISDIR(m_stat.st_mode);
#endif
    }

    bool Filename::Exists() const
    {
        if (IsEmpty())
        {
            return false;
        }
#if defined(_WIN32)
        return ::PathFileExists(m_filename.Widen().c_str());
#else
        return Stat();
#endif
    }

    FILE* Filename::Open(const char* mode) const
    {
        if (IsEmpty())
        {
            return 0;
        } else {
            return std::fopen(m_filename.Encode().c_str(), mode);
        }
    }

    Filename Filename::Concat(const String& string) const
    {
        if (m_filename.IsEmpty())
        {
            return string;
        }
        else if (string.IsEmpty())
        {
            return *this;
        }
        else if (IsSeparator(m_filename.GetBack())
                || IsSeparator(string.GetFront()))
        {
            return m_filename.Concat(string);
        } else {
            return m_filename + "/" + string;
        }
    }

#if !defined(_WIN32)
    bool Filename::Stat() const
    {
        if (m_state == STAT_FAILED)
        {
            return false;
        }
        else if (m_state == STAT_NOT_DONE)
        {
            if (::stat(m_filename.Encode().c_str(), &m_stat) < 0)
            {
                m_state = STAT_FAILED;

                return false;
            }
            m_state = STAT_SUCCEEDED;
        }

        return true;
    }
#endif

    static void append(const String& input, std::vector<String>& path)
    {
        const std::size_t length = input.GetLength();

        if (length == 0)
        {
            return;
        }
        else if (input[0] == '.')
        {
            if (length == 2 && input[1] == '.')
            {
                if (!path.empty())
                {
                    path.pop_back();
                    return;
                }
            }
            else if (length == 1)
            {
                if (!path.empty())
                {
                    return;
                }
            }
        }
        path.push_back(input);
    }

    static String compile(const String& root, const std::vector<String>& path)
    {
        StringBuilder sb;

        if (!root.IsEmpty())
        {
            sb << root;
        }
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            if (i > 0 && !Filename::IsSeparator(sb.GetBack()))
            {
                sb << Filename::kSeparator;
            }
            sb << path[i];
        }

        return sb.ToString();
    }

    static void parse(const String& source,
                      String& result,
                      String& root,
                      std::vector<String>& path)
    {
        const std::size_t length = source.GetLength();
        std::size_t begin = 0;
        std::size_t end = 0;

        if (length == 0)
        {
            return;
        }
        else if (Filename::IsSeparator(source[0]))
        {
            root = source.SubString(0, 1);
            if (length == 1)
            {
                result = root;
                return;
            }
            begin = 1;
        }
#if defined(_WIN32)
        else if (length > 1 && std::isalpha(source[0]) && source[1] == ':')
        {
            if (length == 2)
            {
                root = result = source;
                return;
            }
            else if (Filename::IsSeparator(source[2]))
            {
                root = source.SubString(0, 2);
                begin = 3;
            }
        }
#endif
        for (std::size_t i = begin; i < length; ++i)
        {
            if (Filename::IsSeparator(source[i]))
            {
                if (end > 0)
                {
                    append(source.SubString(begin, end), path);
                }
                begin = i + 1;
                end = 0;
            } else {
                ++end;
            }
        }
        if (end > 0)
        {
            append(source.SubString(begin, end), path);
        }
        result = compile(root, path);
    }
}
