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

    private:
        bool m_committed;
        int m_status;
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
            // TODO: print out exception info
        }
    }

    return EXIT_SUCCESS;
}
