#ifndef TEMPEARLY_COREOBJECT_H_GUARD
#define TEMPEARLY_COREOBJECT_H_GUARD

#include "dictionary.h"

namespace tempearly
{
    class CoreObject : public CountedObject
    {
    public:
        typedef Dictionary<Value> AttributeMap;

        explicit CoreObject();

        virtual Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const = 0;

        /**
         * Returns true if object is instance of given class.
         */
        bool IsInstance(const Handle<Interpreter>& interpreter,
                        const Handle<Class>& cls) const;

        virtual bool HasAttribute(const String& id) const = 0;

        virtual bool GetAttribute(const String& id, Value& value) const = 0;

        virtual void SetAttribute(const String& id, const Value& value) = 0;

        virtual bool IsClass() const
        {
            return false;
        }

        virtual bool IsException() const
        {
            return false;
        }

        virtual bool IsFunction() const
        {
            return false;
        }

        virtual bool IsIterator() const
        {
            return false;
        }

        virtual bool IsList() const
        {
            return false;
        }

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CoreObject);
    };
}

#endif /* !TEMPEARLY_COREOBJECT_H_GUARD */
