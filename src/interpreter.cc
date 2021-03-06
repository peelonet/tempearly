#include "interpreter.h"
#include "api/class.h"
#include "api/iterator.h"
#include "api/map.h"
#include "core/bytestring.h"
#include "core/filename.h"
#include "io/stream.h"
#include "script/parser.h"

namespace tempearly
{
    void init_binary(Interpreter*);
    void init_bool(Interpreter*);
    void init_class(Interpreter*);
    void init_core(Interpreter*);
    void init_exception(Interpreter*);
    void init_file(Interpreter*);
    void init_filters(Interpreter*);
    void init_function(Interpreter*);
    void init_iterable(Interpreter*);
    void init_iterator(Interpreter*);
    void init_list(Interpreter*);
    void init_map(Interpreter*);
    void init_number(Interpreter*);
    void init_object(Interpreter*);
    void init_range(Interpreter*);
    void init_request(Interpreter*);
    void init_response(Interpreter*);
    void init_set(Interpreter*);
    void init_stream(Interpreter*);
    void init_string(Interpreter*);
    void init_void(Interpreter*);

    Interpreter::Interpreter(const Handle<Request>& request,
                             const Handle<Response>& response)
        : m_request(request)
        , m_response(response)
        , m_frame(nullptr)
        , m_global_variables(nullptr)
        , m_exception(nullptr)
        , m_caught_exception(nullptr)
        , m_empty_iterator(nullptr)
        , m_imported_files(nullptr) {}

    Interpreter::~Interpreter()
    {
        if (m_global_variables)
        {
            delete m_global_variables;
        }
        if (m_imported_files)
        {
            delete m_imported_files;
        }
    }

    void Interpreter::Initialize()
    {
        if (cObject)
        {
            return;
        }

        init_object(this);
        init_iterable(this);
        init_stream(this);
        init_bool(this);
        init_number(this);
        init_string(this);
        init_binary(this);
        init_void(this);
        init_iterator(this);
        init_list(this);
        init_map(this);
        init_set(this);
        init_range(this);
        init_exception(this);
        init_class(this);
        init_function(this);
        init_file(this);

        init_core(this);
        init_filters(this);

        init_request(this);
        init_response(this);
    }

    bool Interpreter::Include(const Filename& filename)
    {
        Handle<Stream> stream = filename.Open(Filename::MODE_READ);

        if (stream)
        {
            Handle<ScriptParser> parser = new ScriptParser(stream);
            Handle<Script> script = parser->Compile();

            parser->Close();
            PushFrame();
            if (script)
            {
                bool result;

                result = script->Execute(this);
                PopFrame();

                return result;
            } else {
                Throw(eSyntaxError, parser->GetErrorMessage());
                PopFrame();
            }
        } else {
            Throw(eImportError, "Unable to include file");
        }

        return false;
    }

    Handle<Object> Interpreter::Import(const Filename& filename)
    {
        const String& full_name = filename.GetFullName();
        Handle<Stream> stream;

        if (m_imported_files)
        {
            Dictionary<Object*>::Entry* entry = m_imported_files->Find(full_name);

            if (entry)
            {
                return entry->GetValue();
            }
        }
        if ((stream = filename.Open(Filename::MODE_READ)))
        {
            Handle<ScriptParser> parser = new ScriptParser(stream);
            Handle<Script> script = parser->Compile();
            Handle<Object> result;

            parser->Close();
            PushFrame();
            if (!script)
            {
                Throw(eSyntaxError, parser->GetErrorMessage());

                return Handle<Object>();
            }
            if (!script->Execute(this))
            {
                PopFrame();

                return Handle<Object>();
            }
            result = m_frame->GetLocalVariables(this);
            PopFrame();
            if (!m_imported_files)
            {
                m_imported_files = new Dictionary<Object*>();
            }
            m_imported_files->Insert(full_name, result);

            return result;
        }
        Throw(eImportError, "Unable to import file");

        return Handle<Object>();
    }

    Handle<Class> Interpreter::AddClass(const String& name, const Handle<Class>& base)
    {
        Handle<Class> cls = new Class(base);

        if (!name.IsEmpty())
        {
            cls->SetOwnAttribute("__name__", Object::NewString(name));
            SetGlobalVariable(name, cls);
        }

        return cls;
    }

    namespace
    {
        class GlobalFunction : public FunctionObject
        {
        public:
            explicit GlobalFunction(const Handle<Interpreter>& interpreter,
                                    int arity,
                                    Callback callback)
                : FunctionObject(interpreter)
                , m_arity(arity)
                , m_callback(callback) {}

