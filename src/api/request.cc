#include "interpreter.h"
#include "core/bytestring.h"
#include "api/list.h"
#include "api/set.h"
#include "io/stream.h"
#include "json/parser.h"

namespace tempearly
{
    /**
     * Request#method() => String
     *
     * Returns HTTP request method (GET, POST, etc.).
     */
    TEMPEARLY_NATIVE_METHOD(req_method)
    {
        frame->SetReturnValue(Object::NewString(HttpMethod::ToString(interpreter->GetRequest()->GetMethod())));
    }

    /**
     * Request#path() => String
     *
     * Returns the path which the client requested.
     */
    TEMPEARLY_NATIVE_METHOD(req_path)
    {
        frame->SetReturnValue(Object::NewString(interpreter->GetRequest()->GetPath()));
    }

    /**
     * Request#content_type() => String
     *
     * Returns value of the Content-Type header sent with the request, or null
     * if no such header was included with the request.
     */
    TEMPEARLY_NATIVE_METHOD(req_content_type)
    {
        const String content_type = interpreter->GetRequest()->GetContentType();

        if (!content_type.IsEmpty())
        {
            frame->SetReturnValue(Object::NewString(content_type));
        }
    }

    /**
     * Request#is_get() => Bool
     *
     * Returns true if request method is GET.
     */
    TEMPEARLY_NATIVE_METHOD(req_is_get)
    {
        frame->SetReturnValue(Object::NewBool(interpreter->GetRequest()->GetMethod() == HttpMethod::GET));
    }

    /**
     * Request#is_post() => Bool
     *
     * Returns true if request method is POST.
     */
    TEMPEARLY_NATIVE_METHOD(req_is_post)
    {
        frame->SetReturnValue(Object::NewBool(interpreter->GetRequest()->GetMethod() == HttpMethod::POST));
    }

    /**
     * Request#is_secure() => Bool
     *
     * Returns true if the request was made through a secure channel such as
     * HTTPS.
     */
    TEMPEARLY_NATIVE_METHOD(req_is_secure)
    {
        frame->SetReturnValue(Object::NewBool(interpreter->GetRequest()->IsSecure()));
    }

    /**
     * Request#is_ajax() => Bool
     *
     * Returns true if the request was made with XMLHttpRequest or not. Notice
     * that this might not be reliable as some CGI environments might not
     * support this method.
     */
    TEMPEARLY_NATIVE_METHOD(req_is_ajax)
    {
        frame->SetReturnValue(Object::NewBool(interpreter->GetRequest()->IsAjax()));
    }

    /**
     * Request#body() => Binary
     *
     * Returns body of the request as binary data or null if the request did
     * not contain a body.
     */
    TEMPEARLY_NATIVE_METHOD(req_body)
    {
        const ByteString body = interpreter->GetRequest()->GetBody();

        if (!body.IsEmpty())
        {
            frame->SetReturnValue(Object::NewBinary(body));
        }
    }

    /**
     * Request#json() => Object
     *
     * Returns JSON data sent with the request as an object.
     *
     * Throws: ValueError - If no JSON data was sent with the request or it's
     * malformed.
     */
    TEMPEARLY_NATIVE_METHOD(req_json)
    {
        const ByteString body = interpreter->GetRequest()->GetBody();
        Handle<Stream> stream;
        Handle<JsonParser> parser;
        Handle<Object> value;

        if (body.IsEmpty())
        {
            interpreter->Throw(interpreter->eValueError, "No JSON object could be decoded");
            return;
        }
        stream = body.AsStream();
        parser = new JsonParser(stream);
        if (!parser->ParseValue(interpreter, value))
        {
            if (!interpreter->HasException())
            {
                interpreter->Throw(interpreter->eValueError, "No JSON object could be decoded");
            }
            return;
        }
        frame->SetReturnValue(value);
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

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        else if (interpreter->GetRequest()->GetParameter(name, value))
        {
            frame->SetReturnValue(Object::NewString(value));
        }
    }

