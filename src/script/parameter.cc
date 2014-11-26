#include "interpreter.h"
#include "parameter.h"
#include "api/list.h"

namespace tempearly
{
    Parameter::Parameter(const String& name,
                         const Handle<TypeHint>& type,
                         const Handle<Node>& default_value,
                         bool rest)
        : m_name(name)
        , m_type(type.Get())
        , m_default_value(default_value.Get())
        , m_rest(rest) {}

    bool Parameter::Apply(const Handle<Interpreter>& interpreter,
                          const Vector<Handle<Parameter> >& parameters,
                          const Vector<Value>& arguments)
    {
        const Handle<Frame> frame = interpreter->GetFrame();

        for (std::size_t i = 0; i < parameters.GetSize(); ++i)
        {
            const Handle<Parameter>& parameter = parameters[i];

            if (parameter->m_rest)
            {
                Handle<ListObject> list = new ListObject(interpreter->cList);

                for (std::size_t j = i; j < arguments.GetSize(); ++j)
                {
                    const Value& value = arguments[j];

                    if (parameter->m_type)
                    {
                        bool slot;

                        if (!parameter->m_type->Accepts(interpreter, value, slot))
                        {
                            return false;
                        }
                        else if (!slot)
                        {
                            // TODO: Better exception message would be nice...
                            interpreter->Throw(interpreter->eValueError, "Argument is not expected type");

                            return false;
                        }
                    }
                    list->Append(value);
                }
                frame->SetLocalVariable(parameter->m_name, Value(list));

                return true;
            }
            else if (i < arguments.GetSize())
            {
                const Value& value = arguments[i];

                if (parameter->m_type)
                {
                    bool slot;

                    if (!parameter->m_type->Accepts(interpreter, value, slot))
                    {
                        return false;
                    }
                    else if (!slot)
                    {
                        // TODO: Better exception message would be nice...
                        interpreter->Throw(interpreter->eValueError, "Argument is not expected type");

                        return false;
                    }
                }
                frame->SetLocalVariable(parameter->m_name, arguments[i]);
            }
            else if (parameter->m_default_value)
            {
                Value value = parameter->m_default_value->Evaluate(interpreter);

                if (!value)
                {
                    return false;
                }
                frame->SetLocalVariable(parameter->m_name, value);
            } else {
                interpreter->Throw(interpreter->eValueError, "Too few arguments");

                return false;
            }
        }
        if (arguments.GetSize() > parameters.GetSize())
        {
            interpreter->Throw(interpreter->eValueError, "Too many arguments");

            return false;
        }

        return true;
    }

    void Parameter::Mark()
    {
        CountedObject::Mark();
        if (m_type && !m_type->IsMarked())
        {
            m_type->Mark();
        }
        if (m_default_value && !m_default_value)
        {
            m_default_value->Mark();
        }
    }
}
