#ifndef TEMPEARLY_DICTIONARY_H_GUARD
#define TEMPEARLY_DICTIONARY_H_GUARD

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
        struct Entry
        {
            /** Identifier of the entry. */
            String id;
            /** Value of the entry. */
            T value;
            /** Cached hash code. */
            std::size_t hash;
            /** Pointer to next entry in the dictionary. */
            Entry* next;
            /** Pointer to previous entry in the dictionary. */
            Entry* prev;
            /** Pointer to child entry in the dictionary. */
            Entry* child;
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
                Entry* entry2 = static_cast<Entry*>(std::malloc(sizeof(Entry)));
                const std::size_t index = entry1->hash % kBucketSize;

                new (static_cast<void*>(&entry2->id)) String(entry1->id);
                new (static_cast<void*>(&entry2->value)) T(entry1->value);
                entry2->hash = entry1->hash;
                entry2->next = 0;
                if ((entry2->prev = m_back))
                {
                    m_back->next = entry2;
                } else {
                    m_front = entry2;
                }
                m_back = entry2;
                entry2->child = m_bucket[index];
                m_bucket[index] = entry2;
            }
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
                next = current->next;
                current->id.~String();
                current->value.~T();
                std::free(static_cast<void*>(current));
                current = next;
            }
            delete[] m_bucket;
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
        inline const Entry* GetFront() const
        {
            return m_front;
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
        const Entry* Find(const String& id) const
        {
            const std::size_t hash = id.HashCode();
            const std::size_t index = hash % kBucketSize;

            for (const Entry* entry = m_bucket[index]; entry; entry = entry->child)
            {
                if (entry->hash == hash)
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

            for (entry = m_bucket[index]; entry; entry = entry->child)
            {
                if (entry->hash == hash)
                {
                    entry->value = value;
                    return;
                }
            }
            entry = static_cast<Entry*>(std::malloc(sizeof(Entry)));
            new (static_cast<void*>(&entry->id)) String(id);
            new (static_cast<void*>(&entry->value)) T(value);
            entry->hash = hash;
            entry->next = 0;
            if ((entry->prev = m_back))
            {
                m_back->next = entry;
            } else {
                m_front = entry;
            }
            m_back = entry;
            entry->child = m_bucket[index];
            m_bucket[index] = entry;
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

#endif /* !TEMPEARLY_DICTIONARY_H_GUARD */
