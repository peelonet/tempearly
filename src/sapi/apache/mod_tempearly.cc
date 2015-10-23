#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>

#include <cstring>

#include "interpreter.h"
#include "core/filename.h"
#include "sapi/apache/request.h"
#include "sapi/apache/response.h"

#include <apr_strings.h>

using namespace tempearly;

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

    Handle<Interpreter> interpreter = new Interpreter(
        new ApacheRequest(request),
        new ApacheResponse(request)
    );

    interpreter->Initialize();

    if (!interpreter->Include(Filename(request->filename)))
    {
        interpreter->GetResponse()->SendException(interpreter->GetException());

        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else if (!interpreter->GetResponse()->IsCommitted())
    {
        interpreter->GetResponse()->Commit();
    }

    return OK;
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
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        tempearly_register_hooks
    };
}
