#include <sys/stat.h>
#include <unistd.h>

#include "core/bytestring.h"
#include "core/filename.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    static void parse(const String&, String&, String&, std::vector<String>&);

    Filename::Filename() {}

    Filename::Filename(const Filename& that)
        : m_filename(that.m_filename)
        , m_root(that.m_root)
        , m_path(that.m_path) {}

    Filename::Filename(const String& source)
    {
        parse(source, m_filename, m_root, m_path);
    }

    Filename& Filename::operator=(const Filename& that)
    {
        m_filename = that.m_filename;
        m_root = that.m_root;
        m_path = that.m_path;

        return *this;
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
        struct stat st;

        if (IsEmpty())
        {
            return false;
        }
        if (::stat(m_filename.Encode().c_str(), &st) < 0)
        {
            return false; // File does not exist
        } else {
            return S_ISREG(st.st_mode);
        }
    }

    bool Filename::IsDir() const
    {
        struct stat st;

        if (IsEmpty())
        {
            return false;
        }
        if (::stat(m_filename.Encode().c_str(), &st) < 0)
        {
            return false; // File does not exist
        } else {
            return S_ISDIR(st.st_mode);
        }
    }

    bool Filename::Exists() const
    {
        if (IsEmpty())
        {
            return false;
        } else {
            return !::access(m_filename.Encode().c_str(), F_OK);
        }
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
#if defined(_WIN32)
                sb << '\\';
#else
                sb << '/';
#endif
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
