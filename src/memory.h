#ifndef TEMPEARLY_MEMORY_H_GUARD
#define TEMPEARLY_MEMORY_H_GUARD

#include <cstring>
#include <new>

#include "tempearly.h"

namespace tempearly
{
    /**
     * Contains various memory related utilities.
     */
    class Memory
    {
    public:
        /**
         * Allocates memory for an array and returns pointer to it.
         *
         * \param n Capacity of the allocated array
         * \return  Pointer to the allocated array storage or NULL if either
         *          capacity is zero or memory could not be allocated
         */
        template< class T >
        static inline T* Allocate(std::size_t n)
        {
            if (n > 0)
            {
                void* pointer = std::malloc(sizeof(T) * n);

                if (pointer)
                {
                    return static_cast<T*>(pointer);
                }
            }

            return nullptr;
        }

        /**
         * Unallocates memory allocated with <code>Allocate</code> function.
         * Basically a NULL safe version of standard library function
         * <code>std::free</code>.
         *
         * \param pointer Pointer to the allocated memory which is going to be
         *                unallocated
         */
        template< class T >
        static inline void Unallocate(T* pointer)
        {
            if (pointer)
            {
                std::free(static_cast<void*>(pointer));
            }
        }

        /**
         * Typesafe template wrapper for standard library function
         * <code>std::memcpy</code>.
         */
        template< class T >
        static inline void Copy(T* destination, const T* source, std::size_t n)
        {
            if (n > 0)
            {
                std::memcpy(static_cast<void*>(destination), static_cast<const void*>(source), sizeof(T) * n);
            }
        }

        /**
         * Typesafe template wrapper for standard library function
         * <code>std::memmove</code>.
         */
        template< class T >
        static inline void Move(T* destination, const T* source, std::size_t n)
        {
            if (n > 0)
            {
                std::memmove(static_cast<void*>(destination), static_cast<const void*>(source), sizeof(T) * n);
            }
        }

    private:
        TEMPEARLY_DISALLOW_IMPLICIT_CONSTRUCTORS(Memory);
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
            : m_pointer(nullptr)
            , m_previous(nullptr)
            , m_next(nullptr) {}

        /**
         * Copy constructor.
         */
        Handle(const Handle<T>& that)
            : m_pointer(that.m_pointer)
            , m_previous(nullptr)
            , m_next(nullptr)
        {
            if (m_pointer)
            {
                m_pointer->RegisterHandle(this);
            }
        }

        /**
         * Copy constructor.
         */
        template< class U >
        Handle(const Handle<U>& that)
            : m_pointer(that.Get())
            , m_previous(nullptr)
            , m_next(nullptr)
        {
            if (m_pointer)
            {
                m_pointer->RegisterHandle(this);
            }
        }

        /**
         * Move constructor.
         */
        Handle(Handle<T>&& that)
            : m_pointer(that.m_pointer)
            , m_previous(that.m_previous)
            , m_next(that.m_next)
        {
            that.m_pointer = nullptr;
            that.m_previous = that.m_next = nullptr;
        }

        /**
         * Constructs handle from pointer to an object.
         */
        template< class U >
        Handle(U* pointer)
            : m_pointer(pointer)
            , m_previous(nullptr)
            , m_next(nullptr)
        {
            if (m_pointer)
            {
                m_pointer->RegisterHandle(this);
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
                m_pointer->UnregisterHandle(this);
            }
        }

        /**
         * Replaces object with given object pointer.
         */
        template< class U >
        Handle& operator=(U* pointer)
        {
            if (m_pointer != pointer)
            {
                if (m_pointer)
                {
                    m_pointer->UnregisterHandle(this);
                }
                if ((m_pointer = pointer))
                {
                    m_pointer->RegisterHandle(this);
                }
            }

            return *this;
        }

        /**
         * Replaces object with object from given handle.
         *
         * \param that New object to replace current with
         */
        Handle& operator=(const Handle<T>& that)
        {
            if (m_pointer != that.m_pointer)
            {
                if (m_pointer)
                {
                    m_pointer->UnregisterHandle(this);
                }
                if ((m_pointer = that.m_pointer))
                {
                    m_pointer->RegisterHandle(this);
                }
            }

            return *this;
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
                    m_pointer->UnregisterHandle(this);
                }
                if ((m_pointer = pointer))
                {
                    m_pointer->RegisterHandle(this);
                }
            }

