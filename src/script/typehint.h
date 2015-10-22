#ifndef TEMPEARLY_SCRIPT_TYPEHINT_H_GUARD
#define TEMPEARLY_SCRIPT_TYPEHINT_H_GUARD

#include "memory.h"

namespace tempearly
{
    class TypeHint : public CountedObject
    {
    public:
        explicit TypeHint();

        /**
         * Contructs type hint from an expression. The expression is evaluated
         * into a value during testing and this resulting value must be an
         * class or an exception is thrown. Tested value must be instance of
         * that class or otherwise the test fails.
         */
        static Handle<TypeHint> FromExpression(const Handle<Node>& node);

        /**
         * Tests whether given value is accepted by this type hint.
         *
         * \param interpreter Script interpreter
         * \param value       Value to test
         * \param slot        This is where result of the test is assigned to
         * \return            False if an exception was thrown while testing,
         *                    true otherwise
         */
        virtual bool Accepts(
            const Handle<Interpreter>& interpreter,
            const Handle<Object>& value,
            bool& slot
        ) const = 0;

        /**
         * Constructs an nullable version of this type hint.
         */
        Handle<TypeHint> MakeNullable();

        /**
         * Constructs boolean AND version of this type hint. Boolean AND type
         * hints contains two type hints and the value must pass testing of
         * both of those.
         */
        Handle<TypeHint> MakeAnd(const Handle<TypeHint>& that);

        /**
         * Constructs boolean OR version of this type hint. Boolean OR type
         * hints contains two type hints and the value must pass either one
         * of those.
         */
        Handle<TypeHint> MakeOr(const Handle<TypeHint>& that);

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(TypeHint);
    };
}

#endif /* !TEMPEARLY_SCRIPT_TYPEHINT_H_GUARD */
