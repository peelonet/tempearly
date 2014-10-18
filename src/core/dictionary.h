#ifndef TEMPEARLY_CORE_DICTIONARY_H_GUARD
#define TEMPEARLY_CORE_DICTIONARY_H_GUARD

#include "core/string.h"

namespace tempearly
{
    /**
     * Hash map implementation which uses strings as keys.
     */
    template< class T >
    class Dictionary
    {
    public:
        static const std::size_t kBucketSize = 8;

        /**
         * Represents single key-value pair stored in the dictionary. Can also
         * be used for iteration.
         */
        class Entry
        {
        public:
            /**
             * Returns identifier/name of the entry.
             */
            inline const String& GetName() const
            {
                return m_name;
            }

            /**
             * Returns value of the entry.
             */
            inline T& GetValue()
            {
                return m_value;
            }

            /**
             * Returns value of the entry.
             */
            inline const T& GetValue() const
            {
                return m_value;
            }

            /**
             * Returns pointer to the next entry.
             */
            inline const Entry* GetNext() const
            {
                return m_next;
            }

            /**
             * Returns pointer to the previous entry.
             */
            inline const Entry* GetPrevious() const
            {
                return m_previous;
            }

        private:
            Entry(std::size_t hash_code, const String& name, const T& value)
                : m_hash_code(hash_code)
                , m_name(name)
                , m_value(value)
                , m_next(0)
                , m_previous(0)
                , m_child(0) {}

            /** Cached hash code of the entry. */
            std::size_t m_hash_code;
            /** Name of the entry. */
            String m_name;
            /** Value of the entry. */
            T m_value;
            /** Pointer to next entry in the dictionary. */
            Entry* m_next;
            /** Pointer to previous entry in the dictionary. */
            Entry* m_previous;
            /** Pointer next entry in hash table. */
            Entry* m_child;
            friend class Dictionary;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Entry);
        };

        /**
         * Constructs empty dictionary.
         */
        Dictionary()
            : m_bucket(new Entry*[kBucketSize])
            , m_front(0)
            , m_back(0)
        {
            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = 0;
            }
        }

        /**
         * Copy constructor. Constructs dictionary from values taken from
         * another dictionary.
         */
        template< class U >
        Dictionary(const Dictionary<U>& that)
            : m_bucket(new Entry*[kBucketSize])
            , m_front(0)
            , m_back(0)
        {
            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = 0;
            }
            for (const typename Dictionary<U>::Entry* entry1 = that.GetFront();
                 entry1;
                 entry1 = entry1->next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = 0;
                if ((entry2->m_previous = m_back))
                {
                    m_back->m_next = entry2;
                } else {
                    m_front = entry2;
                }
                m_back = entry2;
                entry2->m_child = m_bucket[index];
                m_bucket[index] = entry2;
            }
        }

        /**
         * Destructor.
         */
        virtual ~Dictionary()
        {
            Clear();
            delete[] m_bucket;
        }

        /**
         * Copies contents from another dictionary into this one.
         */
        template< class U >
        Dictionary& Assign(const Dictionary<U>& that)
        {
            Clear();
            for (const typename Dictionary<U>::Entry* entry1 = that.GetFront();
                 entry1;
                 entry1 = entry1->next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = 0;
                if ((entry2->m_previous = m_back))
                {
                    m_back->m_next = entry2;
                } else {
                    m_front = entry2;
                }
                m_back = entry2;
                entry2->m_child = m_bucket[index];
                m_bucket[index] = entry2;
            }

            return *this;
        }

        /**
         * Assignment operator.
         */
        template< class U >
        inline Dictionary& operator=(const Dictionary<U>& that)
        {
            return Assign(that);
        }

        /**
         * Returns true if the dictionary is empty.
         */
        inline bool IsEmpty() const
        {
            return !m_front;
        }

        /**
         * Returns pointer to the first entry in the dictionary, or NULL if the
         * dictionary is empty.
         */
        inline Entry* GetFront()
        {
            return m_front;
        }

        /**
         * Returns pointer to the first entry in the dictionary, or NULL if the
         * dictionary is empty.
         */
        inline const Entry* GetFront() const
        {
            return m_front;
        }

        /**
         * Returns pointer to the last entry in the dictionary, or NULL if the
         * dictionary is empty.
         */
        inline Entry* GetBack()
        {
            return m_back;
        }

        /**
         * Returns pointer to the last entry in the dictionary, or NULL if the
         * dictionary is empty.
         */
        inline const Entry* GetBack() const
        {
            return m_back;
        }

        /**
         * Searches for an value with given identifier. Returns pointer to the
         * entry holding the value, if such exists. Otherwise NULL is returned.
         *
         * \param id Identifier to look for
         */
        Entry* Find(const String& id)
        {
            const std::size_t hash = id.HashCode();
            const std::size_t index = hash % kBucketSize;

            for (Entry* entry = m_bucket[index]; entry; entry = entry->m_child)
            {
                if (entry->m_hash_code == hash)
                {
                    return entry;
                }
            }

            return 0;
        }

        /**
         * Searches for an value with given identifier. Returns pointer to the
         * entry holding the value, if such exists. Otherwise NULL is returned.
         *
         * \param id Identifier to look for
         */
        const Entry* Find(const String& id) const
        {
            const std::size_t hash = id.HashCode();
            const std::size_t index = hash % kBucketSize;

            for (const Entry* entry = m_bucket[index]; entry; entry = entry->m_child)
            {
                if (entry->m_hash_code == hash)
                {
                    return entry;
                }
            }

            return 0;
        }

        /**
         * Inserts given value into the dictionary. Existing entries with same
         * identifier are overridden.
         *
         * \param id    Identifier of the entry
         * \param value Value to insert
         */
        void Insert(const String& id, const T& value)
        {
            const std::size_t hash = id.HashCode();
            const std::size_t index = hash % kBucketSize;
            Entry* entry;

            for (entry = m_bucket[index]; entry; entry = entry->m_child)
            {
                if (entry->m_hash_code == hash)
                {
                    entry->m_value = value;
                    return;
                }
            }
            entry = new Entry(hash, id, value);
            entry->m_next = 0;
            if ((entry->m_previous = m_back))
            {
                m_back->m_next = entry;
            } else {
                m_front = entry;
            }
            m_back = entry;
            entry->m_child = m_bucket[index];
            m_bucket[index] = entry;
        }

        /**
         * Removes all entries from the dictionary.
         */
        void Clear()
        {
            Entry* current = m_front;
            Entry* next;

            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = 0;
            }
            while (current)
            {
                next = current->m_next;
                delete current;
                current = next;
            }
            m_front = m_back = 0;
        }

    private:
        /** The actual hash table. */
        Entry** m_bucket;
        /** Pointer to the first entry in the dictionary. */
        Entry* m_front;
        /** Pointer to the last entry in the dictionary. */
        Entry* m_back;
    };
}

#endif /* !TEMPEARLY_CORE_DICTIONARY_H_GUARD */
