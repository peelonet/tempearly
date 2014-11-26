#include "interpreter.h"
#include "core/bytestring.h"
#include "net/socket.h"
#include "net/url.h"
#include "sapi/httpd/request.h"
#include "sapi/httpd/response.h"
#include "sapi/httpd/server.h"
#include "script/parser.h"

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
                stream->Pipe(client);
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
        interpreter->PushFrame();
        if (mapping.script)
        {
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
        interpreter->PopFrame();
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

    static bool parse_request_uri(HttpServer::HttpRequest& request,
                                  const Handle<Socket>& client,
                                  const byte* start,
                                  std::size_t remain)
    {
        const byte* begin = start;
        const byte* end = static_cast<const byte*>(std::memchr(begin, '?', remain));

        if (end)
        {
            request.query_string = ByteString(end + 1, remain - (end - begin));
            remain = end - begin;
        }
        if (!Url::Decode(begin, remain, request.path))
        {
            send_error(client, "400 Bad Request", "We were unable to process your request.");

            return false;
        }

        return true;
    }

    static bool parse_request_line(HttpServer::HttpRequest& request,
                                   const Handle<Socket>& client,
                                   const byte* start,
                                   std::size_t remain)
    {
        const byte* begin = start;
        const byte* end = static_cast<const byte*>(std::memchr(begin, ' ', remain));

        if (!end || !HttpMethod::Parse(String::DecodeAscii(begin, end - begin), request.method))
        {
            send_error(client, "400 Bad Request", "We were unable to process your request.");

            return false;
        }
        remain -= end - begin + 1;
        begin = end + 1;
        if ((end = static_cast<const byte*>(std::memchr(begin, ' ', remain))))
        {
            const String version = String::DecodeAscii(end + 1, remain - (end - begin + 1));

            if (!parse_request_uri(request, client, begin, end - begin))
            {
                return false;
            }
            else if (!HttpVersion::Parse(version, request.version))
            {
                send_error(client, "505 HTTP Version Not Supported", "Unsupported HTTP version");

                return false;
            }
        } else {
            request.version = HttpVersion::VERSION_09;

            return parse_request_uri(request, client, begin, remain);
        }

        return true;
    }

    static bool parse_request_header(HttpServer::HttpRequest& request,
                                     const Handle<Socket>& client,
                                     const byte* start,
                                     std::size_t remain)
    {
        const byte* begin = start;
        const byte* end = static_cast<const byte*>(std::memchr(begin, ':', remain));
        std::size_t key_length;

        if (!end)
        {
            send_error(client, "400 Bad Request", "We were unable to process your request.");

            return false;
        }
        key_length = ++end - begin;
        remain -= key_length;
        if (remain > 1 && *end == ' ')
        {
            ++end;
            --remain;
        }
        request.headers.Insert(String::DecodeAscii(begin, key_length - 1), String::DecodeAscii(end, remain));

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
            send_error(client, "400 Bad Request", "We were unable to process your request.");

            return 0;
        }

        // Find out where the first line of the request ends.
        remain -= end - begin + 1;
        start = end + 1;
        if (end > begin && end[-1] == '\r')
        {
            --end;
        }

        // Process first line of the request.
        if (!parse_request_line(request, client, begin, end - begin))
        {
            return 0;
        }

        // Process request headers
        for (;;)
        {
            begin = start;
            if (!(end = static_cast<byte*>(std::memchr(begin, '\n', remain))))
            {
                return 0;
            }
            remain -= end - begin + 1;
            start = end + 1;
            if (end > begin && end[-1] == '\r')
            {
                --end;
            }
            if (end <= begin)
            {
                return start;
            }
            else if (!parse_request_header(request, client, begin, end - begin))
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
            Handle<ScriptParser> parser = new ScriptParser(stream);
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
