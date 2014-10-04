#include "class.h"
#include "interpreter.h"
#include "parser.h"

namespace tempearly
{
    void init_bool(Interpreter*);
    void init_object(Interpreter*);

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
    }

    bool Interpreter::Include(const String& filename)
    {
        FILE* stream = std::fopen(filename.c_str(), "rb");

        if (stream)
        {
            Handle<Parser> parser = new Parser(stream);
            std::vector<Handle<Node> > script;
            bool result = parser->Compile(this, script);

            parser->Close();
            if (result)
            {
                for (std::vector<Handle<Node> >::iterator i = script.begin(); i != script.end(); ++i)
                {
                    const Handle<Node>& node = *i;
                    Result result = node->Execute(this);

                    if (!result.Is(Result::KIND_SUCCESS))
                    {
                        std::fprintf(stdout, "ERROR\n");

                        return EXIT_FAILURE;
                    }
                }
            }
        } else {
            // TODO: throw error
        }

        return false;
    }

    Handle<Class> Interpreter::AddClass(const String& name,
                                        const Handle<Class>& base)
    {
        Handle<Class> cls = new Class(base);

        if (!name.empty())
        {
            cls->SetAttribute("__name__", Value(name));
        }
        if (globals)
        {
            globals->SetVariable(name, Value(cls));
        }

        return cls;
    }

    void Interpreter::Throw(const Handle<Class>& cls, const String& message)
    {
        std::fprintf(stderr, "ERROR: %s\n", message.c_str());
        std::abort();
    }

    void Interpreter::Mark()
    {
        CountedObject::Mark();
        m_exception.Mark();
        if (globals && !globals->IsMarked())
        {
            globals->Mark();
        }
    }
}
