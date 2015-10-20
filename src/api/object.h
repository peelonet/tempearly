#ifndef TEMPEARLY_API_OBJECT_H_GUARD
#define TEMPEARLY_API_OBJECT_H_GUARD

#include "api/class.h"

namespace tempearly
{
    class Object : public CoreObject
    {
    public:
        explicit Object(const Handle<Class>& cls);

        virtual ~Object();

        inline Handle<Class> GetClass(const Handle<Interpreter>&) const
        {
            return m_class;
        }

        Dictionary<Value> GetAllAttributes() const;

        bool GetAttribute(const String& id, Value& value) const;

        void SetAttribute(const String& id, const Value& value);

        void Mark();

    private:
        Class* m_class;
        AttributeMap* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Object);
    };
}

#endif /* !TEMPEARLY_API_OBJECT_H_GUARD */
