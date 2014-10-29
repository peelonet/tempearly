#ifndef TEMPEARLY_IO_STREAM_H_GUARD
#define TEMPEARLY_IO_STREAM_H_GUARD

#include "core/string.h"

namespace tempearly
{
    class Stream : public CountedObject
    {
    public:
        explicit Stream();

        /**
         * Returns true if the stream has an error message.
         */
        inline bool HasError() const
        {
            return !m_error_message.IsEmpty();
        }

        inline const String& GetErrorMessage() const
        {
            return m_error_message;
        }

        inline void SetErrorMessage(const String& error_message)
        {
            m_error_message = error_message;
        }

        virtual bool IsOpen() const = 0;

        virtual bool IsReadable() const = 0;

        virtual bool IsWritable() const = 0;

        virtual void Close() = 0;

        virtual bool ReadData(byte* buffer, std::size_t size, std::size_t& read) = 0;

        bool ReadRune(rune& slot);

        virtual bool WriteData(const byte* data, std::size_t size) = 0;

        bool Write(const char* data, std::size_t size);

        bool Write(const ByteString& data);

        bool Write(const String& text);

        bool Printf(const char* format, ...);

    private:
        String m_error_message;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Stream);
    };
}

#endif /* !TEMPEARLY_IO_STREAM_H_GUARD */
