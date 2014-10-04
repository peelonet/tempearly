#include "interpreter.h"

namespace tempearly
{
    void init_bool(Interpreter* interpreter)
    {
        interpreter->cBool = interpreter->AddClass("Bool", interpreter->cObject);
    }
}
