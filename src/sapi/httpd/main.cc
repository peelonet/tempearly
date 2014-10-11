#include "utils.h"
#include "core/bytestring.h"
#include "core/filename.h"
#include "net/socket.h"
#include <cstring>

using namespace tempearly;

namespace tempearly
{
    extern void httpd_loop(const Handle<Socket>&, const Filename&);
}

static bool parse_host_and_port(const String&, String&, int&);

static const char* httpd_usage = "Usage: %s [[HOST:]PORT] [WWW-ROOT]\n";

int main(int argc, char** argv)
{
    Handle<Socket> socket;
    String host = "0.0.0.0";
    int port = 8000;
    Filename root(".");

    if (argc > 3)
    {
        std::fprintf(stderr, httpd_usage, argv[0]);

        return EXIT_FAILURE;
    }
    else if (argc > 2)
    {
        if (!parse_host_and_port(argv[1], host, port)
            || !(root = argv[2]).IsDir())
        {
            std::fprintf(stderr, httpd_usage, argv[0]);

            return EXIT_FAILURE;
        }
    }
    else if (argc > 1)
    {
        if (!std::strcmp(argv[1], "--help") || !std::strcmp(argv[1], "-h"))
        {
            std::fprintf(stdout, httpd_usage, argv[0]);

            return EXIT_SUCCESS;
        }
        if (!parse_host_and_port(argv[1], host, port))
        {
            std::fprintf(stderr, httpd_usage, argv[0]);

            return EXIT_FAILURE;
        }
    }

    socket = new Socket();
    if (!socket->Create(port, SOCK_STREAM, host) || !socket->Listen(25))
    {
        std::fprintf(stderr, "Couldn't initialize the server: %s\n",
                     socket->GetErrorMessage().Encode().c_str());

        return EXIT_FAILURE;
    }

    std::fprintf(stdout, "HTTP server running at http://%s:%d/\n",
                 host.Encode().c_str(),
                 port);

    tempearly::httpd_loop(socket, root);

    return EXIT_SUCCESS;
}

static bool parse_host_and_port(const String& input, String& host, int& port)
{
    std::size_t index = input.IndexOf(':');
    String port_source;

    if (index != String::npos)
    {
        host = input.SubString(0, index);
        port_source = input.SubString(index + 1);
    }
    else
    {
        port_source = input;
    }

    i64 slot;
    if (!Utils::ParseInt(port_source, slot, 10))
    {
        return false;
    }
    port = static_cast<int>(slot);

    return true;
}
