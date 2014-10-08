#include "api/class.h"

namespace tempearly
{
    CoreObject::CoreObject() {}

    bool CoreObject::IsInstance(const Handle<Interpreter>& interpreter,
                                const Handle<Class>& cls) const
    {
        return GetClass(interpreter)->IsSubclassOf(cls);
    }
}
