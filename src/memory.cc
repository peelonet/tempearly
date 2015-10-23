#include "memory.h"

#if !defined(TEMPEARLY_GC_THRESHOLD0)
# define TEMPEARLY_GC_THRESHOLD0 1024
#endif
#if !defined(TEMPEARLY_GC_THRESHOLD1)
# define TEMPEARLY_GC_THRESHOLD1 16
#endif
#if !defined(TEMPEARLY_GC_THRESHOLD2)
# define TEMPEARLY_GC_THRESHOLD2 16
#endif

namespace tempearly
{
    namespace
    {
        class Block;

        struct Record
        {
            /** Size of the record. */
            std::size_t size;
            /** Pointer to the object. */
            CountedObject* pointer;
            /** Block where the record belongs to. */
            Block* block;
            /** Pointer to next record in the list. */
            Record* next;
            /** Pointer to previous record in the list. */
            Record* prev;
            /** Pointer to next record in generation. */
            Record* next_in_generation;
        };

        struct Generation
        {
            /** Examination threshold of this generation. */
            const int threshold;
            /** Number of examinations performed in this generation. */
            int counter;
            /** Pointer to the first slot in the generation. */
            Record* head;
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
            Record* Allocate(std::size_t size);

            /**
             * Used by the sweep operation to indicate that the given record
             * has been sweeped and needs to be inserted into the linked list
             * of free records.
             */
            void MarkAsFree(Record* record);

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
            , m_free_tail(nullptr)
        {
            if (!m_data)
            {
                throw std::bad_alloc();
            }
        }

        Block::~Block()
        {
            for (Record* record = m_used_head; record; record = record->next)
            {
                delete record->pointer;
            }
            if (m_data)
            {
                std::free(static_cast<void*>(m_data));
            }
        }

        Record* Block::Allocate(std::size_t size)
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

                return record;
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
            record->block = this;
            record->prev = nullptr;
            if ((record->next = m_used_head))
            {
                record->next->prev = record;
            } else {
                m_used_tail = record;
            }
            m_used_head = record;

            return record;
        }

        void Block::MarkAsFree(Record* record)
        {
            if (record->next && record->prev)
            {
                record->next->prev = record->prev;
                record->prev->next = record->next;
            }
            else if (record->next)
            {
                m_used_head = record->next;
                record->next->prev = nullptr;
            }
            else if (record->prev)
            {
                m_used_tail = record->prev;
                record->prev->next = nullptr;
                record->prev = nullptr;
            } else {
                m_used_head = m_used_tail = nullptr;
            }
            if ((record->next = m_free_head))
            {
                record->next->prev = record;
            } else {
                m_free_tail = record;
            }
            m_free_head = record;
            record->next_in_generation = nullptr;
        }
    }

    /** Pointer to latest allocated block of memory. */
    static Block* gc_block_head = nullptr;

    static Generation gc_generation[3] =
    {
        { TEMPEARLY_GC_THRESHOLD0, 0, nullptr },
        { TEMPEARLY_GC_THRESHOLD0, 0, nullptr },
        { TEMPEARLY_GC_THRESHOLD0, 0, nullptr }
    };

    static void gc_mark(Generation& generation)
    {
        for (Record* record = generation.head;
             record;
             record = record->next_in_generation)
        {
            CountedObject* object = record->pointer;

            if (!object->IsMarked() && object->GetReferenceCount())
            {
                object->Mark();
            }
        }
    }

    static void gc_sweep(Generation& young, Generation& old)
    {
#if defined(TEMPEARLY_GC_DEBUG)
        int saved_count = 0;
        int destroyed_count = 0;
#endif
        Record* current = young.head;
        Record* next;
        Record* saved_head = nullptr;
        Record* saved_tail = nullptr;

        young.head = nullptr;
        for (; current; current = next)
        {
            CountedObject* object = current->pointer;

            next = current->next_in_generation;
            if (object->IsMarked())
            {
#if defined(TEMPEARLY_GC_DEBUG)
                ++saved_count;
#endif
                object->UnsetFlag(CountedObject::FLAG_MARKED);
                if (!(current->next_in_generation = saved_head))
                {
                    saved_tail = current;
                }
                saved_head = current;
            } else {
#if defined(TEMPEARLY_GC_DEBUG)
                ++destroyed_count;
#endif
                delete object;
                current->block->MarkAsFree(current);
            }
        }
        if (saved_tail)
        {
            saved_tail->next_in_generation = old.head;
            old.head = saved_head;
        }
#if defined(TEMPEARLY_GC_DEBUG)
        std::fprintf(
            stderr,
            "GC: %d objects trashed, %d remain\n",
            destroyed_count,
            saved_count
        );
#endif
    }

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
        const std::size_t remainder = size % 8;
        Record* record = nullptr;

        if (remainder)
        {
            size += 8 - remainder;
        }
        for (int i = 0; i < 3; ++i)
        {
            if (++gc_generation[i].counter < gc_generation[i].threshold)
            {
                break;
            }
#if defined(TEMPEARLY_GC_DEBUG)
            std::fprintf(
                stderr,
                "GC: Generation %d reached threshold.\n",
                i
            );
#endif
            gc_generation[i].counter = 0;
            gc_mark(gc_generation[i]);
            if (i + 1 < 3)
            {
                gc_sweep(gc_generation[i], gc_generation[i + 1]);
            } else {
                gc_sweep(gc_generation[i], gc_generation[i]);
            }
        }
        for (Block* block = gc_block_head; block; block = block->GetNext())
        {
            if ((record = block->Allocate(size)))
            {
                break;
            }
        }
        if (!record)
        {
            gc_block_head = new Block(gc_block_head);
            if (!(record = gc_block_head->Allocate(size)))
            {
                throw std::bad_alloc();
            }
        }
        record->next_in_generation = gc_generation[0].head;
        gc_generation[0].head = record;

        return static_cast<void*>(record->pointer);
    }

    void CountedObject::operator delete(void*) {}
}
