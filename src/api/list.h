#ifndef TEMPEARLY_API_LIST_H_GUARD
#define TEMPEARLY_API_LIST_H_GUARD

#include "value.h"
#include "api/object.h"

namespace tempearly
{
    /**
     * Implementation of linked list.
     */
    class ListObject : public Object
    {
    public:
        /**
         * Represents single entry in the list.
         */
        class Link : public CountedObject
        {
        public:
            /**
             * Constructs new link.
             *
             * \param value Value of the link
             */
            explicit Link(const Value& value);

            /**
             * Returns value of the link.
             */
            inline const Value& GetValue() const
            {
                return m_value;
            }

            /**
             * Sets value of the link.
             */
            inline void SetValue(const Value& value)
            {
                m_value = value;
            }

            /**
             * Returns pointer to next link in the list.
             */
            inline Handle<Link> GetNext() const
            {
                return m_next;
            }

            /**
             * Used by garbage collector to mark the object as used.
             */
            void Mark();

        private:
            /** Value of the entry. */
            Value m_value;
            /** Pointer to next entry in the list. */
            Link* m_next;
            friend class ListObject;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Link);
        };

        /**
         * Constructs empty list.
         *
         * \param cls Class of the list object
         */
        explicit ListObject(const Handle<Class>& cls);

        /**
         * Returns true if the list is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_front;
        }

        /**
         * Returns the number of elements stored in the list.
         */
        inline std::size_t GetSize() const
        {
            return m_size;
        }

        /**
         * Returns pointer to the first link in the list, or NULL if the list
         * is empty.
         */
        inline Handle<Link> GetFront() const
        {
            return m_front;
        }

        /**
         * Returns pointer to the last link in the list, or NULL if the list is
         * empty.
         */
        inline Handle<Link> GetBack() const
        {
            return m_back;
        }

        /**
         * Inserts given value into the list.
         */
        void Append(const Value& value);

        /**
         * Inserts all elements from given vector to the end of the list.
         */
        void Append(const Vector<Value>& vector);

        /**
         * Inserts all elements from another list to the end of this one.
         */
        void Append(const Handle<ListObject>& that);

        /**
         * Inserts given value in begining of the list.
         */
        void Prepend(const Value& value);

        /**
         * Inserts all elements from given vector into beginning of the list.
         */
        void Prepend(const Vector<Value>& vector);

        /**
         * Removes all values from the list.
         */
        void Clear();

        bool IsList() const
        {
            return true;
        }

        /**
         * Used by garbage collector to mark elements stored in the list.
         */
        void Mark();

    private:
        /** Number of entries stored in the list. */
        std::size_t m_size;
        /** Pointer to the first link in the list. */
        Link* m_front;
        /** Pointer to the last link in the list. */
        Link* m_back;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ListObject);
    };
}

#endif /* !TEMPEARLY_API_LIST_H_GUARD */
