#include <cerrno>

#include "core/bytestring.h"
#include "core/datetime.h"
#include "core/filename.h"
#include "core/stringbuilder.h"
#include "io/stream.h"
#if defined(_WIN32)
# define UNICODE
# include <windows.h>
#else
# include <fcntl.h>
# include <unistd.h>
#endif

namespace tempearly
{
#if defined(_WIN32)
    const rune Filename::kSeparator = '\\';
#else
    const rune Filename::kSeparator = '/';
#endif

    static void parse_filename(const String&, String&, String&, Vector<String>&);
    static String compile_filename(const String&, const Vector<String>&);

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

    Filename::~Filename() {}

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

    Filename& Filename::Assign(const String& source)
    {
        m_full_name.Clear();
        m_root.Clear();
        m_parts.Clear();
#if !defined(_WIN32)
        m_stat_state = STAT_UNINITIALIZED;
#endif
        parse_filename(source, m_full_name, m_root, m_parts);

        return *this;
    }

    bool Filename::IsSeparator(rune r)
    {
        return r == '/' || r == '\\';
    }

    Filename Filename::GetParent() const
    {
        return Filename(); // TODO
    }

    String Filename::GetExtension() const
    {
        if (!m_parts.IsEmpty())
        {
            const String& filename = m_parts.GetBack();
            std::size_t index = filename.LastIndexOf('.');

            if (index != String::npos && index > 0)
            {
                return filename.SubString(index + 1);
            }
        }

        return String();
    }

    Vector<String> Filename::GetParts() const
    {
        Vector<String> result(m_parts);

        if (!m_root.IsEmpty())
        {
            result.PushFront(m_root);
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

    namespace
    {
#if !defined(_WIN32)
        class PosixFileStream : public Stream
        {
        public:
            explicit PosixFileStream(Filename::OpenMode mode, int handle)
                : Stream(mode == Filename::MODE_WRITE ? 0 : Stream::kBufferSize)
                , m_mode(mode)
                , m_handle(handle) {}

            ~PosixFileStream()
            {
                if (m_handle >= 0)
                {
                    ::close(m_handle);
                }
            }

            bool IsOpen() const
            {
                return m_handle >= 0;
            }

            bool IsReadable() const
            {
                return m_mode == Filename::MODE_READ || m_mode == Filename::MODE_READ_WRITE;
            }

            bool IsWritable() const
            {
                return m_mode == Filename::MODE_WRITE || m_mode == Filename::MODE_READ_WRITE;
            }

            void Close()
            {
                if (m_handle >= 0)
                {
                    ::close(m_handle);
                    m_handle = 0;
                }
            }

            bool DirectRead(byte* buffer, std::size_t size, std::size_t& read)
            {
                if (m_handle >= 0)
                {
                    ::ssize_t result = ::read(m_handle, static_cast<void*>(buffer), sizeof(byte) * size);

                    if (result < 0)
                    {
                        SetErrorMessage(std::strerror(errno));
                        errno = 0;

                        return false;
                    }
                    read = static_cast<std::size_t>(result);

                    return true;
                }

                return false;
            }

            bool DirectWrite(const byte* data, std::size_t size)
            {
                if (m_handle >= 0)
                {
                    ::ssize_t result = ::write(m_handle, static_cast<const void*>(data), sizeof(byte) * size);

                    if (result < 0)
                    {
                        SetErrorMessage(std::strerror(errno));
                        errno = 0;

                        return false;
                    }
                    
                    return true;
                }

                return false;
            }

        private:
            const Filename::OpenMode m_mode;
            int m_handle;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(PosixFileStream);
        };
#endif
    }

    Handle<Stream> Filename::Open(OpenMode mode, bool append) const
    {
        if (IsEmpty())
        {
            return Handle<Stream>();
        }
#if defined(_WIN32)
# error "Windows implementation is missing"
#else
        int handle;
        int flags;

        if (mode == MODE_READ)
        {
            flags = O_RDONLY;
        } else {
            if (mode == MODE_WRITE)
            {
                flags = O_WRONLY;
            } else {
                flags = O_RDWR;
            }
            if (append)
            {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            flags |= O_CREAT;
        }

        // Default file permissions are rw+r+r
        handle = ::open(m_full_name.Encode().c_str(),
                        flags,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (handle < 0)
        {
            return Handle<Stream>();
        } else {
            return new PosixFileStream(mode, handle);
        }
#endif
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

    static void append_part(const String& input, Vector<String>& parts)
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
                if (!parts.IsEmpty())
                {
                    parts.Erase(parts.GetSize() - 1);
                    return;
                }
            }
            else if (length == 1)
            {
                if (!parts.IsEmpty())
                {
                    return;
                }
            }
        }
        parts.PushBack(input);
    }

    static void parse_filename(const String& source,
                               String& full_name,
                               String& root,
                               Vector<String>& parts)
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

    static String compile_filename(const String& root, const Vector<String>& parts)
    {
        StringBuilder sb;

        if (!root.IsEmpty())
        {
            sb << root;
        }
        for (std::size_t i = 0; i < parts.GetSize(); ++i)
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
