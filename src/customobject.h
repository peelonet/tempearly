#ifndef TEMPEARLY_CUSTOMOBJECT_H_GUARD
#define TEMPEARLY_CUSTOMOBJECT_H_GUARD

#include "api/class.h"

namespace tempearly
{
    class CustomObject : public Object
    {
    public:
        explicit CustomObject(const Handle<Class>& cls);

        virtual ~CustomObject();

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        Dictionary<Handle<Object>> GetOwnAttributes() const;

        bool GetOwnAttribute(
            const String& name,
            Handle<Object>& slot
        ) const;

        bool SetOwnAttribute(
            const String& name,
            const Handle<Object>& value
        );

        void Mark();

    private:
        /** Class of the object. */
        Class* m_class;
        /** Container for object's attributes. */
        Dictionary<Object*>* m_attributes;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CustomObject);
    };
}

#endif /* !TEMPEARLY_CUSTOMOBJECT_H_GUARD */
