#include "object.h"
#include "value.h"

namespace tempearly
{
    Object::Object(const Handle<Class>& cls)
        : m_class(cls.Get())
        , m_attributes(0) {}

    Object::~Object()
    {
        if (m_attributes)
        {
            delete m_attributes;
        }
    }

    bool Object::HasAttribute(const String& id) const
    {
        return m_attributes && m_attributes->Find(id);
    }

    bool Object::GetAttribute(const String& id, Value& value) const
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

    void Object::SetAttribute(const String& id, const Value& value)
    {
        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(id, value);
    }

    void Object::Mark()
    {
        CoreObject::Mark();
        if (m_class && !m_class->IsMarked())
        {
            m_class->Mark();
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
