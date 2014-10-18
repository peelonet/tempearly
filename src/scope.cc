#include "interpreter.h"

namespace tempearly
{
    Scope::Scope(const Handle<Scope>& previous, const Handle<Scope>& parent)
        : m_previous(previous.Get())
        , m_parent(parent.Get())
        , m_variables(0) {}

    Scope::~Scope()
    {
        if (m_variables)
        {
            delete m_variables;
        }
    }

    bool Scope::HasVariable(const String& id) const
    {
        return m_variables && m_variables->Find(id);
    }

    bool Scope::GetVariable(const String& id, Value& value) const
    {
        if (m_variables)
        {
            const VariableMap::Entry* entry = m_variables->Find(id);

            if (entry)
            {
                value = entry->GetValue();

                return true;
            }
        }

        return false;
    }

    void Scope::SetVariable(const String& id, const Value& value)
    {
        if (!m_variables)
        {
            m_variables = new VariableMap();
        }
        m_variables->Insert(id, value);
    }

    Handle<Object> Scope::ToObject(const Handle<Interpreter>& interpreter) const
    {
        Handle<Object> object = new Object(interpreter->cObject);

        if (m_variables)
        {
            for (const VariableMap::Entry* entry = m_variables->GetFront(); entry; entry = entry->GetNext())
            {
                object->SetAttribute(entry->GetName(), entry->GetValue());
            }
        }

        return object;
    }

    void Scope::Mark()
    {
        CountedObject::Mark();
        if (m_previous && !m_previous->IsMarked())
        {
            m_previous->Mark();
        }
        if (m_parent && !m_parent->IsMarked())
        {
            m_parent->Mark();
        }
        if (m_variables)
        {
            for (const VariableMap::Entry* entry = m_variables->GetFront(); entry; entry = entry->GetNext())
            {
                entry->GetValue().Mark();
            }
        }
    }
}
