#include "memory.h"

namespace tempearly
{
    CountedObject::CountedObject()
        : m_marked(false)
        , m_reference_counter(0) {}

    void CountedObject::Mark()
    {
        m_marked = true;
    }

    void* CountedObject::operator new(std::size_t size)
    {
        return std::malloc(size); // TODO
    }

    void CountedObject::operator delete(void* pointer)
    {
        if (pointer)
        {
            std::free(pointer); // TODO: remove
        }
    }
}
