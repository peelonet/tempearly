#ifndef TEMPEARLY_INTERPRETER_H_GUARD
#define TEMPEARLY_INTERPRETER_H_GUARD

#include "scope.h"
#include "api/exception.h"
#include "sapi/request.h"
#include "sapi/response.h"

namespace tempearly
{
    class Interpreter : public CountedObject
    {
    public:
        explicit Interpreter();

        void Initialize();

        bool Include(const Filename& filename);

        Handle<Class> AddClass(const String& name,
                               const Handle<Class>& base);

        /**
         * Returns true if this interpreter has an uncaught exception.
         */
        inline bool HasException() const
        {
            return !!m_exception;
        }

        /**
         * Returns currently uncaught exception or NULL handle if there isn't
         * any.
         */
        inline Handle<ExceptionObject> GetException() const
        {
            return m_exception;
        }

        /**
         * Clears current exception if such exists.
         */
        inline void ClearException()
        {
            m_exception = 0;
        }

        /**
         * Throws an exception and sets this interpreter into exception state.
         *
         * \param cls     Exception class
         * \param message Exception message
         */
        void Throw(const Handle<Class>& cls, const String& message);

        /**
         * Returns current local variable scope.
         */
        inline Handle<Scope> GetScope() const
        {
            return m_scope;
        }

        /**
         * Inserts new local variable scope into the scope chain.
         *
         * \param parent Optional parent scope in scope chain
         */
        void PushScope(const Handle<Scope>& parent = Handle<Scope>());

        /**
         * Removes topmost local variable scope from the scope chain.
         */
        void PopScope();

        /**
         * Returns shared instance of empty iterator.
         */
        Handle<IteratorObject> GetEmptyIterator();

        /**
         * Used by garbage collector to mark all objects used by the
         * interpreter.
         */
        void Mark();

        Handle<Request> request;
        Handle<Response> response;

        /** Global variable scope. */
        Handle<Scope> globals;

        Handle<Class> cBool;
        Handle<Class> cClass;
        Handle<Class> cException;
        Handle<Class> cFloat;
        Handle<Class> cFunction;
        Handle<Class> cInt;
        Handle<Class> cIterator;
        Handle<Class> cList;
        Handle<Class> cNum;
        Handle<Class> cObject;
        Handle<Class> cSet;
        Handle<Class> cString;
        Handle<Class> cVoid;

        Handle<Class> eArithmeticError;
        Handle<Class> eAttributeError;
        Handle<Class> eNameError;
        Handle<Class> eStateError;
        Handle<Class> eStopIteration;
        Handle<Class> eSyntaxError;
        Handle<Class> eTypeError;
        Handle<Class> eValueError;
        Handle<Class> eZeroDivisionError;

    private:
        /** Current uncaught exception. */
        ExceptionObject* m_exception;
        /** Current local variable scope. */
        Handle<Scope> m_scope;
        /** Shared instance of empty iterator. */
        IteratorObject* m_empty_iterator;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Interpreter);
    };
}

#endif /* !TEMPEARLY_INTERPRETER_H_GUARD */
