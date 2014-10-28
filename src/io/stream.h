#ifndef TEMPEARLY_IO_STREAM_H_GUARD
#define TEMPEARLY_IO_STREAM_H_GUARD

#include "memory.h"

namespace tempearly
{
    class Stream : public CountedObject
    {
    public:
        explicit Stream();

        virtual bool IsOpen() const = 0;

        virtual bool IsReadable() const = 0;

        virtual bool IsWritable() const = 0;

        virtual void Close() = 0;

        virtual bool Read(byte* buffer, std::size_t size, std::size_t& read) = 0;

        virtual bool Write(const byte* data, std::size_t size) = 0;

        bool Write(const ByteString& data);

        bool Write(const String& text);

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Stream);
    };
}

#endif /* !TEMPEARLY_IO_STREAM_H_GUARD */