            return *this;
        }

        /**
         * Moves data from another handle into this one.
         */
        Handle& operator=(Handle<T>&& that)
        {
            m_pointer = that.m_pointer;
            m_previous = that.m_previous;
            m_next = that.m_next;
            that.m_pointer = nullptr;
            that.m_previous = that.m_next = nullptr;

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
        /** Pointer to previous handle which references same object. */
        Handle<CountedObject>* m_previous;
        /** Pointer to next handle which references same object. */
        Handle<CountedObject>* m_next;
        friend class CountedObject;
    };

    class CountedObject
    {
    public:
        enum Flag
        {
            FLAG_MARKED = 2,
            FLAG_FINALIZING = 4,
            FLAG_INSPECTING = 8
        };

        explicit CountedObject();

        virtual ~CountedObject();

        /**
         * Returns true if object has been marked with specified flag.
         */
        inline bool HasFlag(Flag flag) const
        {
            return (m_flags & flag) != 0;
        }

        /**
         * Sets the specified flag on this object.
         */
        inline void SetFlag(Flag flag)
        {
            m_flags |= flag;
        }

        /**
         * Unsets the specified flag on this object.
         */
        inline void UnsetFlag(Flag flag)
        {
            m_flags &= ~flag;
        }

        /**
         * Returns true if this object has been marked by the garbage collector.
         */
        inline bool IsMarked() const
        {
            return (m_flags & FLAG_MARKED) != 0;
        }

        /**
         * This method is used by the garbage collector to mark all the objects
         * which are still accessible and not going to be trashed. Dervived
         * classes should override this method and proceed with marking all zone
         * allocated objects which they use (and thus should not be trashed).
         */
        virtual void Mark();

        /**
         * Returns the number of references this object has, including handles
         * and references from values.
         */
        inline unsigned int GetReferenceCount()
        {
            return m_reference_count;
        }

        /**
         * Increments internal reference counter of the object.
         */
        inline void IncReferenceCount()
        {
            ++m_reference_count;
        }

        /**
         * Decrements internal reference counter of the object.
         */
        inline void DecReferenceCount()
        {
            --m_reference_count;
        }

        template< class T >
        void RegisterHandle(Handle<T>* handle)
        {
            if (!handle)
            {
                return;
            }
            ++m_reference_count;
            if ((handle->m_previous = m_handle_tail))
            {
                m_handle_tail->m_next = reinterpret_cast<Handle<CountedObject>*>(handle);
            } else {
                m_handle_head = reinterpret_cast<Handle<CountedObject>*>(handle);
            }
            m_handle_tail = reinterpret_cast<Handle<CountedObject>*>(handle);
        }

        template< class T >
        void UnregisterHandle(Handle<T>* handle)
        {
            if (!handle)
            {
                return;
            }
            --m_reference_count;
            if (handle->m_previous && handle->m_next)
            {
                handle->m_previous->m_next = handle->m_next;
                handle->m_next->m_previous = handle->m_previous;
            }
            else if (handle->m_next)
            {
                m_handle_head = handle->m_next;
                m_handle_head->m_previous = nullptr;
            }
            else if (handle->m_previous)
            {
                m_handle_tail = handle->m_previous;
                m_handle_tail->m_next = nullptr;
            } else {
                m_handle_head = m_handle_tail = nullptr;
            }
            handle->m_previous = handle->m_next = nullptr;
        }

        static void* operator new(std::size_t);
        static void operator delete(void*);

    private:
        /** Contains various flags for this object. */
        unsigned int m_flags;
        /** Reference counter. */
        unsigned int m_reference_count;
        /** Pointer to first handle which references this object. */
        Handle<CountedObject>* m_handle_head;
        /** Pointer to last handle which references this object. */
        Handle<CountedObject>* m_handle_tail;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CountedObject);
    };
}

#endif /* !TEMPEARLY_MEMORY_H_GUARD */
