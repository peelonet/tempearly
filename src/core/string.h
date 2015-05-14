#ifndef TEMPEARLY_CORE_STRING_H_GUARD
#define TEMPEARLY_CORE_STRING_H_GUARD

#if defined(_WIN32)
# include <string> // For std::wstring
#endif

#include "memory.h"

namespace tempearly
{
    /**
     * Implementation of (mostly) immutable Unicode string. Character data is
     * stored internally as Unicode code points and encoded into UTF-8 when
     * required.
     */
    class String
    {
    public:
        /** Indicates no position. */
        static const std::size_t npos;

        /**
         * Constructs empty string.
         */
        String();

        /**
         * Constructs copy of existing string.
         */
        String(const String& that);

        /**
         * Constructs string from C string literal. The input is expected to be
         * in UTF-8.
         */
        String(const char* input);

        /**
         * Constructs string from array of characters.
         *
         * \param c Pointer to the array
         * \param n Size of the array
         */
        String(const rune* c, std::size_t n);

        /**
         * Constructs string from <i>n</i> copies of given character.
         */
        String(rune c, std::size_t n);

        /**
         * Destructor.
         */
        ~String();

        /**
         * Constructs string from bytes which are expected to be in US-ASCII
         * character encoding.
         *
         * \param input  The bytes to decode, must be null terminated
         */
        static String DecodeAscii(const byte* input);

        /**
         * Constructs string from bytes which are expected to be in US-ASCII
         * character encoding.
         *
         * \param input  The bytes to decode
         * \param length Number of bytes contained in the array
         */
        static String DecodeAscii(const byte* input, std::size_t length);

        /**
         * Digitizes given unsigned 64 bit integer into string.
         *
         * \param number Number to convert
         * \param radix  Radix of the conversion
         */
        static String FromU64(u64 number, int radix = 10);

        /**
         * Digitizes given signed 64 bit integer into string.
         *
         * \param number Number to convert
         * \param radix  Radix of the conversion
         */
        static String FromI64(i64 number, int radix = 10);

        /**
         * Digitizes given double precision number into string.
         *
         * \param number Number to convert
         */
        static String FromDouble(double number);

        /**
         * Copies contents of another string into this one.
         */
        String& Assign(const String& that);

        /**
         * Replaces contents of the string with content copied from C string
         * literal. The input is expected to be in UTF-8.
         */
        String& Assign(const char* input);

        /**
         * Assignment operator.
         */
        inline String& operator=(const String& that)
        {
            return Assign(that);
        }

        /**
         * Assignment operator.
         */
        inline String& operator=(const char* input)
        {
            return Assign(input);
        }

        /**
         * Returns true if the string is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_length;
        }

        /**
         * Returns length of the string.
         */
        inline std::size_t GetLength() const
        {
            return m_length;
        }

        /**
         * Returns pointer to the character data. If the string is empty, this
         * could be NULL.
         */
        inline const rune* GetRunes() const
        {
            return m_runes ? m_runes + m_offset : 0;
        }

        /**
         * Returns first character of the string. No boundary testing is
         * performed.
         */
        inline rune GetFront() const
        {
            return m_runes[m_offset];
        }

        /**
         * Returns last character of the string. No boundary testing is
         * performed.
         */
        inline rune GetBack() const
        {
            return m_runes[m_offset + m_length - 1];
        }

        /**
         * Returns character from specified index. No boundary testing is
         * performed.
         */
        inline rune At(std::size_t i) const
        {
            return m_runes[m_offset + i];
        }

        /**
         * Returns character from specified index. No boundary testing is
         * performed.
         */
        inline rune operator[](std::size_t i) const
        {
            return m_runes[m_offset + i];
        }

        /**
         * Returns UTF-8 encoded form of the string.
         */
        ByteString Encode() const;

#if defined(_WIN32)
        /**
         * Returns UTF-16LE version of the string which is suitable to be used
         * for Windows API calls.
         */
        std::wstring Widen() const;

        /**
         * Narrows UTF-16LE encoded wide char string into Unicode string.
         */
        static String Narrow(const wchar_t* input);
#endif

        /**
         * Calculates hash code for the string. This is usually done only once,
         * since the resulting hash code is cached for performance reasons.
         */
        std::size_t HashCode() const;

        /**
         * Searches for index of the given rune in the string and returns it if
         * found. Otherwise, npos is returned.
         */
        std::size_t IndexOf(rune r, std::size_t pos = 0) const;

