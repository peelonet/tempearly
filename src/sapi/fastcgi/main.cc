#include "fcgi_stdio.h"

#include "interpreter.h"
#include "core/filename.h"
#include "sapi/cgi/cgi-request.h"
#include "sapi/cgi/cgi-response.h"

using namespace tempearly;

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        Filename filename(argv[1]);

        while (FCGI_Accept() >= 0)
        {
            Handle<Interpreter> interpreter = new Interpreter();

            interpreter->request = new CgiRequest();
            interpreter->response = new CgiResponse();
            interpreter->Initialize();
            // TODO: Modify parser so that it doesn't require interpreter
            // instance so we can compile and cache the script before any
            // requests are made.
            if (!interpreter->Include(filename))
            {
                interpreter->response->SendException(interpreter->GetException());
            }
        }
    }

    return EXIT_SUCCESS;
}
