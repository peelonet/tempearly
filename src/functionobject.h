#ifndef TEMPEARLY_FUNCTIONOBJECT_H_GUARD
#define TEMPEARLY_FUNCTIONOBJECT_H_GUARD

#include "object.h"

namespace tempearly
{
    /**
     * Abstract base class for function objects.
     */
    class FunctionObject : public Object
    {
    public:
        typedef Value(*Callback)(const Handle<Interpreter>&,
                                 const std::vector<Value>&);

        /**
         * Default constructor.
         */
        explicit FunctionObject(const Handle<Class>& cls);

        /**
         * Default destructor.
         */
        virtual ~FunctionObject();

        /**
         * Constructs method.
         *
         * \param interpreter Script interpreter
         * \param cls         Class which declares the method
         * \param name        Name of the method
         * \param arity       Method arity, e.g. how many arguments the method
         *                    takes
         * \param callback    Callback function which is invoked when the method
         *                    is invoked
         * \return            Value which is the constructed method
         */
        static Value NewMethod(const Handle<Interpreter>& interpreter,
                               const Handle<Class>& cls,
                               const String& name,
                               int arity,
                               Callback callback);

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
