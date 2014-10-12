#include "interpreter.h"
#include "core/filename.h"
#include "sapi/cgi/request.h"
#include "sapi/cgi/response.h"

using namespace tempearly;

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        Handle<Interpreter> interpreter = new Interpreter();

        interpreter->request = new CgiRequest();
        interpreter->response = new CgiResponse();
        interpreter->Initialize();
        if (!interpreter->Include(Filename(argv[1])))
        {
            interpreter->response->SendException(interpreter->GetException());
        }
        else if (!interpreter->response->IsCommitted())
        {
            interpreter->response->Commit();
        }
    }

    return EXIT_SUCCESS;
}
