#include "frame.h"
#include "interpreter.h"

namespace tempearly
{
    Frame::Frame(const Handle<Frame>& previous,
                 const Handle<Frame>& enclosing_frame,
                 const Handle<FunctionObject>& function,
                 const Vector<Handle<Object>>& arguments)
        : m_previous(previous.Get())
        , m_enclosing_frame(enclosing_frame.Get())
        , m_function(function.Get())
        , m_arguments(arguments)
        , m_local_variables(nullptr)
        , m_return_value(nullptr) {}

    Frame::~Frame()
    {
        if (m_local_variables)
        {
            delete m_local_variables;
        }
    }

    Handle<Object> Frame::GetLocalVariables(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> object = new CustomObject(interpreter->cObject);

        if (m_local_variables)
        {
            for (const Dictionary<Object*>::Entry* e = m_local_variables->GetFront(); e; e = e->GetNext())
            {
                const String& name = e->GetName();

                if (name.IsEmpty() || name.GetFront() == '_')
                {
                    continue;
                }
                object->SetOwnAttribute(name, e->GetValue());
            }
        }

        return object;
    }

    bool Frame::HasLocalVariable(const String& id) const
    {
        return m_local_variables && m_local_variables->Find(id);
    }

    bool Frame::GetLocalVariable(const String& id, Handle<Object>& slot) const
    {
        if (m_local_variables)
        {
            const Dictionary<Object*>::Entry* e = m_local_variables->Find(id);

            if (e)
            {
                slot = e->GetValue();

                return true;
            }
        }

        return false;
    }

    void Frame::SetLocalVariable(const String& id, const Handle<Object>& value)
    {
        if (!m_local_variables)
        {
            m_local_variables = new Dictionary<Object*>();
        }
        m_local_variables->Insert(id, value);
    }

    bool Frame::ReplaceLocalVariable(const String& id,
                                     const Handle<Object>& value)
    {
        if (m_local_variables)
        {
            Dictionary<Object*>::Entry* e = m_local_variables->Find(id);

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
        if (m_function && !m_function->IsMarked())
        {
            m_function->Mark();
        }
        for (std::size_t i = 0; i < m_arguments.GetSize(); ++i)
        {
            if (!m_arguments[i]->IsMarked())
            {
                m_arguments[i]->Mark();
            }
        }
        if (m_local_variables)
        {
            for (const Dictionary<Object*>::Entry* e = m_local_variables->GetFront(); e; e = e->GetNext())
            {
                if (!e->GetValue()->IsMarked())
                {
                    e->GetValue()->Mark();
                }
            }
        }
        if (m_return_value && !m_return_value->IsMarked())
        {
            m_return_value->Mark();
        }
    }
}
