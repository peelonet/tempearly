#include "interpreter.h"
#include "api/function.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    Class::Class(const Handle<Class>& base)
        : m_base(base.Get())
        , m_allocator(m_base ? m_base->m_allocator : 0)
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
                          MethodCallback callback)
    {
        Value method = Value::NewObject(new Method(
            interpreter->cFunction,
            this,
            arity,
            callback
        ));

        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(name, method);
    }

    namespace
    {
        class StaticMethod : public FunctionObject
        {
        public:
            explicit StaticMethod(const Handle<Class>& cls,
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

                // Test that we have correct amount of arguments.
                if (m_arity < 0)
                {
                    if (args.size() < static_cast<unsigned>(-(m_arity + 1)))
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
                else if (args.size() != static_cast<unsigned>(m_arity))
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

            bool IsStaticMethod() const
            {
                return true;
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
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(StaticMethod);
        };
    }

    void Class::AddStaticMethod(const Handle<Interpreter>& interpreter,
                                const String& name,
                                int arity,
                                MethodCallback callback)
    {
        Value method = Value::NewObject(new StaticMethod(
            interpreter->cFunction,
            this,
            arity,
            callback
        ));

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

    /**
     * Class#alloc() => Object
     *
     * Allocates new instance of the class. Returned object must be instance of
     * the class.
     *
     * Throws: TypeError - If new instance cannot be allocated for some reason.
     */
    TEMPEARLY_NATIVE_METHOD(class_alloc)
    {
        Handle<Class> cls = args[0].As<Class>();
        Class::Allocator allocator = cls->GetAllocator();
        Handle<Object> instance;

        if (allocator)
        {
            if (!(instance = allocator(interpreter, cls)))
            {
                interpreter->Throw(interpreter->eTypeError,
                                   "Cannot allocate instance of "
                                   + cls->GetName());

                return Value();
            }
        } else {
            instance = new Object(cls);
        }

        return Value::NewObject(instance);
    }

    /**
     * Class#__call__(arguments...) => Object
     *
     * Allocates new instance of the class and initializes it with the given
     * arguments.
     *
     * Throws: TypeError - If new instance cannot be allocated for some reason.
     */
    TEMPEARLY_NATIVE_METHOD(class_call)
    {
        Value instance = args[0].Call(interpreter, "alloc");

        if (instance)
        {
            std::vector<Value> new_args(args.begin() + 1, args.end());

            if (instance.Call(interpreter, "__init__", new_args))
            {
                return instance;
            }
        }

        return Value();
    }

    /**
     * Class#__str__() => String
     *
     * Returns textual description of the class which is usually name of the
     * class.
     */
    TEMPEARLY_NATIVE_METHOD(class_str)
    {
        Value name;

        if (args[0].As<Class>()->GetAttribute("__name__", name) && name.IsString())
        {
            return name;
        } else {
            return Value::NewString("<anonymous type>");
        }
    }

    void init_class(Interpreter* i)
    {
        i->cClass = i->AddClass("Class", i->cObject);

        i->cClass->AddMethod(i, "alloc", 0, class_alloc);

        i->cClass->AddMethod(i, "__call__", -1, class_call);

        // Conversion methods
        i->cClass->AddMethod(i, "__str__", 0, class_str);
    }
}
