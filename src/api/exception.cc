#include "interpreter.h"

namespace tempearly
{
    ExceptionObject::ExceptionObject(const Handle<Class>& cls)
        : Object(cls) {}

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

    static Handle<CoreObject> ex_alloc(const Handle<Interpreter>& interpreter,
                                       const Handle<Class>& cls)
    {
        return new ExceptionObject(cls);
    }

    void init_exception(Interpreter* i)
    {
        i->cException = i->AddClass("Exception", i->cObject);

        i->cException->SetAllocator(ex_alloc);

        i->eAttributeError = i->AddClass("AttributeError", i->cException);
        i->eNameError = i->AddClass("NameError", i->cException);
        i->eStateError = i->AddClass("StateError", i->cException);
        i->eStopIteration = i->AddClass("StopIteration", i->cException);
        i->eSyntaxError = i->AddClass("SyntaxError", i->cException);
        i->eTypeError = i->AddClass("TypeError", i->cException);
        i->eValueError = i->AddClass("ValueError", i->cException);

        i->eArithmeticError = i->AddClass("ArithmeticError", i->cException);
        i->eZeroDivisionError = i->AddClass("ZeroDivisionError", i->eArithmeticError);
    }
}