        /**
         * Same as IndexOf, but searching is done in reversed order.
         */
        std::size_t LastIndexOf(rune r, std::size_t pos = npos) const;

        /**
         * Returns portion of the string.
         */
        String SubString(std::size_t pos = 0, std::size_t count = npos) const;

        /**
         * Strips whitespace from beginning and end of the string.
         */
        String Trim() const;

        /**
         * Tests whether contents of two strings are equal.
         *
         * \param that Other string to compare with
         */
        bool Equals(const String& that) const;

        /**
         * Tests whether contents of two strings are equal ignoring the
         * character case.
         *
         * \param that Other string to compare with
         */
        bool EqualsIgnoreCase(const String& that) const;

        /**
         * Equality testing operator.
         */
        inline bool operator==(const String& that) const
        {
            return Equals(that);
        }

        /**
         * Non-equality testing operator.
         */
        inline bool operator!=(const String& that) const
        {
            return !Equals(that);
        }

        /**
         * Compares contents of two strings lexicographically.
         *
         * \param that Other string to compare with
         */
        int Compare(const String& that) const;

        /**
         * Compares contents of two strings lexicographically while ignoring
         * the character case.
         *
         * \param that Other string to compare with
         */
        int CompareIgnoreCase(const String& that) const;

        /**
         * Comparison operator.
         */
        inline bool operator<(const String& that) const
        {
            return Compare(that) < 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>(const String& that) const
        {
            return Compare(that) > 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator<=(const String& that) const
        {
            return Compare(that) <= 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>=(const String& that) const
        {
            return Compare(that) >= 0;
        }

        /**
         * Returns true if this strings starts with given substring.
         */
        bool StartsWith(const String& that) const;

        /**
         * Clears all contents of the string.
         */
        void Clear();

        /**
         * Concatenates contents of two strings and returns result.
         */
        String Concat(const String& that) const;

        /**
         * String concatenation operator.
         */
        inline String operator+(const String& that) const
        {
            return Concat(that);
        }

        /**
         * Returns true if every rune in the string matches with given
         * callback.
         *
         * Empty strings return false.
         */
        bool Matches(bool (*callback)(rune)) const;

        /**
         * Constructs new string from contents of the string with given
         * callback.
         */
        String Map(rune (*callback)(rune)) const;

        /**
         * Returns true if the string is valid Tempearly identifier.
         */
        bool IsIdentifier() const;

        /**
         * Tests for lower case letter.
         */
        static bool IsLower(rune c);

        /**
         * Tests for upper case letter.
         */
        static bool IsUpper(rune c);

        /**
         * Tests for control character.
         */
        static bool IsControl(rune c);

        /**
         * Converts given character into lower case.
         */
        static rune ToLower(rune c);

        /**
         * Converts given character into upper case.
         */
        static rune ToUpper(rune c);

        /**
         * Escapes XML entities from the string and returns result.
         */
        String EscapeXml() const;

        /**
         * Escapes control sequences and other illegal JavaScript characters
         * from the string and returns result. Resulting string can be safely
         * used in JavaScript or JSON string literal.
         */
        String EscapeJavaScript() const;

        /**
         * Parses an 64-bit integer value from contents of the string.
         *
         * \param slot  Variable where the result is stored
         * \param radix The radix to use, for example if you want to parse
         *              a hexadecimal number, set this to 16. If you want
         *              this method to determine the readix, leave it to -1
         * \return      True if the conversion was successfull, false if
         *              underflow or overflow error occurred
         */
        bool ParseInt(i64& slot, int radix = -1) const;

        /**
         * Parses an double precision decimal value from contents of the
         * string.
         *
         * \param slot Variable where the result is stored
         * \return     True if the conversion was successfull, false if
         *             underflow or overflow error occurred
         */
        bool ParseDouble(double& slot) const;

    private:
        /** Offset where the string contents begin. */
        std::size_t m_offset;
        /** Length of the string. */
        std::size_t m_length;
        /** Pointer to the character data. */
        rune* m_runes;
        /** Counter used for tracking usage of the data. */
        unsigned int* m_counter;
        /** Cached hash code of the string. */
        mutable std::size_t m_hash_code;
    };

    String operator+(const char* a, const String& b);
}

#endif /* !TEMPEARLY_CORE_STRING_H_GUARD */
