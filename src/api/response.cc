#include "interpreter.h"
#include "core/bytestring.h"

namespace tempearly
{
    /**
     * Response#header(name)        => String
     * Response#header(name, value) => String
     *
     * If only header name is given, searches for an response header with that
     * name and if such header is found in the response, returns it's value as
     * string. Otherwise null is returned.
     *
     * If an optional value is given, sets the appropriate header to the given
     * value and returns old value of the header, if such exists. If headers
     * have already been sent to the client, StateException is thrown.
     */
    TEMPEARLY_NATIVE_METHOD(res_header)
    {
        const Handle<Response> response = interpreter->GetResponse();
        String name;
        String old_value;
        bool exists;

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        exists = response->GetHeader(name, old_value);
        if (args.GetSize() > 2)
        {
            String new_value;

            if (!args[2]->AsString(interpreter, new_value))
            {
                return;
            }
            else if (response->IsCommitted())
            {
                interpreter->Throw(interpreter->eStateError, "Headers are already sent");
                return;
            }
            response->SetHeader(name, new_value);
        }
        if (exists)
        {
            frame->SetReturnValue(Object::NewString(old_value));
        }
    }

    /**
     * Response#is_committed() => Bool
     *
     * Returns true if headers have already been sent to the client.
     */
    TEMPEARLY_NATIVE_METHOD(res_is_committed)
    {
        frame->SetReturnValue(Object::NewBool(interpreter->GetResponse()->IsCommitted()));
    }

    /**
     * Response#redirect(url, permanent = false)
     *
     * Redirects the client to the given URL. If the headers are already sent,
     * StateException is thrown.
     */
    TEMPEARLY_NATIVE_METHOD(res_redirect)
    {
        const Handle<Response> response = interpreter->GetResponse();
        String location;
        bool permanent = false;

        if (!args[1]->AsString(interpreter, location))
        {
            return;
        }
        if (args.GetSize() > 2 && !args[2]->AsBool(interpreter, permanent))
        {
            return;
        }
        if (response->IsCommitted())
        {
            interpreter->Throw(interpreter->eStateError, "Headers are already sent");
            return;
        }
        // TODO: Clear other possibly existing headers.
        response->SetStatus(permanent ? 301 : 302);
        response->SetHeader("Location", location);
        response->Commit();
    }

    /**
     * Response#status()            => Int
     * Response#status(status_code) => Int
     *
     * If called without arguments, returns current response status code.
     *
     * If optional status code is given, sets the response status code to that
     * status code and returns the previously set status code. If headers are
     * already sent, StateException is thrown.
     */
    TEMPEARLY_NATIVE_METHOD(res_status)
    {
        const Handle<Response> response = interpreter->GetResponse();
        const int old_status = response->GetStatus();

        if (args.GetSize() > 1)
        {
            i64 new_status;

            if (!args[1]->AsInt(interpreter, new_status))
            {
                return;
            }
            else if (response->IsCommitted())
            {
                interpreter->Throw(interpreter->eStateError, "Headers are already sent");
                return;
            }
            response->SetStatus(new_status);
        }
        frame->SetReturnValue(Object::NewInt(old_status));
    }

    /**
     * Response#write(object) => Int
     *
     * Takes a string or binary object as a parameter and writes it to the
     * body of the response. Returns the number of bytes written.
     */
    TEMPEARLY_NATIVE_METHOD(res_write)
    {
        ByteString bytes;

        if (args[1]->IsBinary())
        {
            bytes = args[1]->AsBinary();
        }
        else if (args[1]->IsString())
        {
            bytes = args[1]->AsString().Encode();
        } else {
            interpreter->Throw(interpreter->eValueError, "Either string or binary is required");
            return;
        }
        interpreter->GetResponse()->Write(bytes);
        frame->SetReturnValue(Object::NewInt(bytes.GetLength()));
    }

    void init_response(Interpreter* i)
    {
        Handle<Class> cResponse = new Class(i->cStream);
        Handle<Object> instance = new CustomObject(cResponse);

        i->SetGlobalVariable("response", instance);

        cResponse->SetAllocator(Class::kNoAlloc);

        cResponse->AddMethod(i, "header", -2, res_header);
        cResponse->AddMethod(i, "is_committed", 0, res_is_committed);
        cResponse->AddMethod(i, "redirect", -2, res_redirect);
        cResponse->AddMethod(i, "status", -1, res_status);
        cResponse->AddMethod(i, "write", 1, res_write);

        cResponse->AddMethod(i, "__getitem__", 1, res_header);
        cResponse->AddMethod(i, "__setitem__", 2, res_header);
    }
}
