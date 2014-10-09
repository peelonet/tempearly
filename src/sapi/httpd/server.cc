#include <cstring>
#include <vector>

#include <sys/stat.h>

#include "interpreter.h"
#include "core/filename.h"
#include "core/bytestring.h"
#include "sapi/httpd/httpd-request.h"
#include "sapi/httpd/httpd-response.h"
#include "sapi/httpd/socket.h"

#if !defined(HTTPD_MAX_REQUEST_SIZE)
# define HTTPD_MAX_REQUEST_SIZE 4096
#endif

namespace tempearly
{
    enum HttpMethod
    {
        HTTP_METHOD_GET,
        HTTP_METHOD_HEAD,
        HTTP_METHOD_POST,
        HTTP_METHOD_PUT,
        HTTP_METHOD_DELETE,
        HTTP_METHOD_TRACE,
        HTTP_METHOD_OPTIONS,
        HTTP_METHOD_CONNECT,
        HTTP_METHOD_PATCH
    };

    enum HttpVersion
    {
        HTTP_VERSION_09,
        HTTP_VERSION_10,
        HTTP_VERSION_11
    };

    struct HttpRequestData
    {
        String method;
        String path;
        String query_string;
        HttpVersion version;
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

    static Dictionary<HttpMethod> http_method_map;
    static Dictionary<HttpVersion> http_version_map;
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
        HttpRequestData data;
        byte buffer[HTTPD_MAX_REQUEST_SIZE];
        std::size_t buffer_size;
        byte* data_begin;
        Filename path;

        if (!socket->Receive(buffer, HTTPD_MAX_REQUEST_SIZE, buffer_size))
        {
            socket->Close();
            return;
        }

        if (!(data_begin = parse_request(buffer, buffer_size, data)))
        {
            send_error(socket,
                       "400 Bad Request",
                       "We were unable to process your request.\n");
            return;
        }

        std::fprintf(stdout, "%s %s\n",
                     data.method.Encode().c_str(),
                     data.path.Encode().c_str());

        path = root_path + data.path;

        if (!path.Exists())
        {
            send_error(socket,
                       "404 Not Found",
                       "The requested URL "
                       + data.path
                       + " was not found on this server.");
        }
        else if (path.IsDir())
        {
            Filename index = root_path + "index.tly";

            if (index.Exists() && !index.IsDir())
            {
                send_script(socket, data, index, data_begin, buffer_size);
            }
            else if ((index = root_path + "index.html").Exists()
                    && !index.IsDir())
            {
                send_file(socket, data, index, "text/html");
            } else {
                send_error(socket,
                           "403 Forbidden",
                           "You don't have permission to access "
                           + data.path
                           + " on this server");
            }
        } else {
            const String extension = path.GetExtension();

            if (extension == "tly")
            {
                send_script(socket, data, path, data_begin, buffer_size);
            } else {
                send_file(socket, data, path, get_mime_type(extension));
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
                            HttpRequestData& data,
                            const Filename& path,
                            const byte* data_begin,
                            std::size_t data_size)
    {
        Handle<Interpreter> interpreter = new Interpreter();

        interpreter->request = new HttpServerRequest(
            socket,
            data.method,
            data.query_string,
            data.headers,
            data_begin,
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

    static bool is_valid_http_method(const String& string)
    {
        if (http_method_map.IsEmpty())
        {
            http_method_map.Insert("GET", HTTP_METHOD_GET);
            http_method_map.Insert("HEAD", HTTP_METHOD_HEAD);
            http_method_map.Insert("POST", HTTP_METHOD_POST);
            http_method_map.Insert("PUT", HTTP_METHOD_PUT);
            http_method_map.Insert("DELETE", HTTP_METHOD_DELETE);
            http_method_map.Insert("TRACE", HTTP_METHOD_TRACE);
            http_method_map.Insert("OPTIONS", HTTP_METHOD_OPTIONS);
            http_method_map.Insert("CONNECT", HTTP_METHOD_CONNECT);
            http_method_map.Insert("PATCH", HTTP_METHOD_PATCH);
        }

        return http_method_map.Find(string);
    }

    static bool parse_http_version(const String& string, HttpVersion& version)
    {
        const Dictionary<HttpVersion>::Entry* entry;

        if (http_version_map.IsEmpty())
        {
            http_version_map.Insert("HTTP/0.9", HTTP_VERSION_09);
            http_version_map.Insert("HTTP/1.0", HTTP_VERSION_10);
            http_version_map.Insert("HTTP/1.1", HTTP_VERSION_11);
        }
        if ((entry = http_version_map.Find(string)))
        {
            version = entry->value;

            return true;
        }

        return false;
    }

    static bool parse_request_line(const String& line, HttpRequestData& data)
    {
        std::size_t index1;
        std::size_t index2;

        if ((index1 = line.IndexOf(' ')) == String::npos)
        {
            return false;
        }

        if (!is_valid_http_method(data.method = line.SubString(0, index1++)))
        {
            return false;
        }

        if ((index2 = line.IndexOf(' ', index1)) == String::npos)
        {
            data.path = line.SubString(index1);
            data.version = HTTP_VERSION_09;
        } else {
            data.path = line.SubString(index1, index2 - index1);
            if (!parse_http_version(line.SubString(index2 + 1), data.version))
            {
                return false;
            }
        }

        // Split query string from request path.
        if ((index1 = data.path.IndexOf('?')))
        {
            data.query_string = data.path.SubString(index1 + 1);
            data.path = data.path.SubString(0, index1);
        }

        return true;
    }

    static bool parse_request_header(const String& line, HttpRequestData& data)
    {
        std::size_t index = line.IndexOf(':');

        if (index == String::npos || index < 1)
        {
            return false;
        }
        data.headers.Insert(line.SubString(0, index), line.SubString(index + 2));

        return true;
    }

    static byte* parse_request(byte* start,
                               std::size_t& remain,
                               HttpRequestData& data)
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
        if (!parse_request_line(reinterpret_cast<char*>(begin), data))
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
            else if (!parse_request_header(reinterpret_cast<char*>(begin), data))
            {
                return 0;
            }
        }
    }

    static void append(std::vector<String>& elements, const String& entry)
    {
        const std::size_t length = entry.GetLength();

        if (length == 0 || (length == 1 && entry[0] == '.'))
        {
            return;
        }
        else if (length == 2 && entry[0] == '.' && entry[1] == '.')
        {
            if (!elements.empty())
            {
                elements.pop_back();
            }
        } else {
            elements.push_back(entry);
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
