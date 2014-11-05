#include "memory.h"

#if !defined(TEMPEARLY_GC_THRESHOLD0)
# define TEMPEARLY_GC_THRESHOLD0 700
#endif
#if !defined(TEMPEARLY_GC_THRESHOLD1)
# define TEMPEARLY_GC_THRESHOLD1 10
#endif
#if !defined(TEMPEARLY_GC_THRESHOLD2)
# define TEMPEARLY_GC_THRESHOLD2 10
#endif

namespace tempearly
{
    static const std::size_t kAreaSize = 256000;

    namespace
    {
        class Area
        {
        public:
            struct Slot
            {
                /** Size of the object. */
                std::size_t size;
                /** Pointer to the object. */
                CountedObject* object;
                /** Area where this slot belongs to. */
                Area* area;
                /** Pointer to next slot in the list. */
                Slot* next_in_area;
                /** Pointer to previous slot in the list. */
                Slot* prev_in_area;
                /** Pointer to next slot in generation. */
                Slot* next_in_generation;
            };

            explicit Area()
                : m_table(static_cast<byte*>(std::malloc(kAreaSize)))
                , m_next(0)
                , m_free_offset(0)
                , m_used_head(0)
                , m_used_tail(0)
                , m_free_head(0)
                , m_free_tail(0)
            {
                if (!m_table)
                {
                    std::abort();
                }
                if (s_area_head)
                {
                    s_area_head->m_next = this;
                }
                s_area_head = this;
            }

            ~Area()
            {
                for (Slot* slot = m_used_head; slot; slot = slot->next_in_area)
                {
                    slot->object->~CountedObject();
                }
                if (m_table)
                {
                    std::free(static_cast<void*>(m_table));
                }
            }

            inline std::size_t GetFreeMemory() const
            {
                return kAreaSize - m_free_offset;
            }

            static Slot* GlobalAllocate(std::size_t size)
            {
                Area* area;

                for (area = s_area_head; area; area = area->m_next)
                {
                    Slot* slot = area->Allocate(size);

                    if (slot)
                    {
                        return slot;
                    }
                }
                area = new Area();

                return area->Allocate(size);
            }

            Slot* Allocate(std::size_t size)
            {
                Slot* slot;

                for (slot = m_free_head; slot; slot = slot->next_in_area)
                {
                    if (slot->size < size)
                    {
                        continue;
                    }
                    if (slot->prev_in_area && slot->next_in_area)
                    {
                        slot->prev_in_area->next_in_area = slot->next_in_area;
                        slot->next_in_area->prev_in_area = slot->prev_in_area;
                    }
                    else if (slot->next_in_area)
                    {
                        slot->next_in_area->prev_in_area = 0;
                        m_free_head = slot->next_in_area;
                    }
                    else if (slot->prev_in_area)
                    {
                        slot->prev_in_area->next_in_area = 0;
                        m_free_head = slot->prev_in_area;
                    } else {
                        m_free_head = m_free_tail = 0;
                    }
                    slot->next_in_area = 0;
                    if ((slot->prev_in_area = m_used_tail))
                    {
                        m_used_tail->next_in_area = slot;
                    } else {
                        m_used_head = slot;
                    }
                    m_used_tail = slot;
                    slot->next_in_generation = 0;

                    return slot;
                }
                if ((kAreaSize - m_free_offset) < (size + sizeof(Slot)))
                {
                    return 0; // It doesn't fit
                }
                slot = reinterpret_cast<Slot*>(m_table + m_free_offset);
                slot->size = size;
                slot->object = reinterpret_cast<CountedObject*>(m_table + m_free_offset + sizeof(Slot));
                slot->area = this;
                slot->next_in_area = 0;
                if ((slot->prev_in_area = m_used_tail))
                {
                    m_used_tail->next_in_area = slot;
                } else {
                    m_used_head = slot;
                }
                m_used_tail = slot;
                slot->next_in_generation = 0;
                m_free_offset += size + sizeof(Slot);

                return slot;
            }

            void MarkAsFree(Slot* slot)
            {
                if (slot->next_in_area && slot->prev_in_area)
                {
                    slot->next_in_area->prev_in_area = slot->prev_in_area;
                    slot->prev_in_area->next_in_area = slot->next_in_area;
                }
                else if (slot->next_in_area)
                {
                    slot->next_in_area->prev_in_area = 0;
                    m_used_head = slot->next_in_area;
                }
                else if (slot->prev_in_area)
                {
                    slot->prev_in_area->next_in_area = 0;
                    m_used_tail = slot->prev_in_area;
                } else {
                    m_used_head = m_used_tail = 0;
                }
                if ((slot->prev_in_area = m_free_tail))
                {
                    slot->prev_in_area->next_in_area = slot;
                } else {
                    m_free_head = slot;
                }
                slot->next_in_area = 0;
                m_free_tail = slot;
            }

        private:
            static Area* s_area_head;
            /** Pointer to the allocated memory. */
            byte* m_table;
            /** Pointer to next area in the list. */
            Area* m_next;
            /** Offset of the next free slot. */
            std::size_t m_free_offset;
            /** Pointer to first used slot. */
            Slot* m_used_head;
            /** Pointer to last used slot. */
            Slot* m_used_tail;
            /** Pointer to first free slot. */
            Slot* m_free_head;
            /** Pointer to last free slot. */
            Slot* m_free_tail;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Area);
        };

