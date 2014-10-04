#include "interpreter.h"

using namespace tempearly;

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        Handle<Interpreter> interpreter = new Interpreter();

        // TODO: interpreter->Initialize();
        interpreter->Include(argv[1]);

        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "usage: %s <filename>\n", argv[0]);

    return EXIT_FAILURE;
}
