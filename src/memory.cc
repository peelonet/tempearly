#include "memory.h"

namespace tempearly
{
    CountedObject::CountedObject()
        : m_flags(0)
        , m_reference_counter(0) {}

    CountedObject::~CountedObject() {}

    void CountedObject::Mark()
    {
        SetFlag(FLAG_MARKED);
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
