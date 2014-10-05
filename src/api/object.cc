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

    TEMPEARLY_NATIVE_METHOD(obj_cmp)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (self.IsInstance(interpreter, operand.GetClass(interpreter)))
        {
            if (operand.Is(Value::KIND_OBJECT)
                && self.Is(Value::KIND_OBJECT)
                && self.AsObject() == operand.AsObject())
            {
                return Value::NewInt(0);
            } else {
                return Value::NewInt(-1);
            }
        } else {
            interpreter->Throw(interpreter->eTypeError,
                               "Values are not comparable");

            return Value();
        }
    }

    TEMPEARLY_NATIVE_METHOD(obj_eq)
    {
        Value compare = args[0].Call(interpreter,
                                     "__cmp__",
                                     std::vector<Value>(1, args[1]));

        if (compare)
        {
            return Value::NewBool(compare.IsInt() && compare.AsInt() == 0);
        } else {
            return Value();
        }
    }

    TEMPEARLY_NATIVE_METHOD(obj_lt)
    {
        Value compare = args[0].Call(interpreter,
                                     "__cmp__",
                                     std::vector<Value>(1, args[1]));

        if (compare)
        {
            return Value::NewBool(compare.IsInt() && compare.AsInt() < 0);
        } else {
            return Value();
        }
    }

    TEMPEARLY_NATIVE_METHOD(obj_gt)
    {
        Value compare = args[0].Call(interpreter,
                                     "__cmp__",
                                     std::vector<Value>(1, args[1]));

        if (compare)
        {
            return Value::NewBool(compare.IsInt() && compare.AsInt() > 0);
        } else {
            return Value();
        }
    }

    TEMPEARLY_NATIVE_METHOD(obj_lte)
    {
        Value compare = args[0].Call(interpreter,
                                     "__cmp__",
                                     std::vector<Value>(1, args[1]));

        if (compare)
        {
            return Value::NewBool(compare.IsInt() && compare.AsInt() <= 0);
        } else {
            return Value();
        }
    }

    TEMPEARLY_NATIVE_METHOD(obj_gte)
    {
        Value compare = args[0].Call(interpreter,
                                     "__cmp__",
                                     std::vector<Value>(1, args[1]));

        if (compare)
        {
            return Value::NewBool(compare.IsInt() && compare.AsInt() >= 0);
        } else {
            return Value();
        }
    }

    void init_object(Interpreter* i)
    {
        i->cObject = i->AddClass("Object", Handle<Class>());

        i->cObject->AddMethod(i, "__bool__", 0, obj_bool);
        i->cObject->AddMethod(i, "__str__", 0, obj_str);

        // Comparison operators.
        i->cObject->AddMethod(i, "__cmp__", 1, obj_cmp);
        i->cObject->AddMethod(i, "__eq__", 1, obj_eq);
        i->cObject->AddMethod(i, "__lt__", 1, obj_lt);
        i->cObject->AddMethod(i, "__gt__", 1, obj_gt);
        i->cObject->AddMethod(i, "__lte__", 1, obj_lte);
        i->cObject->AddMethod(i, "__gte__", 1, obj_gte);
    }
}
