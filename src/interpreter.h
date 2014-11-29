#ifndef TEMPEARLY_INTERPRETER_H_GUARD
#define TEMPEARLY_INTERPRETER_H_GUARD

#include "frame.h"
#include "sapi/request.h"
#include "sapi/response.h"

namespace tempearly
{
    class Interpreter : public CountedObject
    {
    public:
        explicit Interpreter();

        ~Interpreter();

        void Initialize();

        bool Include(const Filename& filename);

        Value Import(const Filename& filename);

        Handle<Class> AddClass(const String& name,
                               const Handle<Class>& base);

        void AddFunction(const String& name,
                         int arity,
                         Value(*callback)(const Handle<Interpreter>&,
                                          const Vector<Value>&));

        /**
         * Returns current stack frame or null handle if nothing is being
         * executed by this interpreter.
         */
        inline Handle<Frame> GetFrame() const
        {
            return m_frame;
        }

        /**
         * Pushes new stack frame in the frame chain.
         *
         * \param enclosing Optional handle of enclosing frame
         * \param function  Optional handle of function being executed
         */
        void PushFrame(const Handle<Frame>& enclosing = Handle<Frame>(),
                       const Handle<FunctionObject>& function = Handle<FunctionObject>());

        /**
         * Pops the most recent stack frame from frame chain.
         */
        void PopFrame();

        /**
         * Searches for an global variable with specified name.
         *
         * \param id Name of the variable to look for
         * \return   A boolean flag indicating whether variable with given name
         *           was found or not
         */
        bool HasGlobalVariable(const String& id) const;

        /**
         * Attempts to retrieve an global variable with specified name.
         *
         * \param id   Name of the variable to look for
         * \param slot Where value of the variable will be assigned to
         * \return     A boolean flag indicating whether a variable with given
         *             name was found or not
         */
        bool GetGlobalVariable(const String& id, Value& slot) const;

        /**
         * Assigns a new global variable with given name and value. Existing
         * global variables with same name will be overridden.
         *
         * \param id    Name of the variable
         * \param value Value of the variable
         */
        void SetGlobalVariable(const String& id, const Value& value);

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
        inline const Value& GetException() const
        {
            return m_exception;
        }

        /**
         * Sets currently uncaught exception.
         */
        inline void SetException(const Value& exception)
        {
            m_exception = exception;
        }

        /**
         * Clears current uncaught exception if such exists.
         */
        inline void ClearException()
        {
            m_exception.Clear();
        }

        /**
         * Returns currently caught exception or NULL handle if there isn't
         * any.
         */
        inline const Value& GetCaughtException() const
        {
            return m_caught_exception;
        }

        /**
         * Sets currently caught exception.
         */
        inline void SetCaughtException(const Value& caught_exception)
        {
            m_caught_exception = caught_exception;
        }

        /**
         * Clears current caught exception if such exists.
         */
        inline void ClearCaughtException()
        {
            m_caught_exception.Clear();
        }

        /**
         * Throws an exception and sets this interpreter into exception state.
         *
         * \param cls     Exception class
         * \param message Exception message
         */
        void Throw(const Handle<Class>& cls, const String& message);

        /**
         * Returns shared instance of empty iterator.
         */
        Handle<IteratorObject> GetEmptyIterator();

        /**
         * Used by garbage collector to mark all objects used by the
         * interpreter.
         */
        void Mark();

        Request* request;
        Response* response;

        Handle<Class> cBinary;
        Handle<Class> cBool;
        Handle<Class> cClass;
        Handle<Class> cException;
        Handle<Class> cFile;
        Handle<Class> cFloat;
        Handle<Class> cFunction;
        Handle<Class> cInt;
        Handle<Class> cIterable;
        Handle<Class> cIterator;
        Handle<Class> cList;
        Handle<Class> cMap;
        Handle<Class> cNum;
        Handle<Class> cObject;
        Handle<Class> cRange;
        Handle<Class> cSet;
        Handle<Class> cFileStream;
        Handle<Class> cString;
        Handle<Class> cVoid;

        Handle<Class> eArithmeticError;
        Handle<Class> eAttributeError;
        Handle<Class> eKeyError;
        Handle<Class> eImportError;
        Handle<Class> eIndexError;
        Handle<Class> eIOError;
        Handle<Class> eLookupError;
        Handle<Class> eNameError;
        Handle<Class> eStateError;
        Handle<Class> eStopIteration;
        Handle<Class> eSyntaxError;
        Handle<Class> eTypeError;
        Handle<Class> eValueError;
        Handle<Class> eZeroDivisionError;

    private:
        /** Current stack frame. */
        Frame* m_frame;
        /** Container for global variables. */
        Dictionary<Value>* m_global_variables;
        /** Current uncaught exception. */
        Value m_exception;
        /** Current caught exception. */
        Value m_caught_exception;
        /** Shared instance of empty iterator. */
        IteratorObject* m_empty_iterator;
        /** Container for imported files. */
        Dictionary<Value>* m_imported_files;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Interpreter);
    };
}

#endif /* !TEMPEARLY_INTERPRETER_H_GUARD */
