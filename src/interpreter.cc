#include "class.h"
#include "interpreter.h"
#include "parser.h"

namespace tempearly
{
    void init_bool(Interpreter*);
    void init_number(Interpreter*);
    void init_object(Interpreter*);
    void init_string(Interpreter*);
    void init_void(Interpreter*);

    Interpreter::Interpreter() {}

    void Interpreter::Initialize()
    {
        if (cObject)
        {
            return;
        }

        globals = new Scope(Handle<Scope>(), Handle<Scope>());

        init_object(this);
        init_bool(this);
        init_number(this);
        init_string(this);
        init_void(this);
    }

    bool Interpreter::Include(const String& filename)
    {
        FILE* stream = std::fopen(filename.c_str(), "rb");

        if (stream)
        {
            Handle<Parser> parser = new Parser(stream);
            Handle<Script> script = parser->Compile(this);

            parser->Close();
            if (script)
            {
                bool result;

                PushScope(globals);
                result = script->Execute(this);
                PopScope();

                return result;
            }
        } else {
            // TODO: throw exception
        }

        return false;
    }

    Handle<Class> Interpreter::AddClass(const String& name,
                                        const Handle<Class>& base)
    {
        Handle<Class> cls = new Class(base);

        if (!name.empty())
        {
            cls->SetAttribute("__name__", Value::NewString(name));
        }
        if (globals)
        {
            globals->SetVariable(name, Value::NewObject(cls.Get()));
        }

        return cls;
    }

    void Interpreter::Throw(const Handle<Class>& cls, const String& message)
    {
        // TODO: Construct exception object.
        std::fprintf(stderr, "ERROR: %s\n", message.c_str());
        std::abort();
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

    void Interpreter::Mark()
    {
        CountedObject::Mark();
        if (globals && !globals->IsMarked())
        {
            globals->Mark();
        }
        m_exception.Mark();
        if (m_scope && !m_scope->IsMarked())
        {
            m_scope->Mark();
        }
    }
}
