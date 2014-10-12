#include "interpreter.h"
#include "core/filename.h"

namespace tempearly
{
    /**
     * include(filename)
     *
     * Includes given filename into current script.
     *
     * Throws: ImportError - If file cannot be included for some reason.
     */
    TEMPEARLY_NATIVE_METHOD(func_include)
    {
        String filename;

        if (!args[0].AsString(interpreter, filename)
            || !interpreter->Include(Filename(filename)))
        {
            return Value();
        } else {
            return Value::NewBool(true);
        }
    }

    void init_core(Interpreter* i)
    {
        i->AddFunction("include", 1, func_include);
    }
}
