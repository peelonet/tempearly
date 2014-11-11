#include "interpreter.h"
#include "parser.h"
#include "script.h"
#include "core/bytestring.h"
#include "net/socket.h"
#include "sapi/httpd/request.h"
#include "sapi/httpd/response.h"
#include "sapi/httpd/server.h"

#if !defined(HTTPD_MAX_REQUEST_SIZE)
# define HTTPD_MAX_REQUEST_SIZE 4096
#endif

namespace tempearly
{
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

    static byte* parse_request(HttpServer::HttpRequest&, const Handle<Socket>&, byte*, std::size_t&);
    static void send_error(const Handle<Socket>&, const char*, const String&);
    static void compile_script(HttpServer::ScriptMapping&);
    static String get_mime_type(const String&);

    HttpServer::HttpServer(const Filename& root, const Handle<Socket>& socket)
        : m_root(root)
        , m_socket(socket.Get()) {}

    HttpServer::~HttpServer()
    {
        if (m_socket)
        {
            m_socket->Close();
        }
    }

    void HttpServer::Close()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = 0;
        }
    }

    void HttpServer::Run()
    {
        while (m_socket && m_socket->IsOpen())
        {
            Handle<Socket> client = m_socket->Accept();

            if (client)
            {
                Serve(client);
            }
        }
    }

    void HttpServer::Serve(const Handle<Socket>& client)
    {
        HttpRequest request;
        byte buffer[HTTPD_MAX_REQUEST_SIZE];
        std::size_t buffer_size;
        byte* data;
        Filename path;

        if (!client->Read(buffer, HTTPD_MAX_REQUEST_SIZE, buffer_size))
        {
            client->Close();
            return;
        }

        if (!(data = parse_request(request, client, buffer, buffer_size)))
        {
            return;
        }

        std::fprintf(
            stdout,
            "%s %s\n",
            HttpMethod::ToString(request.method).Encode().c_str(),
            request.path.Encode().c_str()
        );

        path = m_root + request.path;

        if (!path.Exists())
        {
            send_error(
                client,
                "404 Not Found",
                "The requested URL "
                + request.path
                + " was not found on this server."
            );
        }
        else if (path.IsDir())
        {
            Filename index = path + "index.tly";

            if (index.Exists() && !index.IsDir())
            {
                ServeScript(client, request, index, data, buffer_size);
            }
            else if ((index = path + "index.html").Exists() && !index.IsDir())
            {
                ServeFile(client, request, index, "text/html");
            } else {
                send_error(
                    client,
                    "403 Forbidden",
                    "You don't have permission to access "
                    + request.path
                    + " on this server"
                );
            }
        } else {
            const String extension = path.GetExtension();

            if (extension == "tly")
            {
                ServeScript(client, request, path, data, buffer_size);
            } else {
                ServeFile(client, request, path, get_mime_type(extension));
            }
        }
    }

    void HttpServer::ServeFile(const Handle<Socket>& client,
                               const HttpRequest& request,
                               const Filename& path,
                               const String& mime_type)
    {
        Handle<Stream> stream = path.Open(Filename::MODE_READ);

        if (stream)
        {
            if (client->Printf("HTTP/1.0 200 OK\r\n")
                && client->Printf("Content-Type: %s\r\n", mime_type.Encode().c_str())
                && client->Printf("Content-Length: %ld\r\n\r\n", path.GetSize()))
            {
                byte buffer[4096];
                std::size_t read;

                while (stream->Read(buffer, sizeof(buffer), read))
                {
                    if (!client->Write(buffer, read))
                    {
                        break;
                    }
                }
            }
            client->Close();
            stream->Close();
        } else {
            send_error(
                client,
                "403 Forbidden",
                "You don't have permission to access "
                + request.path
                + " on this server"
            );
        }
    }

    void HttpServer::ServeScript(const Handle<Socket>& client,
                                 const HttpRequest& request,
                                 const Filename& path,
                                 const byte* data,
                                 std::size_t data_size)
    {
        String full_name = path.GetFullName();
        Dictionary<ScriptMapping>::Entry* entry = m_script_cache.Find(full_name);
        Handle<Interpreter> interpreter;
        ScriptMapping mapping;

        if (entry)
        {
            ScriptMapping& cached = entry->GetValue();

            if (cached.last_cached < path.GetLastModified())
            {
                compile_script(cached);
            }
            mapping = cached;
        } else {
            mapping.filename = path;
            compile_script(mapping);
            m_script_cache.Insert(full_name, mapping);
        }
        interpreter = new Interpreter();
        interpreter->request = new HttpServerRequest(
            client,
            request.method,
            request.path,
            request.query_string,
            request.headers,
            data,
            data_size
        );
        interpreter->response = new HttpServerResponse(client);
        interpreter->Initialize();
        if (mapping.script)
        {
            interpreter->PushScope(interpreter->globals);
            if (!mapping.script->Execute(interpreter))
            {
                interpreter->response->SendException(interpreter->GetException());
            }
            else if (!interpreter->response->IsCommitted())
            {
                interpreter->response->Commit();
            }
        } else {
            interpreter->Throw(interpreter->eSyntaxError, mapping.error);
            interpreter->response->SendException(interpreter->GetException());
        }
        client->Close();
    }

    void HttpServer::Mark()
    {
        CountedObject::Mark();
        if (m_socket && !m_socket->IsMarked())
        {
            m_socket->Mark();
        }
        for (const Dictionary<ScriptMapping>::Entry* entry = m_script_cache.GetFront();
             entry;
             entry = entry->GetNext())
        {
            Script* script = entry->GetValue().script;

            if (!script->IsMarked())
            {
                script->Mark();
            }
        }
    }

    static bool parse_request_line(HttpServer::HttpRequest& request, const Handle<Socket>& client, const String& line)
    {
        std::size_t index1;
        std::size_t index2;

        if ((index1 = line.IndexOf(' ')) == String::npos)
        {
            send_error(
                client,
                "400 Bad Request",
                "We were unable to process your request."
            );

            return false;
        }

        if (!HttpMethod::Parse(line.SubString(0, index1++), request.method))
        {
            send_error(
                client,
                "400 Bad Request",
                "We were unable to process your request."
            );

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
                send_error(
                    client,
                    "505 HTTP Version Not Supported",
                    "Unrecognized HTTP version."
                );

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

    static bool parse_request_header(HttpServer::HttpRequest& request,
                                     const Handle<Socket>& client,
                                     const String& line)
    {
        std::size_t index = line.IndexOf(':');

        if (index == String::npos || index < 1)
        {
            send_error(
                client,
                "400 Bad Request",
                "We were unable to process your request."
            );

            return false;
        }
        request.headers.Insert(line.SubString(0, index), line.SubString(index + 2));

        return true;
    }

    static byte* parse_request(HttpServer::HttpRequest& request,
                               const Handle<Socket>& client,
                               byte* start,
                               std::size_t& remain)
    {
        byte* begin = start;
        byte* end = static_cast<byte*>(std::memchr(begin, '\n', remain));

        if (!end)
        {
            send_error(
                client,
                "400 Bad Request",
                "We were unable to process your request."
            );

            return 0;
        }

        // Find out where the first line of the request ends.
        remain -= end - start + 1;
        start = end + 1;
        if (end > begin && end[-1] == '\r')
        {
            --end;
        }
        *end = '\0';

        // Process first line of request.
        if (!parse_request_line(request, client, reinterpret_cast<char*>(begin)))
        {
            return 0;
        }

        // Process request headers.
        for (;;)
        {
            begin = start;
            // FIXME: This type of line reading bugs if client sends only \n
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
            else if (!parse_request_header(request, client, reinterpret_cast<char*>(begin)))
            {
                return 0;
            }
        }
    }

    static void send_error(const Handle<Socket>& client, const char* status, const String& message)
    {
        ByteString content = message.Encode();

        client->Printf("HTTP/1.0 %s\r\n", status);
        client->Printf("Content-Type: text/plain; charset=utf-8\r\n");
        client->Printf("Content-Length: %ld\r\n\r\n", content.GetLength());
        client->Write(content);
        client->Close();
    }

    static void compile_script(HttpServer::ScriptMapping& mapping)
    {
        Handle<Stream> stream = mapping.filename.Open(Filename::MODE_READ);

        if (stream)
        {
            Handle<Parser> parser = new Parser(stream);
            Handle<Script> script = parser->Compile();

            parser->Close();
            mapping.script = script.Get();
            mapping.error.Clear();
        } else {
            mapping.error = "Unable to include file";
        }
        mapping.last_cached = DateTime::Now();
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
            return entry->GetValue();
        } else {
            return "application/octet-stream";
        }
    }
}
