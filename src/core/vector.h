#ifndef TEMPEARLY_CORE_VECTOR_H_GUARD
#define TEMPEARLY_CORE_VECTOR_H_GUARD

#include "memory.h"

namespace tempearly
{
    /**
     * Container type which provides minimal dynamic array implementation.
     */
    template< class T >
    class Vector
    {
    public:
        /**
         * Constructs empty vector.
         */
        Vector()
            : m_capacity(0)
            , m_size(0)
            , m_data(nullptr) {}

        /**
         * Copy constructor. Contents of the existing vector are copied in the
         * new vector using the copy constructor of the value type.
         *
         * \param that Other vector to construct copy of
         */
        Vector(const Vector<T>& that)
            : m_capacity(that.m_size)
            , m_size(m_capacity)
            , m_data(Memory::Allocate<T>(m_size))
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                new (static_cast<void*>(m_data + i)) T(that.m_data[i]);
            }
        }

        /**
         * Copy constructor. Contents of the existing vector are copied in the
         * new vector using the copy constructor of the value type.
         *
         * \param that Other vector to construct copy of
         */
        template< class U >
        Vector(const Vector<U>& that)
            : m_capacity(that.GetSize())
            , m_size(m_capacity)
            , m_data(Memory::Allocate<T>(m_size))
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                new (static_cast<void*>(m_data + i)) T(that[i]);
            }
        }

        /**
         * Move constructor. Data is being moved from another vector into this
         * one.
         */
        Vector(Vector<T>&& that)
            : m_capacity(that.m_capacity)
            , m_size(that.m_size)
            , m_data(that.m_data)
        {
            that.m_capacity = that.m_size = 0;
            that.m_data = nullptr;
        }

        /**
         * Constructs an vector which contains <i>n</i> copies of the given
         * element.
         */
        template< class U >
        Vector(std::size_t n, const U& value)
            : m_capacity(n)
            , m_size(m_capacity)
            , m_data(Memory::Allocate<T>(m_size))
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                new (static_cast<void*>(m_data + i)) T(value);
            }
        }

        /**
         * Destructor.
         */
        ~Vector()
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                m_data[i].~T();
            }
            Memory::Unallocate<T>(m_data);
        }

        /**
         * Replaces contents of the vector with contents from another vector.
         *
         * \param that Other vector to copy contents from
         */
        Vector& Assign(const Vector<T>& that)
        {
            if (m_data != that.m_data)
            {
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    m_data[i].~T();
                }
                if (m_capacity < (m_size = that.m_size))
                {
                    Memory::Unallocate<T>(m_data);
                    m_data = Memory::Allocate<T>(m_capacity = m_size);
                }
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    new (static_cast<void*>(m_data + i)) T(that.m_data[i]);
                }
            }

            return *this;
        }

        /**
         * Replaces contents of the vector with contents from another vector.
         *
         * \param that Other vector to copy contents from
         */
        template< class U >
        Vector& Assign(const Vector<U>& that)
        {
            if (m_data != that.GetData())
            {
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    m_data[i].~T();
                }
                if (m_capacity < (m_size = that.GetSize()))
                {
                    Memory::Unallocate<T>(m_data);
                    m_data = Memory::Allocate<T>(m_capacity = m_size);
                }
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    new (static_cast<void*>(m_data + i)) T(that[i]);
                }
            }

            return *this;
        }

        /**
         * Assignment operator.
         */
        Vector& operator=(const Vector<T>& that)
        {
            return Assign(that);
        }

        /**
         * Assignment operator.
         */
        template< class U >
        Vector& operator=(const Vector<U>& that)
        {
            return Assign(that);
        }

        /**
         * Move operator.
         */
        Vector& operator=(Vector<T>&& that)
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                m_data[i].~T();
            }
            Memory::Unallocate<T>(m_data);
            m_capacity = that.m_capacity;
            m_size = that.m_size;
            m_data = that.m_data;
            that.m_capacity = that.m_size = 0;
            that.m_data = nullptr;

            return *this;
        }

        /**
         * Returns true if vector is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_size;
        }

        /**
         * Returns the number of values stored in the vector.
         */
        inline std::size_t GetSize() const
        {
            return m_size;
        }

        /**
         * Returns reference to the first value stored in the vector. No
         * boundary testing is performed.
         */
        inline T& GetFront()
        {
            return m_data[0];
        }

        /**
         * Returns reference to the first value stored in the vector. No
         * boundary testing is performed.
         */
        inline const T& GetFront() const
        {
            return m_data[0];
        }

        /**
         * Returns reference to the last value stored in the vector. No
         * boundary testing is performed.
         */
        inline T& GetBack()
        {
            return m_data[m_size - 1];
        }

        /**
         * Returns reference to the last value stored in the vector. No
         * boundary testing is performed.
         */
        inline const T& GetBack() const
        {
            return m_data[m_size - 1];
        }

        /**
         * Returns reference to a value stored in the vector at specified
         * index. No boundary testing is performed.
         */
        inline T& At(std::size_t i)
        {
            return m_data[i];
        }

        /**
         * Returns reference to a value stored in the vector at specified
         * index. No boundary testing is performed.
         */
        inline const T& At(std::size_t i) const
        {
            return m_data[i];
        }

        /**
         * Returns reference to a value stored in the vector at specified
         * index. No boundary testing is performed.
         */
        inline T& operator[](std::size_t i)
        {
            return At(i);
        }

        /**
         * Returns reference to a value stored in the vector at specified
         * index. No boundary testing is performed.
         */
        inline const T& operator[](std::size_t i) const
        {
            return At(i);
        }

        /**
         * Returns pointer to the array data. This could be NULL if the vector
         * is empty.
         */
        inline T* GetData()
        {
            return m_data;
        }

        /**
         * Returns pointer to the array data. This could be NULL if the vector
         * is empty.
         */
        inline const T* GetData() const
        {
            return m_data;
        }

        /**
         * Removes all contents from the vector.
         */
        void Clear()
        {
            for (std::size_t i = 0; i < m_size; ++i)
            {
                m_data[i].~T();
            }
            m_size = 0;
        }

        /**
         * Ensures that the vector has enough capacity to hold at least
         * <i>n</i> values.
         */
        void Reserve(std::size_t n)
        {
            T* old;

            if (!n || m_capacity >= n)
            {
                return;
            }
            old = m_data;
            m_data = Memory::Allocate<T>(m_capacity = n);
            for (std::size_t i = 0; i < m_size; ++i)
            {
                new (static_cast<void*>(m_data + i)) T(old[i]);
                old[i].~T();
            }
            Memory::Unallocate<T>(old);
        }

        /**
         * Inserts given value to the front of the vector.
         */
        void PushFront(const T& value)
        {
            if (m_capacity >= m_size + 1)
            {
                Memory::Move<T>(m_data + 1, m_data, m_size);
            } else {
                T* old = m_data;

                m_data = Memory::Allocate<T>(m_capacity += 16);
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    new (static_cast<void*>(m_data + i + 1)) T(old[i]);
                    old[i].~T();
                }
                Memory::Unallocate<T>(old);
            }
            ++m_size;
            new (static_cast<void*>(m_data)) T(value);
        }

        /**
         * Inserts given value to the end of the vector.
         */
        void PushBack(const T& value)
        {
            if (m_capacity < m_size + 1)
            {
                T* old = m_data;

                m_data = Memory::Allocate<T>(m_capacity += 16);
                for (std::size_t i = 0; i < m_size; ++i)
                {
                    new (static_cast<void*>(m_data + i)) T(old[i]);
                    old[i].~T();
                }
                Memory::Unallocate<T>(old);
            }
            new (static_cast<void*>(m_data + m_size++)) T(value);
        }

        /**
         * Inserts values from given array to the end of the vector.
         *
         * \param t Pointer to the array
         * \param n Size of the array
         */
        void PushBack(const T* t, std::size_t n)
        {
            if (n == 0)
            {
                return;
            }
            Reserve(m_size + n);
            for (std::size_t i = 0; i < n; ++i)
            {
                new (static_cast<void*>(m_data + m_size + i)) T(t[i]);
            }
            m_size += n;
        }

        /**
         * Removes value from specified index.
         */
        void Erase(std::size_t index)
        {
            m_data[index].~T();
            Memory::Move<T>(m_data + index, m_data + index + 1, --m_size - index);
        }

        /**
         * Swaps positions of two elements in the vector.
         */
        void Swap(std::size_t index1, std::size_t index2)
        {
            T tmp(m_data[index1]);

            m_data[index1] = m_data[index2];
            m_data[index2] = tmp;
        }

        /**
         * Returns portion of the vector.
         */
        Vector SubVector(std::size_t pos = 0) const
        {
            Vector result;

            result.Reserve(m_size - pos);
            for (std::size_t i = pos; i < m_size; ++i)
            {
                new (static_cast<void*>(result.m_data + i - pos)) T(m_data[i]);
            }
            result.m_size = m_size - pos;

            return result;
        }

        /**
         * Concatenates contents of two vectors and returns result.
         */
        template< class U >
        Vector Concat(const Vector<U>& that) const
        {
            Vector result;

            result.Reserve(m_size + that.GetSize());
            for (std::size_t i = 0; i < m_size; ++i)
            {
                new (static_cast<void*>(result.m_data + i)) T(m_data[i]);
            }
            for (std::size_t i = 0; i < that.GetSize(); ++i)
            {
                new (static_cast<void*>(result.m_data + i + m_size)) T(that[i]);
            }
            result.m_size = m_size + that.GetSize();

            return result;
        }

        /**
         * Concatenation operator.
         */
        template< class U >
        inline Vector operator+(const Vector<U>& that) const
        {
            return Concat(that);
        }

    private:
        /** Capacity of the array. */
        std::size_t m_capacity;
        /** Number of elements stored in the array. */
        std::size_t m_size;
        /** Pointer to the array data. */
        T* m_data;
    };
}

#endif /* !TEMPEARLY_CORE_VECTOR_H_GUARD */
