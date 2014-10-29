#include "interpreter.h"

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
        String name;
        String old_value;
        bool exists;

        if (!args[1].AsString(interpreter, name))
        {
            return Value();
        }
        exists = interpreter->response->GetHeader(name, old_value);
        if (args.GetSize() > 2)
        {
            String new_value;

            if (!args[2].AsString(interpreter, new_value))
            {
                return Value();
            }
            else if (interpreter->response->IsCommitted())
            {
                interpreter->Throw(interpreter->eStateError,
                                   "Headers are already sent");

                return Value();
            }
            interpreter->response->SetHeader(name, new_value);
        }
        if (exists)
        {
            return Value::NewString(old_value);
        } else {
            return Value::NullValue();
        }
    }

    /**
     * Response#is_committed() => Bool
     *
     * Returns true if headers have already been sent to the client.
     */
    TEMPEARLY_NATIVE_METHOD(res_is_committed)
    {
        return Value::NewBool(interpreter->response->IsCommitted());
    }

    /**
     * Response#redirect(url, permanent = false)
     *
     * Redirects the client to the given URL. If the headers are already sent,
     * StateException is thrown.
     */
    TEMPEARLY_NATIVE_METHOD(res_redirect)
    {
        String location;
        bool permanent = false;

        if (!args[1].AsString(interpreter, location))
        {
            return Value();
        }
        if (args.GetSize() > 2 && !args[2].AsBool(interpreter, permanent))
        {
            return Value();
        }
        if (interpreter->response->IsCommitted())
        {
            interpreter->Throw(interpreter->eStateError,
                               "Headers are already sent");

            return Value();
        }
        interpreter->response->SetStatus(permanent ? 301 : 302);
        interpreter->response->SetHeader("Location", location);
        interpreter->response->Commit();

        return Value::NullValue();
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
        int old_status = interpreter->response->GetStatus();

        if (args.GetSize() > 1)
        {
            i64 new_status;

            if (!args[1].AsInt(interpreter, new_status))
            {
                return Value();
            }
            else if (interpreter->response->IsCommitted())
            {
                interpreter->Throw(interpreter->eStateError,
                                   "Headers are already sent");

                return Value();
            }
            interpreter->response->SetStatus(new_status);
        }

        return Value::NewInt(old_status);
    }

    /**
     * Response#write(string)
     *
     * Writes given string into the body of the response.
     */
    TEMPEARLY_NATIVE_METHOD(res_write)
    {
        String string;

        if (!args[1].AsString(interpreter, string))
        {
            return Value();
        }
        interpreter->response->Write(string);

        return Value::NullValue();
    }

    void init_response(Interpreter* i)
    {
        Handle<Class> cResponse = new Class(i->cObject);
        Handle<Object> instance = new Object(cResponse);

        i->globals->SetVariable("response", Value(instance));

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
