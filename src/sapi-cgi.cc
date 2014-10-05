#include "interpreter.h"

using namespace tempearly;

namespace
{
    class CgiRequest : public Request
    {
    public:
        explicit CgiRequest() {}

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CgiRequest);
    };

    class CgiResponse : public Response
    {
    public:
        explicit CgiResponse()
            : m_committed(false) {}

        bool IsCommitted() const
        {
            return m_committed;
        }

        void Commit()
        {
            if (m_committed)
            {
                return;
            }
            m_committed = true;
            if (GetStatus() != 200)
            {
                std::fprintf(stdout, "Status: %d\r\n", GetStatus());
            }

            const Dictionary<String>& headers = GetHeaders();

            for (const Dictionary<String>::Entry* e = headers.GetFront(); e; e = e->next)
            {
                std::fprintf(stdout, "%s: %s\r\n", e->id.c_str(), e->value.c_str());
            }

            std::fprintf(stdout, "\r\n");
            std::fflush(stdout);
        }

        void Write(std::size_t size, const char* data)
        {
            if (!m_committed)
            {
                Commit();
            }
            std::fwrite(
                static_cast<const void*>(data),
                sizeof(char),
                size,
                stdout
            );
        }

        void SendException(const Handle<ExceptionObject>& exception)
        {
            if (m_committed)
            {
                std::fprintf(
                    stdout,
                    "<p><strong>ERROR:</strong> %s</p>",
                    exception->GetMessage().c_str()
                );
            } else {
                m_committed = true;
                std::fprintf(
                    stdout,
                    "Status: 500\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n"
                );
                std::fflush(stdout);
                std::fprintf(
                    stdout,
                    "ERROR:\n%s\n",
                    exception->GetMessage().c_str()
                );
                std::fflush(stdout);
            }
        }

    private:
        bool m_committed;
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(CgiResponse);
    };
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        Handle<Interpreter> interpreter = new Interpreter();

        interpreter->request = new CgiRequest();
        interpreter->response = new CgiResponse();
        interpreter->Initialize();
        if (!interpreter->Include(argv[1]))
        {
            interpreter->response->SendException(interpreter->GetException());
        }
    }

    return EXIT_SUCCESS;
}
