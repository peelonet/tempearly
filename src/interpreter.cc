#include "interpreter.h"
#include "parser.h"
#include "utils.h"
#include "api/function.h"
#include "api/iterator.h"
#include "api/map.h"
#include "core/bytestring.h"
#include "core/filename.h"
#include "io/stream.h"

namespace tempearly
{
    void init_binary(Interpreter*);
    void init_bool(Interpreter*);
    void init_class(Interpreter*);
    void init_core(Interpreter*);
    void init_exception(Interpreter*);
    void init_file(Interpreter*);
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
    void init_string(Interpreter*);
    void init_void(Interpreter*);

    Interpreter::Interpreter()
        : request(0)
        , response(0)
        , globals(0)
        , m_exception(0)
        , m_scope(0)
        , m_empty_iterator(0)
        , m_imported_files(0) {}

    Interpreter::~Interpreter()
    {
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

        globals = new Scope(Handle<Scope>(), Handle<Scope>());

        init_object(this);
        init_iterable(this);
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

        init_request(this);
        init_response(this);
    }

    bool Interpreter::Include(const Filename& filename)
    {
        Handle<Stream> stream = filename.Open(Filename::MODE_READ);

        if (stream)
        {
            Handle<Parser> parser = new Parser(stream);
            Handle<Script> script = parser->Compile();

            parser->Close();
            if (script)
            {
                bool result;

                PushScope(globals);
                result = script->Execute(this);
                PopScope();

                return result;
            } else {
                Throw(eSyntaxError, parser->GetErrorMessage());
            }
        } else {
            Throw(eImportError, "Unable to include file");
        }

        return false;
    }

    Value Interpreter::Import(const Filename& filename)
    {
        const String& full_name = filename.GetFullName();
        Handle<Stream> stream;

        if (m_imported_files)
        {
            Dictionary<Value>::Entry* entry = m_imported_files->Find(full_name);

            if (entry)
            {
                return entry->GetValue();
            }
        }
        if ((stream = filename.Open(Filename::MODE_READ)))
        {
            Handle<Parser> parser = new Parser(stream);
            Handle<Script> script = parser->Compile();
            Value result;

            parser->Close();
            if (!script)
            {
                Throw(eSyntaxError, parser->GetErrorMessage());

                return Value();
            }
            PushScope(globals);
            if (!script->Execute(this))
            {
                PopScope();

                return Value();
            }
            result = m_scope->ToObject(this);
            PopScope();
            if (!m_imported_files)
            {
                m_imported_files = new Dictionary<Value>();
            }
            m_imported_files->Insert(full_name, result);

            return result;
        } else {
            Throw(eImportError, "Unable to import file");

            return Value();
        }
    }

    Handle<Class> Interpreter::AddClass(const String& name, const Handle<Class>& base)
    {
        Handle<Class> cls = new Class();

        if (base)
        {
            cls->AddBase(base);
        }
        if (!name.IsEmpty())
        {
            cls->SetAttribute("__name__", Value::NewString(name));
        }
        if (globals)
        {
            globals->SetVariable(name, Value(cls));
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

            Value Invoke(const Handle<Interpreter>& interpreter, const Vector<Value>& args)
            {
                Value result;

                // Test that we have correct amount of arguments.
                if (m_arity < 0)
                {
                    if (args.GetSize() < static_cast<unsigned>(-(m_arity + 1)))
                    {
                        StringBuilder sb;

                        sb << "Function expected at least "
                           << (-(m_arity) - 1)
                           << " arguments, got "
                           << Utils::ToString(static_cast<u64>(args.GetSize()));
                        interpreter->Throw(interpreter->eTypeError, sb.ToString());

                        return Value();
                    }
                }
                else if (args.GetSize() != static_cast<unsigned>(m_arity))
                {
                    StringBuilder sb;

                    sb << "Function expected "
                       << m_arity
                       << " arguments, got "
                       << Utils::ToString(static_cast<u64>(args.GetSize()));
                    interpreter->Throw(interpreter->eTypeError, sb.ToString());

                    return Value();
                }
                result = m_callback(interpreter, args);

                return result;
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

        if (globals)
        {
            globals->SetVariable(name, Value(function));
        }
    }

    void Interpreter::Throw(const Handle<Class>& cls, const String& message)
    {
        if (cls)
        {
            Handle<ExceptionObject> exception = new ExceptionObject(cls);

            exception->SetAttribute("message", Value::NewString(message));
            m_exception = exception.Get();
        } else {
            std::fprintf(stderr,
                         "%s (fatal internal error)\n",
                         message.Encode().c_str());
            std::abort();
        }
    }

    void Interpreter::PushScope(const Handle<Scope>& parent)
    {
        m_scope = new Scope(m_scope, parent);
    }

    void Interpreter::PopScope()
    {
        if (m_scope)
        {
            m_scope = m_scope->GetPrevious();
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
        if (request && !request->IsMarked())
        {
            request->Mark();
        }
        if (response && !response->IsMarked())
        {
            response->Mark();
        }
        if (globals && !globals->IsMarked())
        {
            globals->Mark();
        }
        if (m_exception && !m_exception->IsMarked())
        {
            m_exception->Mark();
        }
        if (m_scope && !m_scope->IsMarked())
        {
            m_scope->Mark();
        }
        if (m_empty_iterator && !m_empty_iterator->IsMarked())
        {
            m_empty_iterator->Mark();
        }
        if (m_imported_files)
        {
            for (const Dictionary<Value>::Entry* entry = m_imported_files->GetFront(); entry; entry = entry->GetNext())
            {
                entry->GetValue().Mark();
            }
        }
    }
}
