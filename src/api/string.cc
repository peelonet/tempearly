#include "interpreter.h"

namespace tempearly
{
    /**
     * String#length() => Int
     *
     * Returns length of the string.
     *
     *     "foo bar".length() #=> 7
     */
    TEMPEARLY_NATIVE_METHOD(str_length)
    {
        return Value::NewInt(args[0].AsString().length());
    }

    /**
     * String#__bool__() => Bool
     *
     * Boolean representation of the string. Strings evaluate as true if they
     * are not empty.
     */
    TEMPEARLY_NATIVE_METHOD(str_bool)
    {
        return Value::NewBool(!args[0].AsString().empty());
    }

    void init_string(Interpreter* i)
    {
        i->cString = i->AddClass("String", i->cObject);

        i->cString->AddMethod(i, "length", 0, str_length);

        // Conversion methods.
        i->cString->AddMethod(i, "__bool__", 0, str_bool);
    }
}
