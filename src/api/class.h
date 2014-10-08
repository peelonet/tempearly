#ifndef TEMPEARLY_API_CLASS_H_GUARD
#define TEMPEARLY_API_CLASS_H_GUARD

#include "coreobject.h"

namespace tempearly
{
    class Class : public CoreObject
    {
    public:
        explicit Class(const Handle<Class>& base);

        virtual ~Class();

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        /**
         * Returns name of the class.
         */
        String GetName() const;

        inline Handle<Class> GetBase() const
        {
            return m_base;
        }

        /**
         * Returns true if type is subclass of given type, either direct or
         * virtual. Type is considered to be subclass of itself.
         */
        bool IsSubclassOf(const Handle<Class>& that) const;

        bool HasAttribute(const String& id) const;

        bool GetAttribute(const String& id, Value& value) const;

        void SetAttribute(const String& id, const Value& value);

        void AddMethod(const Handle<Interpreter>& interpreter,
                       const String& name,
                       int arity,
                       Value(*callback)(const Handle<Interpreter>&,
                                        const std::vector<Value>&));

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

#endif /* !TEMPEARLY_API_CLASS_H_GUARD */
