#ifndef TEMPEARLY_OBJECT_H_GUARD
#define TEMPEARLY_OBJECT_H_GUARD

#include "core/dictionary.h"
#include "core/vector.h"

namespace tempearly
{
    class Object : public CountedObject
    {
    public:
        explicit Object();

        virtual ~Object();

        /**
         * Constructs new null object.
         */
        static Handle<Object> NewNull();

        /**
         * Constructs boolean object.
         */
        static Handle<Object> NewBool(bool value);

        /**
         * Constructs integer object.
         */
        static Handle<Object> NewInt(i64 value);

        /**
         * Constructs floating point decimal object.
         */
        static Handle<Object> NewFloat(double value);

        /**
         * Constructs string value.
         */
        static Handle<Object> NewString(const String& value);

        /**
         * Constructs binary value.
         */
        static Handle<Object> NewBinary(const ByteString& value);

        virtual Handle<Class> GetClass(
            const Handle<Interpreter>& interpreter
        ) const = 0;

        /**
         * Returns true if object is instance of given class.
         */
        bool IsInstance(
            const Handle<Interpreter>& interpreter,
            const Handle<Class>& cls
        ) const;

        bool GetAttribute(
            const Handle<Interpreter>& interpreter,
            const String& name,
            Handle<Object>& slot
        );

        /**
         * Returns all the attributes which the object has in an dictionary.
         */
        virtual Dictionary<Handle<Object>> GetOwnAttributes() const = 0;

        virtual bool GetOwnAttribute(
            const String& name,
            Handle<Object>& slot
        ) const = 0;

        virtual bool SetOwnAttribute(
            const String& name,
            const Handle<Object>& value
        ) = 0;

        bool CallMethod(
            const Handle<Interpreter>& interpreter,
            Handle<Object>& slot,
            const String& method_name,
            const Vector<Handle<Object>>& args = Vector<Handle<Object>>()
        );

        bool CallMethod(
            const Handle<Interpreter>& interpreter,
            Handle<Object>& slot,
            const String& method_name,
            const Handle<Object>& arg
        );

        bool CallMethod(
            const Handle<Interpreter>& interpreter,
            const String& method_name,
            const Vector<Handle<Object>>& args = Vector<Handle<Object>>()
        );

        bool CallMethod(
            const Handle<Interpreter>& interpreter,
            const String& method_name,
            const Handle<Object>& arg
        );

        /**
         * Performs equality test between two object. This is done by invoking
         * the "__eq__" method with given object as argument.
         *
         * \param interpreter Script interpreter
         * \param that        Other object to test equality with
         * \param slot        Where result of the comparison is assigned to
         * \return            A boolean flag indicating whether the comparison
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool Equals(
            const Handle<Interpreter>& interpreter,
            const Handle<Object>& that,
            bool& slot
        );

        /**
         * Compares two objects against each other using "__lt__" method and
         * assigns <code>true</code> to <em>slot</em> if this object is less
         * than the object given as argument.
         *
         * \param interpreter Script interpreter
         * \param that        Other object to compare this one against
         * \param slot        Where result of the comparison is assigned to
         * \return            A boolean flag indicating whether the comparison
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool IsLessThan(
            const Handle<Interpreter>& interpreter,
            const Handle<Object>& that,
            bool& slot
        );

        /**
         * Treats object as iterator and attempts to retrieve it's next value.
         *
         * \param interpreter Script interpreter
         * \param slot        Where the next value will be inserted
         * \return            A boolean flag indicating whether a value was
         *                    retrieved or not, false is returned if there was
         *                    no value to retrieve, or if an exception was thrown
         */
        bool GetNext(
            const Handle<Interpreter>& interpreter,
            Handle<Object>& slot
        );

        /**
         * Attempts to generate hash code for the object. This is done by
         * invoking method "__hash__" and converting it's result into
         * integer.
         *
         * \param interpreter Script interpreter
         * \param slot        Where the resulting hash code is assigned to
         * \return            A boolean flag indicating whether the operation
         *                    was successfull or whether an exception was
         *                    thrown
         */
        bool GetHash(
            const Handle<Interpreter>& interpreter,
            i64& slot
        );

        virtual ByteString AsBinary() const;

        bool AsBinary(
            const Handle<Interpreter>& interpreter,
            ByteString& slot
        ) const;

        virtual bool AsBool() const;

        bool AsBool(
            const Handle<Interpreter>& interpreter,
            bool& slot
        ) const;

        virtual i64 AsInt() const;

        bool AsInt(
            const Handle<Interpreter>& interpreter,
            i64& slot
        ) const;

        virtual double AsFloat() const;

        bool AsFloat(
            const Handle<Interpreter>& interpreter,
            double& slot
        ) const;

        virtual String AsString() const;

        bool AsString(
            const Handle<Interpreter>& interpreter,
            String& slot
        ) const;

        bool ToBool(
            const Handle<Interpreter>& interpreter,
            bool& slot
        );

        bool ToString(
            const Handle<Interpreter>& interpreter,
            String& slot
        );

        virtual bool IsBinary() const
        {
            return false;
        }

        virtual bool IsBool() const
        {
            return false;
        }

        virtual bool IsClass() const
        {
            return false;
        }

        virtual bool IsException() const
        {
            return false;
        }

        virtual bool IsFile() const
        {
            return false;
        }

        virtual bool IsFloat() const
        {
            return false;
        }

        virtual bool IsFunction() const
        {
            return false;
        }

        virtual bool IsInt() const
        {
            return false;
        }

        virtual bool IsMap() const
        {
            return false;
        }

        virtual bool IsNull() const
        {
            return false;
        }

        virtual bool IsRange() const
        {
            return false;
        }

        virtual bool IsString() const
        {
            return false;
        }

        virtual bool IsUnboundMethod() const
        {
            return false;
        }

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Object);
    };
}

#endif /* !TEMPEARLY_OBJECT_H_GUARD */
