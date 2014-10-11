#ifndef TEMPEARLY_API_EXCEPTION_H_GUARD
#define TEMPEARLY_API_EXCEPTION_H_GUARD

#include "api/object.h"

namespace tempearly
{
    class ExceptionObject : public Object
    {
    public:
        explicit ExceptionObject(const Handle<Class>& cls);

        String GetMessage() const;

        bool IsException() const
        {
            return true;
        }

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(ExceptionObject);
    };
}

#endif /* !TEMPEARLY_API_EXCEPTION_H_GUARD */
