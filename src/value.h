#ifndef TEMPEARLY_VALUE_H_GUARD
#define TEMPEARLY_VALUE_H_GUARD

#include "coreobject.h"

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
            KIND_OBJECT = 6
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
         * Constructs boolean value.
         */
        Value(bool value);

        /**
         * Constructs integer value.
         */
        Value(int value);

        /**
         * Constructs floating point decimal value.
         */
        Value(double value);

        /**
         * Constructs string value.
         */
        static Value NewString(const String& string);

        /**
         * Constructs value from object.
         *
         * \param object Handle to the object
         */
        static Value NewObject(const Handle<CoreObject>& object);

        /**
         * Destructor.
         */
        virtual ~Value();

        /**
         * Returns null value.
         */
        static const Value& NullValue();

        /**
         * Assignment operator.
         */
        Value& operator=(const Value& that);

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
        bool IsInstance(const Handle<Interpreter>& interpreter,
                        const Handle<Class>& cls) const;

        inline bool IsNull() const
        {
            return m_kind == KIND_NULL;
        }

        inline bool IsBool() const
        {
            return m_kind == KIND_BOOL;
        }

        inline bool IsInt() const
        {
            return m_kind == KIND_INT;
        }

        inline bool IsFloat() const
        {
            return m_kind == KIND_FLOAT;
        }

        inline bool IsFunction() const
        {
            return m_kind == KIND_OBJECT && m_data.o->IsFunction();
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
                   const std::vector<Value>& args = std::vector<Value>()) const;

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

        inline bool AsBool() const
        {
            return m_data.i != 0;
        }

        inline int AsInt() const
        {
            return m_data.i;
        }

        inline double AsFloat() const
        {
            return m_data.f;
        }

        inline const String& AsString() const
        {
            return *m_data.s;
        }

        inline Handle<CoreObject> AsObject() const
        {
            return m_data.o;
        }

        bool ToBool(const Handle<Interpreter>& interpreter, bool& value) const;

        bool ToString(const Handle<Interpreter>& interpreter, String& string) const;

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
            bool b;
            int i;
            double f;
            String* s;
            CoreObject* o;
        } m_data;
    };
}

#endif /* !TEMPEARLY_VALUE_H_GUARD */
