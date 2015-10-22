#include "interpreter.h"
#include "core/stringbuilder.h"

namespace tempearly
{
    static Handle<Object> no_alloc(const Handle<Interpreter>&, const Handle<Class>&);

    const Class::Allocator Class::kNoAlloc = no_alloc;

    Class::Class(const Handle<Class>& base)
        : m_base(base.Get())
        , m_allocator(m_base ? m_base->m_allocator : nullptr)
        , m_attributes(nullptr) {}

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
        Handle<Object> value;

        if (GetOwnAttribute("__name__", value) && value->IsString())
        {
            return value->AsString();
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

    Dictionary<Handle<Object>> Class::GetOwnAttributes() const
    {
        if (m_attributes)
        {
            return *m_attributes;
        } else {
            return Dictionary<Handle<Object>>();
        }
    }

    bool Class::GetOwnAttribute(const String& id, Handle<Object>& slot) const
    {
        if (m_attributes)
        {
            const Dictionary<Object*>::Entry* entry = m_attributes->Find(id);

            if (entry)
            {
                slot = entry->GetValue();

                return true;
            }
        }
        if (m_base)
        {
            return m_base->GetOwnAttribute(id, slot);
        } else {
            return false;
        }
    }

    bool Class::SetOwnAttribute(const String& id, const Handle<Object>& value)
    {
        if (!m_attributes)
        {
            m_attributes = new Dictionary<Object*>();
        }
        m_attributes->Insert(id, value);

        return true;
    }

    void Class::AddMethod(const Handle<Interpreter>& interpreter,
                          const String& name,
                          int arity,
                          MethodCallback callback)
    {
        Handle<FunctionObject> method = FunctionObject::NewUnboundMethod(
            interpreter,
            this,
            arity,
            callback
        );

        if (!m_attributes)
        {
            m_attributes = new Dictionary<Object*>();
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

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                const Vector<Handle<Object>> args = frame->GetArguments();

                // Test that we have correct amount of arguments.
                if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1)))
                    {
                        interpreter->Throw(
                            interpreter->eTypeError,
                            "Method expected at least "
                            + String::FromI64(-m_arity - 1)
                            + " arguments, got "
                            + String::FromU64(args.GetSize())
                        );

                        return false;
                    }
                }
                else if (args.GetSize() != static_cast<unsigned>(m_arity))
                {
                    interpreter->Throw(
                        interpreter->eTypeError,
                        "Method expected "
                        + String::FromI64(m_arity)
                        + " arguments, got "
                        + String::FromU64(args.GetSize())
                    );

                    return false;
                }
                m_callback(interpreter, frame, args);

                return !interpreter->HasException();
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
        Handle<Object> method = new StaticMethod(interpreter, this, arity, callback);

        if (!m_attributes)
        {
            m_attributes = new Dictionary<Object*>();
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

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                const Vector<Handle<Object>> args = frame->GetArguments();
                Handle<Object> result;

                // Arguments must not be empty.
                if (args.IsEmpty())
                {
                    interpreter->Throw(
                        interpreter->eTypeError,
                        "Missing method receiver"
                    );

                    return false;
                }
                if (!args[0]->CallMethod(interpreter,
                                         result,
                                         m_alias,
                                         args.SubVector(1)))
                {
                    return false;
                }
                frame->SetReturnValue(result);

                return true;
            }

            bool IsUnboundMethod() const
            {
                return true;
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
        Handle<Object> method = new AliasMethod(interpreter, aliased_name);

        if (!m_attributes)
        {
            m_attributes = new Dictionary<Object*>();
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
            for (Dictionary<Object*>::Entry* entry = m_attributes->GetFront();
                 entry;
                 entry = entry->GetNext())
            {
                if (!entry->GetValue()->IsMarked())
                {
                    entry->GetValue()->Mark();
                }
            }
        }
    }

    static Handle<Object> no_alloc(const Handle<Interpreter>& interpreter,
                                   const Handle<Class>& cls)
    {
        return Handle<Object>();
    }

    static Handle<Object> class_alloc_callback(const Handle<Interpreter>& interpreter,
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
        Handle<Object> instance;

        if (allocator)
        {
            if (!(instance = allocator(interpreter, cls)))
            {
                interpreter->Throw(interpreter->eTypeError,
                                   "Cannot allocate instance of "
                                   + cls->GetName());
                return;
            }
        } else {
            instance = new CustomObject(cls);
        }
        frame->SetReturnValue(instance);
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
        Handle<Object> instance;

        if (args[0]->CallMethod(interpreter, instance, "alloc")
            && instance->CallMethod(interpreter, "__init__", args.SubVector(1)))
        {
            frame->SetReturnValue(instance);
        }
    }

    /**
     * Class#__str__() => String
     *
     * Returns textual description of the class which is usually name of the
     * class.
     */
    TEMPEARLY_NATIVE_METHOD(class_str)
    {
        Handle<Object> name;

        if (args[0].As<Class>()->GetOwnAttribute("__name__", name) && name->IsString())
        {
            frame->SetReturnValue(name);
        } else {
            frame->SetReturnValue(Object::NewString("<anonymous type>"));
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
