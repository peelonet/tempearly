#include "scope.h"

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
                value = entry->value;

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
            for (const VariableMap::Entry* e = m_variables->GetFront(); e; e = e->next)
            {
                e->value.Mark();
            }
        }
    }
}
