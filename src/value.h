#ifndef TEMPEARLY_VALUE_H_GUARD
#define TEMPEARLY_VALUE_H_GUARD

#include "coreobject.h"
#include "core/vector.h"

namespace tempearly
{
    /**
     * Value is the main ingredient of scripting engine. Everything is always
     * based on values. Value has a type (class) and well, a value, which is
     * either a boolean, integer number, floating point decimal, string or
     * reference to an object.
     */
    class Value
    {
    public:
        enum Kind
        {
            KIND_ERROR  = 0,
            KIND_NULL   = 1,
            KIND_BOOL   = 2,
            KIND_INT    = 3,
            KIND_FLOAT  = 4,
            KIND_STRING = 5,
            KIND_BINARY = 6,
            KIND_OBJECT = 7
        };

        /**
         * Constructs error value.
         */
        Value();

        /**
         * Constructs copy of existing value which references the same class
         * and object as the original one.
         *
         * \param that Other value to construct copy of
         */
        Value(const Value& that);

        /**
         * Constructs value from object.
         *
         * \param object Handle to the object
         */
        Value(const Handle<CoreObject>& object);

        /**
         * Destructor.
         */
        virtual ~Value();

        /**
         * Returns null value.
         */
        static const Value& NullValue();

        /**
         * Constructs boolean value.
         */
        static Value NewBool(bool b);

        /**
         * Constructs integer value.
         */
        static Value NewInt(i64 number);

        /**
         * Constructs floating point decimal value.
         */
        static Value NewFloat(double number);

        /**
         * Constructs string value.
         */
        static Value NewString(const String& string);

        /**
         * Constructs binary value.
         */
        static Value NewBinary(const ByteString& bytes);

        /**
         * Assignment operator.
         */
        Value& operator=(const Value& that);

        /**
         * Assignment operator.
         */
        Value& operator=(const Handle<CoreObject>& object);

        /**
         * Returns kind of the value.
         */
        inline Kind GetKind() const
        {
            return m_kind;
        }

        /**
         * Tests whether value is given kind.
         */
        inline bool Is(Kind kind) const
        {
            return m_kind == kind;
        }

        /**
         * Tests whether the value is instance of given class.
         */
        bool IsInstance(const Handle<Interpreter>& interpreter, const Handle<Class>& cls) const;

        inline bool IsNull() const
        {
            return m_kind == KIND_NULL;
        }

        inline bool IsBinary() const
        {
            return m_kind == KIND_BINARY;
        }

        inline bool IsBool() const
        {
            return m_kind == KIND_BOOL;
        }

        inline bool IsClass() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsClass();
        }

        inline bool IsInt() const
        {
            return m_kind == KIND_INT;
        }

