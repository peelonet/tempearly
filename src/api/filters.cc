#include "interpreter.h"
#include "api/class.h"

namespace tempearly
{
    /**
     * Object#escape_js() => String
     *
     * Converts object into string and escapes all characters which could cause
     * problems when embedded into JavaScript string literal. Resulting string
     * will be safe to use inside a JavaScript string literal.
     */
    TEMPEARLY_NATIVE_METHOD(obj_escape_js)
    {
        String string;

        if (args[0].ToString(interpreter, string))
        {
            return Value::NewString(string.EscapeJavaScript());
        } else {
            return Value();
        }
    }

    /**
     * Object#escape_xml() => String
     *
     * Converts object into string and escapes all XML entities from it. The
     * resulting string will be safe to use inside XML attribute or text node.
     */
    TEMPEARLY_NATIVE_METHOD(obj_escape_xml)
    {
        String string;

        if (args[0].ToString(interpreter, string))
        {
            return Value::NewString(string.EscapeXml());
        } else {
            return Value();
        }
    }

    void init_filters(Interpreter* i)
    {
        i->cObject->AddMethod(i, "escape_js", 0, obj_escape_js);
        i->cObject->AddMethod(i, "escape_xml", 0, obj_escape_xml);
    }
}
