#include "interpreter.h"
#include "api/file.h"
#include "api/iterator.h"
#include "api/list.h"
#include "core/bytestring.h"

#if defined(_WIN32)
# define UNICODE
# include <windows.h>
#else
# include <cstring>
# include <dirent.h>
# include <unistd.h>
#endif

namespace tempearly
{
    FileObject::FileObject(const Handle<Interpreter>& interpreter, const Filename& path)
        : Object(interpreter->cFile)
        , m_path(path) {}

    /**
     * File.__call__(path) => File
     *
     * Constructs new file object from given path.
     *
     *     File("/home/rauli") => /home/rauli
     *
     * Throws: ValueError - If the given string cannot be parsed into a
     * filename.
     */
    TEMPEARLY_NATIVE_METHOD(file_s_call)
    {
        String source;

        if (args[0].AsString(interpreter, source))
        {
            Filename path(source);

            if (!path.IsEmpty())
            {
                return Value::NewObject(new FileObject(interpreter, path));
            }
            interpreter->Throw(interpreter->eValueError,
                               "Unable to parse given string into path");
        }

        return Value();
    }

    /**
     * File.pwd() => File
     *
     * Returns current working directory.
     *
     * Throws: IOError - If current working directory cannot be retrieved for
     * some reason.
     */
    TEMPEARLY_NATIVE_METHOD(file_s_pwd)
    {
        Filename path;
#if defined(_WIN32)
        wchar_t buffer[1024];

        if (::GetCurrentDirectoryW(1024, buffer) == 0)
        {
            interpreter->Throw(interpreter->eIOError, "Unable to retrieve current working directory");

            return Value();
        }
        path = String::narrow(buffer);
#else
        char buffer[1024];

        if (!::getcwd(buffer, 1024))
        {
            interpreter->Throw(interpreter->eIOError, "Unable to retrieve current working directory");

            return Value();
        }
        path = buffer;
#endif

        return Value::NewObject(new FileObject(interpreter, path));
    }

    /**
     * File#parts() => List
     *
     * Returns all components of the filename as a list.
     *
     *     File("/usr/local/bin").parts()   #=> ["/", "usr", "local", "bin"]
     */
    TEMPEARLY_NATIVE_METHOD(file_parts)
    {
        Handle<ListObject> list = new ListObject(interpreter->cList);
        std::vector<String> parts = args[0].As<FileObject>()->GetPath().GetParts();

        for (std::size_t i = 0; i < parts.size(); ++i)
        {
            list->Append(Value::NewString(parts[i]));
        }

        return Value::NewObject(list);
    }

    /**
     * File#name() => String
     *
     * Returns the last component of the filename.
     *
     *     File("/home/user").name()    #=> "user"
     */
    TEMPEARLY_NATIVE_METHOD(file_name)
    {
        return Value::NewString(args[0].As<FileObject>()->GetPath().GetName());
    }

    /**
     * File#extension() => String
     *
     * Returns the extension (the portion of filename in path starting from the
     * last period).
     *
     * If path is a dotfile, or starts with a period, then starting dot is not
     * dealth with the start of the extension.
     *
     * Null is also returned when period is the last character of the path.
     *
     *     File("/home/user/file.txt").extension()      #=> "txt"
     *     File("/home/user/file.").extension()         #=> null
     *     File("/home/user/file").extension()          #=> null
     *     File("/home/user/.profile").extension()      #=> null
     *     File("/home/user/.profile.sh").extension()   #=> "sh"
     */
    TEMPEARLY_NATIVE_METHOD(file_extension)
    {
        String extension = args[0].As<FileObject>()->GetPath().GetExtension();

        if (extension.IsEmpty())
        {
            return Value::NullValue();
        } else {
            return Value::NewString(extension);
        }
    }

    /**
     * File#exists() => Bool
     *
     * Returns true if file exists in file system, false otherwise.
     */
    TEMPEARLY_NATIVE_METHOD(file_exists)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().Exists());
    }

    /**
     * File#is_dir() => Bool
     *
     * Returns true if file exists in file system and is a directory, false
     * otherwise.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_dir)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsDir());
    }

    /**
     * File#is_symlink() => Bool
     *
     * Returns true if file exists in file system and is pointing to symbolic
     * link, false otherwise.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_symlink)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsSymlink());
    }

    /**
     * File#is_socket() => Bool
     *
     * Returns true if file exist in file system and is pointing to Unix
     * socket.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_socket)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsSocket());
    }

    /**
     * File#is_fifo() => Bool
     *
     * Returns true if file exists in file system and is pointing to a FIFO.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_fifo)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsFifo());
    }

    /**
     * File#is_char_device() => Bool
     *
     * Returns true if file exists in file system and is pointing to a
     * character device.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_char_device)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsCharDevice());
    }

    /**
     * File#is_block_device() => Bool
     *
     * Returns true if file exists in file system and is pointing to a block
     * device.
     */
    TEMPEARLY_NATIVE_METHOD(file_is_block_device)
    {
        return Value::NewBool(args[0].As<FileObject>()->GetPath().IsBlockDevice());
    }

    /**
     * File#__hash__() => Int
     *
     * Generates hash code from the filename.
     */
    TEMPEARLY_NATIVE_METHOD(file_hash)
    {
        const String path = args[0].As<FileObject>()->GetPath().GetFullName();

#if defined(_WIN32)
        return Value::NewInt(path.Map(String::ToLower).HashCode() ^ 1234321);
#else
        return Value::NewInt(path.HashCode() ^ 1234321);
#endif
    }

    namespace
    {
#if defined(_WIN32)
        class WinFileIterator : public IteratorObject
        {
        public:
            explicit WinFileIterator(const Handle<Class>& cls,
                                     const Filename& parent,
                                     HANDLE handle)
                : IteratorObject(cls)
                , m_parent(parent)
                , m_handl(handle) {}

            ~WinFileIterator()
            {
                if (m_handle)
                {
                    ::FindClose(m_handle);
                }
            }

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_handle)
                {
                    while (::FindNextFile(m_handle, m_find_data))
                    {
                        if (::wscmp(m_find_data.cFileName, L".") && ::wscmp(m_find_data.cFileName, L".."))
                        {
                            return Result(
                                Result::KIND_SUCCESS,
                                Value::NewObject(new FileObject(interpreter, String::Narrow(m_find_data.cFileName)))
                            );
                        }
                    }
                    ::FindClose(m_handle);
                    m_handle = 0;
                }

                return Result(Result::KIND_BREAK);
            }

        private:
            const Filename m_parent;
            HANDLE m_handle;
            WIN32_FIND_DATA m_find_data;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(WinFileIterator);
        };
