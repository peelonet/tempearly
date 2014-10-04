#include <cmath>
#include <sstream>

#include "functionobject.h"
#include "interpreter.h"

namespace tempearly
{
    Value::Value()
        : m_kind(KIND_ERROR) {}

    Value::Value(const Value& that)
        : m_kind(that.m_kind)
    {
        switch (m_kind)
        {
            case KIND_BOOL:
            case KIND_INT:
                m_data.i = that.m_data.i;
                break;

            case KIND_FLOAT:
                m_data.f = that.m_data.f;
                break;

            case KIND_STRING:
                m_data.s = new String(*that.m_data.s);
                break;

            case KIND_OBJECT:
                (m_data.o = that.m_data.o)->IncReferenceCounter();
                break;

            default:
                break;
        }
    }

    Value Value::NewBool(bool b)
    {
        Value value;

        value.m_kind = KIND_BOOL;
        value.m_data.i = b ? 1 : 0;

        return value;
    }

    Value Value::NewInt(i64 number)
    {
        Value value;

        value.m_kind = KIND_INT;
        value.m_data.i = number;

        return value;
    }

    Value Value::NewFloat(double number)
    {
        Value value;

        value.m_kind = KIND_FLOAT;
        value.m_data.f = number;

        return value;
    }

    Value Value::NewString(const String& string)
    {
        Value value;

        value.m_kind = KIND_STRING;
        value.m_data.s = new String(string);

        return value;
    }

    Value Value::NewObject(const Handle<CoreObject>& object)
    {
        Value value;

        if (object)
        {
            value.m_kind = KIND_OBJECT;
            value.m_data.o = object.Get();
            value.m_data.o->IncReferenceCounter();
        } else {
            value.m_kind = KIND_NULL;
        }

        return value;
    }

    Value::~Value()
    {
        if (m_kind == KIND_STRING)
        {
            delete m_data.s;
        }
        else if (m_kind == KIND_OBJECT)
        {
            m_data.o->DecReferenceCounter();
        }
    }

    const Value& Value::NullValue()
    {
        static Value value;

        if (value.m_kind == Value::KIND_ERROR)
        {
            value.m_kind = Value::KIND_NULL;
        }

        return value;
    }

    Value& Value::operator=(const Value& that)
    {
        if (m_kind == KIND_STRING)
        {
            delete m_data.s;
        }
        else if (m_kind == KIND_OBJECT)
        {
            m_data.o->DecReferenceCounter();
        }
        switch (m_kind = that.m_kind)
        {
            case KIND_BOOL:
            case KIND_INT:
                m_data.i = that.m_data.i;
                break;

            case KIND_FLOAT:
                m_data.f = that.m_data.f;
                break;

            case KIND_STRING:
                m_data.s = new String(*that.m_data.s);
                break;

            case KIND_OBJECT:
                (m_data.o = that.m_data.o)->IncReferenceCounter();
                break;

            default:
                break;
        }

        return *this;
    }

    bool Value::IsInstance(const Handle<Interpreter>& interpreter,
                           const Handle<Class>& cls) const
    {
        return GetClass(interpreter)->IsSubclassOf(cls);
    }

    Handle<Class> Value::GetClass(const Handle<Interpreter>& interpreter) const
    {
        switch (m_kind)
        {
            case KIND_ERROR:
                return interpreter->cObject;

            case KIND_NULL:
                return interpreter->cVoid;

            case KIND_BOOL:
                return interpreter->cBool;

            case KIND_INT:
                return interpreter->cInt;

            case KIND_FLOAT:
                return interpreter->cFloat;

            case KIND_STRING:
                return interpreter->cString;

            case KIND_OBJECT:
                return m_data.o->GetClass(interpreter);
        }
    }

    bool Value::HasAttribute(const String& id) const
    {
        return m_kind == KIND_OBJECT && m_data.o->HasAttribute(id);
    }

    bool Value::GetAttribute(const Handle<Interpreter>& interpreter,
                             const String& id,
                             Value& value) const
    {
        if (m_kind == KIND_OBJECT && m_data.o->GetAttribute(id, value))
        {
            return true;
        } else {
            Handle<Class> cls = GetClass(interpreter);

            if (cls->GetAttribute(id, value))
            {
                if (value.IsFunction())
                {
                    std::vector<Value> args;

                    args.push_back(*this);
                    value = value.As<FunctionObject>()->Curry(interpreter, args);
                }

                return true;
            }
            interpreter->Throw(interpreter->eAttributeError,
                               "Missing attribute: " + id);

            return false;
        }
    }

