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
        return Value(interpreter->GetEmptyIterator());
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

    /**
     * Void#as_json() => String
     *
     * Returns "null".
     */
    TEMPEARLY_NATIVE_METHOD(void_as_json)
    {
        return Value::NewString("null");
    }

    void init_void(Interpreter* i)
    {
        Handle<Class> cVoid = i->AddClass("Void", i->cIterable);

        i->cVoid = cVoid;

        cVoid->SetAllocator(Class::kNoAlloc);

        cVoid->AddMethod(i, "__iter__", 0, void_iter);

        // Conversion methods
        cVoid->AddMethod(i, "__str__", 0, void_str);
        cVoid->AddMethod(i, "as_json", 0, void_as_json);
    }
}
