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
         * Constructs empty filename.
         */
        Filename();

        /**
         * Constructs copy of existing filename.
         */
        Filename(const Filename& that);

        /**
         * Parses given string into filename.
         */
        Filename(const String& source);

        /**
         * Assignment operator.
         */
        Filename& operator=(const Filename& that);

        /**
         * Assignment operator which replaces filename data with filename
         * parsed from the given string.
         */
        Filename& operator=(const String& source);

        /**
         * Returns extension from filename or empty string if filename does not
         * have an extension.
         */
        String GetExtension() const;

        /**
         * Returns size of the file in bytes, or 0 if the file does not exist
         * or it's empty.
         */
        std::size_t GetSize() const;

        /**
         * Returns true if given character is filename separator.
         */
        static bool IsSeparator(rune r);

        /**
         * Returns true if filename is empty.
         */
        bool IsEmpty() const;

        /**
         * Returns true if filename is absolute.
         */
        bool IsAbsolute() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a regular file.
         */
        bool IsFile() const;

        /**
         * Returns true if filename exists in the file system and is pointing
         * to a directory.
         */
        bool IsDir() const;

        /**
         * Returns true if filename exists in the file system.
         */
        bool Exists() const;

        /**
         * Passes filename into std::fopen and returns resulting file handle.
         *
         * \param mode Open mode, see manual page for std::fopen for more
         *             details
         * \return     Pointer to file handle which can be used to read/write
         *             contents of the file, or NULL if file cannot be opened
         *             for some reason
         */
        FILE* Open(const char* mode) const;

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
        void Stat() const;
#endif

    private:
        String m_filename;
        String m_root;
        std::vector<String> m_path;
#if !defined(_WIN32)
        mutable struct ::stat m_stat;
        mutable bool m_stat_done;
        mutable bool m_stat_succeeded;
#endif
    };
}

#endif /* !TEMPEARLY_CORE_FILENAME_H_GUARD */