    bool Value::SetAttribute(const String& id, const Value& value) const
    {
        if (m_kind == KIND_OBJECT)
        {
            m_data.o->SetAttribute(id, value);

            return true;
        }

        return false;
    }

    Value Value::Call(const Handle<Interpreter>& interpreter,
                      const String& id,
                      const std::vector<Value>& args) const
    {
        Value value;

        if (m_kind == KIND_OBJECT && m_data.o->GetAttribute(id, value))
        {
            if (value.IsFunction())
            {
                return value.As<FunctionObject>()->Invoke(interpreter, args);
            } else {
                return value.Call(interpreter, "__call__", args);
            }
        } else {
            Handle<Class> cls = GetClass(interpreter);

            if (cls->GetAttribute(id, value))
            {
                std::vector<Value> new_args(args);

                new_args.insert(new_args.begin(), *this);
                if (value.IsFunction())
                {
                    return value.As<FunctionObject>()->Invoke(interpreter, new_args);
                } else {
                    return value.Call(interpreter, "__call__", new_args);
                }
            }
            interpreter->Throw(interpreter->eAttributeError,
                               "Missing attribute: " + id);

            return Value();
        }
    }

    void Value::Mark() const
    {
        if (m_kind == KIND_OBJECT && !m_data.o->IsMarked())
        {
            m_data.o->Mark();
        }
    }

    i64 Value::AsInt() const
    {
        if (m_kind == KIND_INT)
        {
            return m_data.i;
        }
        else if (m_kind == KIND_FLOAT)
        {
            double f = m_data.f;

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

        return 0;
    }

    bool Value::AsInt(const Handle<Interpreter>& interpreter, i64& slot) const
    {
        if (m_kind == KIND_INT)
        {
            slot = m_data.i;
        }
        else if (m_kind == KIND_FLOAT)
        {
            double f = m_data.f;

            if (f > 0.0)
            {
                f = std::floor(f);
            }
            if (f < 0.0)
            {
                f = std::ceil(f);
            }
            slot = static_cast<i64>(f);
        } else {
            std::stringstream ss;

            ss << "'Int' required instead of '"
               << GetClass(interpreter)->GetName()
               << "'";
            interpreter->Throw(interpreter->eTypeError, ss.str());

            return false;
        }

        return true;
    }

    double Value::AsFloat() const
    {
        if (m_kind == KIND_FLOAT)
        {
            return m_data.f;
        }
        else if (m_kind == KIND_INT)
        {
            return static_cast<double>(m_data.i);
        } else {
            return 0;
        }
    }

    bool Value::AsFloat(const Handle<Interpreter>& interpreter, double& slot) const
    {
        if (m_kind == KIND_FLOAT)
        {
            slot = m_data.f;
        }
        else if (m_kind == KIND_INT)
        {
            slot = static_cast<double>(m_data.i);
        } else {
            std::stringstream ss;

            ss << "'Float' required instead of '"
               << GetClass(interpreter)->GetName()
               << "'";
            interpreter->Throw(interpreter->eTypeError, ss.str());

            return false;
        }

        return true;
    }

    bool Value::ToBool(const Handle<Interpreter>& interpreter, bool& value) const
    {
        switch (m_kind)
        {
            case KIND_ERROR:
            case KIND_NULL:
                value = false;
                break;

            case KIND_BOOL:
                value = m_data.i != 0;
                break;

            default:
            {
                Value result = Call(interpreter, "__bool__");

                if (result.m_kind == KIND_BOOL)
                {
                    value = result.m_data.i != 0;
                } else {
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(interpreter->eTypeError,
                                           "Cannot convert into boolean");
                    }

                    return false;
                }
            }
        }

        return true;
    }

    bool Value::ToString(const Handle<Interpreter>& interpreter, String& string) const
    {
        switch (m_kind)
        {
            case KIND_ERROR:
            case KIND_NULL:
                string.clear();
                break;

            case KIND_STRING:
                string = *m_data.s;
                break;

            default:
            {
                Value result = Call(interpreter, "__str__");

                if (result.m_kind == KIND_STRING)
                {
                    string = *result.m_data.s;
                } else {
                    if (!interpreter->HasException())
                    {
                        interpreter->Throw(interpreter->eTypeError,
                                           "Cannot convert into string");
                    }

                    return false;
                }
            }
        }

        return true;
    }
}
