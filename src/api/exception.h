#ifndef TEMPEARLY_API_EXCEPTION_H_GUARD
#define TEMPEARLY_API_EXCEPTION_H_GUARD

#include "frame.h"

namespace tempearly
{
    class ExceptionObject : public CustomObject
    {
    public:
        explicit ExceptionObject(const Handle<Class>& cls, const Handle<Frame>& frame);

        inline Handle<Frame> GetFrame() const
        {
            return m_frame;
        }

        String GetMessage() const;

        bool IsException() const
        {
            return true;
        }

        void Mark();

    private:
        Frame* m_frame;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ExceptionObject);
    };
}

#endif /* !TEMPEARLY_API_EXCEPTION_H_GUARD */
