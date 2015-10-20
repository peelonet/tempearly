#include "memory.h"

#if !defined(TEMPEARLY_GC_THRESHOLD)
# define TEMPEARLY_GC_THRESHOLD 700
#endif

namespace tempearly
{
    namespace
    {
        struct Record
        {
            /** Size of the record. */
            std::size_t size;
            /** Pointer to the object. */
            CountedObject* pointer;
            /** Pointer to next record in the list. */
            Record* next;
            /** Pointer to previous record in the list. */
            Record* prev;
        };

        class Block
        {
        public:
            /** Size of single memory block. */
            static const std::size_t kBlockSize = 4096 * 32;

            explicit Block(Block* next);

            ~Block();

            /**
             * Returns pointer to next block in the sequence.
             */
            inline Block* GetNext()
            {
                return m_next;
            }

            /**
             * Attempts to allocate <i>size</i> amount of bytes memory from
             * this block. Returns null if this block does not have enough
             * free space available.
             */
            void* Allocate(std::size_t size);

            void Mark();

            void Collect();

        private:
            /** Pointer to next memory block in sequence. */
            Block* m_next;
            /** Pointer to the memory contained by the block. */
            byte* m_data;
            /** Amount of memory still available in this block. */
            std::size_t m_remaining;
            /** Pointer to first used record. */
            Record* m_used_head;
            /** Pointer to last used record. */
            Record* m_used_tail;
            /** Pointer to first free record. */
            Record* m_free_head;
            /** Pointer to last free record. */
            Record* m_free_tail;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Block);
        };

        Block::Block(Block* next)
            : m_next(next)
            , m_data(static_cast<byte*>(std::malloc(kBlockSize)))
            , m_remaining(kBlockSize)
            , m_used_head(nullptr)
            , m_used_tail(nullptr)
            , m_free_head(nullptr)
            , m_free_tail(nullptr) {}

        Block::~Block()
        {
            for (Record* record = m_used_head; record; record = record->next)
            {
                delete record->pointer;
            }
            std::free(static_cast<void*>(m_data));
        }

        void* Block::Allocate(std::size_t size)
        {
            Record* record;

            for (record = m_free_head; record; record = record->next)
            {
                if (record->size < size)
                {
                    continue;
                }
                else if (record->next && record->prev)
                {
                    record->next->prev = record->prev;
                    record->prev->next = record->next;
                }
                else if (record->next)
                {
                    record->next->prev = nullptr;
                    m_free_head = record->next;
                }
                else if (record->prev)
                {
                    record->prev->next = nullptr;
                    m_free_tail = record->prev;
                } else {
                    m_free_head = m_free_tail = nullptr;
                }
                record->prev = nullptr;
                if ((record->next = m_used_head))
                {
                    record->next->prev = record;
                } else {
                    m_used_tail = record;
                }
                m_used_head = record;

                return static_cast<void*>(record->pointer);
            }
            if (m_remaining < sizeof(Record) + size)
            {
                return nullptr;
            }
            record = reinterpret_cast<Record*>(m_data);
            record->size = size;
            record->pointer = reinterpret_cast<CountedObject*>(m_data + sizeof(Record));
            m_data += sizeof(Record) + size;
            m_remaining -= sizeof(Record) + size;
            record->prev = nullptr;
            if ((record->next = m_used_head))
            {
                record->next->prev = record;
            } else {
                m_used_tail = record;
            }
            m_used_head = record;

            return static_cast<void*>(record->pointer);
        }

        void Block::Mark()
        {
            for (Record* record = m_used_head; record; record = record->next)
            {
                CountedObject* object = record->pointer;

                if (!object->IsMarked() && object->GetReferenceCount())
                {
                    object->Mark();
                }
            }
            if (m_next)
            {
                m_next->Mark();
            }
        }

        void Block::Collect()
        {
#if defined(TEMPEARLY_GC_DEBUG)
            int destroyed_count = 0;
            int saved_count = 0;
#endif
            Record* record = m_used_head;
            Record* next;

            m_used_head = m_used_tail = nullptr;
            for (; record; record = next)
            {
                CountedObject* object = record->pointer;

                next = record->next;
                record->prev = nullptr;
                if (object->IsMarked())
                {
#if defined(TEMPEARLY_GC_DEBUG)
                    ++saved_count;
#endif
                    object->UnsetFlag(CountedObject::FLAG_MARKED);
                    if ((record->next = m_used_head))
                    {
                        record->next->prev = record;
                    } else {
                        m_used_tail = record;
                    }
                    m_used_head = record;
                } else {
#if defined(TEMPEARLY_GC_DEBUG)
                    ++destroyed_count;
#endif
                    delete object;
                    if ((record->next = m_free_head))
                    {
                        record->next->prev = record;
                    } else {
                        m_free_tail = record;
                    }
                    m_free_head = record;
                }
            }
#if defined(TEMPEARLY_GC_DEBUG)
            std::fprintf(
                stderr,
                "GC: %d objects trashed, %d remain\n",
                destroyed_count,
                saved_count
            );
#endif
            if (m_next)
            {
                m_next->Collect();
            }
        }
    }

    /** Number of allocations performed since last collect. */
    static std::size_t gc_counter = 0;
    /** Pointer to latest allocated block of memory. */
    static Block* gc_block_head = nullptr;

    CountedObject::CountedObject()
        : m_flags(0)
        , m_reference_count(0) {}

    CountedObject::~CountedObject() {}

    void CountedObject::Mark()
    {
        SetFlag(FLAG_MARKED);
    }

    void* CountedObject::operator new(std::size_t size)
    {
        void* pointer;

        if (++gc_counter >= TEMPEARLY_GC_THRESHOLD)
        {
#if defined(TEMPEARLY_GC_DEBUG)
            std::fprintf(stderr, "GC: Threshold has been reached.\n");
#endif
            gc_counter = 0;
            gc_block_head->Mark();
            gc_block_head->Collect();
        }
        for (Block* block = gc_block_head; block; block = block->GetNext())
        {
            if ((pointer = block->Allocate(size)))
            {
                return pointer;
            }
        }
        gc_block_head = new Block(gc_block_head);
        if (!(pointer = gc_block_head->Allocate(size)))
        {
            throw std::bad_alloc();
        }

        return pointer;
    }

    void CountedObject::operator delete(void*) {}
}
