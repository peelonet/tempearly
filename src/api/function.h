#ifndef TEMPEARLY_API_FUNCTION_H_GUARD
#define TEMPEARLY_API_FUNCTION_H_GUARD

#include "api/object.h"

namespace tempearly
{
    /**
     * Abstract base class for function objects.
     */
    class FunctionObject : public Object
    {
    public:
        typedef void (*Callback)(const Handle<Interpreter>&, const Handle<Frame>&, const Vector<Value>&);

        /**
         * Default constructor.
         */
        explicit FunctionObject(const Handle<Interpreter>& interpreter);

        /**
         * Default destructor.
         */
        virtual ~FunctionObject();

        /**
         * Constructs scripted function.
         *
         * \param interpreter Script interpreter
         * \param parameters  Parameters for the function
         * \param nodes       Function body
         */
        static Handle<FunctionObject> NewScripted(const Handle<Interpreter>& interpreter,
                                                  const Vector<Handle<Parameter> >& parameters,
                                                  const Vector<Handle<Node> >& nodes);

        /**
         * Invokes the function.
         *
         * \param interpreter Script interpreter
         * \param args        Arguments given for the function
         * \param slot        Where result of the function execution will be
         *                    assigned to
         * \return            A boolean flag indicating whether execution of
         *                    the function was successfull or not, or whether
         *                    an exception was thrown
         */
        virtual bool Invoke(const Handle<Interpreter>& interpreter,
                            const Vector<Value>& args,
                            Value& slot) = 0;

        /**
         * Creates curry function which uses this function as it's base.
         *
         * \param interpreter Script interpreter
         * \param args        Additional arguments for the function
         * \return            New function which curries this one with given
         *                    arguments
         */
        Value Curry(const Handle<Interpreter>& interpreter, const Vector<Value>& args);

        bool IsFunction() const
        {
            return true;
        }

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FunctionObject);
    };
}

#endif /* !TEMPEARLY_API_FUNCTION_H_GUARD */
