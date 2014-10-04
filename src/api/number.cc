#include <cmath>

#include "interpreter.h"
#include "utils.h"

namespace tempearly
{
    /**
     * Int#__str__(base = 10) => String
     *
     * Converts integer to string with given optional radix.
     *
     * Throws: ValueError - If radix is not between 2 and 36.
     */
    TEMPEARLY_NATIVE_METHOD(int_str)
    {
        i64 radix = 10;

        if (args.size() > 1)
        {
            if (!args[1].AsInt(interpreter, radix))
            {
                return Value();
            }
            else if (radix < 2 || radix > 36)
            {
                interpreter->Throw(interpreter->eValueError, "Invalid radix");

                return Value();
            }
        }

        return Value::NewString(Utils::ToString(args[0].AsInt(), radix));
    }

    /**
     * Int#__add__(Int)   => Int
     * Int#__add__(Float) => Float
     *
     * Performs addition on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_add)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsFloat())
        {
            const double a = self.AsFloat();
            const double b = operand.AsFloat();

            return Value::NewFloat(a + b);
        } else {
            const i64 a = self.AsInt();
            i64 b;

            if (operand.AsInt(interpreter, b))
            {
                return Value::NewInt(a + b);
            } else {
                return Value();
            }
        }
    }

    /**
     * Int#__sub__(Int)   => Int
     * Int#__sub__(Float) => Float
     *
     * Performs substraction on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_sub)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsFloat())
        {
            const double a = self.AsFloat();
            const double b = operand.AsFloat();

            return Value::NewFloat(a - b);
        } else {
            const i64 a = self.AsInt();
            i64 b;

            if (operand.AsInt(interpreter, b))
            {
                return Value::NewInt(a - b);
            } else {
                return Value();
            }
        }
    }

    /**
     * Int#__mul__(Int)   => Int
     * Int#__mul__(Float) => Float
     *
     * Performs multiplication on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_mul)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsFloat())
        {
            const double a = self.AsFloat();
            const double b = operand.AsFloat();

            return Value::NewFloat(a * b);
        } else {
            const i64 a = self.AsInt();
            i64 b;

            if (operand.AsInt(interpreter, b))
            {
                return Value::NewInt(a * b);
            } else {
                return Value();
            }
        }
    }

    /**
     * Int#__div__(other) => Float
     *
     * Performs division on given numbers.
     *
     * Throws: ZeroDivisionError - If operand is zero.
     */
    TEMPEARLY_NATIVE_METHOD(int_div)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            if (b != 0.0)
            {
                return Value::NewFloat(a / b);
            }
            interpreter->Throw(interpreter->eZeroDivisionError,
                               "Division by zero");
        }

        return Value();
    }

    /**
     * Int#__mod__(other) => Int
     *
     * Performs modulation on two integer numbers.
     *
     * Throws: ZeroDivisionError - If operand is zero.
     */
    TEMPEARLY_NATIVE_METHOD(int_mod)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            if (b == 0)
            {
                interpreter->Throw(interpreter->eZeroDivisionError,
                                   "Division by zero");
            } else {
                return Value::NewInt(a % b);
            }
        }
        
        return Value();
    }

    /**
     * Int#__and__(other) => Int
     *
     * Performs bitwise AND on two integer numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_and)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            return Value::NewInt(a & b);
        } else {
            return Value();
        }
    }

    /**
     * Int#__or__(other) => Int
     *
     * Performs bitwise AND on two integer numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_or)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            return Value::NewInt(a | b);
        } else {
            return Value();
        }
    }

    /**
     * Int#__xor__(other) => Int
     *
     * Performs bitwise exclusive OR on two integer numbers.
     */
    TEMPEARLY_NATIVE_METHOD(int_xor)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            return Value::NewInt(a ^ b);
        } else {
            return Value();
        }
    }

    /**
     * Int#__lsh__(count) => Int
     *
     * Shifts integer left +count+ positions. (right if +count+ is negative)
     */
    TEMPEARLY_NATIVE_METHOD(int_lsh)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            return Value::NewInt(b < 0 ? a >> -b : a << b);
        } else {
            return Value();
        }
    }

    /**
     * Int#__rsh__(count) => Int
     *
     * Shifts integer right +count+ positions. (left if +count+ is negative)
     */
    TEMPEARLY_NATIVE_METHOD(int_rsh)
    {
        const i64 a = args[0].AsInt();
        i64 b;

        if (args[1].AsInt(interpreter, b))
        {
            return Value::NewInt(b < 0 ? a << -b : a >> b);
        } else {
            return Value();
        }
    }

    /**
     * Int#__cmp__(other) => Int
     *
     * Compares two numbers against each other.
     */
    TEMPEARLY_NATIVE_METHOD(int_cmp)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsInt())
        {
            const i64 a = self.AsInt();
            const i64 b = operand.AsInt();

            return Value::NewInt(a > b ? 1 : a < b ? -1 : 0);
        }
        else if (operand.IsFloat())
        {
            const double a = static_cast<double>(self.AsInt());
            const double b = operand.AsFloat();

            return Value::NewInt(a > b ? 1 : a < b ? -1 : 0);
        }

        return Value::NullValue();
    }

    /**
     * Int#__inc__() => Int
     *
     * Increments integer number by one.
     */
    TEMPEARLY_NATIVE_METHOD(int_inc)
    {
        return Value::NewInt(args[0].AsInt() + 1);
    }

    /**
     * Int#__dec__() => Int
     *
     * Decrements integer number by one.
     */
    TEMPEARLY_NATIVE_METHOD(int_dec)
    {
        return Value::NewInt(args[0].AsInt() - 1);
    }

    /**
     * Int#__neg__() => Int
     *
     * Returns receivers value negated.
     */
    TEMPEARLY_NATIVE_METHOD(int_neg)
    {
        return Value::NewInt(-args[0].AsInt());
    }

    /**
     * Int#__invert__() => Int
     *
     * Return integer where each bit is flipped.
     */
    TEMPEARLY_NATIVE_METHOD(int_invert)
    {
        return Value::NewInt(~args[0].AsInt());
    }

    /**
     * Float#__str__() => String
     *
     * Returns textual presentation of the floating point number.
     */
    TEMPEARLY_NATIVE_METHOD(flo_str)
    {
        return Value::NewString(Utils::ToString(args[0].AsFloat()));
    }

    /**
     * Float#__add__(Float) => Float
     *
     * Performs addition on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(flo_add)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            return Value::NewFloat(a + b);
        } else {
            return Value();
        }
    }

    /**
     * Float#__sub__(Float) => Float
     *
     * Performs substraction on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(flo_sub)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            return Value::NewFloat(a - b);
        } else {
            return Value();
        }
    }

    /**
     * Float#__sub__(other) => Float
     *
     * Performs multiplication on given numbers.
     */
    TEMPEARLY_NATIVE_METHOD(flo_mul)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            return Value::NewFloat(a * b);
        } else {
            return Value();
        }
    }

    /**
     * Float#__div__(other) => Float
     *
     * Performs division on given numbers.
     *
     * Throws: ZeroDivisionError - If operand is zero.
     */
    TEMPEARLY_NATIVE_METHOD(flo_div)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            if (b != 0.0)
            {
                return Value::NewFloat(a / b);
            }
            interpreter->Throw(interpreter->eZeroDivisionError,
                               "Float division by zero");
        }

        return Value();
    }

    /**
     * Float#__mod__(other) => Float
     *
     * Performs modulation on given numbers.
     *
     * Throws: ZeroDivisionError - If operand is zero.
     */
    TEMPEARLY_NATIVE_METHOD(flo_mod)
    {
        const double a = args[0].AsFloat();
        double b;

        if (args[1].AsFloat(interpreter, b))
        {
            if (b != 0.0)
            {
                double mod = std::fmod(a, b);
                double div = (a - mod) / b;

                if (b * mod < 0.0)
                {
                    mod += b;
                    div -= 1.0;
                }

                return Value::NewFloat(mod);
            }
            interpreter->Throw(interpreter->eZeroDivisionError,
                               "Float modulo");
        }

        return Value();
    }

    /**
     * Float#__cmp__(other) => Int
     *
     * Compares two numbers against each other.
     */
    TEMPEARLY_NATIVE_METHOD(flo_cmp)
    {
        const Value& operand = args[1];
        const double a = args[0].AsFloat();
        double b;

        if (operand.IsFloat() || operand.IsInt())
        {
            b = operand.AsFloat();
        } else {
            return Value::NullValue();
        }

        return Value::NewInt(a > b ? 1 : a < b ? -1 : 0);
    }

    /**
     * Float#__inc__() => Float
     *
     * Increments float by one.
     */
    TEMPEARLY_NATIVE_METHOD(flo_inc)
    {
        return Value::NewFloat(args[0].AsFloat() + 1.0);
    }

    /**
     * Float#__dec__() => Float
     *
     * Increments float by one.
     */
    TEMPEARLY_NATIVE_METHOD(flo_dec)
    {
        return Value::NewFloat(args[0].AsFloat() - 1.0);
    }

    /**
     * Float#__neg__() => Float
     *
     * Returns receivers value negated.
     */
    TEMPEARLY_NATIVE_METHOD(flo_neg)
    {
        return Value::NewFloat(-args[0].AsFloat());
    }

    void init_number(Interpreter* i)
    {
        i->cNum = i->AddClass("Num", i->cObject);

        i->cInt = i->AddClass("Int", i->cNum);
        i->cInt->AddMethod(i, "__str__", -1, int_str);
        i->cInt->AddMethod(i, "__add__", 1, int_add);
        i->cInt->AddMethod(i, "__sub__", 1, int_sub);
        i->cInt->AddMethod(i, "__mul__", 1, int_mul);
        i->cInt->AddMethod(i, "__div__", 1, int_div);
        i->cInt->AddMethod(i, "__mod__", 1, int_mod);
        i->cInt->AddMethod(i, "__and__", 1, int_and);
        i->cInt->AddMethod(i, "__or__", 1, int_or);
        i->cInt->AddMethod(i, "__xor__", 1, int_xor);
        i->cInt->AddMethod(i, "__lsh__", 1, int_lsh);
        i->cInt->AddMethod(i, "__rsh__", 1, int_rsh);
        i->cInt->AddMethod(i, "__cmp__", 1, int_cmp);
        i->cInt->AddMethod(i, "__inc__", 0, int_inc);
        i->cInt->AddMethod(i, "__dec__", 0, int_dec);
        i->cInt->AddMethod(i, "__neg__", 0, int_neg);
        i->cInt->AddMethod(i, "__invert__", 0, int_invert);

        i->cFloat = i->AddClass("Float", i->cNum);
        i->cFloat->AddMethod(i, "__str__", -1, flo_str);
        i->cFloat->AddMethod(i, "__add__", 1, flo_add);
        i->cFloat->AddMethod(i, "__sub__", 1, flo_sub);
        i->cFloat->AddMethod(i, "__mul__", 1, flo_mul);
        i->cFloat->AddMethod(i, "__div__", 1, flo_div);
        i->cFloat->AddMethod(i, "__mod__", 1, flo_mod);
        i->cFloat->AddMethod(i, "__cmp__", 1, flo_cmp);
        i->cFloat->AddMethod(i, "__inc__", 0, flo_inc);
        i->cFloat->AddMethod(i, "__dec__", 0, flo_dec);
        i->cFloat->AddMethod(i, "__neg__", 0, flo_neg);
    }
}