        Area* Area::s_area_head = 0;
    }

    struct Generation
    {
        /** Examination threshold of this generation. */
        const int threshold;
        /** Number of examinations performed in this generation. */
        int counter;
        /** Pointer to the first slot in the generation. */
        Area::Slot* head;
    };

    static Generation gc_generations[3] =
    {
        { TEMPEARLY_GC_THRESHOLD0, 0, 0 },
        { TEMPEARLY_GC_THRESHOLD1, 0, 0 },
        { TEMPEARLY_GC_THRESHOLD2, 0, 0 },
    };

    static void gc_generation_mark(Generation&);
    static void gc_generation_unmark(Generation&);
    static void gc_generation_examine(Generation&, Generation&);

    CountedObject::CountedObject()
        : m_flags(0)
        , m_reference_count(0)
        , m_handle_head(0)
        , m_handle_tail(0) {}

    CountedObject::~CountedObject()
    {
        for (Handle<CountedObject>* handle = m_handle_head; handle; handle = handle->m_next)
        {
            handle->m_pointer = 0;
        }
    }

    void CountedObject::Mark()
    {
        SetFlag(FLAG_MARKED);
    }

    void* CountedObject::operator new(std::size_t size)
    {
        Area::Slot* slot;
        const std::size_t remainder = size % 8;

        if (remainder > 0)
        {
            size += 8 - remainder;
        }
        if (++gc_generations[0].counter >= gc_generations[0].threshold)
        {
#if defined(TEMPEARLY_GC_DEBUG)
            std::fprintf(stderr, "GC: Generation 0 reached threshold.\n");
#endif
            gc_generations[0].counter = 0;
            for (int i = 0; i < 3; ++i)
            {
                gc_generation_mark(gc_generations[i]);
            }
            gc_generation_examine(gc_generations[0], gc_generations[1]);
            if (++gc_generations[1].counter >= gc_generations[1].threshold)
            {
#if defined(TEMPEARLY_GC_DEBUG)
                std::fprintf(stderr, "GC: Generation 1 reached threshold.\n");
#endif
                gc_generations[1].counter = 0;
                gc_generation_examine(gc_generations[1], gc_generations[2]);
                if (++gc_generations[2].counter >= gc_generations[2].threshold)
                {
#if defined(TEMPEARLY_GC_DEBUG)
                    std::fprintf(stderr, "GC: Generation 2 reached threshold.\n");
#endif
                    gc_generations[2].counter = 0;
                    gc_generation_examine(gc_generations[2], gc_generations[2]);
                }
            } else {
                gc_generation_unmark(gc_generations[1]);
            }
            gc_generation_unmark(gc_generations[2]);
        }
        if (!(slot = Area::GlobalAllocate(size)))
        {
            std::abort();
        }
        slot->next_in_generation = gc_generations[0].head;
        gc_generations[0].head = slot;

        return static_cast<void*>(slot->object);
    }

    void CountedObject::operator delete(void* pointer) {}

    static void gc_generation_mark(Generation& generation)
    {
        for (Area::Slot* slot = generation.head; slot; slot = slot->next_in_generation)
        {
            CountedObject* object = slot->object;

            if (!object->IsMarked() && object->GetReferenceCount() > 0)
            {
                object->Mark();
            }
        }
    }

    static void gc_generation_unmark(Generation& generation)
    {
        for (Area::Slot* slot = generation.head; slot; slot = slot->next_in_generation)
        {
            CountedObject* object = slot->object;

            if (object->IsMarked())
            {
                object->UnsetFlag(CountedObject::FLAG_MARKED);
            }
        }
    }

    static void gc_generation_examine(Generation& younger, Generation& older)
    {
        Area::Slot* current = younger.head;
        Area::Slot* next;
        Area::Slot* saved_head = 0;
        Area::Slot* saved_tail = 0;
#if defined(TEMPEARLY_GC_DEBUG)
        int destroyed_count = 0;
        int saved_count = 0;
#endif

        younger.head = 0;
        for (; current; current = next)
        {
            next = current->next_in_generation;
            if (current->object->IsMarked())
            {
                current->next_in_generation = 0;
                if (saved_tail)
                {
                    saved_tail->next_in_generation = current;
                } else {
                    saved_head = current;
                }
#if defined(TEMPEARLY_GC_DEBUG)
                ++saved_count;
#endif
            } else {
                current->object->~CountedObject();
                current->area->MarkAsFree(current);
                // TODO: destroy entire area if it's empty
#if defined(TEMPEARLY_GC_DEBUG)
                ++destroyed_count;
#endif
            }
            if (saved_tail)
            {
                saved_tail->next_in_generation = older.head;
                older.head = saved_head;
            }
        }
#if defined(TEMPEARLY_GC_DEBUG)
        std::fprintf(stderr, "GC: Examination: %d saved, %d destroyed.\n", saved_count, destroyed_count);
#endif
    }
}
