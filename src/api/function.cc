#include "interpreter.h"
#include "script/node.h"
#include "script/parameter.h"

namespace tempearly
{
    FunctionObject::FunctionObject(const Handle<Interpreter>& interpreter,
                                   const Handle<Frame>& enclosing_frame)
        : Object(interpreter->cFunction)
        , m_enclosing_frame(enclosing_frame) {}

    FunctionObject::~FunctionObject() {}

    namespace
    {
        class ScriptedFunction : public FunctionObject
        {
        public:
            explicit ScriptedFunction(const Handle<Interpreter>& interpreter,
                                      const Vector<Handle<Parameter> >& parameters,
                                      const Vector<Handle<Node> >& nodes)
                : FunctionObject(interpreter, interpreter->GetFrame())
                , m_parameters(parameters)
                , m_nodes(nodes) {}

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                if (!Parameter::Apply(interpreter,
                                      m_parameters,
                                      frame->GetArguments()))
                {
                    return false;
                }
                for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
                {
                    const Result result = m_nodes[i]->Execute(interpreter);

                    switch (result.GetKind())
                    {
                        case Result::KIND_SUCCESS:
                            continue;

                        case Result::KIND_RETURN:
                            frame->SetReturnValue(result.GetValue());
                            return true;

                        case Result::KIND_BREAK:
                            interpreter->Throw(
                                interpreter->eSyntaxError,
                                "Unexpected 'break'"
                            );
                            return false;

                        case Result::KIND_CONTINUE:
                            interpreter->Throw(
                                interpreter->eSyntaxError,
                                "Unexpected 'continue'"
                            );
                            return false;

                        default:
                            return false;
                    }
                }

                return true;
            }

            void Mark()
            {
                FunctionObject::Mark();
                for (std::size_t i = 0; i < m_parameters.GetSize(); ++i)
                {
                    if (!m_parameters[i]->IsMarked())
                    {
                        m_parameters[i]->Mark();
                    }
                }
                for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
                {
                    if (!m_nodes[i]->IsMarked())
                    {
                        m_nodes[i]->Mark();
                    }
                }
            }

        private:
            const Vector<Parameter*> m_parameters;
            const Vector<Node*> m_nodes;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ScriptedFunction);
        };
    }

    Handle<FunctionObject> FunctionObject::NewScripted(const Handle<Interpreter>& interpreter,
                                                       const Vector<Handle<Parameter> >& parameters,
                                                       const Vector<Handle<Node> >& nodes)
    {
        return new ScriptedFunction(interpreter, parameters, nodes);
    }

    namespace
    {
        class UnboundMethod : public FunctionObject
        {
        public:
            explicit UnboundMethod(const Handle<Interpreter>& interpreter,
                                   const Handle<Class>& declaring_class,
                                   int arity,
                                   Callback callback)
                : FunctionObject(interpreter)
                , m_declaring_class(declaring_class)
                , m_arity(arity)
                , m_callback(callback) {}

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                const Vector<Value>& args = frame->GetArguments();

                // Arguments must not be empty.
                if (args.IsEmpty())
                {
                    interpreter->Throw(
                        interpreter->eTypeError,
                        "Missing method receiver"
                    );

                    return false;
                }
                // Test that the first argument is correct type.
                else if (!args[0].IsInstance(interpreter, m_declaring_class))
                {
                    interpreter->Throw(
                        interpreter->eTypeError,
                        "Method requires a '"
                        + m_declaring_class->GetName()
                        + "' object but received a '"
                        + args[0].GetClass(interpreter)->GetName()
                    );

                    return false;
                }
                // Test that we have correct amount of arguments.
                else if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1) + 1))
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
                else if (args.GetSize() != static_cast<unsigned>(m_arity) + 1)
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

            bool IsUnboundMethod() const
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
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(UnboundMethod);
        };
    }

    Handle<FunctionObject> FunctionObject::NewUnboundMethod(
        const Handle<Interpreter>& interpreter,
        const Handle<Class>& cls,
        int arity,
        Callback callback
    )
    {
        return new UnboundMethod(interpreter, cls, arity, callback);
    }

    bool FunctionObject::Invoke(const Handle<Interpreter>& interpreter,
                                Value& slot,
                                const Vector<Value>& args)
    {
        const Handle<Frame> frame = interpreter->PushFrame(
            m_enclosing_frame,
            this,
            args
        );
        const bool result = Invoke(interpreter, frame);

        interpreter->PopFrame();
        if (result)
        {
            slot = frame->GetReturnValue();
        }

        return result;
    }

    bool FunctionObject::Invoke(const Handle<Interpreter>& interpreter,
                                const Vector<Value>& args)
    {
        const Handle<Frame> frame = interpreter->PushFrame(
            m_enclosing_frame,
            this,
            args
        );
        const bool result = Invoke(interpreter, frame);

        interpreter->PopFrame();

        return result;
    }

    namespace
    {
        class CurryFunction : public FunctionObject
        {
        public:
            explicit CurryFunction(const Handle<Interpreter>& interpreter,
                                   FunctionObject* base,
                                   const Vector<Value>& args)
                : FunctionObject(interpreter)
                , m_base(base)
                , m_args(args) {}

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                Value result;

                if (!m_base->Invoke(interpreter,
                                    result,
                                    m_args + frame->GetArguments()))
                {
                    return false;
                }
                frame->SetReturnValue(result);

                return true;
            }

            void Mark()
            {
                FunctionObject::Mark();
                if (!m_base->IsMarked())
                {
                    m_base->Mark();
                }
                for (std::size_t i = 0; i < m_args.GetSize(); ++i)
                {
                    m_args[i].Mark();
                }
            }

        private:
            FunctionObject* m_base;
            const Vector<Value> m_args;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CurryFunction);
        };
    }

    Value FunctionObject::Curry(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
    {
        return Value(new CurryFunction(interpreter, this, args));
    }

    void FunctionObject::Mark()
    {
        Object::Mark();
        if (m_enclosing_frame && !m_enclosing_frame->IsMarked())
        {
            m_enclosing_frame->Mark();
        }
    }

    /**
     * Function#__call__(args...)
     *
     * Invokes function with given arguments.
     */
    TEMPEARLY_NATIVE_METHOD(func_call)
    {
        Value result;

        if (args[0].As<FunctionObject>()->Invoke(interpreter,
                                                 result,
                                                 args.SubVector(1)))
        {
            frame->SetReturnValue(result);
        }
    }

    void init_function(Interpreter* i)
    {
        Handle<Class> cFunction = i->cFunction;

        i->cFunction->SetAllocator(Class::kNoAlloc);

        i->cFunction->AddMethod(i, "__call__", -1, func_call);
    }
}
