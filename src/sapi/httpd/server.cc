#include <cstring>
#include <vector>

#include "interpreter.h"
#include "core/filename.h"
#include "core/bytestring.h"
#include "http/method.h"
#include "http/version.h"
#include "net/socket.h"
#include "sapi/httpd/httpd-request.h"
#include "sapi/httpd/httpd-response.h"

#if !defined(HTTPD_MAX_REQUEST_SIZE)
# define HTTPD_MAX_REQUEST_SIZE 4096
#endif

namespace tempearly
{
    struct HttpRequestData
    {
        HttpMethod::Kind method;
        String path;
        String query_string;
        HttpVersion::Kind version;
        Dictionary<String> headers;
    };

    struct MimeTypeMapping
    {
        const char* ext;
        const char* mime_type;
    };

    static const MimeTypeMapping default_mime_types[] =
    {
        { "html", "text/html" },
        { "htm", "text/htm" },
        { "js", "text/javascript" },
        { "css", "text/css" },
        { "gif", "image/gif" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "jpe", "image/jpeg" },
        { "pdf", "application/pdf" },
        { "png", "image/png" },
        { "svg", "image/svg+xml" },
        { "txt", "text/plain" },
        { 0, 0 }
    };

    static Dictionary<String> mime_type_map;

    static byte* parse_request(byte*, std::size_t&, HttpRequestData&);
    static void send_error(const Handle<Socket>&, const char*, const String&);
    static void send_file(const Handle<Socket>&,
                          HttpRequestData&,
                          const Filename&,
                          const String&);
    static void send_script(const Handle<Socket>&,
                            HttpRequestData&,
                            const Filename&,
                            const byte*,
                            std::size_t);
    static String get_mime_type(const String&);

    static void serve(const Handle<Socket>& socket, const Filename& root_path)
    {
        HttpRequestData request;
        byte buffer[HTTPD_MAX_REQUEST_SIZE];
        std::size_t buffer_size;
        byte* data;
        Filename path;

        if (!socket->Receive(buffer, HTTPD_MAX_REQUEST_SIZE, buffer_size))
        {
            socket->Close();
            return;
        }

        if (!(data = parse_request(buffer, buffer_size, request)))
        {
            send_error(socket,
                       "400 Bad Request",
                       "We were unable to process your request.\n");
            return;
        }

        std::fprintf(stdout, "%s %s\n",
                     HttpMethod::ToString(request.method).Encode().c_str(),
                     request.path.Encode().c_str());

        path = root_path + request.path;

        if (!path.Exists())
        {
            send_error(socket,
                       "404 Not Found",
                       "The requested URL "
                       + request.path
                       + " was not found on this server.");
        }
        else if (path.IsDir())
        {
            Filename index = root_path + "index.tly";

            if (index.Exists() && !index.IsDir())
            {
                send_script(socket, request, index, data, buffer_size);
            }
            else if ((index = root_path + "index.html").Exists()
                    && !index.IsDir())
            {
                send_file(socket, request, index, "text/html");
            } else {
                send_error(socket,
                           "403 Forbidden",
                           "You don't have permission to access "
                           + request.path
                           + " on this server");
            }
        } else {
            const String extension = path.GetExtension();

            if (extension == "tly")
            {
                send_script(socket, request, path, data, buffer_size);
            } else {
                send_file(socket, request, path, get_mime_type(extension));
            }
        }
    }

    void httpd_loop(const Handle<Socket>& server_socket,
                    const Filename& root_path)
    {
        for (;;)
        {
            Handle<Socket> client_socket = server_socket->Accept();

            if (client_socket)
            {
                serve(client_socket, root_path);
            } else {
                std::fprintf(stderr, "%s\n",
                             server_socket->GetErrorMessage().Encode().c_str());
            }
        }
    }

    static void send_error(const Handle<Socket>& socket, const char* status, const String& message)
    {
        ByteString content = message.Encode();

        socket->Printf("HTTP/1.0 %s\r\n", status);
        socket->Printf("Content-Type: text/plain; charset=utf-8\r\n");
        socket->Printf("Content-Length: %ld\r\n\r\n", content.GetLength());
        socket->Send(content.c_str(), content.GetLength());
    }

