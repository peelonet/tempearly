#include "interpreter.h"
#include "api/exception.h"

namespace tempearly
{
    ExceptionObject::ExceptionObject(const Handle<Class>& cls, const Handle<Frame>& frame)
        : Object(cls)
        , m_frame(frame.Get()) {}

    String ExceptionObject::GetMessage() const
    {
        Value message;

        if (GetAttribute("message", message) && message.IsString())
        {
            return message.AsString();
        } else {
            return String();
        }
    }

    void ExceptionObject::Mark()
    {
        Object::Mark();
        if (m_frame && !m_frame->IsMarked())
        {
            m_frame->Mark();
        }
    }

    /**
     * Exception#__init__(message = null)
     *
     * Constructs exception with given error message.
     *
     * Throws: TypeError - If anything else than String or null is given as
     * argument.
     */
    TEMPEARLY_NATIVE_METHOD(ex_init)
    {
        if (args.GetSize() == 2)
        {
            const Value& message = args[1];

            if (message.Is(Value::KIND_STRING))
            {
                args[0].SetAttribute("message", args[1]);
            }
            else if (!message.Is(Value::KIND_NULL))
            {
                interpreter->Throw(interpreter->eTypeError, "String required");

                return Value();
            }
        }
        else if (args.GetSize() > 2)
        {
            interpreter->Throw(interpreter->eValueError, "Too many arguments");

            return Value();
        }

        return Value::NullValue();
    }

    static Handle<CoreObject> ex_alloc(const Handle<Interpreter>& interpreter,
                                       const Handle<Class>& cls)
    {
        return new ExceptionObject(cls, interpreter->GetFrame());
    }

    void init_exception(Interpreter* i)
    {
        i->cException = i->AddClass("Exception", i->cObject);

        i->cException->SetAllocator(ex_alloc);
        i->cException->AddMethod(i, "__init__", -1, ex_init);

        i->eAttributeError = i->AddClass("AttributeError", i->cException);
        i->eIOError = i->AddClass("IOError", i->cException);
        i->eNameError = i->AddClass("NameError", i->cException);
        i->eStateError = i->AddClass("StateError", i->cException);
        i->eStopIteration = i->AddClass("StopIteration", i->cException);
        i->eSyntaxError = i->AddClass("SyntaxError", i->cException);
        i->eTypeError = i->AddClass("TypeError", i->cException);
        i->eValueError = i->AddClass("ValueError", i->cException);

        i->eArithmeticError = i->AddClass("ArithmeticError", i->cException);
        i->eZeroDivisionError = i->AddClass("ZeroDivisionError", i->eArithmeticError);

        i->eLookupError = i->AddClass("LookupError", i->cException);
        i->eIndexError = i->AddClass("IndexError", i->eLookupError);
        i->eKeyError = i->AddClass("KeyError", i->eLookupError);
    }
}
