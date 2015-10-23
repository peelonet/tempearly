#include <fcgi_stdio.h>

#include "interpreter.h"
#include "core/filename.h"
#include "io/stream.h"
#include "sapi/cgi/request.h"
#include "sapi/cgi/response.h"
#include "script/parser.h"

using namespace tempearly;

static Handle<Script> compile_script(const Filename&, String&);

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        String error_message;
        Handle<Script> script = compile_script(Filename(argv[1]), error_message);

        while (FCGI_Accept() >= 0)
        {
            const Handle<Interpreter> interpreter = new Interpreter(
                new CgiRequest(),
                new CgiResponse()
            );

            interpreter->Initialize();
            interpreter->PushFrame();
            if (script)
            {
                if (!script->Execute(interpreter))
                {
                    interpreter->GetResponse()->SendException(interpreter->GetException());
                }
                else if (!interpreter->GetResponse()->IsCommitted())
                {
                    interpreter->GetResponse()->Commit();
                }
            } else {
                interpreter->Throw(interpreter->eSyntaxError, error_message);
                interpreter->GetResponse()->SendException(interpreter->GetException());
            }
            interpreter->PopFrame();
        }
    }

    return EXIT_SUCCESS;
}

static Handle<Script> compile_script(const Filename& filename, String& error_message)
{
    Handle<Stream> stream = filename.Open(Filename::MODE_READ);

    if (stream)
    {
        Handle<ScriptParser> parser = new ScriptParser(stream);
        Handle<Script> script = parser->Compile();

        parser->Close();

        return script;
    }
    error_message = "Unable to include file";

    return Handle<Script>();
}
