#include "core/bytestring.h"
#include "core/string.h"
#include "io/stream.h"

namespace tempearly
{
    Stream::Stream() {}

    bool Stream::Write(const ByteString& data)
    {
        return Write(data.GetBytes(), data.GetLength());
    }

    bool Stream::Write(const String& text)
    {
        return Write(text.Encode());
    }
}