    static void send_file(const Handle<Socket>& socket,
                          HttpRequestData& request,
                          const Filename& path,
                          const String& mime_type)
    {
        FILE* handle = path.Open("rb");
        
        if (handle)
        {
            char buffer[4096];
            std::size_t read;

            socket->Printf("HTTP/1.0 200 OK\r\n");
            socket->Printf("Content-Type: %s\r\n", mime_type.Encode().c_str());
            socket->Printf("Content-Length: %ld\r\n\r\n", path.GetSize());
            while ((read = std::fread(buffer, 1, sizeof(buffer), handle)) > 0)
            {
                if (!socket->Send(buffer, read))
                {
                    break;
                }
            }
            socket->Close();
        } else {
            send_error(socket,
                       "403 Forbidden",
                       "You don't have permission to access "
                       + request.path
                       + " on this server");
        }
    }

    static void send_script(const Handle<Socket>& socket,
                            HttpRequestData& request_data,
                            const Filename& path,
                            const byte* data,
                            std::size_t data_size)
    {
        Handle<Interpreter> interpreter = new Interpreter();

        interpreter->request = new HttpServerRequest(
            socket,
            request_data.method,
            request_data.path,
            request_data.query_string,
            request_data.headers,
            data,
            data_size
        );
        interpreter->response = new HttpServerResponse(socket);
        interpreter->Initialize();
        if (!interpreter->Include(path))
        {
            interpreter->response->SendException(interpreter->GetException());
        }
        socket->Close();
    }

    static bool parse_request_line(const String& line, HttpRequestData& request)
    {
        std::size_t index1;
        std::size_t index2;

        if ((index1 = line.IndexOf(' ')) == String::npos)
        {
            return false;
        }

        if (!HttpMethod::Parse(line.SubString(0, index1++), request.method))
        {
            return false;
        }

        if ((index2 = line.IndexOf(' ', index1)) == String::npos)
        {
            request.path = line.SubString(index1);
            request.version = HttpVersion::VERSION_09;
        } else {
            request.path = line.SubString(index1, index2 - index1);
            if (!HttpVersion::Parse(line.SubString(index2 + 1), request.version))
            {
                return false;
            }
        }

        // Split query string from request path.
        if ((index1 = request.path.IndexOf('?')) != String::npos)
        {
            request.query_string = request.path.SubString(index1 + 1);
            request.path = request.path.SubString(0, index1);
        }

        return true;
    }

    static bool parse_request_header(const String& line, HttpRequestData& request)
    {
        std::size_t index = line.IndexOf(':');

        if (index == String::npos || index < 1)
        {
            return false;
        }
        request.headers.Insert(line.SubString(0, index), line.SubString(index + 2));

        return true;
    }

    static byte* parse_request(byte* start,
                               std::size_t& remain,
                               HttpRequestData& request)
    {
        byte* begin = start;
        byte* end = static_cast<byte*>(std::memchr(begin, '\n', remain));

        if (!end)
        {
            return 0;
        }

        remain -= end - start + 1;
        start = end + 1;
        if (end > begin && end[-1] == '\r')
        {
            --end;
        }
        *end = '\0';

        // Process first line of request
        if (!parse_request_line(reinterpret_cast<char*>(begin), request))
        {
            return 0;
        }

        // Process request headers.
        for (;;)
        {
            begin = start;
            if (!(end = static_cast<byte*>(std::memchr(begin, '\n', remain))))
            {
                return 0;
            }

            remain -= end - start + 1;
            start = end + 1;
            if (end > begin && end[-1] == '\r')
            {
                --end;
            }
            *end = '\0';

            if (!*begin)
            {
                return start;
            }
            else if (!parse_request_header(reinterpret_cast<char*>(begin), request))
            {
                return 0;
            }
        }
    }

    static String get_mime_type(const String& extension)
    {
        const Dictionary<String>::Entry* entry;

        if (mime_type_map.IsEmpty())
        {
            for (int i = 0; default_mime_types[i].ext; ++i)
            {
                mime_type_map.Insert(default_mime_types[i].ext,
                                     default_mime_types[i].mime_type);
            }
        }
        if ((entry = mime_type_map.Find(extension)))
        {
            return entry->value;
        } else {
            return "application/octet-stream";
        }
    }
}
