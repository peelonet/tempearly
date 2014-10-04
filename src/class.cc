#include "functionobject.h"
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

    String Class::GetName() const
    {
        Value value;

        if (GetAttribute("__name__", value) && value.IsString())
        {
            return value.AsString();
        } else {
            return "<anonymous class>";
        }
    }

    bool Class::IsSubclassOf(const Handle<Class>& that) const
    {
        if (this == that)
        {
            return true;
        } else {
            return m_base && m_base->IsSubclassOf(that);
        }
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

            return false;
        }
        else if (m_base)
        {
            return m_base->GetAttribute(id, value);
        } else {
            return false;
        }
    }

    void Class::SetAttribute(const String& id, const Value& value)
    {
        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(id, value);
    }

    void Class::AddMethod(const Handle<Interpreter>& interpreter,
                          const String& name,
                          int arity,
                          Value (*callback)(const Handle<Interpreter>&,
                                            const std::vector<Value>&))
    {
        Value method = FunctionObject::NewMethod(interpreter,
                                                 this,
                                                 name,
                                                 arity,
                                                 callback);

        if (!m_attributes)
        {
            m_attributes = new AttributeMap();
        }
        m_attributes->Insert(name, method);
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
