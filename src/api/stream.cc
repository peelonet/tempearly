#include "interpreter.h"

namespace tempearly
{
    /**
     * Stream#close()
     *
     * Closes the stream. Default implementation does nothing.
     */
    TEMPEARLY_NATIVE_METHOD(stream_close)
    {
        return Value::NullValue();
    }

    /**
     * Stream#read(amount = null) => Binary or String
     *
     * Reads the specified amount of bytes or characters (depends on whether
     * the stream was opened in binary mode or not) and returns them either
     * as a Binary object or a String.
     *
     * If the amount is null, entire contents of the stream are read.
     *
     * Throws: IOError - If stream is not readable or if any IO related error
     * occurs while reading.
     */
    TEMPEARLY_NATIVE_METHOD(stream_read)
    {
        interpreter->Throw(interpreter->eIOError, "Stream is not readable");

        return Value();
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

        return Value();
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
            if (!args[i].ToString(interpreter, string))
            {
                return Value();
            }
            else if (!args[0].Call(interpreter, "write", Vector<Value>(1, Value::NewString(string))))
            {
                return Value();
            }
        }

        return Value::NullValue();
    }

    void init_stream(Interpreter* i)
    {
        Handle<Class> cStream = i->AddClass("Stream", i->cObject);

        i->cStream = cStream.Get();

        cStream->AddMethod(i, "close", 0, stream_close);
        cStream->AddMethod(i, "read", -1, stream_read);
        cStream->AddMethod(i, "write", -1, stream_write);

        cStream->AddMethod(i, "print", -1, stream_print);

        cStream->AddMethodAlias(i, "<<", "print");
    }
}
