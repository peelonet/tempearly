#include "interpreter.h"
#include "script/node.h"
#include "script/parameter.h"

namespace tempearly
{
    FunctionObject::FunctionObject(const Handle<Interpreter>& interpreter)
        : Object(interpreter->cFunction) {}

    FunctionObject::~FunctionObject() {}

    namespace
    {
        class ScriptedFunction : public FunctionObject
        {
        public:
            explicit ScriptedFunction(const Handle<Interpreter>& interpreter,
                                      const Vector<Handle<Parameter> >& parameters,
                                      const Vector<Handle<Node> >& nodes)
                : FunctionObject(interpreter)
                , m_enclosing_frame(interpreter->GetFrame())
                , m_parameters(parameters)
                , m_nodes(nodes) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                interpreter->PushFrame(m_enclosing_frame, this);
                if (!Parameter::Apply(interpreter, m_parameters, args))
                {
                    interpreter->PopFrame();

                    return Value();
                }
                for (std::size_t i = 0; i < m_nodes.GetSize(); ++i)
                {
                    Result result = m_nodes[i]->Execute(interpreter);

                    switch (result.GetKind())
                    {
                        case Result::KIND_SUCCESS:
                            break;

                        case Result::KIND_RETURN:
                            interpreter->PopFrame();
                            if (result.HasValue())
                            {
                                return result.GetValue();
                            } else {
                                return Value::NullValue();
                            }

                        case Result::KIND_BREAK:
                            interpreter->Throw(interpreter->eSyntaxError, "Unexpected 'break'");
                            interpreter->PopFrame();
                            return Value();

                        case Result::KIND_CONTINUE:
                            interpreter->Throw(interpreter->eSyntaxError, "Unexpected 'continue'");
                            interpreter->PopFrame();
                            return Value();

                        default:
                            interpreter->PopFrame();
                            return Value();
                    }
                }
                interpreter->PopFrame();

                return Value::NullValue();
            }

            void Mark()
            {
                FunctionObject::Mark();
                if (m_enclosing_frame && !m_enclosing_frame->IsMarked())
                {
                    m_enclosing_frame->Mark();
                }
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
            Frame* m_enclosing_frame;
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
        class CurryFunction : public FunctionObject
        {
        public:
            explicit CurryFunction(const Handle<Interpreter>& interpreter,
                                   FunctionObject* base,
                                   const Vector<Value>& args)
                : FunctionObject(interpreter)
                , m_base(base)
                , m_args(args) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                return m_base->Invoke(interpreter, m_args + args);
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

    /**
     * Function#__call__(args...)
     *
     * Invokes function with given arguments.
     */
    TEMPEARLY_NATIVE_METHOD(func_call)
    {
        return args[0].As<FunctionObject>()->Invoke(interpreter, args.SubVector(1));
    }

    void init_function(Interpreter* i)
    {
        i->cFunction = i->AddClass("Function", i->cObject);

        i->cFunction->SetAllocator(Class::kNoAlloc);

        i->cFunction->AddMethod(i, "__call__", -1, func_call);
    }
}
