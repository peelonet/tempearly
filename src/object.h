#ifndef TEMPEARLY_OBJECT_H_GUARD
#define TEMPEARLY_OBJECT_H_GUARD

#include "class.h"

namespace tempearly
{
    class Object : public CoreObject
    {
    public:
        typedef Dictionary<Value> AttributeMap;

        explicit Object(const Handle<Class>& cls);

        virtual ~Object();

        inline Handle<Class> GetClass(const Handle<Interpreter>&) const
        {
            return m_class;
        }

        bool HasAttribute(const String& id) const;

        bool GetAttribute(const String& id, Value& value) const;

        void SetAttribute(const String& id, const Value& value);

        void Mark();

    private:
        Class* m_class;
        AttributeMap* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Object);
    };
}

#endif /* !TEMPEARLY_OBJECT_H_GUARD */
