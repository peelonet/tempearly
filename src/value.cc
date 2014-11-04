#include <cmath>

#include "interpreter.h"
#include "api/function.h"
#include "core/bytestring.h"

namespace tempearly
{
    Value::Value()
        : m_kind(KIND_ERROR)
        , m_previous(0)
        , m_next(0) {}

    Value::Value(const Value& that)
        : m_kind(that.m_kind)
        , m_previous(0)
        , m_next(0)
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

            case KIND_BINARY:
                m_data.b = new ByteString(*that.m_data.b);
                break;

            case KIND_OBJECT:
                if (that.m_data.o)
                {
                    m_data.o = that.m_data.o;
                    m_data.o->RegisterValue(this);
                } else {
                    m_kind = KIND_NULL;
                }
                break;

            default:
                break;
        }
    }

    Value::Value(const Handle<CoreObject>& object)
        : m_kind(KIND_NULL)
        , m_previous(0)
        , m_next(0)
    {
        if (object)
        {
            m_kind = KIND_OBJECT;
            m_data.o = object.Get();
            m_data.o->RegisterValue(this);
        }
    }

    Value::~Value()
    {
        if (m_kind == KIND_STRING)
        {
            delete m_data.s;
        }
        else if (m_kind == KIND_BINARY)
        {
            delete m_data.b;
        }
        else if (m_kind == KIND_OBJECT)
        {
            if (m_data.o)
            {
                m_data.o->UnregisterValue(this);
            }
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

    Value Value::NewBinary(const ByteString& bytes)
    {
        Value value;

        value.m_kind = KIND_BINARY;
        value.m_data.b = new ByteString(bytes);

        return value;
    }

    Value& Value::operator=(const Value& that)
    {
        Clear();
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

            case KIND_BINARY:
                m_data.b = new ByteString(*that.m_data.b);
                break;

            case KIND_OBJECT:
                if (that.m_data.o)
                {
                    m_data.o = that.m_data.o;
                    m_data.o->RegisterValue(this);
                } else {
                    m_kind = KIND_NULL;
                }
                break;

            default:
                break;
        }

        return *this;
    }

    Value& Value::operator=(const Handle<CoreObject>& object)
    {
        Clear();
        if ((m_data.o = object.Get()))
        {
            m_data.o->RegisterValue(this);
            m_kind = KIND_OBJECT;
        } else {
            m_kind = KIND_NULL;
        }

        return *this;
    }

    bool Value::IsInstance(const Handle<Interpreter>& interpreter, const Handle<Class>& cls) const
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

            case KIND_BINARY:
                return interpreter->cBinary;

            case KIND_OBJECT:
                return m_data.o->GetClass(interpreter);
        }

        return interpreter->cObject;
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
                    value = value.As<FunctionObject>()->Curry(interpreter, Vector<Value>(1, *this));
                }

                return true;
            }
            if (m_kind == KIND_OBJECT && m_data.o->GetAttribute("__getattr__", value) && value.IsFunction())
            {
                return value = value.As<FunctionObject>()->Invoke(interpreter, Vector<Value>(1, NewString(id)));
            }
            else if (cls->GetAttribute("__getattr__", value) && value.IsFunction())
            {
                Vector<Value> args;

                args.Reserve(2);
                args.PushBack(*this);
                args.PushBack(NewString(id));

                return value = value.As<FunctionObject>()->Invoke(interpreter, args);
            }
            interpreter->Throw(interpreter->eAttributeError, "Missing attribute: " + id);

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
                      const Vector<Value>& args) const
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
                if (value.IsStaticMethod())
                {
                    return value.As<FunctionObject>()->Invoke(interpreter, args);
                } else {
                    Vector<Value> new_args(args);

                    new_args.PushFront(*this);
                    if (value.IsFunction())
                    {
                        return value.As<FunctionObject>()->Invoke(interpreter, new_args);
                    } else {
                        return value.Call(interpreter, "__call__", new_args);
                    }
                }
            }
            interpreter->Throw(interpreter->eAttributeError,
                               "Missing attribute: " + id);

            return Value();
        }
    }

    Value Value::Call(const Handle<Interpreter>& interpreter,
                      const String& id,
                      const Value& arg) const
    {
        return Call(interpreter, id, Vector<Value>(1, arg));
    }

    bool Value::Equals(const Handle<Interpreter>& interpreter, const Value& that, bool& slot) const
    {
        Value result = Call(interpreter, "__eq__", that);

        if (!result)
        {
            return false;
        }
        else if (result.IsBool())
        {
            slot = result.AsBool();
        } else {
            slot = false;
        }

        return true;
    }

    bool Value::Compare(const Handle<Interpreter>& interpreter, const Value& that, int& slot) const
    {
        Value result = Call(interpreter, "__cmp__", that);

        if (result)
        {
            if (result.IsNull())
            {
                slot = 0;
            } else {
                i64 i;

                if (!result.AsInt(interpreter, i))
                {
                    return false;
                }
                slot = static_cast<int>(i);
            }

            return true;
        }

        return false;
    }

    bool Value::GetNext(const Handle<Interpreter>& interpreter, Value& slot) const
    {
        Value result = Call(interpreter, "next");

        if (interpreter->HasException())
        {
            Handle<ExceptionObject> exception = interpreter->GetException();

            if (exception->IsInstance(interpreter, interpreter->eStopIteration))
            {
                interpreter->ClearException();
            }

            return false;
        }
        else if (result)
        {
            slot = result;
        } else {
            slot = NullValue();
        }

        return true;
    }

    bool Value::GetHash(const Handle<Interpreter>& interpreter, i64& slot) const
    {
        Value result = Call(interpreter, "__hash__");

        if (result.m_kind == KIND_INT)
        {
            slot = result.m_data.i;

            return true;
        }
        interpreter->Throw(interpreter->eTypeError,
                           "Cannot generate hash code for "
                           + GetClass(interpreter)->GetName());

        return false;
    }

    void Value::Clear()
    {
        if (m_kind == KIND_STRING)
        {
            delete m_data.s;
        }
        else if (m_kind == KIND_OBJECT)
        {
            if (m_data.o)
            {
                m_data.o->UnregisterValue(this);
            }
        }
        m_kind = KIND_ERROR;
    }

    void Value::Mark() const
    {
        if (m_kind == KIND_OBJECT && !m_data.o->IsMarked())
        {
            m_data.o->Mark();
        }
    }

    bool Value::AsBinary(const Handle<Interpreter>& interpreter, ByteString& slot) const
    {
        if (m_kind == KIND_BINARY)
        {
            slot = *m_data.b;

            return true;
        }
        interpreter->Throw(interpreter->eTypeError,
                           "'Binary' required instead of '"
                           + GetClass(interpreter)->GetName()
                           + "'");

        return false;
    }

    bool Value::AsBool(const Handle<Interpreter>& interpreter, bool& slot) const
    {
        if (m_kind == KIND_BOOL)
        {
            slot = m_data.i != 0;

            return true;
        }
        interpreter->Throw(interpreter->eTypeError,
                           "'Bool' required instead of '"
                           + GetClass(interpreter)->GetName()
                           + "'");

        return false;
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
            interpreter->Throw(interpreter->eTypeError,
                               "'Int' required instead of '"
                               + GetClass(interpreter)->GetName()
                               + "'");

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
            interpreter->Throw(interpreter->eTypeError,
                               "'Float' required instead of "
                               + GetClass(interpreter)->GetName()
                               + "'");

            return false;
        }

        return true;
    }

    bool Value::AsString(const Handle<Interpreter>& interpreter, String& slot) const
    {
        if (m_kind == KIND_STRING)
        {
            slot = *m_data.s;

            return true;
        }
        interpreter->Throw(interpreter->eTypeError,
                           "'String' required instead of '"
                           + GetClass(interpreter)->GetName()
                           + "'");

        return false;
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
                string.Clear();
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
