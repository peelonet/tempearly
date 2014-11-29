#include "interpreter.h"
#include "api/file.h"
#include "api/iterator.h"
#include "api/list.h"
#include "core/bytestring.h"
#include "io/stream.h"

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
                return Value(new FileObject(interpreter, path));
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

        return Value(new FileObject(interpreter, path));
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
        Vector<String> parts = args[0].As<FileObject>()->GetPath().GetParts();

        for (std::size_t i = 0; i < parts.GetSize(); ++i)
        {
            list->Append(Value::NewString(parts[i]));
        }

        return Value(list);
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
                            return Value(new FileObject(interpreter, String::Narrow(m_find_data.cFileName)));
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
                            return Value(new FileObject(interpreter, m_parent + data->d_name));
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

            iterator->Feed(Value(new FileObject(interpreter, parent + String::Narrow(find_data.cFileName))));

            return Value(iterator);
        }
#else
        DIR* handle = ::opendir(parent.GetFullName().Encode().c_str());

        if (handle)
        {
            return Value(new PosixFileIterator(interpreter->cIterator, parent, handle));
        }
#endif
        interpreter->Throw(interpreter->eIOError, "Unable to iterate directory");

        return Value();
    }

    /**
     * File#__eq__(other) => Bool
     *
     * Compares two filenames lexicographically against each other and returns
     * true if they are equal.
     */
    TEMPEARLY_NATIVE_METHOD(file_eq)
    {
        const Value& self = args[0];
        const Value& operand = args[1];

        if (operand.IsFile())
        {
            return Value::NewBool(self.As<FileObject>()->GetPath().Equals(operand.As<FileObject>()->GetPath()));
        } else {
            return Value::NewBool(false);
        }
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

    namespace
    {
        class FileStreamObject : public Object
        {
        public:
            explicit FileStreamObject(const Handle<Interpreter>& interpreter,
                                      const String& path,
                                      const Filename::OpenMode mode,
                                      bool binary_mode,
                                      bool append)
                : Object(interpreter->cFileStream),
                m_filename(path),
                m_stream(m_filename.Open(mode, append)),
                m_is_binary(binary_mode) {}

            ~FileStreamObject()
            {
                Close();
            }

            inline bool IsOpen() const
            {
                return m_stream && m_stream->IsOpen();
            }

            inline bool IsReadable() const
            {
                return m_stream && m_stream->IsReadable();
            }

            inline bool IsWritable() const
            {
                return m_stream && m_stream->IsWritable();
            }

            inline bool Exists() const
            {
                return m_filename.Exists();
            }

            inline bool IsFile() const
            {
                return m_filename.IsFile();
            }

            inline bool IsBinary() const
            {
                return m_is_binary;
            }

            std::size_t Write(ByteString data)
            {
                if (m_stream && m_stream->Write(data))
                {
                    return data.GetLength();
                }

                return 0;
            }

            bool Read(ByteString& out, std::size_t size)
            {
                std::size_t read;
                std::size_t left;
                byte buffer[Stream::kBufferSize];

                if (m_stream)
                {
                    left = size ? size : String::npos;

                    while (true)
                    {
                        if (m_stream->Read(buffer,
                                           (left < Stream::kBufferSize)
                                            ? left : Stream::kBufferSize,
                                           read))
                        {
                            out = out.Concat(ByteString(buffer, read));
                            left -= read;

                            if (!left || read < Stream::kBufferSize)
                            {
                                return true;
                            }

                            continue;
                        }

                        break;
                    }
                }

                return false;
            }

            void Close()
            {
                if (m_stream)
                {
                    m_stream->Close();
                    m_stream = 0;
                }
            }

            void Mark()
            {
                if (m_stream && !m_stream->IsMarked())
                {
                    m_stream->Mark();
                }
            }

        private:
            Filename m_filename;
            Stream* m_stream;
            const bool m_is_binary;
        };
    }

    /**
     * File.open(Path, [Mode = "r"]) => File Stream Object
     *
     * Opens a stream to a file and returns a handle (the stream object) to it
     */
    TEMPEARLY_NATIVE_METHOD(stream_s_open)
    {
        String mode_str;
        String path;
        Filename::OpenMode mode = Filename::MODE_READ;
        bool binary = false;
        bool append = false;

        if (args.GetSize() > 1)
        {
            if (!args[1].AsString(interpreter, mode_str))
            {
                interpreter->Throw(interpreter->eValueError,
                                   "File mode must be a string");
                return Value();
            }

            bool got_read = false;

            for(size_t i = 0; i < mode_str.GetLength(); ++i)
            {
                switch (mode_str[i])
                {
                    case 'a':
                        append = true; // Case fall-trough is intended
                    case 'w':
                        if (got_read)
                        {
                            mode = Filename::MODE_READ_WRITE;
                        } else {
                            mode = Filename::MODE_WRITE;
                        }
                        break;
                    case 'b':
                        binary = true;
                        break;
                    case 'r':
                        got_read = true;

                        if (mode != Filename::MODE_READ)
                        {
                            mode = Filename::MODE_READ_WRITE;
                        }
                        break;
                    default:
                        interpreter->Throw(interpreter->eValueError,
                                           "Invalid file mode");
                        return Value();
                }
            }
        }

        if (!args[0].AsString(interpreter, path))
        {
            interpreter->Throw(interpreter->eValueError,
                               "Invalid file path");
            return Value();
        }

        if (mode == Filename::MODE_READ && !append && !(Filename(path).Exists()))
        {
            interpreter->Throw(interpreter->eValueError,
                               "File does not exist");
            return Value();
        }

        return Value(new FileStreamObject(interpreter, path, mode, binary, append));
    }

    /**
     * Stream#close() => Void
     *
     * Closes the stream
     */
    TEMPEARLY_NATIVE_METHOD(stream_close)
    {
        Handle<FileStreamObject> stream = args[0].As<FileStreamObject>();

        stream->Close();

        return Value::NullValue();
    }

    /**
     * Stream#read([size = null]) => String
     *
     * Reads the contents of the underlying file pointed by the stream and
     * returns the result as a string or in binary if binary mode is set
     */
    TEMPEARLY_NATIVE_METHOD(stream_read)
    {
        Handle<FileStreamObject> stream = args[0].As<FileStreamObject>();
        i64 read_size = 0;

        if (args.GetSize() > 1)
        {
            if (!args[1].IsNull())
            {
                if (!args[1].AsInt(interpreter, read_size))
                {
                    interpreter->Throw(interpreter->eValueError,
                                       "Read size must be an integer");
                    return Value();
                }

                if (read_size < 0)
                {
                    interpreter->Throw(interpreter->eValueError,
                                       "Read size cannot be negative");
                    return Value();
                }
            }
        }

        ByteString data;

        if (!stream->Read(data, read_size))
        {
            interpreter->Throw(interpreter->eValueError, "File is not readable");
            return Value();
        }

        if (stream->IsBinary())
        {
            return Value::NewBinary(data);
        }

        return Value::NewString(data.c_str());
    }

    /**
     * Stream#write(Object) => Int
     *
     * Takes a string or binary object as a parameter and writes it to the stream
     * Returns the number of bytes written
     */
    TEMPEARLY_NATIVE_METHOD(stream_write)
    {
        Handle<FileStreamObject> stream = args[0].As<FileStreamObject>();
        std::size_t len;

        if (!stream->IsWritable())
        {
            interpreter->Throw(interpreter->eValueError,
                               "File is not writable");
            return Value();
        }

        if (stream->IsBinary())
        {
            ByteString data;

            if (!args[1].AsBinary(interpreter, data))
            {
                interpreter->Throw(interpreter->eValueError,
                                   "Cannot convert the argument to binary");
                return Value();
            }
            else if(data.IsEmpty())
            {
                return Value::NewInt(0);
            }

            len = stream->Write(data);
        }
        else
        {
            String data;

            if (!args[1].AsString(interpreter, data))
            {
                interpreter->Throw(interpreter->eValueError,
                                   "Cannot convert the argument to a string");
                return Value();
            }
            else if (data.IsEmpty())
            {
                return Value::NewInt(0);
            }

            len = stream->Write(data.Encode());
        }

        if (len)
        {
            return Value::NewInt(static_cast<i64>(len));
        }

        return Value();
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
        i->cFile->AddMethod(i, "__eq__", 1, file_eq);

        // Conversion methods
        i->cFile->AddMethod(i, "__str__", 0, file_str);

        // File Stream Object initializer
        i->cFile->AddStaticMethod(i, "open", -1, stream_s_open);

        // File Stream Object methods
        i->cFileStream = new Class(i->cObject);
        i->cFileStream->AddMethod(i, "close", 0, stream_close);
        i->cFileStream->AddMethod(i, "read", -1, stream_read);
        i->cFileStream->AddMethod(i, "write", -1, stream_write);
    }
}
