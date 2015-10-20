#ifndef TEMPEARLY_SCRIPT_RESULT_H_GUARD
#define TEMPEARLY_SCRIPT_RESULT_H_GUARD

#include "value.h"

namespace tempearly
{
    /**
     * Represents result of script execution.
     */
    class Result
    {
    public:
        /**
         * Type of the result.
         */
        enum Kind
        {
            /**
             * An exception was thrown.
             */
            KIND_ERROR = 0,
            /**
             * Execution was successfull.
             */
            KIND_SUCCESS = 1,
            /**
             * Execution was broken with "return" statement.
             */
            KIND_RETURN = 2,
            /**
             * Execution was broken with "break" statement.
             */
            KIND_BREAK = 3,
            /**
             * Execution was broken with "continue" statement.
             */
            KIND_CONTINUE = 4
        };

        /**
         * Constructs new script execution result.
         *
         * \param kind  Type of the result
         * \param value Optional value returned by result
         */
        explicit Result(Kind kind = KIND_SUCCESS, const Value& value = Value());

        /**
         * Constructs successfull result with an value.
         */
        Result(const Value& value);

        /**
         * Copy constructor.
         *
         * \param that Existing result to construct copy of
         */
        Result(const Result& that);

        /**
         * Assignment operator.
         */
        Result& operator=(const Result& that);

        /**
         * Returns type of the result.
         */
        inline Kind GetKind() const
        {
            return m_kind;
        }

        /**
         * Returns true if result is given kind.
         */
        inline bool Is(Kind kind) const
        {
            return m_kind == kind;
        }

        /**
         * Returns an optional value returned by the result, or null value if
         * no value was returned.
         */
        inline const Value& GetValue() const
        {
            return m_value;
        }

        /**
         * Used by garbage collector to mark value of the result.
         */
        void Mark() const;

        inline operator bool() const
        {
            return m_kind == KIND_SUCCESS;
        }

        inline bool operator!() const
        {
            return m_kind != KIND_SUCCESS;
        }

    private:
        /** Type of the result. */
        Kind m_kind;
        /** Optional value returned with the result. */
        Value m_value;
    };
}

#endif /* !TEMPEARLY_SCRIPT_RESULT_H_GUARD */