    /**
     * Request#int(name, default = 0) => Int
     *
     * Returns value of request parameter identified by given name as an
     * integer number, or default value if no request parameter exist with
     * that name or it's value cannot be parsed as integer number.
     */
    TEMPEARLY_NATIVE_METHOD(req_int)
    {
        String name;
        String value;

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        else if (interpreter->GetRequest()->GetParameter(name, value))
        {
            i64 number;

            if (value.ParseInt(number, 10))
            {
                frame->SetReturnValue(Object::NewInt(number));
                return;
            }
        }
        if (args.GetSize() > 2)
        {
            frame->SetReturnValue(args[2]);
        } else {
            frame->SetReturnValue(Object::NewInt(0));
        }
    }

    /**
     * Request#float(name, default = 0.0) => Float
     *
     * Returns value of request parameter identified by given name as an
     * floating point decimal number, or default value if no request parameter
     * exist with that name or it's value cannot be parsed as floating point
     * decimal number.
     */
    TEMPEARLY_NATIVE_METHOD(req_float)
    {
        String name;
        String value;

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        else if (interpreter->GetRequest()->GetParameter(name, value))
        {
            double number;

            if (value.ParseDouble(number))
            {
                frame->SetReturnValue(Object::NewFloat(number));
                return;
            }
        }
        if (args.GetSize() > 2)
        {
            frame->SetReturnValue(args[2]);
        } else {
            frame->SetReturnValue(Object::NewFloat(0.0));
        }
    }

    /**
     * Request#list(name) => List
     *
     * Returns all request parameters with given name in a list.
     */
    TEMPEARLY_NATIVE_METHOD(req_list)
    {
        Handle<ListObject> list;
        Vector<String> values;
        String name;

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        list = new ListObject(interpreter->cList);
        if (interpreter->GetRequest()->GetAllParameters(name, values))
        {
            for (std::size_t i = 0; i < values.GetSize(); ++i)
            {
                list->Append(Object::NewString(values[i]));
            }
        }
        frame->SetReturnValue(list);
    }

    /**
     * Request#set(name) => Set
     *
     * Returns all request parameters with given name in a set.
     */
    TEMPEARLY_NATIVE_METHOD(req_set)
    {
        Handle<SetObject> set;
        Vector<String> values;
        String name;

        if (!args[1]->AsString(interpreter, name))
        {
            return;
        }
        set = new SetObject(interpreter->cSet);
        if (interpreter->GetRequest()->GetAllParameters(name, values))
        {
            for (std::size_t i = 0; i < values.GetSize(); ++i)
            {
                const Handle<Object> value = Object::NewString(values[i]);
                i64 hash;

                if (!value->GetHash(interpreter, hash))
                {
                    return;
                }
                set->Add(hash, value);
            }
        }
        frame->SetReturnValue(set);
    }

    void init_request(Interpreter* i)
    {
        Handle<Class> cRequest = new Class(i->cObject);
        Handle<Object> instance = new CustomObject(cRequest);

        i->SetGlobalVariable("request", instance);

        cRequest->SetAllocator(Class::kNoAlloc);

        cRequest->AddMethod(i, "method", 0, req_method);
        cRequest->AddMethod(i, "path", 0, req_path);
        cRequest->AddMethod(i, "content_type", 0, req_content_type);
        cRequest->AddMethod(i, "is_get", 0, req_is_get);
        cRequest->AddMethod(i, "is_post", 0, req_is_post);
        cRequest->AddMethod(i, "is_secure", 0, req_is_secure);
        cRequest->AddMethod(i, "is_ajax", 0, req_is_ajax);
        cRequest->AddMethod(i, "body", 0, req_body);
        cRequest->AddMethod(i, "json", 0, req_json);

        cRequest->AddMethod(i, "__getitem__", 1, req_getitem);
        cRequest->AddMethod(i, "int", -2, req_int);
        cRequest->AddMethod(i, "float", -2, req_float);
        cRequest->AddMethod(i, "list", 1, req_list);
        cRequest->AddMethod(i, "set", 1, req_set);
    }
}
