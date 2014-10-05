#include "interpreter.h"

namespace tempearly
{
    /**
     * Request#method() => String
     *
     * Returns HTTP request method (GET, POST, etc.).
     */
    TEMPEARLY_NATIVE_METHOD(req_method)
    {
        return Value::NewString(interpreter->request->GetMethod());
    }

    /**
     * Request#__getitem__(name) => String
     *
     * Returns request parameter with given name or null if no such parameter
     * was given with the request.
     */
    TEMPEARLY_NATIVE_METHOD(req_getitem)
    {
        String name;
        String value;

        if (!args[1].AsString(interpreter, name))
        {
            return Value();
        }
        else if (interpreter->request->GetParameter(name, value))
        {
            return Value::NewString(value);
        } else {
            return Value::NullValue();
        }
    }

    void init_request(Interpreter* i)
    {
        Handle<Class> cRequest = new Class(i->cObject);
        Handle<Object> instance = new Object(cRequest);

        i->globals->SetVariable("request", Value::NewObject(instance));

        cRequest->AddMethod(i, "method", 0, req_method);

        cRequest->AddMethod(i, "__getitem__", 1, req_getitem);
    }
}
