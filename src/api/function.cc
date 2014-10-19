#include "interpreter.h"
#include "node.h"
#include "parameter.h"
#include "api/function.h"

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
                                      const std::vector<Handle<Parameter> >& parameters,
                                      const std::vector<Handle<Node> >& nodes)
                : FunctionObject(interpreter)
                , m_parameters(parameters.begin(), parameters.end())
                , m_nodes(nodes.begin(), nodes.end()) {}

            Value Invoke(const Handle<Interpreter>& interpreter, const std::vector<Value>& args)
            {
                interpreter->PushScope(interpreter->GetScope());
                if (!Parameter::Apply(interpreter,
                                      std::vector<Handle<Parameter> >(m_parameters.begin(), m_parameters.end()),
                                      args))
                {
                    interpreter->PopScope();

                    return Value();
                }
                for (std::size_t i = 0; i < m_nodes.size(); ++i)
                {
                    Result result = m_nodes[i]->Execute(interpreter);

                    switch (result.GetKind())
                    {
                        case Result::KIND_SUCCESS:
                            break;

                        case Result::KIND_RETURN:
                            interpreter->PopScope();
                            if (result.HasValue())
                            {
                                return result.GetValue();
                            } else {
                                return Value::NullValue();
                            }

                        case Result::KIND_BREAK:
                            interpreter->Throw(interpreter->eSyntaxError, "Unexpected 'break'");
                            interpreter->PopScope();
                            return Value();

                        case Result::KIND_CONTINUE:
                            interpreter->Throw(interpreter->eSyntaxError, "Unexpected 'continue'");
                            interpreter->PopScope();
                            return Value();

                        default:
                            interpreter->PopScope();
                            return Value();
                    }
                }
                interpreter->PopScope();

                return Value::NullValue();
            }

            void Mark()
            {
                FunctionObject::Mark();
                for (std::size_t i = 0; i < m_parameters.size(); ++i)
                {
                    if (!m_parameters[i]->IsMarked())
                    {
                        m_parameters[i]->Mark();
                    }
                }
                for (std::size_t i = 0; i < m_nodes.size(); ++i)
                {
                    if (!m_nodes[i]->IsMarked())
                    {
                        m_nodes[i]->Mark();
                    }
                }
            }

        private:
            const std::vector<Parameter*> m_parameters;
            const std::vector<Node*> m_nodes;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ScriptedFunction);
        };
    }

    Handle<FunctionObject> FunctionObject::NewScripted(const Handle<Interpreter>& interpreter,
                                                       const std::vector<Handle<Parameter> >& parameters,
                                                       const std::vector<Handle<Node> >& nodes)
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
                                   const std::vector<Value>& args)
                : FunctionObject(interpreter)
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
        return Value::NewObject(new CurryFunction(interpreter, this, args));
    }

    /**
     * Function#__call__(args...)
     *
     * Invokes function with given arguments.
     */
    TEMPEARLY_NATIVE_METHOD(func_call)
    {
        return args[0].As<FunctionObject>()->Invoke(
            interpreter,
            std::vector<Value>(args.begin() + 1, args.end())
        );
    }

    void init_function(Interpreter* i)
    {
        i->cFunction = i->AddClass("Function", i->cObject);

        i->cFunction->SetAllocator(Class::kNoAlloc);

        i->cFunction->AddMethod(i, "__call__", -1, func_call);
    }
}
