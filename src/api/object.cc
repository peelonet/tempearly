#include "interpreter.h"

namespace tempearly
{
    /**
     * Object#__bool__() => Bool
     *
     * Boolean representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_bool)
    {
        const Value& self = args[0];

        if (self.IsBool())
        {
            return self;
        } else {
            return Value::NewBool(!self.IsNull());
        }
    }

    /**
     * Object#__str__() => String
     *
     * String representation of the object.
     */
    TEMPEARLY_NATIVE_METHOD(obj_str)
    {
        const Value& self = args[0];

        if (self.IsString())
        {
            return self;
        } else {
            return Value::NewString("<object>");
        }
    }

    void init_object(Interpreter* i)
    {
        i->cObject = i->AddClass("Object", Handle<Class>());

        i->cObject->AddMethod(i, "__bool__", 0, obj_bool);
        i->cObject->AddMethod(i, "__str__", 0, obj_str);
    }
}
