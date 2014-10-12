#ifndef TEMPEARLY_API_MAP_H_GUARD
#define TEMPEARLY_API_MAP_H_GUARD

#include "value.h"

namespace tempearly
{
    /**
     * Implementation of hash map object which stores pairs of values.
     */
    class MapObject : public Object
    {
    public:
        /**
         * Represents single key-value entry in the map.
         */
        class Entry : public CountedObject
        {
        public:
            /**
             * Constructs new entry.
             *
             * \param hash  Hash code of the entry
             * \param key   Key value of the entry
             * \param value Value of the entry
             */
            explicit Entry(i64 hash, const Value& key, const Value& value);

            /**
             * Returns hash code of the entry.
             */
            inline i64 GetHash() const
            {
                return m_hash;
            }

            /**
             * Returns key of the entry.
             */
            inline const Value& GetKey() const
            {
                return m_key;
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
            /** Cached hash code of the entry. */
            const i64 m_hash;
            /** Key of the entry. */
            Value m_key;
            /** Value of the entry. */
            Value m_value;
            /** Pointer to next entry. */
            Entry* m_next;
            /** Pointer to previous entry. */
            Entry* m_prev;
            /** Pointer to next entry in hash table. */
            Entry* m_child;
            friend class MapObject;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Entry);
        };

        /**
         * Constructs new map object.
         *
         * \param cls         Class of the object
         * \param bucket_size Size of the hash table.
         */
        explicit MapObject(const Handle<Class>& cls, std::size_t bucket_size = 32);

        /**
         * Destructor.
         */
        ~MapObject();

        /**
         * Returns true if the map is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_front;
        }

        /**
         * Returns the number of key-value entries stored in the map.
         */
        inline std::size_t GetSize() const
        {
            return m_size;
        }

        /**
         * Returns pointer to the first entry in the map, or NULL if the map is
         * empty.
         */
        inline Handle<Entry> GetFront() const
        {
            return m_front;
        }

        /**
         * Returns pointer to the last entry in the map, or NULL if the map is
         * empty.
         */
        inline Handle<Entry> GetBack() const
        {
            return m_back;
        }

        /**
         * Searches for an entry from the map.
         *
         * \param hash Hash code to look for
         * \return     Value for given hash code, or error value if no such
         *             entry exists in the map
         */
        Value Find(i64 hash) const;

        /**
         * Inserts an entry into the map. Existing entries with same hash code
         * are overridden.
         *
         * \param hash  Hash code of the entry
         * \param key   Key value of the entry
         * \param value Value of the entry
         */
        void Insert(i64 hash, const Value& key, const Value& value);

        /**
         * Removes all entries from the map.
         */
        void Clear();

        bool IsMap() const
        {
            return this;
        }

        /**
         * Used by the garbage collector to mark all the objects which the map
         * contains.
         */
        void Mark();

    private:
        /** Size of the hash table. */
        const std::size_t m_bucket_size;
        /** The hash table. */
        Entry** m_bucket;
        /** First entry in the map. */
        Entry* m_front;
        /** Last entry in the map. */
        Entry* m_back;
        /** Number of key-value entries stored in the map. */
        std::size_t m_size;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(MapObject);
    };
}

#endif /* !TEMPEARLY_API_MAP_H_GUARD */
