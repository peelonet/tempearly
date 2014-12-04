#include "interpreter.h"
#include "api/class.h"
#include "core/random.h"

namespace tempearly
{
    /**
     * Bool.parse(string) => Bool
     *
     * Attempts to parse string into boolean value. Values "true", "yes" and
     * "on" are parsed as true, while values "false", "no" and "off" are parsed
     * as false. Anything else is considered to be invalid value. Case ignorant.
     *
     * Throws: ValueError - If invalid value is passed as argument.
     */
    TEMPEARLY_NATIVE_METHOD(bool_s_parse)
    {
        String input;

        if (args[0].AsString(interpreter, input))
        {
            input = input.Trim();
            if (input.EqualsIgnoreCase("true")
                || input.EqualsIgnoreCase("yes")
                || input.EqualsIgnoreCase("on"))
            {
                frame->SetReturnValue(Value::NewBool(true));
                return;
            }
            else if (input.EqualsIgnoreCase("false")
                    || input.EqualsIgnoreCase("no")
                    || input.EqualsIgnoreCase("off"))
            {
                frame->SetReturnValue(Value::NewBool(false));
                return;
            }
            interpreter->Throw(interpreter->eValueError, "Invalid boolean");
        }
    }

    /**
     * Bool.rand() => Bool
     *
     * Returns boolean value generated by the random number generator included
     * with the interpreter.
     */
    TEMPEARLY_NATIVE_METHOD(bool_s_rand)
    {
        frame->SetReturnValue(Value::NewBool(Random::NextBool()));
    }

    /**
     * Bool#__hash__() => Int
     *
     * Returns hash code for boolean value, 1231 for true and 1237 for false.
     */
    TEMPEARLY_NATIVE_METHOD(bool_hash)
    {
        frame->SetReturnValue(Value::NewInt(args[0].AsBool() ? 1231 : 1237));
    }

    /**
     * Bool#__str__() => String
     *
     * Returns string representation of the boolean object, either "true" or
     * "false".
     */
    TEMPEARLY_NATIVE_METHOD(bool_str)
    {
        frame->SetReturnValue(Value::NewString(args[0].AsBool() ? "true" : "false"));
    }

    /**
     * Bool#__and__(object) => Bool
     *
     * Bitwise AND. Returns true if the given object evaluates as true and 
     * boolean value itself is true, otherwise false.
     */
    TEMPEARLY_NATIVE_METHOD(bool_and)
    {
        if (args[0].AsBool())
        {
            bool b;

            if (!args[1].ToBool(interpreter, b))
            {
                return;
            }
            frame->SetReturnValue(b ? args[0] : Value::NewBool(false));
        } else {
            frame->SetReturnValue(Value::NewBool(false));
        }
    }

    /**
     * Bool#__or__(object) => Bool
     *
     * Bitwise OR. Returns true if boolean value itself is true or the object
     * given as argument evaluates as true, otherwise false.
     */
    TEMPEARLY_NATIVE_METHOD(bool_or)
    {
        if (args[0].AsBool())
        {
            frame->SetReturnValue(args[0]);
        } else {
            bool b;

            if (args[1].ToBool(interpreter, b))
            {
                frame->SetReturnValue(Value::NewBool(b));
            }
        }
    }

    /**
     * Bool#__xor__(object) => Bool
     *
     * Bitwise exclusive or. Returns true if the given object evaluates as
     * false and boolean value itself is true, and vice versa.
     */
    TEMPEARLY_NATIVE_METHOD(bool_xor)
    {
        bool b;

        if (args[1].ToBool(interpreter, b))
        {
            if (b)
            {
                frame->SetReturnValue(Value::NewBool(!args[0].AsBool()));
            } else {
                frame->SetReturnValue(args[0]);
            }
        }
    }

    void init_bool(Interpreter* i)
    {
        Handle<Class> cBool = i->AddClass("Bool", i->cObject);

        i->cBool = cBool;

        cBool->SetAllocator(Class::kNoAlloc);

        cBool->AddStaticMethod(i, "parse", 1, bool_s_parse);
        cBool->AddStaticMethod(i, "rand", 0, bool_s_rand);

        cBool->AddMethod(i, "__hash__", 0, bool_hash);

        cBool->AddMethod(i, "__str__", 0, bool_str);

        cBool->AddMethod(i, "__and__", 1, bool_and);
        cBool->AddMethod(i, "__or__", 1, bool_or);
        cBool->AddMethod(i, "__xor__", 1, bool_xor);

        cBool->AddMethodAlias(i, "as_json", "__str__");
    }
}
