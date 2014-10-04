#include "interpreter.h"

namespace tempearly
{
    /**
     * Void#__str__() => String
     *
     * String representation of "null" is empty string.
     */
    TEMPEARLY_NATIVE_METHOD(void_str)
    {
        return Value::NewString(String());
    }

    void init_void(Interpreter* i)
    {
        i->cVoid = i->AddClass("Void", i->cObject);

        i->cVoid->AddMethod(i, "__str__", 0, void_str);
    }
}
