#include "interpreter.h"
#include "api/function.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    Class::Class(const Handle<Class>& base)
        : m_base(base.Get())
        , m_attributes(0) {}

    Class::~Class()
    {
        if (m_attributes)
        {
            delete m_attributes;
        }
    }

    Handle<Class> Class::GetClass(const Handle<Interpreter>& interpreter) const
    {
        return interpreter->cClass;
    }

    String Class::GetName() const
    {
        Value value;

        if (GetAttribute("__name__", value) && value.IsString())
        {
            return value.AsString();
        } else {
            return "<anonymous class>";
        }
    }

    bool Class::IsSubclassOf(const Handle<Class>& that) const
    {
        if (this == that)
        {
            return true;
        } else {
            return m_base && m_base->IsSubclassOf(that);
        }
    }

    bool Class::HasAttribute(const String& id) const
    {
        if (m_attributes && m_attributes->Find(id))
        {
            return true;
        } else {
            return m_base && m_base->HasAttribute(id);
        }
    }

    bool Class::GetAttribute(const String& id, Value& value) const
    {
        if (m_attributes)
        {
            const AttributeMap::Entry* entry = m_attributes->Find(id);

            if (entry)
            {
                value = entry->value;

                return true;
            }
        }
        if (m_base)
        {
            return m_base->GetAttribute(id, value);
        } else {
            return false;
        }
    }

    void Class::SetAttribute(const String& id, const Value& value)
    {
        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(id, value);
    }

    namespace
    {
        class Method : public FunctionObject
        {
        public:
            explicit Method(const Handle<Class>& cls,
                            const Handle<Class>& declaring_class,
                            int arity,
                            Callback callback)
                : FunctionObject(cls)
                , m_declaring_class(declaring_class.Get())
                , m_arity(arity)
                , m_callback(callback) {}

            Value Invoke(const Handle<Interpreter>& interpreter,
                         const std::vector<Value>& args)
            {
                Value result;

                // Arguments must not be empty.
                if (args.empty())
                {
                    interpreter->Throw(interpreter->eTypeError,
                                       "Missing method receiver");

                    return Value();
                }
                // Test that the first argument is correct type.
                else if (!args[0].IsInstance(interpreter, m_declaring_class))
                {
                    StringBuilder sb;

                    sb << "Method requires a '"
                       << m_declaring_class->GetName()
                       << "' object but received a '"
                       << args[0].GetClass(interpreter)->GetName();
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());

                    return Value();
                }
                // Test that we have correct amount of arguments.
                else if (m_arity < 0)
                {
                    if (args.size() < static_cast<unsigned>(-(m_arity + 1) + 1))
                    {
                        StringBuilder sb;

                        sb << "Method expected at least "
                           << (-(m_arity) - 1)
                           << " arguments, got "
                           << args.size();
                        interpreter->Throw(interpreter->eTypeError, sb.ToString());

                        return Value();
                    }
                }
                else if (args.size() != static_cast<unsigned>(m_arity) + 1)
                {
                    StringBuilder sb;

                    sb << "Method expected "
                       << m_arity
                       << " arguments, got "
                       << args.size();
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());

                    return Value();
                }
                result = m_callback(interpreter, args);

                return result;
            }

            void Mark()
            {
                FunctionObject::Mark();
                if (!m_declaring_class->IsMarked())
                {
                    m_declaring_class->Mark();
                }
            }

        private:
            Class* m_declaring_class;
            const int m_arity;
            const Callback m_callback;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Method);
        };
    }

    void Class::AddMethod(const Handle<Interpreter>& interpreter,
                          const String& name,
                          int arity,
                          Value (*callback)(const Handle<Interpreter>&,
                                            const std::vector<Value>&))
    {
        Value method = Value::NewObject(new Method(interpreter->cFunction,
                                                   this,
                                                   arity,
                                                   callback));

        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(name, method);
    }

    void Class::Mark()
    {
        CountedObject::Mark();
        if (m_base && !m_base->IsMarked())
        {
            m_base->Mark();
        }
        if (m_attributes)
        {
            for (const AttributeMap::Entry* e = m_attributes->GetFront(); e; e = e->next)
            {
                e->value.Mark();
            }
        }
    }
}
