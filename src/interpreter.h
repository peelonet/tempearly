#ifndef TEMPEARLY_INTERPRETER_H_GUARD
#define TEMPEARLY_INTERPRETER_H_GUARD

#include "class.h"
#include "request.h"
#include "response.h"
#include "scope.h"

namespace tempearly
{
    class Interpreter : public CountedObject
    {
    public:
        explicit Interpreter();

        void Initialize();

        bool Include(const String& filename);

        Handle<Class> AddClass(const String& name,
                               const Handle<Class>& base);

        inline bool HasException() const
        {
            return !!m_exception;
        }

        inline const Value& GetException() const
        {
            return m_exception;
        }

        inline void SetException(const Value& exception)
        {
            m_exception = exception;
        }

        /**
         * Throws an exception and sets this interpreter into exception state.
         *
         * \param cls     Exception class
         * \param message Exception message
         */
        void Throw(const Handle<Class>& cls, const String& message);

        void Mark();

        Handle<Request> request;
        Handle<Response> response;

        /** Global variable scope. */
        Handle<Scope> globals;

        Handle<Class> cBool;
        Handle<Class> cClass;
        Handle<Class> cFloat;
        Handle<Class> cFunction;
        Handle<Class> cInt;
        Handle<Class> cNum;
        Handle<Class> cObject;
        Handle<Class> cString;
        Handle<Class> cVoid;

        Handle<Class> eAttributeError;
        Handle<Class> eSyntaxError;
        Handle<Class> eTypeError;

    private:
        /** Current uncaught exception. */
        Value m_exception;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Interpreter);
    };
}

#endif /* !TEMPEARLY_INTERPRETER_H_GUARD */
