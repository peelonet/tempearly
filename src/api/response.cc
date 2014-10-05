#include "interpreter.h"

namespace tempearly
{
    void init_response(Interpreter* i)
    {
        Handle<Class> cResponse = new Class(i->cObject);
        Handle<Object> instance = new Object(cResponse);

        i->globals->SetVariable("response", Value::NewObject(instance));
    }
}
