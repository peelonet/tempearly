#include "interpreter.h"

namespace tempearly
{
    void init_object(Interpreter* interpreter)
    {
        interpreter->cObject = interpreter->AddClass("Object", Handle<Class>());
    }
}
