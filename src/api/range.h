#ifndef TEMPEARLY_API_RANGE_H_GUARD
#define TEMPEARLY_API_RANGE_H_GUARD

#include "customobject.h"

namespace tempearly
{
    class RangeObject : public CustomObject
    {
    public:
        explicit RangeObject(const Handle<Interpreter>& interpreter,
                             const Handle<Object>& begin,
                             const Handle<Object>& end,
                             bool exclusive);

        inline Handle<Object> GetBegin() const
        {
            return m_begin;
        }

        inline Handle<Object> GetEnd() const
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
        Object* m_begin;
        Object* m_end;
        const bool m_exclusive;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(RangeObject);
    };
}

#endif /* !TEMPEARLY_API_RANGE_H_GUARD */
