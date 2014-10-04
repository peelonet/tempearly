#include "interpreter.h"

namespace tempearly
{
    /**
     * Bool#__str__() => String
     *
     * Returns string representation of the boolean object, either "true" or
     * "false".
     */
    TEMPEARLY_NATIVE_METHOD(bool_str)
    {
        return Value::NewString(args[0].AsBool() ? "true" : "false");
    }

    void init_bool(Interpreter* i)
    {
        i->cBool = i->AddClass("Bool", i->cObject);

        i->cBool->AddMethod(i, "__str__", 0, bool_str);
    }
}
