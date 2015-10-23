#include "interpreter.h"
#include "api/iterator.h"
#include "core/bytestring.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    /**
     * Stream#close()
     *
     * Closes the stream. Default implementation does nothing.
     */
    TEMPEARLY_NATIVE_METHOD(stream_close) {}

    /**
     * Stream#read(amount = null) => Binary or String
     *
     * Reads the specified amount of bytes or characters (depends on whether
     * the stream was opened in binary mode or not) and returns them either
     * as a Binary object or a String.
     *
     * If the amount is null, entire contents of the stream are read.
     *
     * If end of input input is reached, null is returned instead.
     *
     * Throws: IOError - If stream is not readable or if any IO related error
     * occurs while reading.
     */
    TEMPEARLY_NATIVE_METHOD(stream_read)
    {
        interpreter->Throw(interpreter->eIOError, "Stream is not readable");
    }

    /**
     * Stream#readline() => Binary or String
     *
     * Reads single line (separated either by \r\n, \n or just \r) from the
     * stream and returns it as an Binary or String object, depending on
     * whether the stream reads binary data or not.
     *
     * If end of input is reached, null is returned instead.
     *
     * Throws: IOError - If stream is not readable or if any IO related error
     * occurs while reading.
     */
    TEMPEARLY_NATIVE_METHOD(stream_readline)
    {
        const Handle<Object>& receiver = args[0];
        Vector<Handle<Object>> one(1, Object::NewInt(1));
        Handle<Object> result;

        if (!receiver->CallMethod(interpreter, result, "read", one))
        {
            return;
        }
        else if (result->IsBinary())
        {
            Vector<byte> buffer;
            ByteString bytes = result->AsBinary();

            while (!bytes.IsEmpty() && bytes.GetBack() != '\n')
            {
                buffer.PushBack(bytes.GetBytes(), bytes.GetLength());
                if (!receiver->CallMethod(interpreter, result, "read", one))
                {
                    return;
                }
                else if (!result->IsBinary())
                {
                    break;
                }
                bytes = result->AsBinary();
            }
            if (!buffer.IsEmpty() && buffer.GetBack() == '\r')
            {
                buffer.Erase(buffer.GetSize() - 1);
            }
            frame->SetReturnValue(Object::NewBinary(ByteString(buffer.GetData(), buffer.GetSize())));
        }
        else if (result->IsString())
        {
            StringBuilder buffer;
            String runes = result->AsString();

            while (!runes.IsEmpty() && runes.GetBack() != '\n')
            {
                buffer.Append(runes);
                if (!receiver->CallMethod(interpreter, result, "read", one))
                {
                    return;
                }
                else if (!result->IsString())
                {
                    break;
                }
                runes = result->AsString();
            }
            if (!buffer.IsEmpty() && buffer.GetBack() == '\r')
            {
                buffer.Erase(buffer.GetLength() - 1);
            }
            frame->SetReturnValue(Object::NewString(buffer.ToString()));
        }
    }

    /**
     * Stream#write(object) => Int
     *
     * Takes either String or Binary object and writes it's contents to the
     * stream. Returns number of bytes written.
     *
     * Throws: IOError - If stream is not writable or if any IO related error
     * occurs while writing.
     */
    TEMPEARLY_NATIVE_METHOD(stream_write)
    {
        interpreter->Throw(interpreter->eIOError, "Stream is not writable");
    }

    /**
     * Stream#print(object...)
     *
     * Converts each object given as arguments into a string with the "__str__"
     * method and writes them into the stream.
     *
     * Throws: IOError - If stream is not writable or if any IO related error
     * occurs while writing.
     */
    TEMPEARLY_NATIVE_METHOD(stream_print)
    {
        String string;

        for (std::size_t i = 1; i < args.GetSize(); ++i)
        {
            if (!args[i]->ToString(interpreter, string))
            {
                return;
            }
            else if (!args[0]->CallMethod(interpreter, "write", Object::NewString(string)))
            {
                return;
            }
        }
    }

    namespace
    {
        class StreamIterator : public IteratorObject
        {
        public:
            explicit StreamIterator(const Handle<Interpreter>& interpreter,
                                    const Handle<Object>& stream)
                : IteratorObject(interpreter->cIterator)
                , m_stream(stream) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_stream)
                {
                    Handle<Object> line;

                    if (!m_stream->CallMethod(interpreter, line, "readline"))
                    {
                        return Result(Result::KIND_ERROR);
                    }
                    else if (!line->IsNull())
                    {
                        return line;
                    }
                    m_stream = nullptr;
                }

                return Result(Result::KIND_BREAK);
            }

            void Mark()
            {
                IteratorObject::Mark();
                if (m_stream && !m_stream->IsMarked())
                {
                    m_stream->Mark();
                }
            }

        private:
            Object* m_stream;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(StreamIterator);
        };
    }

    /**
     * Stream#__iter__() => Iterator
     *
     * Returns an iterator which iterates over each line in the stream, using
     * "readline" method.
     */
    TEMPEARLY_NATIVE_METHOD(stream_iter)
    {
        frame->SetReturnValue(new StreamIterator(interpreter, args[0]));
    }

    void init_stream(Interpreter* i)
    {
        Handle<Class> cStream = i->AddClass("Stream", i->cIterable);

        i->cStream = cStream.Get();

        cStream->AddMethod(i, "close", 0, stream_close);
        cStream->AddMethod(i, "read", -1, stream_read);
        cStream->AddMethod(i, "readline", 0, stream_readline);
        cStream->AddMethod(i, "write", -1, stream_write);

        cStream->AddMethod(i, "print", -1, stream_print);

        cStream->AddMethod(i, "__iter__", 0, stream_iter);

        cStream->AddMethodAlias(i, "<<", "print");
    }
}
