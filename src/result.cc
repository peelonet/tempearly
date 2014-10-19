#include "result.h"

namespace tempearly
{
    Result::Result(Kind kind, const Value& value)
        : m_kind(kind)
        , m_value(value) {}

    Result::Result(const Value& value)
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
        m_value.Mark();
    }
}
