#include "interpreter.h"
#include "api/function.h"

namespace tempearly
{
    FunctionObject::FunctionObject(const Handle<Class>& cls)
        : Object(cls) {}

    FunctionObject::~FunctionObject() {}

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
