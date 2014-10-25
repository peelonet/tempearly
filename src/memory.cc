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
            explicit Generation(int threshold)
                : m_head(0)
                , m_threshold(threshold)
                , m_count(0) {}

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

            inline int GetThreshold() const
            {
                return m_threshold;
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

            bool Collect(Generation& that)
            {
                Slot* current = m_head;
                Slot* next;
                Slot* saved_head = 0;
                Slot* saved_tail = 0;

                m_head = 0;
                while (current)
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
                    } else {
                        current->object->~CountedObject();
                        std::free(static_cast<void*>(current));
                    }
                    current = next;
                }
                if (saved_tail)
                {
                    saved_tail->next = that.m_head;
                    that.m_head = saved_head;
                }
                if (m_count++ >= m_threshold)
                {
                    m_count = 0;

                    return true;
                }

                return false;
            }

            void Collect()
            {
                Slot* current = m_head;
                Slot* next;

                m_head = 0;
                for (; current; current = next)
                {
                    next = current->next;
                    if (current->object->IsMarked())
                    {
                        current->next = m_head;
                        m_head = current;
                    } else {
                        current->object->~CountedObject();
                        std::free(static_cast<void*>(current));
                    }
                }
            }

            void Mark()
            {
                for (Slot* slot = m_head; slot; slot = slot->next)
                {
                    if (!slot->object->IsMarked()
                        && slot->object->GetReferenceCount() > 0)
                    {
                        slot->object->Mark();
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
            /** Collection threshold. */
            const int m_threshold;
            /** Count of allocations or collections of younger generations. */
            int m_count;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(Generation);
        };
    }

    static Generation generation0(700);
    static Generation generation1(10);
    static Generation generation2(10);

    static std::size_t allocation_counter = 0;

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
        if (allocation_counter++ >= static_cast<std::size_t>(generation0.GetThreshold()))
        {
            generation0.Mark();
            generation1.Mark();
            generation2.Mark();
            if (generation0.Collect(generation1))
            {
                if (generation1.Collect(generation2))
                {
                    generation2.Collect();
                }
            } else {
                generation1.Unmark();
            }
            generation2.Unmark();
            allocation_counter = 0;
        }

        return generation0.Allocate(size);
    }

    void CountedObject::operator delete(void* pointer) {}
}
