#include <sstream>

#include "functionobject.h"
#include "interpreter.h"

namespace tempearly
{
    FunctionObject::FunctionObject(const Handle<Class>& cls)
        : Object(cls) {}

    FunctionObject::~FunctionObject() {}

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
                    std::stringstream ss;

                    ss << "Method requires a '"
                       << m_declaring_class->GetName()
                       << "' object but received a '"
                       << args[0].GetClass(interpreter)->GetName();
                    interpreter->Throw(interpreter->eTypeError, ss.str());

                    return Value();
                }
                // Test that we have correct amount of arguments.
                else if (m_arity < 0)
                {
                    if (args.size() < static_cast<unsigned>(-(m_arity + 1) + 1))
                    {
                        std::stringstream ss;

                        ss << "Method expected at least "
                           << (-(m_arity) - 1)
                           << " arguments, got "
                           << args.size();
                        interpreter->Throw(interpreter->eTypeError, ss.str());

                        return Value();
                    }
                }
                else if (args.size() != static_cast<unsigned>(m_arity) + 1)
                {
                    std::stringstream ss;

                    ss << "Method expected "
                       << m_arity
                       << " arguments, got "
                       << args.size();
                    interpreter->Throw(interpreter->eTypeError, ss.str());

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

    Value FunctionObject::NewMethod(const Handle<Interpreter>& interpreter,
                                    const Handle<Class>& cls,
                                    const String& name,
                                    int arity,
                                    Callback callback)
    {
        return Value::NewObject(new Method(interpreter->cFunction,
                                           cls,
                                           arity,
                                           callback));
    }

    namespace
    {
        class CurryFunction : public FunctionObject
        {
        public:
            explicit CurryFunction(const Handle<Class>& cls,
                                   FunctionObject* base,
                                   const std::vector<Value>& args)
                : FunctionObject(cls)
                , m_base(base)
                , m_args(args) {}

            Value Invoke(const Handle<Interpreter>& interpreter,
                         const std::vector<Value>& args)
            {
                std::vector<Value> new_args;

                new_args.reserve(m_args.size() + args.size());
                new_args.insert(new_args.begin(), args.begin(), args.end());
                new_args.insert(new_args.begin(), m_args.begin(), m_args.end());

                return m_base->Invoke(interpreter, new_args);
            }

            void Mark()
            {
                FunctionObject::Mark();
                if (!m_base->IsMarked())
                {
                    m_base->Mark();
                }
                for (std::size_t i = 0; i < m_args.size(); ++i)
                {
                    m_args[i].Mark();
                }
            }

        private:
            FunctionObject* m_base;
            const std::vector<Value> m_args;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CurryFunction);
        };
    }

    Value FunctionObject::Curry(const Handle<Interpreter>& interpreter,
                                const std::vector<Value>& args)
    {
        return Value::NewObject(new CurryFunction(interpreter->cFunction, this, args));
    }
}
