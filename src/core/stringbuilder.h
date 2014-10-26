#ifndef TEMPEARLY_CORE_STRINGBUILDER_H_GUARD
#define TEMPEARLY_CORE_STRINGBUILDER_H_GUARD

#include "core/string.h"

namespace tempearly
{
    /**
     * String builder is a helper class which can be used to construct Unicode
     * strings.
     */
    class StringBuilder
    {
    public:
        /**
         * Constructs empty string builder.
         *
         * \param capacity Initial capacity of the string builder
         */
        StringBuilder(std::size_t capacity = 32);

        /**
         * Copy constructor.
         *
         * \param that Other string builder to construct copy of
         */
        StringBuilder(const StringBuilder& that);

        /**
         * Constructs string builder from a string.
         */
        StringBuilder(const String& string);

        /**
         * Copies content from another string builder.
         *
         * \param that Other string builder to copy content from
         */
        StringBuilder& operator=(const StringBuilder& that);

        /**
         * Replaces contents of the string builder with contents of a string.
         *
         * \param s String to copy content from
         */
        StringBuilder& operator=(const String& s);

        /**
         * Replaces contents of the string builder with a single character.
         *
         * \param c Character to be the sole contents of the string builder
         */
        StringBuilder& operator=(rune c);

        /**
         * Destructor.
         */
        ~StringBuilder();

        /**
         * Returns true if the string builder does not contain any content.
         */
        inline bool IsEmpty() const
        {
            return !m_length;
        }

        /**
         * Returns current length of the string builder.
         */
        inline std::size_t GetLength() const
        {
            return m_length;
        }

        /**
         * Returns reference to the first character in the string builder.
         */
        inline rune& GetFront()
        {
            return m_runes[0];
        }

        /**
         * Returns reference to the first character in the string builder.
         */
        inline const rune& GetFront() const
        {
            return m_runes[0];
        }

        /**
         * Returns reference to the last character in the string builder.
         */
        inline rune& GetBack()
        {
            return m_runes[m_length - 1];
        }

        /**
         * Returns reference to the last character in the string builder.
         */
        inline const rune& GetBack() const
        {
            return m_runes[m_length - 1];
        }

        /**
         * Returns reference to a character from specified index.
         */
        inline rune& At(std::size_t i)
        {
            return m_runes[i];
        }

        /**
         * Returns reference to a character from specified index.
         */
        inline const rune& At(std::size_t i) const
        {
            return m_runes[i];
        }

        /**
         * Returns reference to a character from specified index.
         */
        inline rune& operator[](std::size_t i)
        {
            return m_runes[i];
        }

        /**
         * Returns reference to a character from specified index.
         */
        inline const rune& operator[](std::size_t i) const
        {
            return m_runes[i];
        }

        /**
         * Removes all content from the string builder.
         */
        void Clear();

        /**
         * Makes sure that the string builder has capacity for at least
         * <i>n</i> characters.
         */
        void Reserve(std::size_t n);

        StringBuilder& Assign(std::size_t n, rune r);

        /**
         * Appends given content to the end of the buffer.
         */
        void Append(rune c);

        /**
         * Appends given content to the end of the buffer.
         *
         * \param c Pointer to array of characters
         * \param n Size of the array
         */
        void Append(const rune* c, std::size_t n);

        /**
         * Appends given content to the end of the buffer.
         */
        void Append(const String& s);

        /**
         * Appends given content to the end of the buffer.
         */
        inline StringBuilder& operator<<(rune c)
        {
            Append(c);

            return *this;
        }

        /**
         * Appends given content to the end of the buffer.
         */
        inline StringBuilder& operator<<(const String& s)
        {
            Append(s);

            return *this;
        }

        /**
         * Prepends given content to the beginning of the buffer.
         */
        void Prepend(rune c);

        /*
         * Prepends given content to the beginning of the buffer.
         *
         * \param c Pointer to array of characters
         * \param n Size of the array
         */
        void Prepend(const rune* c, std::size_t n);

        /**
         * Prepends given content to the beginning of the buffer.
         */
        void Prepend(const String& s);

        /**
         * Removes and returns first character from the string builder.
         */
        rune PopFront();

        /**
         * Removes and returns last character from the string builder.
         */
        rune PopBack();

        /**
         * Removes character from the specified offset.
         */
        void Erase(std::size_t i);

        /**
         * Removes last character from the buffer and assigns it to the given
         * slot.
         */
        inline StringBuilder& operator>>(rune& c)
        {
            c = PopBack();

            return *this;
        }

        /**
         * Constructs string from contents of the string builder.
         */
        inline String ToString() const
        {
            return String(m_runes, m_length);
        }

    private:
        /** Current capacity of the string builder. */
        std::size_t m_capacity;
        /** Number of characters stored in the string builder. */
        std::size_t m_length;
        /** Pointer to the character data. */
        rune* m_runes;
    };
}

#endif /* !TEMPEARLY_CORE_STRINGBUILDER_H_GUARD */
