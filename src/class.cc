#include "interpreter.h"

namespace tempearly
{
    Class::Class(const Handle<Class>& base)
        : m_base(base.Get())
        , m_attributes(0) {}

    Class::~Class()
    {
        if (m_attributes)
        {
            delete m_attributes;
        }
    }

    Handle<Class> Class::GetClass(const Handle<Interpreter>& interpreter) const
    {
        return interpreter->cClass;
    }

    bool Class::HasAttribute(const String& id) const
    {
        return m_attributes && m_attributes->Find(id);
    }

    bool Class::GetAttribute(const String& id, Value& value) const
    {
        if (m_attributes)
        {
            const AttributeMap::Entry* entry = m_attributes->Find(id);

            if (entry)
            {
                value = entry->value;

                return true;
            }
        }

        return false;
    }

    void Class::SetAttribute(const String& id, const Value& value)
    {
        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(id, value);
    }

    void Class::Mark()
    {
        CountedObject::Mark();
        if (m_base && !m_base->IsMarked())
        {
            m_base->Mark();
        }
        if (m_attributes)
        {
            for (const AttributeMap::Entry* e = m_attributes->GetFront(); e; e = e->next)
            {
                e->value.Mark();
            }
        }
    }
}
