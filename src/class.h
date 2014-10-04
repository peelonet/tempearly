#ifndef TEMPEARLY_CLASS_H_GUARD
#define TEMPEARLY_CLASS_H_GUARD

#include "coreobject.h"
#include "dictionary.h"

namespace tempearly
{
    class Class : public CoreObject
    {
    public:
        typedef Dictionary<Value> AttributeMap;

        explicit Class(const Handle<Class>& base);

        virtual ~Class();

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        inline Handle<Class> GetBase() const
        {
            return m_base;
        }

        bool HasAttribute(const String& id) const;

        bool GetAttribute(const String& id, Value& value) const;

        void SetAttribute(const String& id, const Value& value);

        void Mark();

        bool IsClass() const
        {
            return true;
        }

    private:
        Class* m_base;
        AttributeMap* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Class);
    };
}

#endif /* !TEMPEARLY_CLASS_H_GUARD */
