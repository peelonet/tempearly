#ifndef TEMPEARLY_CORE_BYTESTRING_H_GUARD
#define TEMPEARLY_CORE_BYTESTRING_H_GUARD

#include "memory.h"

namespace tempearly
{
    /**
     * Immutable string like container for binary data. Ensured to be always
     * null terminated to ensure it's with C functions and structs.
     */
    class ByteString
    {
    public:
        /**
         * Constructs empty byte string.
         */
        ByteString();

        /**
         * Constructs copy of existing byte string.
         *
         * \param that Existing byte string to construct copy of
         */
        ByteString(const ByteString& that);

        /**
         * Constructs binary string from C type string.
         *
         * \param input String to copy bytes from
         */
        ByteString(const char* input);

        /**
         * Constructs binary string from array of bytes.
         *
         * \param b Pointer to the array of bytes
         * \param n Size of the array
         */
        ByteString(const byte* b, std::size_t n);

        /**
         * Destructor.
         */
        ~ByteString();

        /**
         * Copies contents from another binary string.
         *
         * \param that Other binary string to copy content from
         */
        ByteString& Assign(const ByteString& that);

        /**
         * Assignment operator.
         */
        inline ByteString& operator=(const ByteString& that)
        {
            return Assign(that);
        }

        /**
         * Returns true if the binary string is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_length;
        }

        /**
         * Returns the number of bytes stored in the binary string.
         */
        inline std::size_t GetLength() const
        {
            return m_length;
        }

        /**
         * Returns reference to the first byte in the string.
         */
        inline byte GetFront() const
        {
            return m_bytes[0];
        }

        /**
         * Returns reference to the last byte in the string.
         */
        inline byte GetBack() const
        {
            return m_bytes[m_length - 1];
        }

        /**
         * Returns byte from specified index.
         */
        inline byte At(std::size_t i) const
        {
            return m_bytes[i];
        }

        /**
         * Returns byte from specified index.
         */
        inline byte operator[](std::size_t i) const
        {
            return m_bytes[i];
        }

        /**
         * Returns pointer to the byte data.
         */
        inline const byte* GetBytes() const
        {
            return m_bytes;
        }

        /**
         * Returns C like string pointer to the byte data.
         */
        inline const char* c_str() const
        {
            return reinterpret_cast<const char*>(m_bytes);
        }

        /**
         * Tests whether contents of two byte strings are equal.
         *
         * \param that Other byte string to compare with
         */
        bool Equals(const ByteString& that) const;

        /**
         * Equality operator.
         */
        inline bool operator==(const ByteString& that) const
        {
            return Equals(that);
        }

        /**
         * Non-equality operator.
         */
        inline bool operator!=(const ByteString& that) const
        {
            return !Equals(that);
        }

        /**
         * Compares contents of two byte strings lexicographically.
         *
         * \param that Other byte string to compare with
         */
        int Compare(const ByteString& that) const;

        /**
         * Comparison operator.
         */
        inline bool operator<(const ByteString& that) const
        {
            return Compare(that) < 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>(const ByteString& that) const
        {
            return Compare(that) > 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator<=(const ByteString& that) const
        {
            return Compare(that) <= 0;
        }

        /**
         * Comparison operator.
         */
        inline bool operator>=(const ByteString& that) const
        {
            return Compare(that) >= 0;
        }

        /**
         * Concatenates contents of two byte strings.
         */
        ByteString Concat(const ByteString& that) const;

        /**
         * Concatenation operator.
         */
        inline ByteString operator+(const ByteString& that) const
        {
            return Concat(that);
        }

    private:
        /** Length of the binary string. */
        std::size_t m_length;
        /** Pointer to the byte data. */
        byte* m_bytes;
        /** Counter used for tracking usage of the data. */
        unsigned int* m_counter;
    };
}

#endif /* !TEMPEARLY_CORE_BYTESTRING_H_GUARD */
