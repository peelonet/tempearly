#ifndef TEMPEARLY_COREOBJECT_H_GUARD
#define TEMPEARLY_COREOBJECT_H_GUARD

#include "core/dictionary.h"

namespace tempearly
{
    class CoreObject : public CountedObject
    {
    public:
        typedef Dictionary<Value> AttributeMap;

        explicit CoreObject();

        virtual ~CoreObject();

        virtual Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const = 0;

        /**
         * Returns true if object is instance of given class.
         */
        bool IsInstance(const Handle<Interpreter>& interpreter, const Handle<Class>& cls) const;

        /**
         * Returns all attributes which the object has in an dictionary.
         */
        virtual Dictionary<Value> GetAllAttributes() const = 0;

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

        virtual bool IsFile() const
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

        virtual bool IsMap() const
        {
            return false;
        }

        virtual bool IsRange() const
        {
            return false;
        }

        virtual bool IsSet() const
        {
            return false;
        }

        virtual bool IsUnboundMethod() const
        {
            return false;
        }

        void RegisterValue(Value* value);

        void UnregisterValue(Value* value);

    private:
        /** Pointer to first value which references this object. */
        Value* m_value_head;
        /** Pointer to last value which references this object. */
        Value* m_value_tail;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CoreObject);
    };
}

#endif /* !TEMPEARLY_COREOBJECT_H_GUARD */
