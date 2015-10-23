#include "result.h"

namespace tempearly
{
    Result::Result(Kind kind, const Handle<Object>& value)
        : m_kind(kind)
        , m_value(value) {}

    Result::Result(const Handle<Object>& value)
        : m_kind(KIND_SUCCESS)
        , m_value(value) {}

    Result::Result(const Result& that)
        : m_kind(that.m_kind)
        , m_value(that.m_value) {}

    Result& Result::operator=(const Result& that)
    {
        m_kind = that.m_kind;
        m_value = that.m_value;

        return *this;
    }

    void Result::Mark() const
    {
        if (m_value && !m_value->IsMarked())
        {
            m_value->Mark();
        }
    }
}
