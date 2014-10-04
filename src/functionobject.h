#ifndef TEMPEARLY_FUNCTIONOBJECT_H_GUARD
#define TEMPEARLY_FUNCTIONOBJECT_H_GUARD

#include <vector>

#include "object.h"

namespace tempearly
{
    /**
     * Abstract base class for function objects.
     */
    class FunctionObject : public Object
    {
    public:
        /**
         * Default constructor.
         */
        explicit FunctionObject(const Handle<Class>& cls);

        /**
         * Default destructor.
         */
        virtual ~FunctionObject();

        /**
         * Invokes the function.
         *
         * \param interpreter Script interpreter
         * \param args        Arguments given for the function
         * \return            Value returned by the function, or error value if
         *                    an exception was thrown
         */
        virtual Value Invoke(const Handle<Interpreter>& interpreter,
                             const std::vector<Value>& args) = 0;

        /**
         * Creates curry function which uses this function as it's base.
         *
         * \param interpreter Script interpreter
         * \param args        Additional arguments for the function
         * \return            New function which curries this one with given
         *                    arguments
         */
        Value Curry(const Handle<Interpreter>& interpreter,
                    const std::vector<Value>& args);

        bool IsFunction() const
        {
            return true;
        }

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FunctionObject);
    };
}

#endif /* !TEMPEARLY_FUNCTIONOBJECT_H_GUARD */
