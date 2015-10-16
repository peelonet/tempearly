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
             * Sets new value for the entry.
             */
            inline void SetValue(const T& value)
            {
                m_value = value;
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
                , m_next(nullptr)
                , m_previous(nullptr)
                , m_child(nullptr) {}

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
            : m_bucket(Memory::Allocate<Entry*>(kBucketSize))
            , m_front(nullptr)
            , m_back(nullptr)
        {
            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = nullptr;
            }
        }

        /**
         * Copy constructor. Constructs dictionary from values taken from
         * another dictionary.
         */
        Dictionary(const Dictionary<T>& that)
            : m_bucket(Memory::Allocate<Entry*>(kBucketSize))
            , m_front(nullptr)
            , m_back(nullptr)
        {
            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = nullptr;
            }
            for (const Entry* entry1 = that.m_front; entry1; entry1 = entry1->m_next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = nullptr;
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
         * Copy constructor. Constructs dictionary from values taken from
         * another dictionary.
         */
        template< class U >
        Dictionary(const Dictionary<U>& that)
            : m_bucket(Memory::Allocate<Entry*>(kBucketSize))
            , m_front(nullptr)
            , m_back(nullptr)
        {
            for (std::size_t i = 0; i < kBucketSize; ++i)
            {
                m_bucket[i] = nullptr;
            }
            for (const typename Dictionary<U>::Entry* entry1 = that.GetFront();
                 entry1;
                 entry1 = entry1->m_next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = nullptr;
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
         * Move constructor. Data from another dictionary is moved into this
         * one and the original becomes unusable after that.
         */
        Dictionary(Dictionary<T>&& that)
            : m_bucket(that.m_bucket)
            , m_front(that.m_front)
            , m_back(that.m_back)
        {
            that.m_bucket = nullptr;
            that.m_front = that.m_back = nullptr;
        }

        /**
         * Destructor.
         */
        virtual ~Dictionary()
        {
            Entry* current = m_front;
            Entry* next;

            while (current)
            {
                next = current->m_next;
                delete current;
                current = next;
            }
            Memory::Unallocate<Entry*>(m_bucket);
        }

        /**
         * Copies contents from another dictionary into this one.
         */
        Dictionary& Assign(const Dictionary<T>& that)
        {
            Clear();
            for (const Entry* entry1 = that.m_front; entry1; entry1 = entry1->m_next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = nullptr;
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
         * Copies contents from another dictionary into this one.
         */
        template< class U >
        Dictionary& Assign(const Dictionary<U>& that)
        {
            Clear();
            for (const typename Dictionary<U>::Entry* entry1 = that.GetFront();
                 entry1;
                 entry1 = entry1->m_next)
            {
                Entry* entry2 = new Entry(entry1->m_hash_code, entry1->m_name, entry1->m_value);
                const std::size_t index = entry1->m_hash_code % kBucketSize;

                entry2->m_next = nullptr;
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
        inline Dictionary& operator=(const Dictionary<T>& that)
        {
            return Assign(that);
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
         * Move operator.
         */
        Dictionary& operator=(Dictionary<T>&& that)
        {
            Entry* current = m_front;
            Entry* next;

            while (current)
            {
                next = current->m_next;
                delete current;
                current = next;
            }
            Memory::Unallocate<Entry*>(m_bucket);
            m_bucket = that.m_bucket;
            m_front = that.m_front;
            m_back = that.m_back;
            that.m_bucket = nullptr;
            that.m_front = that.m_back = nullptr;

            return *this;
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

            return nullptr;
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

            return nullptr;
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
            entry->m_next = nullptr;
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
                m_bucket[i] = nullptr;
            }
            while (current)
            {
                next = current->m_next;
                delete current;
                current = next;
            }
            m_front = m_back = nullptr;
        }

        /**
         * Removes an entry from the dictionary.
         *
         * \param id Identifier of the entry to remove
         */
        void Erase(const String& id)
        {
            const std::size_t hash = id.HashCode();
            const std::size_t index = hash % kBucketSize;
            Entry* entry = m_bucket[index];

            if (!entry)
            {
                return;
            }
            if (entry->m_hash_code == hash)
            {
                m_bucket[index] = entry->m_child;
                if (entry->m_next && entry->m_previous)
                {
                    entry->m_next->m_previous = entry->m_previous;
                    entry->m_previous->m_next = entry->m_next;
                }
                else if (entry->m_next)
                {
                    entry->m_next->m_previous = nullptr;
                    m_front = entry->m_next;
                }
                else if (entry->m_previous)
                {
                    entry->m_previous->m_next = nullptr;
                    m_back = entry->m_previous;
                } else {
                    m_front = m_back = nullptr;
                }
                delete entry;
                return;
            }
            for (; entry->m_child; entry = entry->m_child)
            {
                if (entry->m_child->m_hash_code == hash)
                {
                    Entry* child = entry->m_child;

                    entry->m_child = entry->m_child->m_child;
                    if (child->m_next && child->m_previous)
                    {
                        child->m_next->m_previous = child->m_previous;
                        child->m_previous->m_next = child->m_next;
                    }
                    else if (child->m_next)
                    {
                        child->m_next->m_previous = nullptr;
                        m_front = child->m_next;
                    }
                    else if (child->m_previous)
                    {
                        child->m_previous->m_next = nullptr;
                        m_back = child->m_previous;
                    } else {
                        m_front = m_back = nullptr;
                    }
                    delete child;
                    return;
                }
            }
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
