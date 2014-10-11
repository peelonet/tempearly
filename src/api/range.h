#ifndef TEMPEARLY_API_RANGE_H_GUARD
#define TEMPEARLY_API_RANGE_H_GUARD

#include "value.h"

namespace tempearly
{
    class RangeObject : public Object
    {
    public:
        explicit RangeObject(const Handle<Interpreter>& interpreter,
                             const Value& begin,
                             const Value& end,
                             bool exclusive);

        inline const Value& GetBegin() const
        {
            return m_begin;
        }

        inline const Value& GetEnd() const
        {
            return m_end;
        }

        inline bool IsExclusive() const
        {
            return m_exclusive;
        }

        void Mark();

        bool IsRange() const
        {
            return true;
        }

    private:
        const Value m_begin;
        const Value m_end;
        const bool m_exclusive;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(RangeObject);
    };
}

#endif /* !TEMPEARLY_API_RANGE_H_GUARD */
