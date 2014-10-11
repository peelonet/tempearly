#ifndef TEMPEARLY_API_SET_H_GUARD
#define TEMPEARLY_API_SET_H_GUARD

#include "value.h"
#include "api/object.h"

namespace tempearly
{
    /**
     * Set is an unordered container where each value can exist only once.
     * Values are identified by their hash codes.
     */
    class SetObject : public Object
    {
    public:
        /**
         * Represents single entry in the set.
         */
        class Entry : public CountedObject
        {
        public:
            /**
             * Constructs new entry.
             *
             * \param hash  Hash code of the entry
             * \param value Value of the entry
             */
            explicit Entry(i64 hash, const Value& value);

            /**
             * Returns hash code of the entry.
             */
            inline i64 GetHash() const
            {
                return m_hash;
            }

            /**
             * Returns value of the entry.
             */
            inline const Value& GetValue() const
            {
                return m_value;
            }

            /**
             * Sets value of the entry.
             */
            inline void SetValue(const Value& value)
            {
                m_value = value;
            }

            /**
             * Returns pointer to next entry.
             */
            inline Handle<Entry> GetNext() const
            {
                return m_next;
            }

            /**
             * Returns pointer to previous entry.
             */
            inline Handle<Entry> GetPrev() const
            {
                return m_prev;
            }

            /**
             * Used by garbage collector to mark the object as used.
             */
            void Mark();

        private:
            /** Cached hash code of the value. */
            i64 m_hash;
            /** Value of the entry. */
            Value m_value;
            /** Pointer to next entry in the set. */
            Entry* m_next;
            /** Pointer to previous entry in the set. */
            Entry* m_prev;
            /** Pointer to child entry in hash table. */
            Entry* m_child;
            friend class SetObject;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Entry);
        };

        /**
         * Constructs empty set.
         *
         * \param cls         Class of the object
         * \param bucket_size Capacity of the hash table which provides fast
         *                    access of elements
         */
        explicit SetObject(const Handle<Class>& cls, std::size_t bucket_size = 32);

        /**
         * Destructor.
         */
        ~SetObject();

        /**
         * Returns true if the set is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_front;
        }

        /**
         * Returns number of elements stored in the set.
         */
        inline std::size_t GetSize() const
        {
            return m_size;
        }

        /**
         * Returns pointer to the first entry in the set. If the set is empty,
         * NULL is returned instead.
         */
        inline Handle<Entry> GetFront() const
        {
            return m_front;
        }

        /**
         * Returns pointer to the last entry in the set. If the set is empty,
         * NULL is returned instead.
         */
        inline Handle<Entry> GetBack() const
        {
            return m_back;
        }

        /**
         * Searches for an entry based on the hash code.
         *
         * \param hash Hash code to look for
         * \return     Entry with the given hash code or NULL if such entry
         *             does not appear in the set
         */
        Handle<Entry> Find(i64 hash) const;

        /**
         * Inserts an entry to the set. Existing entries with same hash code
         * are overridden.
         *
         * \param hash  Hash code of the entry to insert
         * \param value Value of the entry to insert
         */
        void Add(i64 hash, const Value& value);

        /**
         * Removes all entries from the set.
         */
        void Clear();

        bool IsSet() const
        {
            return true;
        }

        /**
         * Used by garbage collector to mark all the objects in the set.
         */
        void Mark();

    private:
        /** Capacity of the hash table. */
        const std::size_t m_bucket_size;
        /** Hash table for fast lookup. */
        Entry** m_bucket;
        /** Number of entries stored in the set. */
        std::size_t m_size;
        /** Pointer to first entry. */
        Entry* m_front;
        /** Pointer to last entry. */
        Entry* m_back;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(SetObject);
    };
}

#endif /* !TEMPEARLY_API_SET_H_GUARD */