            bool Invoke(const Handle<Interpreter>& interpreter,
                        const Handle<Frame>& frame)
            {
                const Vector<Handle<Object>> args = frame->GetArguments();

                // Test that we have correct amount of arguments.
                if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1)))
                    {
                        StringBuilder sb;

                        sb << "Function expected at least "
                           << String::FromU64(-m_arity - 1)
                           << " arguments, got "
                           << String::FromU64(args.GetSize());
                        interpreter->Throw(interpreter->eTypeError, sb.ToString());

                        return false;
                    }
                }
                else if (args.GetSize() != static_cast<unsigned>(m_arity))
                {
                    StringBuilder sb;

                    sb << "Function expected "
                       << String::FromI64(m_arity)
                       << " arguments, got "
                       << String::FromU64(args.GetSize());
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());

                    return false;
                }
                m_callback(interpreter, frame, args);

                return !interpreter->HasException();
            }

        private:
            const int m_arity;
            const Callback m_callback;
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(GlobalFunction);
        };
    }

    void Interpreter::AddFunction(const String& name,
                                  int arity,
                                  FunctionObject::Callback callback)
    {
        Handle<FunctionObject> function = new GlobalFunction(this, arity, callback);

        if (!name.IsEmpty())
        {
            function->SetOwnAttribute("__name__", Object::NewString(name));
            SetGlobalVariable(name, function);
        }
    }

    Handle<Frame> Interpreter::PushFrame(const Handle<Frame>& enclosing,
                                         const Handle<FunctionObject>& function,
                                         const Vector<Handle<Object>>& arguments)
    {
        Handle<Frame> frame = new Frame(m_frame, enclosing, function, arguments);

        m_frame = frame.Get();

        return frame;
    }

    void Interpreter::PopFrame()
    {
        if (m_frame)
        {
            m_frame = m_frame->GetPrevious().Get();
        }
    }

    bool Interpreter::HasGlobalVariable(const String& id) const
    {
        return m_global_variables && m_global_variables->Find(id);
    }

    bool Interpreter::GetGlobalVariable(const String& id, Handle<Object>& slot) const
    {
        if (m_global_variables)
        {
            const Dictionary<Object*>::Entry* e = m_global_variables->Find(id);

            if (e)
            {
                slot = e->GetValue();

                return true;
            }
        }

        return false;
    }

    void Interpreter::SetGlobalVariable(const String& id, const Handle<Object>& value)
    {
        if (!m_global_variables)
        {
            m_global_variables = new Dictionary<Object*>();
        }
        m_global_variables->Insert(id, value);
    }

    bool Interpreter::HasException(const Handle<Class>& cls)
    {
        return m_exception && m_exception->IsInstance(this, cls);
    }

    void Interpreter::Throw(const Handle<Class>& cls, const String& message)
    {
        if (cls)
        {
            Handle<ExceptionObject> exception = new ExceptionObject(cls, m_frame);

            exception->SetOwnAttribute("message", Object::NewString(message));
            m_exception = exception;
        } else {
            std::fprintf(stderr, "%s (fatal internal error)\n", message.Encode().c_str());
            std::abort();
        }
    }

    namespace
    {
        class EmptyIterator : public IteratorObject
        {
        public:
            explicit EmptyIterator(const Handle<Class>& cls)
                : IteratorObject(cls) {}

            Result Generate(const Handle<Interpreter>& interpreter)
            {
                return Result(Result::KIND_BREAK);
            }

        private:
            TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(EmptyIterator);
        };
    }

    Handle<IteratorObject> Interpreter::GetEmptyIterator()
    {
        if (!m_empty_iterator)
        {
            m_empty_iterator = new EmptyIterator(cIterator);
        }

        return m_empty_iterator;
    }

    void Interpreter::Mark()
    {
        CountedObject::Mark();
        if (!m_request->IsMarked())
        {
            m_request->Mark();
        }
        if (!m_response->IsMarked())
        {
            m_response->Mark();
        }
        if (m_frame && !m_frame->IsMarked())
        {
            m_frame->Mark();
        }
        if (m_global_variables)
        {
            for (Dictionary<Object*>::Entry* e = m_global_variables->GetFront();
                 e;
                 e = e->GetNext())
            {
                if (!e->GetValue()->IsMarked())
                {
                    e->GetValue()->Mark();
                }
            }
        }
        if (m_exception && !m_exception->IsMarked())
        {
            m_exception->Mark();
        }
        if (m_caught_exception && !m_caught_exception->IsMarked())
        {
            m_caught_exception->Mark();
        }
        if (m_empty_iterator && !m_empty_iterator->IsMarked())
        {
            m_empty_iterator->Mark();
        }
        if (m_imported_files)
        {
            for (Dictionary<Object*>::Entry* e = m_imported_files->GetFront();
                 e;
                 e = e->GetNext())
            {
                if (!e->GetValue()->IsMarked())
                {
                    e->GetValue()->Mark();
                }
            }
        }
    }
}
