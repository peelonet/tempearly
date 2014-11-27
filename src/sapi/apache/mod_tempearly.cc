#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>

#include <cstring>

#include "interpreter.h"
#include "core/filename.h"
#include "io/stream.h"
#include "sapi/apache/request.h"
#include "sapi/apache/response.h"
#include "script/parser.h"

#include <apr_strings.h>

using namespace tempearly;

struct ScriptMapping
{
    /** Timestamp when the script was last compiled. */
    apr_time_t last_cached;
    /** Contains compiled script or NULL if syntax error occurred. */
    Handle<Script> script;
    /** Contains error message if an syntax error occurred. */
    String error;
};

static Dictionary<ScriptMapping> script_cache;

static void compile_script(const Filename& filename, ScriptMapping& mapping)
{
    Handle<Stream> stream = filename.Open(Filename::MODE_READ);

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
    mapping.last_cached = apr_time_now();
}

static bool serve_script(request_rec* request)
{
    const String filename = request->finfo.fname;
    Dictionary<ScriptMapping>::Entry* entry = script_cache.Find(filename);
    Handle<Interpreter> interpreter;
    ScriptMapping mapping;

    if (!entry)
    {
        ScriptMapping& cached = entry->GetValue();

        if (cached.last_cached < request->finfo.mtime)
        {
            compile_script(filename, cached);
        }
        mapping = cached;
    } else {
        compile_script(filename, mapping);
    }
    interpreter = new Interpreter();
    interpreter->request = new ApacheRequest(request);
    interpreter->response = new ApacheResponse(request);
    interpreter->Initialize();
    interpreter->PushFrame();
    if (mapping.script)
    {
        const bool result = mapping.script->Execute(interpreter);

        interpreter->PopFrame();
        if (result)
        {
            if (!interpreter->response->IsCommitted())
            {
                interpreter->response->Commit();
            }

            return true;
        } else {
            interpreter->response->SendException(interpreter->GetException());

            return false;
        }
    } else {
        interpreter->Throw(interpreter->eSyntaxError, mapping.error);
        interpreter->response->SendException(interpreter->GetException());
        interpreter->PopFrame();

        return false;
    }
}

extern "C" int tempearly_handler(request_rec* request)
{
    // Test whether proper script type is set
    if (!request->handler || std::strcmp(request->handler, "tempearly-script"))
    {
        return DECLINED;
    }

    // Test whether the file actually exists
    if (request->finfo.filetype == 0)
    {
        return HTTP_NOT_FOUND;
    }

    // Test whether we are trying to execute a directory as script
    if (request->finfo.filetype == APR_DIR)
    {
        return HTTP_FORBIDDEN;
    }

    return serve_script(request) ? OK : HTTP_INTERNAL_SERVER_ERROR;
}

extern "C" void tempearly_register_hooks(apr_pool_t* pool)
{
    ap_hook_handler(tempearly_handler, 0, 0, APR_HOOK_LAST);
}

extern "C"
{
    module AP_MODULE_DECLARE_DATA tempearly_module =
    {
        STANDARD20_MODULE_STUFF,
        0,
        0,
        0,
        0,
        0,
        tempearly_register_hooks
    };
}
