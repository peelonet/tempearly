#ifndef TEMPEARLY_API_ITERATOR_H_GUARD
#define TEMPEARLY_API_ITERATOR_H_GUARD

#include "object.h"
#include "result.h"

namespace tempearly
{
    /**
     * Base class for iterator objects.
     */
    class IteratorObject : public Object
    {
    public:
        /**
         * Default constructor.
         *
         * \param cls Class of the iterator
         */
        explicit IteratorObject(const Handle<Class>& cls);

        /**
         * Peeks the next value from iteration without removing it.
         *
         * \param interpreter Script interpreter
         */
        Value Peek(const Handle<Interpreter>& interpreter);

        /**
         * Returns next value from iteration.
         *
         * \param interpreter Script interpreter
         */
        Value Next(const Handle<Interpreter>& interpreter);

        /**
         * Feeds a pushback value into the iterator.
         *
         * \param value Value to insert
         */
        void Feed(const Value& value);

        /**
         * Generates an iteration.
         */
        virtual Result Generate(const Handle<Interpreter>& interpreter) = 0;

        bool IsIterator() const
        {
            return true;
        }

        /**
         * Used by garbage collector to mark the object.
         */
        virtual void Mark();

    private:
        /** Container for pushback values. */
        std::vector<Value> m_pushback;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IteratorObject);
    };
}

#endif /* !TEMPEARLY_API_ITERATOR_H_GUARD */
