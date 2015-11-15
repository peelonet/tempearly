#ifndef TEMPEARLY_API_FUNCTION_H_GUARD
#define TEMPEARLY_API_FUNCTION_H_GUARD

#include "customobject.h"

namespace tempearly
{
    /**
     * Abstract base class for function objects.
     */
    class FunctionObject : public CustomObject
    {
    public:
        typedef void (*Callback)(
            const Handle<Interpreter>&,
            const Handle<Frame>&,
            const Vector<Handle<Object>>&
        );

        /**
         * Default constructor.
         */
        explicit FunctionObject(
            const Handle<Interpreter>& interpreter,
            const Handle<Frame>& enclosing_frame = Handle<Frame>()
        );

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
        static Handle<FunctionObject> NewScripted(
            const Handle<Interpreter>& interpreter,
            const Vector<Handle<Parameter> >& parameters,
            const Vector<Handle<Node> >& nodes
        );

        /**
         * Constructs unbound method suitable to be added as an attribute of a
         * class.
         *
         * \param interpreter Script interpreter
         * \param cls         Class which declared the method
         * \param arity
         * \param callback
         */
        static Handle<FunctionObject> NewUnboundMethod(
            const Handle<Interpreter>& interpreter,
            const Handle<Class>& cls,
            int arity,
            Callback callback
        );

        /**
         * Returns name of the function or "<anonymous function>" if this
         * function has no name.
         */
        String GetName() const;

        /**
         * Invokes the function.
         *
         * \param interpreter Script interpreter
         * \param slot        Where result of the function execution will be
         *                    assigned to
         * \param args        Arguments given for the function
         * \return            A boolean flag indicating whether execution of
         *                    the function was successfull or not, or whether
         *                    an exception was thrown
         */
        bool Invoke(
            const Handle<Interpreter>& interpreter,
            Handle<Object>& slot,
            const Vector<Handle<Object>>& args
        );

        /**
         * Invokes the function. Return value of the function will be ignored.
         *
         * \param interpreter Script interpreter
         * \param args        Arguments given for the function
         * \return            A boolean flag indicating whether execution of
         *                    the function was successfull or not, or whether
         *                    an exception was thrown
         */
        bool Invoke(
            const Handle<Interpreter>& interpreter,
            const Vector<Handle<Object>>& args
        );

        /**
         * Invokes the function with already constructed stack frame.
         *
         * \param interpreter Script interpreter
         * \param frame       Stack frame containing details about the function
         *                    invocation such as arguments
         * \return            A boolean flag indicating whether execution of
         *                    the function was successfull or not, or whether
         *                    an exception was thrown
         */
        virtual bool Invoke(
            const Handle<Interpreter>& interpreter,
            const Handle<Frame>& frame
        ) = 0;

        /**
         * Creates curry function which uses this function as it's base.
         *
         * \param interpreter Script interpreter
         * \param args        Additional arguments for the function
         * \return            New function which curries this one with given
         *                    arguments
         */
        Handle<FunctionObject> Curry(
            const Handle<Interpreter>& interpreter,
            const Vector<Handle<Object>>& args
        );

        bool IsFunction() const
        {
            return true;
        }

        virtual void Mark();

    private:
        Frame* m_enclosing_frame;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FunctionObject);
    };
}

#endif /* !TEMPEARLY_API_FUNCTION_H_GUARD */
