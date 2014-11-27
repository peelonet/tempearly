#include "interpreter.h"
#include "utils.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    static Handle<CoreObject> no_alloc(const Handle<Interpreter>&, const Handle<Class>&);

    const Class::Allocator Class::kNoAlloc = no_alloc;

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

    Dictionary<Value> Class::GetAllAttributes() const
    {
        if (m_attributes)
        {
            return *m_attributes;
        } else {
            return Dictionary<Value>();
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
                value = entry->GetValue();

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
            explicit Method(const Handle<Interpreter>& interpreter,
                            const Handle<Class>& declaring_class,
                            int arity,
                            Callback callback)
                : FunctionObject(interpreter)
                , m_declaring_class(declaring_class.Get())
                , m_arity(arity)
                , m_callback(callback) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                Handle<Frame> frame;
                Value result;

                // Arguments must not be empty.
                if (args.IsEmpty())
                {
                    interpreter->Throw(interpreter->eTypeError,
                                       "Missing method receiver");

                    return Value();
                }
                frame = interpreter->PushFrame(Handle<Frame>(), this, args[0], args.SubVector(1));
                // Test that the first argument is correct type.
                if (!args[0].IsInstance(interpreter, m_declaring_class))
                {
                    StringBuilder sb;

                    sb << "Method requires a '"
                       << m_declaring_class->GetName()
                       << "' object but received a '"
                       << args[0].GetClass(interpreter)->GetName();
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());
                    interpreter->PopFrame();

                    return Value();
                }
                // Test that we have correct amount of arguments.
                else if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1) + 1))
                    {
                        StringBuilder sb;

                        sb << "Method expected at least "
                           << Utils::ToString(static_cast<u64>(-(m_arity) - 1))
                           << " arguments, got "
                           << Utils::ToString(static_cast<u64>(args.GetSize()));
                        interpreter->Throw(interpreter->eTypeError, sb.ToString());
                        interpreter->PopFrame();

                        return Value();
                    }
                }
                else if (args.GetSize() != static_cast<unsigned>(m_arity) + 1)
                {
                    StringBuilder sb;

                    sb << "Method expected "
                       << Utils::ToString(static_cast<u64>(m_arity))
                       << " arguments, got "
                       << Utils::ToString(static_cast<u64>(args.GetSize()));
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());
                    interpreter->PopFrame();

                    return Value();
                }
                result = m_callback(interpreter, args);
                interpreter->PopFrame();

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
        Value method = Value(new Method(interpreter, this, arity, callback));

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
            explicit StaticMethod(const Handle<Interpreter>& interpreter,
                                  const Handle<Class>& declaring_class,
                                  int arity,
                                  Callback callback)
                : FunctionObject(interpreter)
                , m_declaring_class(declaring_class.Get())
                , m_arity(arity)
                , m_callback(callback) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                Handle<Frame> frame = interpreter->PushFrame(Handle<Frame>(), this, Value(m_declaring_class), args);
                Value result;

                // Test that we have correct amount of arguments.
                if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1)))
                    {
                        StringBuilder sb;

                        sb << "Method expected at least "
                           << Utils::ToString(static_cast<u64>(-(m_arity) - 1))
                           << " arguments, got "
                           << Utils::ToString(static_cast<u64>(args.GetSize()));
                        interpreter->Throw(interpreter->eTypeError, sb.ToString());
                        interpreter->PopFrame();

                        return Value();
                    }
                }
                else if (args.GetSize() != static_cast<unsigned>(m_arity))
                {
                    StringBuilder sb;

                    sb << "Method expected "
                       << Utils::ToString(static_cast<u64>(m_arity))
                       << " arguments, got "
                       << Utils::ToString(static_cast<u64>(args.GetSize()));
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());
                    interpreter->PopFrame();

                    return Value();
                }
                result = m_callback(interpreter, args);
                interpreter->PopFrame();

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
        Value method = Value(new StaticMethod(interpreter, this, arity, callback));

        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(name, method);
    }

    namespace
    {
        class AliasMethod : public FunctionObject
        {
        public:
            explicit AliasMethod(const Handle<Interpreter>& interpreter, const String& alias)
                : FunctionObject(interpreter)
                , m_alias(alias) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                // Arguments must not be empty.
                if (args.IsEmpty())
                {
                    interpreter->Throw(interpreter->eTypeError,
                                       "Missing method receiver");

                    return Value();
                }

                return args[0].Call(interpreter, m_alias, args.SubVector(1));
            }

        private:
            const String m_alias;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(AliasMethod);
        };
    }

    void Class::AddMethodAlias(const Handle<Interpreter>& interpreter,
                               const String& alias_name,
                               const String& aliased_name)
    {
        Value method = Value(new AliasMethod(interpreter, aliased_name));

        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(alias_name, method);
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
            for (const AttributeMap::Entry* entry = m_attributes->GetFront(); entry; entry = entry->GetNext())
            {
                entry->GetValue().Mark();
            }
        }
    }

    static Handle<CoreObject> no_alloc(const Handle<Interpreter>& interpreter,
                                       const Handle<Class>& cls)
    {
        return Handle<CoreObject>();
    }

    static Handle<CoreObject> class_alloc_callback(const Handle<Interpreter>& interpreter,
                                                   const Handle<Class>& cls)
    {
        return new Class(interpreter->cObject);
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
        Handle<CoreObject> instance;

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

        return Value(instance);
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
            if (instance.Call(interpreter, "__init__", args.SubVector(1)))
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

        i->cClass->SetAllocator(class_alloc_callback);

        i->cClass->AddMethod(i, "alloc", 0, class_alloc);

        i->cClass->AddMethod(i, "__call__", -1, class_call);

        // Conversion methods
        i->cClass->AddMethod(i, "__str__", 0, class_str);
    }
}
