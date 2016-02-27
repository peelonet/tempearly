#ifndef TEMPEARLY_API_FILE_H_GUARD
#define TEMPEARLY_API_FILE_H_GUARD

#include "customobject.h"
#include "core/filename.h"

namespace tempearly
{
    class FileObject : public CustomObject
    {
    public:
        explicit FileObject(const Handle<Interpreter>& interpreter, const Filename& path);

        inline const Filename& GetPath() const
        {
            return m_path;
        }

        bool IsFile() const
        {
            return true;
        }

    private:
        const Filename m_path;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(FileObject);
    };
}

#endif /* !TEMPEARLY_API_FILE_H_GUARD */
