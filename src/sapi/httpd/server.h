#ifndef TEMPEARLY_SAPI_HTTPD_SERVER_H_GUARD
#define TEMPEARLY_SAPI_HTTPD_SERVER_H_GUARD

#include "core/datetime.h"
#include "core/dictionary.h"
#include "core/filename.h"
#include "http/method.h"
#include "http/version.h"

namespace tempearly
{
    /**
     * Minimal HTTP server implementation capable of serving files and
     * Tempearly scripts.
     */
    class HttpServer : public CountedObject
    {
    public:
        struct HttpRequest
        {
            HttpMethod::Kind method;
            String path;
            ByteString query_string;
            HttpVersion::Kind version;
            Dictionary<String> headers;
        };

        /**
         * Structure which is used for caching compiled scripts.
         */
        struct ScriptMapping
        {
            /** Path pointing to the script. */
            Filename filename;
            /** Date and time when the script was last compiled. */
            DateTime last_cached;
            /** Contains compiled script or NULL if syntax error occured. */
            Script* script;
            /** Contains error message if an syntax error occured. */
            String error;
        };

        /**
         * Constructs new HTTP server.
         *
         * \param root   Root path where to serve files and scripts from
         * \param socket Fully initialized and open socket capable of listening
         *               incoming connections
         */
        explicit HttpServer(const Filename& root, const Handle<Socket>& socket);

        ~HttpServer();

        /**
         * Returns root path of the server where files and scripts are served
         * from.
         */
        inline const Filename& GetRoot() const
        {
            return m_root;
        }

        /**
         * Initializes server shutdown by closing the underlying socket.
         */
        void Close();

        /**
         * Loop which serves clients as long as the socket is open.
         */
        void Run();

        void Mark();

    private:
        void Serve(const Handle<Socket>& client);

        void ServeFile(const Handle<Socket>& client,
                       const HttpRequest& request,
                       const Filename& path,
                       const String& mime_type);

        void ServeScript(const Handle<Socket>& client,
                         const HttpRequest& request,
                         const Filename& path,
                         const byte* data,
                         std::size_t size);

    private:
        const Filename m_root;
        Socket* m_socket;
        Dictionary<ScriptMapping> m_script_cache;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(HttpServer);
    };
}

#endif /* !TEMPEARLY_SAPI_HTTPD_SERVER_H_GUARD */
