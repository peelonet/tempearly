#include "interpreter.h"
#include "api/file.h"

namespace tempearly
{
    static bool value_to_file(const Handle<Interpreter>& interpreter, const Value& value, Filename& slot)
    {
        if (value.IsFile())
        {
            slot = value.As<FileObject>()->GetPath();

            return true;
        } else {
            String string;

            if (value.AsString(interpreter, string))
            {
                slot = string;

                return true;
            }

            return false;
        }
    }

    /**
     * include(filename)
     *
     * Includes given file into current script.
     *
     * Throws: ImportError - If file cannot be included for some reason.
     */
    TEMPEARLY_NATIVE_METHOD(func_include)
    {
        Filename file;

        if (value_to_file(interpreter, args[0], file) && interpreter->Include(file))
        {
            return Value::NewBool(true);
        } else {
            return Value();
        }
    }

    /**
     * import(filename) => Map
     *
     * Includes given file into current script and returns it's variable scope
     * as hash map.
     *
     * Files are imported only once, their resulting variable scope is cached
     * into the interpreter and the cached hash map is returned on subsequent
     * import calls of the same file.
     *
     * This is usually used to import common utility functions declared in
     * other files.
     */
    TEMPEARLY_NATIVE_METHOD(func_import)
    {
        Filename file;

        if (value_to_file(interpreter, args[0], file))
        {
            return interpreter->Import(file);
        } else {
            return Value();
        }
    }

    void init_core(Interpreter* i)
    {
        i->AddFunction("include", 1, func_include);
        i->AddFunction("import", 1, func_import);
    }
}
