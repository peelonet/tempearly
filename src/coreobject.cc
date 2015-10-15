#include "value.h"
#include "api/class.h"

namespace tempearly
{
    CoreObject::CoreObject()
        : m_value_head(nullptr)
        , m_value_tail(nullptr) {}

    CoreObject::~CoreObject()
    {
        for (Value* value = m_value_head; value; value = value->m_next)
        {
            value->m_kind = Value::KIND_NULL;
            value->m_previous = value->m_next = nullptr;
        }
    }

    bool CoreObject::IsInstance(const Handle<Interpreter>& interpreter, const Handle<Class>& cls) const
    {
        return GetClass(interpreter)->IsSubclassOf(cls);
    }

    void CoreObject::RegisterValue(Value* value)
    {
        if (!value)
        {
            return;
        }
        if ((value->m_previous = m_value_tail))
        {
            m_value_tail->m_next = value;
        } else {
            m_value_head = value;
        }
        m_value_tail = value;
    }

    void CoreObject::UnregisterValue(Value* value)
    {
        if (!value)
        {
            return;
        }
        if (value->m_next && value->m_previous)
        {
            value->m_previous->m_next = value->m_next;
            value->m_next->m_previous = value->m_previous;
        }
        else if (value->m_next)
        {
            m_value_head = value->m_next;
            m_value_head->m_previous = nullptr;
        }
        else if (value->m_previous)
        {
            m_value_tail = value->m_previous;
            m_value_tail->m_next = nullptr;
        } else {
            m_value_head = m_value_tail = nullptr;
        }
        value->m_previous = value->m_next = nullptr;
    }
}
