#ifndef TEMPEARLY_FRAME_H_GUARD
#define TEMPEARLY_FRAME_H_GUARD

#include "value.h"
#include "api/function.h"

namespace tempearly
{
    /**
     * Object presentation of stack frame. Stack frames are used by the
     * interpreter to track origins of exceptions as well as to store
     * local variables.
     */
    class Frame : public CountedObject
    {
    public:
        /**
         * Constructs new stack frame.
         *
         * \param previous        Handle to previous frame
         * \param enclosing_frame Handle to enclosing frame
         * \param function        Handle to function being executed by this
         *                        frame
         * \param receiver        Value which is calling the function
         * \param args            Arguments given for the function
         */
        explicit Frame(const Handle<Frame>& previous,
                       const Handle<Frame>& enclosing_frame,
                       const Handle<FunctionObject>& function,
                       const Value& receiver,
                       const Vector<Value>& args);

        ~Frame();

        /**
         * Returns previous frame or null handle if this is the topmost frame.
         */
        inline Handle<Frame> GetPrevious() const
        {
            return m_previous;
        }

        /**
         * Returns enclosing frame or null handle if this frame has no
         * enclosing frame.
         */
        inline Handle<Frame> GetEnclosingFrame() const
        {
            return m_enclosing_frame;
        }

        /**
         * Returns the function which is being executed by this frame or null
         * handle if this frame does not execute a function.
         */
        inline Handle<FunctionObject> GetFunction() const
        {
            return m_function;
        }

        /**
         * Returns value which is calling the function.
         */
        inline const Value& GetReceiver() const
        {
            return m_receiver;
        }

        /**
         * Returns arguments given for the function.
         */
        inline const Vector<Value>& GetArgs() const
        {
            return m_args;
        }

        /**
         * Retrieves all local variables from the frame as an object.
         *
         * \param interpreter Interpreter instance required for constructing
         *                    the object
         * \return            An object which has all the local variables which
         *                    name doesn't begin with an underscore as an
         *                    attribute
         */
        Handle<Object> GetLocalVariables(const Handle<Interpreter>& interpreter) const;

        /**
         * Returns true if the frame has a specified local variable declared in
         * it. Enclosing frames are not included in the search.
         *
         * \param id Name of the variable to look for
         * \return   A boolean flag indicating whether this frame has variable
         *           with given identifier or not
         */
        bool HasLocalVariable(const String& id) const;

        /**
         * Attempts to retrieve local variable with specified name from the
         * frame. Enclosing frames are not included in the search.
         *
         * \param id   Name of the variable to look for
         * \param slot Where value of the variable will be assigned to
         * \return     A boolean flag indicating whether variable with given
         *             identifier was found or not
         */
        bool GetLocalVariable(const String& id, Value& slot) const;

        /**
         * Assigns an local variable inside this frame. Existing local
         * variables with same identifier will be overridden.
         *
         * \param id    Name of the variable
         * \param value Value of the variable
         */
        void SetLocalVariable(const String& id, const Value& value);

        /**
         * Replaces existing local variable with specified name with a new
         * value. If the local variable does not already exist, a new one
         * will not be created.
         *
         * \param id    Name of the variable to look for
         * \param value New value of the variable
         * \return      A boolean flag indicating whether variable with
         *              given name was found or not
         */
        bool ReplaceLocalVariable(const String& id, const Value& value);

        void Mark();

    private:
        /** Pointer to previous frame. */
        Frame* m_previous;
        /** Pointer to enclosing frame. */
        Frame* m_enclosing_frame;
        /** Pointer to function being executed by this frame. */
        FunctionObject* m_function;
        /** Value which is calling the function. */
        const Value m_receiver;
        /** Arguments given for the function. */
        const Vector<Value> m_args;
        /** Container for local variables. */
        Dictionary<Value>* m_local_variables;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Frame);
    };
}

#endif /* !TEMPEARLY_FRAME_H_GUARD */
