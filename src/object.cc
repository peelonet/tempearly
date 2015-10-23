#include "api/class.h"
#include "api/function.h"
#include "core/bytestring.h"
#include "interpreter.h"

#include <cmath>

namespace tempearly
{
    Object::Object() {}

    Object::~Object() {}

    bool Object::IsInstance(const Handle<Interpreter>& interpreter,
                            const Handle<Class>& cls) const
    {
        return GetClass(interpreter)->IsSubclassOf(cls);
    }

    bool Object::GetAttribute(const Handle<Interpreter>& interpreter,
                              const String& name,
                              Handle<Object>& slot)
    {
        if (GetOwnAttribute(name, slot))
        {
            return true;
        }

        const Handle<Class> cls = GetClass(interpreter);

        if (cls->GetAttribute(interpreter, name, slot))
        {
            if (slot->IsUnboundMethod())
            {
                slot = slot.As<FunctionObject>()->Curry(
                    interpreter,
                    Vector<Handle<Object>>(1, this)
                );
            }

            return true;
        }
        else if (cls->GetOwnAttribute("__getattr__", slot)
                && slot->IsFunction())
        {
            Vector<Handle<Object>> args;

            args.Reserve(2);
            args.PushBack(this);
            args.PushBack(NewString(name));

            return slot.As<FunctionObject>()->Invoke(interpreter, slot, args);
        }
        interpreter->Throw(
            interpreter->eAttributeError,
            "Missing attribute: " + name
        );

        return false;
    }

