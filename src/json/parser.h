#ifndef TEMPEARLY_JSON_PARSER_H_GUARD
#define TEMPEARLY_JSON_PARSER_H_GUARD

#include "core/parser.h"

namespace tempearly
{
    class JsonParser : public Parser
    {
    public:
        explicit JsonParser(const Handle<Stream>& stream);

        bool ParseValue(const Handle<Interpreter>& interpreter, Value& slot);

    private:
        TEMPEARLY_DISALLOW_COPY_AND_ASSIGN(JsonParser);
    };
}

#endif /* !TEMPEARLY_JSON_PARSER_H_GUARD */
