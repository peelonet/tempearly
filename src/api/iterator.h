#ifndef TEMPEARLY_API_ITERATOR_H_GUARD
#define TEMPEARLY_API_ITERATOR_H_GUARD

#include "customobject.h"
#include "script/result.h"

namespace tempearly
{
    /**
     * Base class for iterator objects.
     */
    class IteratorObject : public CustomObject
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
         * \param slot        Where the next value will be assigned into
         * \return            True if there are more values to be iterated,
         *                    false otherwise or if there was an exception
         *                    being thrown
         */
        bool Peek(const Handle<Interpreter>& interpreter);

        /**
         * Peeks the next value from iteration without removing it.
         *
         * \param interpreter Script interpreter
         * \param slot        Where the next value will be assigned into
         * \return            True if there are more values to be iterated,
         *                    false otherwise, or if there was an exception
         *                    being thrown
         */
        bool Peek(
            const Handle<Interpreter>& interpreter,
            Handle<Object>& slot
        );

        /**
         * Returns next value from iteration.
         *
         * \param interpreter Script interpreter
         */
        bool Next(const Handle<Interpreter>& interpreter,
                  Handle<Object>& slot);

        /**
         * Feeds a pushback value into the iterator.
         *
         * \param value Value to insert
         */
        void Feed(const Handle<Object>& value);

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
        Vector<Object*> m_pushback;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(IteratorObject);
    };
}

#endif /* !TEMPEARLY_API_ITERATOR_H_GUARD */