    bool Object::CallMethod(const Handle<Interpreter>& interpreter,
                            Handle<Object>& slot,
                            const String& method_name,
                            const Vector<Handle<Object>>& args)
    {
        Handle<Object> function;

        if (GetAttribute(interpreter, method_name, function))
        {
            if (function->IsUnboundMethod())
            {
                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    slot,
                    Vector<Handle<Object>>(1, this) + args
                );
            }
            else if (function->IsFunction())
            {
                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    slot,
                    args
                );
            }
            else if (function->GetAttribute(interpreter, "__call__", function))
            {
                if (!function->IsFunction())
                {
                    return true;
                }

                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    slot,
                    args
                );
            }
        }
        if (!interpreter->HasException())
        {
            interpreter->Throw(
                interpreter->eTypeError,
                "Instance is not callable"
            );
        }

        return false;
    }

    bool Object::CallMethod(const Handle<Interpreter>& interpreter,
                            Handle<Object>& slot,
                            const String& method_name,
                            const Handle<Object>& arg)
    {
        return CallMethod(
            interpreter,
            slot,
            method_name,
            Vector<Handle<Object>>(1, arg)
        );
    }

    bool Object::CallMethod(const Handle<Interpreter>& interpreter,
                            const String& method_name,
                            const Vector<Handle<Object>>& args)
    {
        Handle<Object> function;

        if (GetAttribute(interpreter, method_name, function))
        {
            if (function->IsUnboundMethod())
            {
                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    Vector<Handle<Object>>(1, this) + args
                );
            }
            else if (function->IsFunction())
            {
                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    args
                );
            }
            else if (function->GetAttribute(interpreter, "__call__", function))
            {
                if (!function->IsFunction())
                {
                    return true;
                }

                return function.As<FunctionObject>()->Invoke(
                    interpreter,
                    args
                );
            }
        }
        if (!interpreter->HasException())
        {
            interpreter->Throw(
                interpreter->eTypeError,
                "Instance is not callable"
            );
        }

        return false;
    }

    bool Object::CallMethod(const Handle<Interpreter>& interpreter,
                            const String& method_name,
                            const Handle<Object>& arg)
    {
        return CallMethod(
            interpreter,
            method_name,
            Vector<Handle<Object>>(1, arg)
        );
    }

    bool Object::Equals(const Handle<Interpreter>& interpreter,
                        const Handle<Object>& that,
                        bool& slot)
    {
        Handle<Object> result;

        if (!CallMethod(interpreter, result, "__eq__", that))
        {
            return false;
        }
        slot = result->IsBool() ? result->AsBool() : false;

        return true;
    }

    bool Object::IsLessThan(const Handle<Interpreter>& interpreter,
                            const Handle<Object>& that,
                            bool& slot)
    {
        Handle<Object> result;

        if (!CallMethod(interpreter, result, "__lt__", that))
        {
            return false;
        }
        slot = result->IsBool() ? result->AsBool() : false;

        return true;
    }

    bool Object::GetNext(const Handle<Interpreter>& interpreter,
                         Handle<Object>& slot)
    {
        if (CallMethod(interpreter, slot, "next"))
        {
            return true;
        }
        else if (interpreter->HasException(interpreter->eStopIteration))
        {
            interpreter->ClearException();
        }

        return false;
    }

    bool Object::GetHash(const Handle<Interpreter>& interpreter, i64& slot)
    {
        Handle<Object> result;

        if (CallMethod(interpreter, result, "__hash__"))
        {
            if (result->IsInt())
            {
                slot = result->AsInt();

                return true;
            }
            interpreter->Throw(
                interpreter->eTypeError,
                "Cannot generate hash code for '"
                + GetClass(interpreter)->GetName()
                + "'"
            );
        }

        return false;
    }

    ByteString Object::AsBinary() const
    {
        return ByteString();
    }

    bool Object::AsBinary(const Handle<Interpreter>& interpreter,
                          ByteString& slot) const
    {
        if (IsBinary())
        {
            slot = AsBinary();

            return true;
        }
        interpreter->Throw(
            interpreter->eTypeError,
            "'Binary' required instead of '"
            + GetClass(interpreter)->GetName()
            + "'"
        );

        return false;
    }

    bool Object::AsBool() const
    {
        return true;
    }

    bool Object::AsBool(const Handle<Interpreter>& interpreter,
                        bool& slot) const
    {
        if (IsBool())
        {
            slot = AsBool();

            return true;
        }
        interpreter->Throw(
            interpreter->eTypeError,
            "'Bool' required instead of '"
            + GetClass(interpreter)->GetName()
            + "'"
        );

        return false;
    }

    double Object::AsFloat() const
    {
        return 0.0;
    }

    bool Object::AsFloat(const Handle<Interpreter>& interpreter,
                         double& slot) const
    {
        if (IsFloat() || IsInt())
        {
            slot = AsFloat();

            return true;
        }
        interpreter->Throw(
            interpreter->eTypeError,
            "'Float' required instead of '"
            + GetClass(interpreter)->GetName()
            + "'"
        );

        return false;
    }

    i64 Object::AsInt() const
    {
        return 0;
    }

    bool Object::AsInt(const Handle<Interpreter>& interpreter, i64& slot) const
    {
        if (IsInt() || IsFloat())
        {
            slot = AsInt();

            return true;
        }
        interpreter->Throw(
            interpreter->eTypeError,
            "'Int' required instead of '"
            + GetClass(interpreter)->GetName()
            + "'"
        );

        return false;
    }

    String Object::AsString() const
    {
        return String();
    }

    bool Object::AsString(const Handle<Interpreter>& interpreter,
                          String& slot) const
    {
        if (IsString())
        {
            slot = AsString();

            return true;
        }
        interpreter->Throw(
            interpreter->eTypeError,
            "'String' required instead of '"
            + GetClass(interpreter)->GetName()
            + "'"
        );

        return false;
    }

    bool Object::ToBool(const Handle<Interpreter>& interpreter,
                        bool& slot)
    {
        if (IsBool())
        {
            slot = AsBool();
        }
        else if (IsNull())
        {
            slot = false;
        } else {
            Handle<Object> result;

            if (!CallMethod(interpreter, result, "__bool__"))
            {
                return false;
            }
            else if (result->IsBool())
            {
                slot = result->AsBool();
            } else {
                interpreter->Throw(
                    interpreter->eTypeError,
                    "Cannot convert into boolean"
                );

                return false;
            }
        }

        return true;
    }

    bool Object::ToString(const Handle<Interpreter>& interpreter, String& slot)
    {
        if (IsString())
        {
            slot = AsString();
        }
        else if (IsNull())
        {
            slot.Clear();
        } else {
            Handle<Object> result;

            if (!CallMethod(interpreter, result, "__str__"))
            {
                return false;
            }
            else if (!result->IsString())
            {
                interpreter->Throw(
                    interpreter->eTypeError,
                    "Cannot convert into string"
                );

                return false;
            } else {
                slot = result->AsString();
            }
        }

        return true;
    }

    namespace
    {
        class PrimaryObject : public Object
        {
        public:
            explicit PrimaryObject() {}

            Dictionary<Handle<Object>> GetOwnAttributes() const
            {
                return Dictionary<Handle<Object>>();
            }

            bool GetOwnAttribute(const String&, Handle<Object>&) const
            {
                return false;
            }

            bool SetOwnAttribute(const String&, const Handle<Object>&)
            {
                return false;
            }

        private:
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(PrimaryObject);
        };

        class NullObject : public PrimaryObject
        {
        public:
            explicit NullObject() {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cVoid;
            }

            bool IsNull() const
            {
                return true;
            }

        private:
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(NullObject);
        };

        class BoolObject : public PrimaryObject
        {
        public:
            explicit BoolObject(bool value)
                : m_value(value) {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cBool;
            }

            bool IsBool() const
            {
                return true;
            }

            bool AsBool() const
            {
                return m_value;
            }

        private:
            const bool m_value;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(BoolObject);
        };

        class IntObject : public PrimaryObject
        {
        public:
            explicit IntObject(i64 value)
                : m_value(value) {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cInt;
            }

            double AsFloat() const
            {
                return m_value;
            }

            i64 AsInt() const
            {
                return m_value;
            }

            bool IsInt() const
            {
                return true;
            }

        private:
            const i64 m_value;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IntObject);
        };

        class FloatObject : public PrimaryObject
        {
        public:
            explicit FloatObject(double value)
                : m_value(value) {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cFloat;
            }

            double AsFloat() const
            {
                return m_value;
            }

            i64 AsInt() const
            {
                double f = m_value;

                if (f > 0.0)
                {
                    f = std::floor(f);
                }
                if (f < 0.0)
                {
                    f = std::ceil(f);
                }
                
                return static_cast<i64>(f);
            }

            bool IsFloat() const
            {
                return true;
            }

        private:
            const double m_value;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FloatObject);
        };

        class StringObject : public PrimaryObject
        {
        public:
            explicit StringObject(const String& value)
                : m_value(value) {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cString;
            }

            bool IsString() const
            {
                return true;
            }

            String AsString() const
            {
                return m_value;
            }

        private:
            const String m_value;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(StringObject);
        };

        class BinaryObject : public PrimaryObject
        {
        public:
            explicit BinaryObject(const ByteString& value)
                : m_value(value) {}

            Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const
            {
                return interpreter->cBinary;
            }

            bool IsBinary() const
            {
                return true;
            }

            ByteString AsBinary() const
            {
                return m_value;
            }

        private:
            const ByteString m_value;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(BinaryObject);
        };
    }

    Handle<Object> Object::NewNull()
    {
        static Handle<Object> instance;

        if (!instance)
        {
            instance = new NullObject();
        }

        return instance;
    }

    Handle<Object> Object::NewBool(bool value)
    {
        static Handle<Object> true_instance;
        static Handle<Object> false_instance;

        if (value)
        {
            if (!true_instance)
            {
                true_instance = new BoolObject(true);
            }

            return true_instance;
        }
        else if (!false_instance)
        {
            false_instance = new BoolObject(false);
        }

        return false_instance;
    }

    Handle<Object> Object::NewInt(i64 value)
    {
        return new IntObject(value);
    }

    Handle<Object> Object::NewFloat(double value)
    {
        return new FloatObject(value);
    }

    Handle<Object> Object::NewString(const String& value)
    {
        return new StringObject(value);
    }

    Handle<Object> Object::NewBinary(const ByteString& value)
    {
        return new BinaryObject(value);
    }
}
