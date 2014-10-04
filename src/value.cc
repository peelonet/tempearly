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
                m_data.b = that.m_data.b;
                break;

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

    Value::Value(bool value)
        : m_kind(KIND_BOOL)
    {
        m_data.b = value;
    }

    Value::Value(int value)
        : m_kind(KIND_INT)
    {
        m_data.i = value;
    }

    Value::Value(double value)
        : m_kind(KIND_FLOAT)
    {
        m_data.f = value;
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
                m_data.b = that.m_data.b;
                break;

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

    bool Value::ToBool(const Handle<Interpreter>& interpreter, bool& value) const
    {
        switch (m_kind)
        {
            case KIND_ERROR:
            case KIND_NULL:
                value = false;
                break;

            case KIND_BOOL:
                value = m_data.b;
                break;

            default:
            {
                Value result = Call(interpreter, "__bool__");

                if (result.m_kind == KIND_BOOL)
                {
                    value = result.m_data.b;
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
