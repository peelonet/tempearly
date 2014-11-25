#include "frame.h"
#include "interpreter.h"

namespace tempearly
{
    Frame::Frame(const Handle<Frame>& previous,
                 const Handle<Frame>& enclosing_frame,
                 const Handle<FunctionObject>& function)
        : m_previous(previous.Get())
        , m_enclosing_frame(enclosing_frame.Get())
        , m_function(function.Get())
        , m_local_variables(0) {}

    Frame::~Frame()
    {
        if (m_local_variables)
        {
            delete m_local_variables;
        }
    }

    Handle<Object> Frame::GetLocalVariables(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> object = new Object(interpreter->cObject);

        if (m_local_variables)
        {
            for (const Dictionary<Value>::Entry* e = m_local_variables->GetFront(); e; e = e->GetNext())
            {
                const String& name = e->GetName();

                if (name.IsEmpty() || name.GetFront() == '_')
                {
                    continue;
                }
                object->SetAttribute(name, e->GetValue());
            }
        }

        return object;
    }

    bool Frame::HasLocalVariable(const String& id) const
    {
        return m_local_variables && m_local_variables->Find(id);
    }

    bool Frame::GetLocalVariable(const String& id, Value& slot) const
    {
        if (m_local_variables)
        {
            const Dictionary<Value>::Entry* e = m_local_variables->Find(id);

            if (e)
            {
                slot = e->GetValue();

                return true;
            }
        }

        return false;
    }

    void Frame::SetLocalVariable(const String& id, const Value& value)
    {
        if (!m_local_variables)
        {
            m_local_variables = new Dictionary<Value>();
        }
        m_local_variables->Insert(id, value);
    }

    bool Frame::ReplaceLocalVariable(const String& id, const Value& value)
    {
        if (m_local_variables)
        {
            Dictionary<Value>::Entry* e = m_local_variables->Find(id);

            if (e)
            {
                e->SetValue(value);

                return true;
            }
        }

        return false;
    }

    void Frame::Mark()
    {
        CountedObject::Mark();
        if (m_previous && !m_previous->IsMarked())
        {
            m_previous->Mark();
        }
        if (m_enclosing_frame && !m_enclosing_frame->IsMarked())
        {
            m_enclosing_frame->Mark();
        }
        if (m_local_variables)
        {
            for (const Dictionary<Value>::Entry* e = m_local_variables->GetFront(); e; e = e->GetNext())
            {
                e->GetValue().Mark();
            }
        }
    }
}
