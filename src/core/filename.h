#ifndef TEMPEARLY_CORE_FILENAME_H_GUARD
#define TEMPEARLY_CORE_FILENAME_H_GUARD

#include "core/string.h"
#if !defined(_WIN32)
# include <sys/stat.h>
#endif

namespace tempearly
{
    class Filename
    {
    public:
        /**
         * Platform specific filename separator.
         */
        static const rune kSeparator;

        /**
         * Constructs empty filename.
         */
        Filename();

        /**
         * Constructs copy of existing filename.
         *
         * \param that Other filename to construct copy of
         */
        Filename(const Filename& that);

        /**
         * Parses given string into filename.
         */
        Filename(const String& source);

        /**
         * Returns true if given character is filename separator.
         */
        static bool IsSeparator(rune r);

        /**
         * Copies contents of another filename into this one.
         */
        Filename& Assign(const Filename& that);

        /**
         * Parses given string into filename and replaces this filenames data
         * with it.
         */
        Filename& Assign(const String& source);

        /**
         * Assignment operator.
         */
        inline Filename& operator=(const Filename& that)
        {
            return Assign(that);
        }

        /**
         * Assignment operator.
         */
        inline Filename& operator=(const String& source)
        {
            return Assign(source);
        }

        /**
         * Returns the full filename.
         */
        inline const String& GetFullName() const
        {
            return m_full_name;
        }

        /**
         * Returns string representing the (local or global) root of the
         * filename. If filename does not have a root, empty string is
         * returned instead.
         */
        inline const String& GetRoot() const
        {
            return m_root;
        }

        /**
         * Returns logical parent of the filename.
         */
        Filename GetParent() const;

        /**
         * Returns a string representing final part of the filename, or empty
         * string is filename does not have any.
         */
        inline String GetName() const
        {
            return m_parts.empty() ? String() : m_parts[m_parts.size() - 1];
        }

        /**
         * Returns file extension from the filename, or empty string if the
         * filename does not have any extension.
         */
        String GetExtension() const;

        /**
         * Returns all components of filename in an vector.
         */
        std::vector<String> GetParts() const;

        /**
         * Returns true if the filename is invalid, e.g. completely empty.
         */
        inline bool IsEmpty() const
        {
            return m_full_name.IsEmpty();
        }

        /**
         * Returns true if the filename is absolute. Absolute filenames have
         * somekind of root part.
         */
        inline bool IsAbsolute() const
        {
            return !m_root.IsEmpty();
        }

        /**
         * Returns true if filename exists in the file system.
         */
        bool Exists() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a directory.
         */
        bool IsDir() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a regular file.
         */
        bool IsFile() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to symbolic link.
         */
        bool IsSymlink() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to Unix socket.
         */
        bool IsSocket() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a FIFO.
         */
        bool IsFifo() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a character device.
         */
        bool IsCharDevice() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a block device.
         */
        bool IsBlockDevice() const;

        /**
         * Returns size of the file in bytes, or 0 if the file does not exist
         * or it's empty.
         */
        std::size_t GetSize() const;

        /**
         * Returns the date and time when the file was last modified.
         */
        DateTime GetLastModified() const;

        /**
         * Passes filename into std::fopen and returns resulting file handle.
         *
         * \param mode Open mode, see manual page for std::fopen for more
         *             details
         * \return     Pointer to file handle which can be used to read/write
         *             contents of the file, or NULL if file cannot be opened
         *             for some reason
         */
        FILE* Open(const String& mode) const;

        /**
         * Returns true if filename is equal with another filename.
         */
        bool Equals(const Filename& that) const;

        /**
         * Equality testing operator.
         */
        inline bool operator==(const Filename& that) const
        {
            return Equals(that);
        }

        /**
         * Non-equality testing operator.
         */
        inline bool operator!=(const Filename& that) const
        {
            return !Equals(that);
        }

        /**
         * Compares two filenames lexicographically against each other.
         */
        int Compare(const Filename& that) const;

        /**
         * Comparison operator.
         */
        inline bool operator<(const Filename& that) const
        {
            return Compare(that) < 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>(const Filename& that) const
        {
            return Compare(that) > 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator<=(const Filename& that) const
        {
            return Compare(that) <= 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>=(const Filename& that) const
        {
            return Compare(that) >= 0;
        }

        /**
         * Appends given string into the end of the filename and returns
         * result.
         */
        Filename Concat(const String& string) const;

        /**
         * Concatenation operator.
         */
        inline Filename operator+(const String& string) const
        {
            return Concat(string);
        }

#if !defined(_WIN32)
    private:
        bool Stat() const;
#endif

    private:
        String m_full_name;
        String m_root;
        std::vector<String> m_parts;
#if !defined(_WIN32)
        mutable enum
        {
            STAT_UNINITIALIZED,
            STAT_SUCCEEDED,
            STAT_FAILED
        } m_stat_state;
        mutable struct ::stat m_stat;
#endif
    };
}

#endif /* !TEMPEARLY_CORE_FILENAME_H_GUARD */
