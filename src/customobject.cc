#include "customobject.h"

namespace tempearly
{
    CustomObject::CustomObject(const Handle<Class>& cls)
        : m_class(cls)
        , m_attributes(nullptr) {}

    CustomObject::~CustomObject()
    {
        if (m_attributes)
        {
            delete m_attributes;
        }
    }

    Handle<Class> CustomObject::GetClass(const Handle<Interpreter>&) const
    {
        return m_class;
    }

    Dictionary<Handle<Object>> CustomObject::GetOwnAttributes() const
    {
        if (m_attributes)
        {
            return *m_attributes;
        } else {
            return Dictionary<Handle<Object>>();
        }
    }

    bool CustomObject::GetOwnAttribute(const String& name,
                                       Handle<Object>& slot) const
    {
        if (m_attributes)
        {
            const Dictionary<Object*>::Entry* e = m_attributes->Find(name);

            if (e)
            {
                slot = e->GetValue();

                return true;
            }
        }

        return false;
    }

    bool CustomObject::SetOwnAttribute(const String& name,
                                       const Handle<Object>& value)
    {
        if (!m_attributes)
        {
            m_attributes = new Dictionary<Object*>();
        }
        m_attributes->Insert(name, value);

        return true;
    }

    void CustomObject::Mark()
    {
        Object::Mark();
        if (!m_class->IsMarked())
        {
            m_class->Mark();
        }
        if (m_attributes)
        {
            for (Dictionary<Object*>::Entry* e = m_attributes->GetFront();
                 e;
                 e = e->GetNext())
            {
                if (!e->GetValue()->IsMarked())
                {
                    e->GetValue()->Mark();
                }
            }
        }
    }
}
