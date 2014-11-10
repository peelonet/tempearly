#ifndef TEMPEARLY_IO_STREAM_H_GUARD
#define TEMPEARLY_IO_STREAM_H_GUARD

#include "core/string.h"

namespace tempearly
{
    /**
     * Abstract class for implementing buffered streams with a line counter.
     */
    class Stream : public CountedObject
    {
    public:
        /** Default buffer size. */
        static const std::size_t kBufferSize;

        /**
         * Constructor for the abstract Stream class.
         *
         * \param buffer_size Size of the builtin buffer
         */
        explicit Stream(std::size_t buffer_size = kBufferSize);

        /**
         * This destructor destroys the buffer but does not close the stream.
         */
        virtual ~Stream();

        /**
         * Returns true if the stream has an error message.
         */
        inline bool HasError() const
        {
            return !m_error_message.IsEmpty();
        }

        /**
         * Returns the error message of the stream, or empty string if this
         * stream has no error.
         */
        inline const String& GetErrorMessage() const
        {
            return m_error_message;
        }

        /**
         * Sets the error message of the stream.
         */
        inline void SetErrorMessage(const String& error_message)
        {
            m_error_message = error_message;
        }

        /**
         * Returns current line number.
         */
        inline int GetLine() const
        {
            return m_line;
        }

        /**
         * Sets the current line number.
         *
         * \param line New line number
         */
        inline void SetLine(int line)
        {
            m_line = line;
        }

        /**
         * Returns <code>true</code> if the stream is open.
         */
        virtual bool IsOpen() const = 0;

        /**
         * Returns <code>true</code> if data can be read from the stream.
         */
        virtual bool IsReadable() const = 0;

        /**
         * Returns <code>true</code> if data can be written into the stream.
         */
        virtual bool IsWritable() const = 0;

        /**
         * Closes the stream.
         */
        virtual void Close() = 0;

        /**
         * Flushes buffer of the stream.
         */
        void Flush();

        /**
         * Reads bytes directly from the stream, bypassing the stream buffer.
         *
         * \param buffer Array where the bytes will be stored into
         * \param size   Number of bytes to read
         * \param read   This is where the number of bytes successfully read
         *               will be stored into
         * \return       A boolean flag indicating whether the operation was
         *               successfull or not
         */
        virtual bool DirectRead(byte* buffer, std::size_t size, std::size_t& read) = 0;

        /**
         * Reads bytes from the stream, using the builtin buffer functionality
         * included within the class. This is the preferred way of input.
         *
         * \param buffer Array where the bytes will be stored into
         * \param size   Number of bytes to read
         * \param read   This is where the number of bytes successfully read
         *               will be stored into
         * \return       A boolean flag indicating whether the operation was
         *               successfull or not
         */
        bool Read(byte* buffer, std::size_t size, std::size_t& read);

        /**
         * Reads a single UTF-8 encoded character from the stream.
         *
         * \param slot Where the resulting Unicode code point will be stored
         *             into
         * \return     A boolean flag indicating whether the operation was
         *             successfull or not
         */
        bool ReadRune(rune& slot);

        /**
         * Writes bytes directly to the stream, bypassing the stream buffer.
         *
         * \param data Array where the bytes are read from
         * \param size Number of bytes to write
         * \return     A boolean flag indicating whether the operation was
         *             successfull or not
         */
        virtual bool DirectWrite(const byte* data, std::size_t size) = 0;

        /**
         * Writes bytes into the stream, using the builtin buffer functionality
         * included within the class. This is the preferred way of output.
         *
         * \param data Array where the bytes are read from
         * \param size Number of bytes to write
         * \return     A boolean flag indicating whether the operation was
         *             successfull or not
         */
        bool Write(const byte* data, std::size_t size);

        /**
         * Outputs bytes from given byte string into the stream.
         *
         * \param data Where the bytes will be read from
         * \return     A boolean flag indicating whether the operation was
         *             successfull or not
         */
        bool Write(const ByteString& data);

        /**
         * Encodes given string into UTF-8 and outputs it into the stream.
         *
         * \param text String which is encoded into UTF-8 and written into the
         *             stream
         * \return     A boolean flag indicating whether the operation was
         *             successfull or not
         */
        bool Write(const String& text);

        /**
         * Outputs formatted text into the stream. See manual page of
         * <code>printf</code> for more information about formatted strings.
         *
         * \return A boolean flag indicating whether the operation was
         *         successfull or not
         */
        bool Printf(const char* format, ...);

    private:
        /** Size of the buffer. */
        const std::size_t m_buffer_size;
        /** Pointer to the actual buffer. */
        byte* m_buffer;
        /** Current offset of the buffer. */
        std::size_t m_offset;
        /** How many bytes are still unread from the buffer. */
        std::size_t m_remain;
        /** Current line number. */
        int m_line;
        /** Error message of the stream. */
        String m_error_message;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Stream);
    };
}

#endif /* !TEMPEARLY_IO_STREAM_H_GUARD */