#else
        class PosixFileIterator : public IteratorObject
        {
        public:
            explicit PosixFileIterator(const Handle<Class>& cls,
                                       const Filename& parent,
                                       DIR* handle)
                : IteratorObject(cls)
                , m_parent(parent)
                , m_handle(handle) {}

            ~PosixFileIterator()
            {
                if (m_handle)
                {
                    ::closedir(m_handle);
                }
            }

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                if (m_handle)
                {
                    dirent* data;

                    while ((data = ::readdir(m_handle)))
                    {
                        if (std::strcmp(data->d_name, ".") && std::strcmp(data->d_name, ".."))
                        {
                            return Result(
                                Result::KIND_SUCCESS,
                                Value::NewObject(new FileObject(interpreter, m_parent + data->d_name))
                            );
                        }
                    }
                    ::closedir(m_handle);
                    m_handle = 0;
                }

                return Result(Result::KIND_BREAK);
            }

        private:
            const Filename m_parent;
            DIR* m_handle;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(PosixFileIterator);
        };
#endif
    }

    /**
     * File#__iter__() => Iterator
     *
     * If filename is pointing to a directory, returns an iterator which can be
     * used to iterate all files in that directory. Otherwise an exception is
     * thrown.
     *
     * Throws: IOError - If filename is not pointing to a directory.
     */
    TEMPEARLY_NATIVE_METHOD(file_iter)
    {
        const Filename& parent = args[0].As<FileObject>()->GetPath();

#if defined(_WIN32)
        WIN32_FIND_DATA find_data;
        HANDLE handle = ::FindFirstFileW(parent.GetFullName().Widen().c_str(), &find_data);

        if (handle != INVALID_HANDLE_VALUE)
        {
            Handle<IteratorObject> iterator = new WinFileIterator(interpreter->cIterator, parent, handle);

            iterator->Feed(Value::NewObject(
                new FileObject(interpreter, parent + String::Narrow(find_data.cFileName)))
            );

            return Value::NewObject(iterator);
        }
#else
        DIR* handle = ::opendir(parent.GetFullName().Encode().c_str());

        if (handle)
        {
            return Value::NewObject(new PosixFileIterator(interpreter->cIterator, parent, handle));
        }
#endif
        interpreter->Throw(interpreter->eIOError, "Unable to iterate directory");

        return Value();
    }

    /**
     * File#__cmp__(other) => Int
     *
     * Compares two filenames lexicographically against each other.
     */
    TEMPEARLY_NATIVE_METHOD(file_cmp)
    {
        if (args[1].IsFile())
        {
            const Filename a = args[0].As<FileObject>()->GetPath();
            const Filename b = args[1].As<FileObject>()->GetPath();

            return Value::NewInt(a.Compare(b));
        }

        return Value::NullValue();
    }

    /**
     * File#__str__() => String
     *
     * Returns path of the filename.
     */
    TEMPEARLY_NATIVE_METHOD(file_str)
    {
        return Value::NewString(args[0].As<FileObject>()->GetPath().GetFullName());
    }

    void init_file(Interpreter* i)
    {
        i->cFile = i->AddClass("File", i->cObject);

        i->cFile->AddStaticMethod(i, "__call__", 1, file_s_call);
        i->cFile->AddStaticMethod(i, "pwd", 0, file_s_pwd);

        i->cFile->AddMethod(i, "parts", 0, file_parts);
        i->cFile->AddMethod(i, "name", 0, file_name);
        i->cFile->AddMethod(i, "extension", 0, file_extension);

        i->cFile->AddMethod(i, "exists", 0, file_exists);
        i->cFile->AddMethod(i, "is_dir", 0, file_is_dir);
        i->cFile->AddMethod(i, "is_symlink", 0, file_is_symlink);
        i->cFile->AddMethod(i, "is_socket", 0, file_is_socket);
        i->cFile->AddMethod(i, "is_fifo", 0, file_is_fifo);
        i->cFile->AddMethod(i, "is_char_device", 0, file_is_char_device);
        i->cFile->AddMethod(i, "is_block_device", 0, file_is_block_device);

        i->cFile->AddMethod(i, "__hash__", 0, file_hash);
        i->cFile->AddMethod(i, "__iter__", 0, file_iter);

        // Operators
        i->cFile->AddMethod(i, "__cmp__", 1, file_cmp);

        // Conversion methods
        i->cFile->AddMethod(i, "__str__", 0, file_str);
    }
}
