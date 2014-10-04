#ifndef TEMPEARLY_MEMORY_H_GUARD
#define TEMPEARLY_MEMORY_H_GUARD

#include "tempearly.h"

namespace tempearly
{
    class CountedObject
    {
    public:
        explicit CountedObject();

        /**
         * Returns true if this object has been marked by the garbage collector.
         */
        inline bool IsMarked() const
        {
            return m_marked;
        }

        /**
         * This method is used by the garbage collector to mark all the objects
         * which are still accessible and not going to be trashed. Dervived
         * classes should override this method and proceed with marking all zone
         * allocated objects which they use (and thus should not be trashed).
         */
        virtual void Mark();

        /**
         * Increments internal reference counter of the object.
         */
        inline void IncReferenceCounter()
        {
            ++m_reference_counter;
        }

        /**
         * Decrements internal reference counter of the object.
         */
        inline void DecReferenceCounter()
        {
            --m_reference_counter;
        }

        void* operator new(std::size_t);
        void operator delete(void*);

    private:
        /** Whether the object has been marked or not. */
        bool m_marked;
        /** Reference counter. */
        unsigned int m_reference_counter;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CountedObject);
    };

    /**
     * Handle is a smart pointer implementation for garbage collected objects.
     * It automatically increments and decrements reference counter of the
     * wrapped object which makes handles as the recommended way of referencing
     * garbage collected objects.
     */
    template< class T >
    class Handle
    {
    public:
        /**
         * Constructs NULL handle.
         */
        Handle()
            : m_pointer(0) {}

        /**
         * Copy constructor.
         */
        template< class U >
        Handle(const Handle<U>& that)
            : m_pointer(that.Get())
        {
            if (m_pointer)
            {
                m_pointer->IncReferenceCounter();
            }
        }

        /**
         * Constructs handle from pointer to an object.
         */
        template< class U >
        Handle(U* pointer)
            : m_pointer(pointer)
        {
            if (m_pointer)
            {
                m_pointer->IncReferenceCounter();
            }
        }

        /**
         * Handle destructor. Decrements reference counter of the wrapped
         * object unless this handle is pointing to a NULL pointer.
         */
        virtual ~Handle()
        {
            if (m_pointer)
            {
                m_pointer->DecReferenceCounter();
            }
        }

        /**
         * Replaces object with object from given handle.
         *
         * \param that New object to replace current with
         */
        template< class U >
        Handle& operator=(const Handle<U>& that)
        {
            U* pointer = that.Get();

            if (m_pointer != pointer)
            {
                if (m_pointer)
                {
                    m_pointer->DecReferenceCount();
                }
                if ((m_pointer = pointer))
                {
                    m_pointer->IncReferenceCount();
                }
            }

            return *this;
        }

        /**
         * Returns pointer to the managed object.
         */
        inline T* Get() const
        {
            return m_pointer;
        }

        /**
         * Dereferences pointer to the managed object.
         */
        inline T* operator->() const
        {
            return m_pointer;
        }

        /**
         * Casting operator. It's bad design but useful in some situtations.
         */
        inline operator T*() const
        {
            return m_pointer;
        }

        /**
         * Equality testing operator.
         */
        template< class U >
        inline bool operator==(const Handle<U>& that) const
        {
            return m_pointer == that.Get();
        }

        /**
         * Non-equality testing operator.
         */
        template< class U >
        inline bool operator!=(const Handle<U>& that) const
        {
            return m_pointer != that.Get();
        }

        /**
         * Equality testing operator.
         */
        template< class U >
        inline bool operator==(const U* pointer) const
        {
            return m_pointer == pointer;
        }

        /**
         * Non-equality testing operator.
         */
        template< class U >
        inline bool operator!=(const U* pointer) const
        {
            return m_pointer != pointer;
        }

        /**
         * Boolean coercion. Returns true if the wrapped object is not NULL.
         */
        inline operator bool() const
        {
            return !!m_pointer;
        }

        /**
         * Boolean negation. Returns true if the wrapped object is NULL.
         */
        inline bool operator!() const
        {
            return !m_pointer;
        }

    private:
        /** Pointer to the managed object. */
        T* m_pointer;
    };
}

#endif /* !TEMPEARLY_MEMORY_H_GUARD */
