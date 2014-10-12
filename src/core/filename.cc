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
#if defined(_WIN32)
    const rune Filename::kSeparator = '\\';
#else
    const rune Filename::kSeparator = '/';
#endif

    static void parse_filename(const String&, String&, String&, std::vector<String>&);
    static String compile_filename(const String&, const std::vector<String>&);

    Filename::Filename()
#if !defined(_WIN32)
        : m_stat_state(STAT_UNINITIALIZED)
#endif
        {}

    Filename::Filename(const Filename& that)
        : m_full_name(that.m_full_name)
        , m_root(that.m_root)
        , m_parts(that.m_parts)
#if !defined(_WIN32)
        , m_stat_state(that.m_stat_state)
        , m_stat(that.m_stat)
#endif
        {}

    Filename::Filename(const String& source)
#if !defined(_WIN32)
        : m_stat_state(STAT_UNINITIALIZED)
#endif
    {
        parse_filename(source, m_full_name, m_root, m_parts);
    }

    Filename& Filename::Assign(const Filename& that)
    {
        m_full_name = that.m_full_name;
        m_root = that.m_root;
        m_parts = that.m_parts;
#if !defined(_WIN32)
        m_stat_state = that.m_stat_state;
        m_stat = that.m_stat;
#endif

        return *this;
    }

    bool Filename::IsSeparator(rune r)
    {
        return r == '/' || r == '\\';
    }

    Filename& Filename::Assign(const String& source)
    {
        m_full_name.Clear();
        m_root.Clear();
        m_parts.clear();
#if !defined(_WIN32)
        m_stat_state = STAT_UNINITIALIZED;
#endif
        parse_filename(source, m_full_name, m_root, m_parts);

        return *this;
    }

    Filename Filename::GetParent() const
    {
        return Filename(); // TODO
    }

    String Filename::GetExtension() const
    {
        if (!m_parts.empty())
        {
            const String& filename = m_parts[m_parts.size() - 1];
            std::size_t index = filename.LastIndexOf('.');

            if (index != String::npos && index > 0)
            {
                return filename.SubString(index + 1);
            }
        }

        return String();
    }

    std::vector<String> Filename::GetParts() const
    {
        std::vector<String> result(m_parts);

        if (!m_root.IsEmpty())
        {
            result.insert(result.begin(), m_root);
        }

        return result;
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

    bool Filename::IsSymlink() const
    {
#if !defined(_WIN32) && defined(S_ISLNK)
        if (!IsEmpty())
        {
            return Stat() && S_ISLNK(m_stat.st_mode);
        }
#endif

        return false;
    }

    bool Filename::IsSocket() const
    {
#if !defined(_WIN32) && defined(S_ISSOCK)
        if (!IsEmpty())
        {
            return Stat() && S_ISSOCK(m_stat.st_mode);
        }
#endif

        return false;
    }

    bool Filename::IsFifo() const
    {
#if !defined(_WIN32) && defined(S_ISFIFO)
        if (!IsEmpty())
        {
            return Stat() && S_ISFIFO(m_stat.st_mode);
        }
#endif

        return false;
    }

    bool Filename::IsCharDevice() const
    {
#if !defined(_WIN32) && defined(S_ISCHR)
        if (!IsEmpty())
        {
            return Stat() && S_ISCHR(m_stat.st_mode);
        }
#endif

        return false;
    }

    bool Filename::IsBlockDevice() const
    {
#if !defined(_WIN32) && defined(S_ISBLK)
        if (!IsEmpty())
        {
            return Stat() && S_ISBLK(m_stat.st_mode);
        }
#endif

        return false;
    }

    std::size_t Filename::GetSize() const
    {
        if (m_full_name.IsEmpty())
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

    FILE* Filename::Open(const String& mode) const
    {
        FILE* handle;

        if (IsEmpty())
        {
            return 0;
        }
#if defined(_WIN32)
        handle = ::_wfopen(m_full_name.Widen().c_str(), mode.Widen().c_str());
#else
        handle = std::fopen(m_full_name.Encode().c_str(), mode.Encode().c_str());
#endif

        return handle;
    }

    bool Filename::Equals(const Filename& that) const
    {
        if (IsEmpty())
        {
            return that.IsEmpty();
        }

#if defined(_WIN32)
        return m_full_name.EqualsIgnoreCase(that.m_full_name);
#else
        return m_full_name.Equals(that.m_full_name);
#endif
    }

    int Filename::Compare(const Filename& that) const
    {
        if (IsEmpty())
        {
            return that.IsEmpty() ? 0 : -1;
        }

#if defined(_WIN32)
        return m_full_name.CompareIgnoreCase(that.m_full_name);
#else
        return m_full_name.Compare(that.m_full_name);
#endif
    }

    Filename Filename::Concat(const String& string) const
    {
        if (m_full_name.IsEmpty())
        {
            return string;
        }
        else if (string.IsEmpty())
        {
            return *this;
        }
        else if (IsSeparator(m_full_name.GetBack())
                || IsSeparator(string.GetFront()))
        {
            return m_full_name.Concat(string);
        } else {
            return m_full_name + "/" + string;
        }
    }

#if !defined(_WIN32)
    bool Filename::Stat() const
    {
        if (m_stat_state == STAT_FAILED)
        {
            return false;
        }
        else if (m_stat_state == STAT_UNINITIALIZED)
        {
            if (::stat(m_full_name.Encode().c_str(), &m_stat) < 0)
            {
                m_stat_state = STAT_FAILED;

                return false;
            }
            m_stat_state = STAT_SUCCEEDED;
        }

        return true;
    }
#endif

    static void append_part(const String& input, std::vector<String>& parts)
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
                if (!parts.empty())
                {
                    parts.pop_back();
                    return;
                }
            }
            else if (length == 1)
            {
                if (!parts.empty())
                {
                    return;
                }
            }
        }
        parts.push_back(input);
    }

    static void parse_filename(const String& source,
                               String& full_name,
                               String& root,
                               std::vector<String>& parts)
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
                full_name = root;
                return;
            }
            begin = 1;
        }
#if defined(_WIN32)
        // Process drive letter on Windows platform.
        else if (length > 1 && std::isalpha(source[0]) && source[1] == ':')
        {
            if (length == 2)
            {
                full_name = root = source;
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
                    append_part(source.SubString(begin, end), parts);
                }
                begin = i + 1;
                end = 0;
            } else {
                ++end;
            }
        }
        if (end > 0)
        {
            append_part(source.SubString(begin), parts);
        }
        full_name = compile_filename(root, parts);
    }

    static String compile_filename(const String& root, const std::vector<String>& parts)
    {
        StringBuilder sb;

        if (!root.IsEmpty())
        {
            sb << root;
        }
        for (std::size_t i = 0; i < parts.size(); ++i)
        {
            if (i > 0 && !Filename::IsSeparator(sb.GetBack()))
            {
                sb << Filename::kSeparator;
            }
            sb << parts[i];
        }

        return sb.ToString();
    }
}
