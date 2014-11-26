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
    namespace
    {
        struct Slot
        {
            /** Size of the object. */
            std::size_t size;
            /** Pointer to the object. */
            CountedObject* object;
            /** Pointer to next slot in list. */
            Slot* next;
        };

        class Generation
        {
        public:
            /** Number of examinations performed in this generation. */
            unsigned int counter;

            explicit Generation()
                : counter(0)
                , m_head(0) {}

            ~Generation()
            {
                Slot* current;
                Slot* next;

                for (current = m_head; current; current = next)
                {
                    next = current->next;
                    current->object->~CountedObject();
                    std::free(static_cast<void*>(current));
                }
            }

            void* Allocate(std::size_t size)
            {
                byte* data = static_cast<byte*>(std::malloc(sizeof(Slot) + size));
                Slot* slot;

                if (!data)
                {
                    throw std::bad_alloc();
                }
                slot = reinterpret_cast<Slot*>(data);
                slot->size = size;
                slot->object = reinterpret_cast<CountedObject*>(data + sizeof(Slot));
                slot->next = m_head;
                m_head = slot;

                return static_cast<void*>(slot->object);
            }

            void Examine(Generation& that)
            {
#if defined(TEMPEARLY_GC_DEBUG)
                int destroyed_count = 0;
                int saved_count = 0;
#endif
                Slot* current = m_head;
                Slot* next;
                Slot* saved_head = 0;
                Slot* saved_tail = 0;

                m_head = 0;
                for (; current; current = next)
                {
                    next = current->next;
                    if (current->object->IsMarked())
                    {
                        current->next = 0;
                        if (saved_tail)
                        {
                            saved_tail->next = current;
                        } else {
                            saved_head = current;
                        }
                        saved_tail = current;
#if defined(TEMPEARLY_GC_DEBUG)
                        ++saved_count;
#endif
                    } else {
                        current->object->~CountedObject();
                        std::free(static_cast<void*>(current));
#if defined(TEMPEARLY_GC_DEBUG)
                        ++destroyed_count;
#endif
                    }
                }
                if (saved_tail)
                {
                    saved_tail->next = that.m_head;
                    that.m_head = saved_head;
                }
#if defined(TEMPEARLY_GC_DEBUG)
                std::fprintf(stderr, "GC: Examination: %d saved, %d destroyed\n", saved_count, destroyed_count);
#endif
            }

            void Mark()
            {
                for (Slot* slot = m_head; slot; slot = slot->next)
                {
                    CountedObject* object = slot->object;

                    if (!object->IsMarked() && object->GetReferenceCount() > 0)
                    {
                        object->Mark();
                    }
                }
            }

            void Unmark()
            {
                for (Slot* slot = m_head; slot; slot = slot->next)
                {
                    slot->object->UnsetFlag(CountedObject::FLAG_MARKED);
                }
            }

        private:
            /** Pointer to first object in the generation. */
            Slot* m_head;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Generation);
        };
    }

    static Generation generation0;
    static Generation generation1;
    static Generation generation2;

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
        if (++generation0.counter >= TEMPEARLY_GC_THRESHOLD0)
        {
#if defined(TEMPEARLY_GC_DEBUG)
            std::fprintf(stderr, "GC: Generation 0 reached threshold.\n");
#endif
            generation0.counter = 0;
            generation2.Mark();
            generation1.Mark();
            generation0.Mark();
            generation0.Examine(generation1);
            if (++generation1.counter >= TEMPEARLY_GC_THRESHOLD1)
            {
#if defined(TEMPEARLY_GC_DEBUG)
                std::fprintf(stderr, "GC: Generation 1 reached threshold.\n");
#endif
                generation1.counter = 0;
                generation1.Examine(generation2);
                if (++generation2.counter >= TEMPEARLY_GC_THRESHOLD2)
                {
#if defined(TEMPEARLY_GC_DEBUG)
                    std::fprintf(stderr, "GC: Generation 2 reached threshold.\n");
#endif
                    generation2.counter = 0;
                    generation2.Examine(generation2);
                }
            } else {
                generation1.Unmark();
            }
            generation2.Unmark();
        }

        return generation0.Allocate(size);
    }

    void CountedObject::operator delete(void* pointer) {}
}
