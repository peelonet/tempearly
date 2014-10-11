#include "interpreter.h"
#include "api/iterator.h"

namespace tempearly
{
    /**
     * Void#__iter__() => Iterator
     *
     * Returns an iterator which iterates nothing.
     */
    TEMPEARLY_NATIVE_METHOD(void_iter)
    {
        return Value::NewObject(interpreter->GetEmptyIterator());
    }

    /**
     * Void#__str__() => String
     *
     * String representation of "null" is empty string.
     */
    TEMPEARLY_NATIVE_METHOD(void_str)
    {
        return Value::NewString(String());
    }

    void init_void(Interpreter* i)
    {
        i->cVoid = i->AddClass("Void", i->cObject);

        i->cVoid->SetAllocator(Class::kNoAlloc);

        i->cVoid->AddMethod(i, "__iter__", 0, void_iter);

        // Conversion methods
        i->cVoid->AddMethod(i, "__str__", 0, void_str);
    }
}