        inline bool IsException() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsException();
        }

        inline bool IsFile() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsFile();
        }

        inline bool IsFloat() const
        {
            return m_kind == KIND_FLOAT;
        }

        inline bool IsFunction() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsFunction();
        }

        inline bool IsIterator() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsIterator();
        }

        inline bool IsList() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsList();
        }

        inline bool IsMap() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsMap();
        }

        inline bool IsRange() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsRange();
        }

        inline bool IsSet() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsSet();
        }

        inline bool IsStaticMethod() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsStaticMethod();
        }

        inline bool IsString() const
        {
            return m_kind == KIND_STRING;
        }

        Handle<Class> GetClass(const Handle<Interpreter>& interpreter) const;

        bool HasAttribute(const String& id) const;

        bool GetAttribute(const Handle<Interpreter>& interpreter,
                          const String& id,
                          Value& value) const;

        bool SetAttribute(const String& id, const Value& value) const;

        Value Call(const Handle<Interpreter>& interpreter,
                   const String& id,
                   const Vector<Value>& args = Vector<Value>()) const;

        Value Call(const Handle<Interpreter>& interpreter,
                   const String& id,
                   const Value& arg) const;

        /**
         * Performs equality test between two values. This is done by invoking
         * the "__eq__" method with given value as argument.
         *
         * \param interpreter Script interpreter
         * \param that        Other value to test equality with
         * \param slot        Where result of the comparison is assigned to
         * \return            A boolean flag indicating whether the comparison
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool Equals(const Handle<Interpreter>& interpreter, const Value& that, bool& slot) const;

        /**
         * Compares two values against each other using "__lt__" method and
         * assigns <code>true</code> to <em>slot</em> if this value is less
         * than the value given as argument.
         *
         * \param interpreter Script interpreter
         * \param that        Other value to compare this one against
         * \param slot        Where result of the comparison is assigned to
         * \return            A boolean flag indicating whether the comparison
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool IsLessThan(const Handle<Interpreter>& interpreter, const Value& that, bool& slot) const;

        /**
         * Treats object as iterator and attempts to retrieve it's next value.
         *
         * \param interpreter Script interpreter
         * \param slot        Where the next value will be inserted
         * \return            A boolean flag indicating whether a value was
         *                    retrieved or not, false is returned if there was
         *                    no value to retrieve, or if an exception was thrown
         */
        bool GetNext(const Handle<Interpreter>& interpreter, Value& slot) const;

        /**
         * Attempts to generate hash code for the value. This is done by
         * invoking method "__hash__" and converting it's result into
         * integer.
         *
         * \param interpreter Script interpreter
         * \param slot        Where the resulting hash code is assigned to
         * \return            A boolean flag indicating whether the operation
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool GetHash(const Handle<Interpreter>& interpreter, i64& slot) const;

        /**
         * Resets value to error state.
         */
        void Clear();

        /**
         * Used by garbage collector to mark objects which this value
         * references as marked.
         */
        void Mark() const;

        template< class T >
        inline Handle<T> As() const
        {
            return static_cast<T*>(m_data.o);
        }

        inline const ByteString& AsBinary() const
        {
            return *m_data.b;
        }

        bool AsBinary(const Handle<Interpreter>& interpreter, ByteString& slot) const;

        inline bool AsBool() const
        {
            return m_data.i != 0;
        }

        bool AsBool(const Handle<Interpreter>& interpreter, bool& slot) const;

        i64 AsInt() const;

        bool AsInt(const Handle<Interpreter>& interpreter, i64& slot) const;

        double AsFloat() const;

        bool AsFloat(const Handle<Interpreter>& interpreter, double& slot) const;

        inline const String& AsString() const
        {
            return *m_data.s;
        }

        bool AsString(const Handle<Interpreter>& interpreter, String& slot) const;

        inline Handle<CoreObject> AsObject() const
        {
            return m_data.o;
        }

        bool ToBool(const Handle<Interpreter>& interpreter, bool& value) const;

        bool ToString(const Handle<Interpreter>& interpreter, String& string) const;

        inline bool HasFlag(CountedObject::Flag flag) const
        {
            return m_kind == KIND_OBJECT && m_data.o->HasFlag(flag);
        }

        inline void SetFlag(CountedObject::Flag flag) const
        {
            if (m_kind == KIND_OBJECT)
            {
                m_data.o->SetFlag(flag);
            }
        }

        inline void UnsetFlag(CountedObject::Flag flag) const
        {
            if (m_kind == KIND_OBJECT)
            {
                m_data.o->UnsetFlag(flag);
            }
        }

        inline operator bool() const
        {
            return m_kind != KIND_ERROR;
        }

        inline bool operator!() const
        {
            return m_kind == KIND_ERROR;
        }

    private:
        Kind m_kind;
        union
        {
            i64 i;
            double f;
            String* s;
            ByteString* b;
            CoreObject* o;
        } m_data;
        /** Pointer to previous value in object's value list. */
        Value* m_previous;
        /** Pointer to next value in object's value list. */
        Value* m_next;
        friend class CoreObject;
    };
}

#endif /* !TEMPEARLY_VALUE_H_GUARD */
