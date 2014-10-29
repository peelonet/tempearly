#include "fcgi_stdio.h"

#include "interpreter.h"
#include "parser.h"
#include "core/filename.h"
#include "io/stream.h"
#include "sapi/cgi/request.h"
#include "sapi/cgi/response.h"

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
            Handle<Interpreter> interpreter = new Interpreter();

            interpreter->request = new CgiRequest();
            interpreter->response = new CgiResponse();
            interpreter->Initialize();
            if (script)
            {
                interpreter->PushScope(interpreter->globals);
                if (!script->Execute(interpreter))
                {
                    interpreter->response->SendException(interpreter->GetException());
                }
                else if (!interpreter->response->IsCommitted())
                {
                    interpreter->response->Commit();
                }
            } else {
                interpreter->Throw(interpreter->eSyntaxError, error_message);
                interpreter->response->SendException(interpreter->GetException());
            }
        }
    }

    return EXIT_SUCCESS;
}

static Handle<Script> compile_script(const Filename& filename, String& error_message)
{
    Handle<Stream> stream = filename.Open("rb");

    if (stream)
    {
        Handle<Parser> parser = new Parser(stream);
        Handle<Script> script = parser->Compile();

        parser->Close();

        return script;
    }
    error_message = "Unable to include file";

    return Handle<Script>();
}
